/*
 * Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
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

#include "hazelcast/client/Socket.h"
#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/client/exception/IOException.h"
#include "hazelcast/client/SocketInterceptor.h"

namespace hazelcast {
    namespace client {
        namespace internal {
            namespace socket {
                template<typename T>
                class BaseSocket : public Socket {
                public:
                    template<typename = std::enable_if<std::is_same<T, boost::asio::ip::tcp::socket>::value>>
                    BaseSocket(boost::asio::ip::tcp::resolver &ioResolver,
                            const Address &address, client::config::SocketOptions &socketOptions,
                            boost::asio::io_context &io, std::chrono::steady_clock::duration &connectTimeoutInMillis)
                            : socketOptions(socketOptions), remoteEndpoint(address), io(io),
                              socketStrand(io), connectTimer(socketStrand),
                              connectTimeout(connectTimeoutInMillis), resolver(ioResolver), socket_(socketStrand) {
                    }

                    template<typename CONTEXT, typename = std::enable_if<std::is_same<T, boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>::value>>
                    BaseSocket(boost::asio::ip::tcp::resolver &ioResolver,
                            const Address &address, client::config::SocketOptions &socketOptions,
                            boost::asio::io_context &io, std::chrono::steady_clock::duration &connectTimeoutInMillis,
                            CONTEXT &context)
                            : socketOptions(socketOptions), remoteEndpoint(address), io(io),
                              socketStrand(io), connectTimer(socketStrand),
                              connectTimeout(connectTimeoutInMillis), resolver(ioResolver),
                              socket_(socketStrand, context) {
                    }

                    void connect(const std::shared_ptr<connection::Connection> connection) override {
                        using namespace boost::asio;
                        using namespace boost::asio::ip;

                        connectTimer.expires_from_now(connectTimeout);
                        connectTimer.async_wait([=](const boost::system::error_code &ec) {
                            if (ec == boost::asio::error::operation_aborted) {
                                return;
                            }
                            close();
                            return;
                        });
                        try {
                            auto addresses = resolver.resolve(remoteEndpoint.getHost(), std::to_string(remoteEndpoint.getPort()));
                            boost::asio::async_connect(socket_.lowest_layer(), addresses,
                                                                     boost::asio::use_future).get();
                            post_connect();
                            connectTimer.cancel();
                            setSocketOptions(socketOptions);
                            static constexpr const char *PROTOCOL_TYPE_BYTES = "CP2";
                            write(socket_, boost::asio::buffer(PROTOCOL_TYPE_BYTES, 3));
                        } catch (...) {
                            connectTimer.cancel();
                            close();
                            throw;
                        }

                        socket_.lowest_layer().native_non_blocking(true);
                        do_read(std::move(connection));
                    }

                    void asyncWrite(const std::shared_ptr<connection::Connection> connection,
                                    const std::shared_ptr<spi::impl::ClientInvocation> invocation) override {
                        auto message = invocation->getClientMessage();
                        boost::asio::post(socket_.get_executor(), [=]() {
                            if (!socket_.lowest_layer().is_open()) {
                                invocation->notifyException(
                                        std::make_exception_ptr(boost::enable_current_exception(exception::IOException(
                                                "Connection::write", (boost::format{
                                                        "Socket closed. Invocation write for %1% on connection %2% failed"} %
                                                                      *invocation % *connection).str()))));
                                return;
                            }
                            auto correlationId = message->getCorrelationId();
                            auto result = connection->invocations.insert({correlationId, invocation});
                            if (!result.second) {
                                auto existingEntry = *result.first;
                                invocation->notifyException(std::make_exception_ptr(
                                        boost::enable_current_exception(
                                                exception::IllegalStateException("Connection::write", (boost::format(
                                                        "There is already an existing invocation with the same correlation id: %1%. Existing: %2% New invocation:%3%") %
                                                                                                       correlationId %
                                                                                                       (*existingEntry.second) %
                                                                                                       *invocation).str()))));
                                return;
                            }

                            auto handler = [=](const boost::system::error_code &ec,
                                               std::size_t bytesWritten) {
                                if (ec) {
                                    auto invocationIt = connection->invocations.find(
                                            correlationId);

                                    assert(invocationIt != connection->invocations.end());

                                    auto message = (boost::format{
                                            "Error %1% during invocation write for %2% on connection %3%"} %
                                                    ec % *invocation % *connection).str();
                                    invocationIt->second->notifyException(
                                            boost::enable_current_exception(
                                                    std::make_exception_ptr(
                                                            exception::IOException(
                                                                    "Connection::write",
                                                                    message))));

                                    connection->close(message);
                                    connection->invocations.erase(invocationIt);
                                }
                            };

                            auto &datas = message->getBuffer();
                            if (datas.size() == 1) {
                                boost::asio::async_write(socket_, boost::asio::buffer(datas[0]), handler);
                            } else {
                                std::vector<boost::asio::const_buffer> buffers;
                                buffers.reserve(datas.size());
                                for (auto &d : datas) {
                                    buffers.push_back(boost::asio::buffer(d));
                                }
                                boost::asio::async_write(socket_, buffers, handler);
                            }
                        });
                    }

                    void close() override {
                        boost::system::error_code ignored;
                        socket_.lowest_layer().close(ignored);
                    }

                    Address getAddress() const override {
                        return Address(socket_.lowest_layer().remote_endpoint().address().to_string(),
                                       remoteEndpoint.getPort());
                    }

                    /**
                     *
                     * This function is used to obtain the locally bound endpoint of the socket.
                     *
                     * @returns An address that represents the local endpoint of the socket.
                     */
                    boost::optional<Address> localSocketAddress() const override {
                        boost::system::error_code ec;
                        boost::asio::ip::basic_endpoint<boost::asio::ip::tcp> localEndpoint = socket_.lowest_layer().local_endpoint(
                                ec);
                        if (ec) {
                            return boost::none;
                        }
                        return boost::optional<Address>(Address(localEndpoint.address().to_string(), localEndpoint.port()));
                    }

                    const Address &getRemoteEndpoint() const override {
                        return remoteEndpoint;
                    }

                    boost::asio::executor get_executor() noexcept override {
                        return socket_.get_executor();
                    }

                protected:
                    void setSocketOptions(const client::config::SocketOptions &options) {
                        auto &lowestLayer = socket_.lowest_layer();

                        lowestLayer.set_option(boost::asio::ip::tcp::no_delay(options.isTcpNoDelay()));

                        lowestLayer.set_option(boost::asio::socket_base::keep_alive(options.isKeepAlive()));

                        lowestLayer.set_option(boost::asio::socket_base::reuse_address(options.isReuseAddress()));

                        int lingerSeconds = options.getLingerSeconds();
                        if (lingerSeconds > 0) {
                            lowestLayer.set_option(boost::asio::socket_base::linger(true, lingerSeconds));
                        }

                        int bufferSize = options.getBufferSizeInBytes();
                        if (bufferSize > 0) {
                            lowestLayer.set_option(boost::asio::socket_base::receive_buffer_size(bufferSize));
                            lowestLayer.set_option(boost::asio::socket_base::send_buffer_size(bufferSize));
                        }

                        // SO_NOSIGPIPE seems to be internally handled by asio on connect and accept. no such option
                        // is defined at the api, hence not setting this option
                    }

                    void do_read(const std::shared_ptr<connection::Connection> connection) {
                        using namespace boost::asio;
                        using namespace boost::asio::ip;

                        socket_.async_read_some(buffer(connection->readHandler.byteBuffer.ix(),
                                                        connection->readHandler.byteBuffer.remaining()),
                                                 [=](const boost::system::error_code &ec, std::size_t bytesRead) {
                                                     if (ec) {
                                                         // prevent any exceptions
                                                         util::IOUtil::closeResource(connection.get(),
                                                                 (boost::format("Socket read error. %1% for %2%")
                                                                 %ec %(*connection)).str().c_str());
                                                         return;
                                                     }

                                                     connection->readHandler.byteBuffer.safeIncrementPosition(
                                                             bytesRead);

                                                     connection->readHandler.handle();

                                                     do_read(connection);
                                                 });
                    }

                    virtual void post_connect() {
                    }

                    client::config::SocketOptions &socketOptions;
                    Address remoteEndpoint;
                    boost::asio::io_context &io;
                    boost::asio::io_context::strand socketStrand;
                    boost::asio::steady_timer connectTimer;
                    std::chrono::steady_clock::duration connectTimeout;
                    boost::asio::ip::tcp::resolver &resolver;
                    T socket_;
                };
            }
        }
    }
}

