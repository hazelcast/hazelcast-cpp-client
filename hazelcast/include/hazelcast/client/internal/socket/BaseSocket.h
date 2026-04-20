/*
 * Copyright (c) 2008-2025, Hazelcast, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once

#include <boost/asio.hpp>
#include <boost/format.hpp>
#include <boost/lockfree/queue.hpp>
#ifdef HZ_BUILD_WITH_SSL
#include <boost/asio/ssl.hpp>
#endif

#include "hazelcast/client/socket.h"
#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/client/exception/protocol_exceptions.h"
#include "hazelcast/client/socket_interceptor.h"

namespace hazelcast {
namespace client {
namespace internal {
namespace socket {

struct OutboundEntry
{
    int64_t correlation_id;
    std::shared_ptr<protocol::ClientMessage> message;
    std::shared_ptr<spi::impl::ClientInvocation> invocation;
};

template<typename T>
class BaseSocket : public hazelcast::client::socket
{
public:
    template<typename = std::enable_if<
               std::is_same<T, boost::asio::ip::tcp::socket>::value>>
    BaseSocket(boost::asio::ip::tcp::resolver& io_resolver,
               const address& addr,
               client::config::socket_options& socket_options,
               boost::asio::io_context& io,
               std::chrono::milliseconds& connect_timeout_in_millis)
      : socket_options_(socket_options)
      , remote_endpoint_(addr)
      , io_(io)
      , socket_strand_(io)
      , connect_timeout_(connect_timeout_in_millis)
      , resolver_(io_resolver)
      , socket_(io)
      , write_queue_(WRITE_QUEUE_INITIAL_SIZE)
    {
    }

#ifdef HZ_BUILD_WITH_SSL

    template<typename CONTEXT,
             typename = std::enable_if<std::is_same<
               T,
               boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>::value>>
    BaseSocket(boost::asio::ip::tcp::resolver& io_resolver,
               const address& addr,
               client::config::socket_options& socket_options,
               boost::asio::io_context& io,
               std::chrono::milliseconds& connect_timeout_in_millis,
               CONTEXT& context)
      : socket_options_(socket_options)
      , remote_endpoint_(addr)
      , io_(io)
      , socket_strand_(io)
      , connect_timeout_(connect_timeout_in_millis)
      , resolver_(io_resolver)
      , socket_(io, context)
      , write_queue_(WRITE_QUEUE_INITIAL_SIZE)
    {
    }
#endif // HZ_BUILD_WITH_SSL

    void connect(
      const std::shared_ptr<connection::Connection> connection) override
    {
        boost::asio::steady_timer connectTimer(io_);
        connectTimer.expires_after(connect_timeout_);
        connectTimer.async_wait([=](const boost::system::error_code& ec) {
            if (ec == boost::asio::error::operation_aborted) {
                return;
            }
            resolver_.cancel();
            close();
            return;
        });
        try {
            auto addresses =
              resolver_
                .async_resolve(remote_endpoint_.get_host(),
                               std::to_string(remote_endpoint_.get_port()),
                               boost::asio::use_future)
                .get();
            boost::asio::async_connect(
              socket_.lowest_layer(), addresses, boost::asio::use_future)
              .get();
            post_connect();
            connectTimer.cancel();
            set_socket_options(socket_options_);
            static constexpr const char* PROTOCOL_TYPE_BYTES = "CP2";
            boost::asio::write(socket_,
                               boost::asio::buffer(PROTOCOL_TYPE_BYTES, 3));
        } catch (...) {
            connectTimer.cancel();
            close();
            throw;
        }

        socket_.lowest_layer().native_non_blocking(true);
        do_read(std::move(connection));
    }

    void enqueue_write(const std::shared_ptr<connection::Connection> connection,
                       OutboundEntry* entry) override
    {
        write_queue_.push(entry);
        // Always post a flush to the strand. The strand serializes execution
        // and flush_write_queue is cheap when the queue is empty or a write
        // is already in progress.
        boost::asio::post(socket_strand_, [this, connection]() {
            flush_write_queue(connection);
        });
    }

    // always called from within the socket_strand_
    void close() override
    {
        boost::system::error_code ignored;
        socket_.lowest_layer().close(ignored);

        // Drain write queue to prevent memory leaks
        OutboundEntry* e;
        while (write_queue_.pop(e)) {
            delete e;
        }
    }

    address get_address() const override
    {
        return address(
          socket_.lowest_layer().remote_endpoint().address().to_string(),
          remote_endpoint_.get_port());
    }

    /**
     *
     * This function is used to obtain the locally bound endpoint of the socket.
     *
     * @returns An address that represents the local endpoint of the socket.
     */
    boost::optional<address> local_socket_address() const override
    {
        boost::system::error_code ec;
        boost::asio::ip::basic_endpoint<boost::asio::ip::tcp> localEndpoint =
          socket_.lowest_layer().local_endpoint(ec);
        if (ec) {
            return boost::none;
        }
        return boost::optional<address>(
          address(localEndpoint.address().to_string(), localEndpoint.port()));
    }

    const address& get_remote_endpoint() const override
    {
        return remote_endpoint_;
    }

    boost::asio::io_context::strand& get_executor() noexcept override
    {
        return socket_strand_;
    }

