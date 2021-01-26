/*
 * Copyright (c) 2008-2021, Hazelcast, Inc. All Rights Reserved.
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

#include <unordered_map>

#include <boost/asio.hpp>
#include <boost/format.hpp>

#include "hazelcast/client/socket.h"
#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/client/exception/protocol_exceptions.h"
#include "hazelcast/client/socket_interceptor.h"

namespace hazelcast {
    namespace client {
        namespace internal {
            namespace socket {
                template<typename T>
                class BaseSocket : public hazelcast::client::socket {
                public:
                    template<typename = std::enable_if<std::is_same<T, boost::asio::ip::tcp::socket>::value>>
                    BaseSocket(boost::asio::ip::tcp::resolver &io_resolver,
                               const address &addr, client::config::socket_options &socket_options,
                               boost::asio::io_context &io, std::chrono::milliseconds &connect_timeout_in_millis)
                            : socket_options_(socket_options), remote_endpoint_(addr), io_(io), socket_strand_(io),
                              connect_timeout_(connect_timeout_in_millis), resolver_(io_resolver),
                              socket_(io) {
                    }
                    
#ifdef HZ_BUILD_WITH_SSL

                    template<typename CONTEXT, typename = std::enable_if<std::is_same<T, boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>::value>>
                    BaseSocket(boost::asio::ip::tcp::resolver &io_resolver,
                               const address &addr, client::config::socket_options &socket_options,
                               boost::asio::io_context &io, std::chrono::milliseconds &connect_timeout_in_millis,
                               CONTEXT &context)
                            : socket_options_(socket_options), remote_endpoint_(addr), io_(io), socket_strand_(io),
                              connect_timeout_(connect_timeout_in_millis), resolver_(io_resolver),
                              socket_(io, context) {
                    }
#endif // HZ_BUILD_WITH_SSL

                    void connect(const std::shared_ptr<connection::Connection> connection) override {
                        boost::asio::steady_timer connectTimer(io_);
                        connectTimer.expires_from_now(connect_timeout_);
                        connectTimer.async_wait([=](const boost::system::error_code &ec) {
                            if (ec == boost::asio::error::operation_aborted) {
                                return;
                            }
                            resolver_.cancel();
                            close();
                            return;
                        });
                        try {
                            auto addresses = resolver_.async_resolve(remote_endpoint_.get_host(),
                                                                     std::to_string(remote_endpoint_.get_port()),
                                                                     boost::asio::use_future).get();
                            boost::asio::async_connect(socket_.lowest_layer(), addresses,
                                                       boost::asio::use_future).get();
                            post_connect();
                            connectTimer.cancel();
                            set_socket_options(socket_options_);
                            static constexpr const char *PROTOCOL_TYPE_BYTES = "CP2";
                            boost::asio::write(socket_, boost::asio::buffer(PROTOCOL_TYPE_BYTES, 3));
                        } catch (...) {
                            connectTimer.cancel();
                            close();
                            throw;
                        }

                        socket_.lowest_layer().native_non_blocking(true);
                        do_read(std::move(connection));
                    }

                    void async_write(const std::shared_ptr<connection::Connection> connection,
                                    const std::shared_ptr<spi::impl::ClientInvocation> invocation) override {
                        check_connection(connection, invocation);
                        auto message = invocation->get_client_message();
                        socket_strand_.post([=]() {
                            if (!check_connection(connection, invocation)) {
                                return;
                            }

                            bool success;
                            int64_t message_call_id;
                            do {
                                auto call_id = ++call_id_counter_;
                                struct correlation_id {
                                    int32_t connnection_id;
                                    int32_t call_id;
                                };
                                union {
                                    int64_t id;
                                    correlation_id composed_id;
                                } c_id_union;
                                c_id_union.composed_id = {connection->get_connection_id(), call_id};
                                message_call_id = c_id_union.id;
                                message->set_correlation_id(c_id_union.id);
                                success = connection->invocations.insert({message_call_id, invocation}).second;
                            } while (!success);

                            auto handler = [=](const boost::system::error_code &ec,
                                               std::size_t bytes_written) {
                                if (ec) {
                                    auto invocationIt = connection->invocations.find(message_call_id);

                                    assert(invocationIt != connection->invocations.end());

                                    auto message = (boost::format{
                                            "Error %1% during invocation write for %2% on connection %3%"} %
                                                    ec % *invocation % *connection).str();
                                    invocationIt->second->notify_exception(
                                            boost::enable_current_exception(
                                                    std::make_exception_ptr(
                                                            exception::io(
                                                                    "Connection::write",
                                                                    message))));

                                    connection->close(message);
                                    connection->invocations.erase(invocationIt);
                                }
                            };

                            auto &datas = message->get_buffer();
                            if (datas.size() == 1) {
                                boost::asio::async_write(socket_, boost::asio::buffer(datas[0]),
                                                         socket_strand_.wrap(handler));
                            } else {
                                std::vector<boost::asio::const_buffer> buffers;
                                buffers.reserve(datas.size());
                                for (auto &d : datas) {
                                    buffers.push_back(boost::asio::buffer(d));
                                }
                                boost::asio::async_write(socket_, buffers, socket_strand_.wrap(handler));
                            }
                        });
                    }

                    // always called from within the socket_strand_
                    void close() override {
                        boost::system::error_code ignored;
                        socket_.lowest_layer().close(ignored);
                    }

                    address get_address() const override {
                        return address(socket_.lowest_layer().remote_endpoint().address().to_string(),
                                       remote_endpoint_.get_port());
                    }

                    /**
                     *
                     * This function is used to obtain the locally bound endpoint of the socket.
                     *
                     * @returns An address that represents the local endpoint of the socket.
                     */
                    boost::optional<address> local_socket_address() const override {
                        boost::system::error_code ec;
                        boost::asio::ip::basic_endpoint<boost::asio::ip::tcp> localEndpoint = socket_.lowest_layer().local_endpoint(
                                ec);
                        if (ec) {
                            return boost::none;
                        }
                        return boost::optional<address>(
                                address(localEndpoint.address().to_string(), localEndpoint.port()));
                    }

                    const address &get_remote_endpoint() const override {
                        return remote_endpoint_;
                    }

                    boost::asio::io_context::strand &get_executor() noexcept override {
                        return socket_strand_;
                    }

                protected:
                    void set_socket_options(const client::config::socket_options &options) {
                        auto &lowestLayer = socket_.lowest_layer();

                        lowestLayer.set_option(boost::asio::ip::tcp::no_delay(options.is_tcp_no_delay()));

                        lowestLayer.set_option(boost::asio::socket_base::keep_alive(options.is_keep_alive()));

                        lowestLayer.set_option(boost::asio::socket_base::reuse_address(options.is_reuse_address()));

                        int lingerSeconds = options.get_linger_seconds();
                        if (lingerSeconds > 0) {
                            lowestLayer.set_option(boost::asio::socket_base::linger(true, lingerSeconds));
                        }

                        int bufferSize = options.get_buffer_size_in_bytes();
                        if (bufferSize > 0) {
                            lowestLayer.set_option(boost::asio::socket_base::receive_buffer_size(bufferSize));
                            lowestLayer.set_option(boost::asio::socket_base::send_buffer_size(bufferSize));
                        }

                        // SO_NOSIGPIPE seems to be internally handled by asio on connect and accept. no such option
                        // is defined at the api, hence not setting this option
                    }

                    void do_read(const std::shared_ptr<connection::Connection> connection) {
                        socket_.async_read_some(boost::asio::buffer(connection->read_handler.byte_buffer.ix(),
                                                                    connection->read_handler.byte_buffer.remaining()),
                                                socket_strand_.wrap([=](const boost::system::error_code &ec,
                                                                        std::size_t bytes_read) {
                                                    if (ec) {
                                                        // prevent any exceptions
                                                        util::IOUtil::close_resource(connection.get(),
                                                                                     (boost::format(
                                                                                             "Socket read error. %1% for %2%")
                                                                                      % ec %
                                                                                      (*connection)).str().c_str());
                                                        return;
                                                    }

                                                    connection->read_handler.byte_buffer.safe_increment_position(
                                                            bytes_read);

                                                    connection->read_handler.handle();

                                                    do_read(connection);
                                                }));
                    }

                    virtual void post_connect() {
                    }

                    bool check_connection(const std::shared_ptr<connection::Connection> &connection,
                                          const std::shared_ptr<spi::impl::ClientInvocation> &invocation) {
                        if (!connection->is_alive()) {
                            invocation->notify_exception(
                                    std::make_exception_ptr(boost::enable_current_exception(exception::io(
                                            "Connection::write", (boost::format{
                                                    "Socket closed. Invocation write for %1% on connection %2% failed"} %
                                                                  *invocation % *connection).str()))));
                            return false;
                        }

                        return true;
                    }

                    client::config::socket_options &socket_options_;
                    address remote_endpoint_;
                    boost::asio::io_context &io_;
                    boost::asio::io_context::strand socket_strand_;
                    std::chrono::milliseconds connect_timeout_;
                    boost::asio::ip::tcp::resolver &resolver_;
                    T socket_;
                    int32_t call_id_counter_;
                };
            }
        }
    }
}