protected:
    static constexpr std::size_t WRITE_QUEUE_INITIAL_SIZE = 128;

    void set_socket_options(const client::config::socket_options& options)
    {
        auto& lowestLayer = socket_.lowest_layer();

        lowestLayer.set_option(
          boost::asio::ip::tcp::no_delay(options.is_tcp_no_delay()));

        lowestLayer.set_option(
          boost::asio::socket_base::keep_alive(options.is_keep_alive()));

        lowestLayer.set_option(
          boost::asio::socket_base::reuse_address(options.is_reuse_address()));

        int lingerSeconds = options.get_linger_seconds();
        if (lingerSeconds > 0) {
            lowestLayer.set_option(
              boost::asio::socket_base::linger(true, lingerSeconds));
        }

        int bufferSize = options.get_buffer_size_in_bytes();
        if (bufferSize > 0) {
            lowestLayer.set_option(
              boost::asio::socket_base::receive_buffer_size(bufferSize));
            lowestLayer.set_option(
              boost::asio::socket_base::send_buffer_size(bufferSize));
        }

        // SO_NOSIGPIPE seems to be internally handled by asio on connect and
        // accept. no such option is defined at the api, hence not setting this
        // option
    }

    void do_read(const std::shared_ptr<connection::Connection> connection)
    {
        socket_.async_read_some(
          boost::asio::buffer(connection->read_handler.byte_buffer.ix(),
                              connection->read_handler.byte_buffer.remaining()),
          socket_strand_.wrap(
            [=](const boost::system::error_code& ec, std::size_t bytes_read) {
                if (ec) {
                    // prevent any exceptions
                    util::IOUtil::close_resource(
                      connection.get(),
                      (boost::format("Socket read error. %1% for %2%") % ec %
                       (*connection))
                        .str()
                        .c_str());
                    return;
                }

                connection->read_handler.byte_buffer.safe_increment_position(
                  bytes_read);

                connection->read_handler.handle();

                do_read(std::move(connection));
            }));
    }

    // Called on strand: drain lock-free queue, batch into scatter-gather
    // buffers, single async_write syscall
    void flush_write_queue(
      const std::shared_ptr<connection::Connection>& connection)
    {
        if (write_in_progress_) {
            return; // completion handler will re-trigger flush
        }

        // Drain queue into batch
        drain_batch_.clear();
        batch_buffers_.clear();

        OutboundEntry* e;
        while (write_queue_.pop(e)) {
            drain_batch_.push_back(e);
        }

        if (drain_batch_.empty()) {
            return;
        }

        // Correlation IDs already set and registered in global map by
        // caller thread (in invoke() + send()), matching Java client pattern.
        // Build scatter-gather buffers for batch write.
        for (auto* entry : drain_batch_) {
            auto& datas = entry->message->get_buffer();
            for (const auto& data : datas) {
                batch_buffers_.emplace_back(boost::asio::buffer(data));
            }
        }

        write_in_progress_ = true;
        boost::asio::async_write(
          socket_,
          batch_buffers_,
          socket_strand_.wrap(
            [this, connection](const boost::system::error_code& ec,
                               std::size_t /* bytes_written */) {
                write_in_progress_ = false;

                // Clean up batch entries
                for (auto* entry : drain_batch_) {
                    delete entry;
                }
                drain_batch_.clear();
                batch_buffers_.clear();

                if (ec) {
                    connection->close(
                      (boost::format(
                         "Error %1% during batch write on connection %2%") %
                       ec % *connection)
                        .str());
                    fail_queued_writes();
                    return;
                }

                connection->last_write_time(std::chrono::steady_clock::now());

                // Check for more entries that arrived during write
                if (!write_queue_.empty()) {
                    flush_write_queue(connection);
                }
            }));
    }

    // Always called from within the socket_strand_
    void fail_queued_writes()
    {
        OutboundEntry* e;
        while (write_queue_.pop(e)) {
            delete e;
        }
        for (auto* entry : drain_batch_) {
            delete entry;
        }
        drain_batch_.clear();
    }

    virtual void post_connect() {}

    client::config::socket_options& socket_options_;
    address remote_endpoint_;
    boost::asio::io_context& io_;
    boost::asio::io_context::strand socket_strand_;
    std::chrono::milliseconds connect_timeout_;
    boost::asio::ip::tcp::resolver& resolver_;
    T socket_;

    // Lock-free MPSC write queue: user threads push, IO strand drains
    boost::lockfree::queue<OutboundEntry*> write_queue_;

    // Strand-only state for batch writing
    bool write_in_progress_{ false };
    std::vector<OutboundEntry*> drain_batch_;
    std::vector<boost::asio::const_buffer> batch_buffers_;
};
} // namespace socket
} // namespace internal
} // namespace client
} // namespace hazelcast
