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

#include <boost/asio.hpp>
#include <boost/format.hpp>

#include "hazelcast/client/Socket.h"
#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/client/connection/AuthenticationFuture.h"
#include "hazelcast/client/exception/IOException.h"
#include "hazelcast/client/SocketInterceptor.h"

namespace hazelcast {
    namespace client {
        namespace internal {
            namespace socket {
                template<typename T>
                class BaseSocket : public Socket {
                public:
                    BaseSocket(std::unique_ptr<T> socket, const Address &address,
                               client::config::SocketOptions &socketOptions,
                               boost::asio::io_context &io, int64_t connectTimeoutInMillis) : socketOptions(
                            socketOptions), socket_(std::move(socket)),
                                                                                              remoteEndpoint(address),
                                                                                              io(io), resolver(io),
                                                                                              connectTimer(io),
                                                                                              connectTimeoutMillis(
                                                                                                      connectTimeoutInMillis) {
                    }

                    ~BaseSocket() {
                        close();
                    }

                    void asyncStart(const std::shared_ptr<connection::Connection> &connection,
                                    const std::shared_ptr<connection::AuthenticationFuture> &authFuture) override {
                        using namespace boost::asio;
                        using namespace boost::asio::ip;

                        connectTimer.expires_from_now(connectTimeoutMillis);
                        connectTimer.async_wait([=](const boost::system::error_code &ec) {
                            if (ec == boost::asio::error::operation_aborted) {
                                return;
                            }
                            authFuture->onFailure(std::make_shared<exception::IOException>(
                                    "Connection::asyncStart", (boost::format(
                                            "Connection establishment to server %1% timed out in %2% msecs. %3%") %
                                                               remoteEndpoint % connectTimeoutMillis.count() %
                                                               ec).str()));
                            return;
                        });
                        resolver.async_resolve(remoteEndpoint.getHost(), std::to_string(remoteEndpoint.getPort()),
                                               [=](const boost::system::error_code &ec,
                                                   tcp::resolver::results_type resolvedAddresses) {
                                                   if (ec) {
                                                       authFuture->onFailure(std::make_shared<exception::IOException>(
                                                               "Connection::asyncStart", (boost::format(
                                                                       "Could not resolve server address %1%. %2%") %
                                                                                          remoteEndpoint % ec).str()));
                                                       return;
                                                   }

                                                   async_connect(socket_->lowest_layer(), resolvedAddresses,
                                                                 [=](const boost::system::error_code &ec,
                                                                     const tcp::endpoint &) {
                                                                     connectTimer.cancel();
                                                                     if (ec) {
                                                                         authFuture->onFailure(
                                                                                 std::make_shared<exception::IOException>(
                                                                                         "Connection::asyncStart",
                                                                                         (boost::format(
                                                                                                 "Socket failed to connect to server address %1%. %2%") %
                                                                                          remoteEndpoint % ec).str()));
                                                                         return;
                                                                     }

                                                                     async_handle_connect(connection, authFuture);
                                                                 });
                                               });
                    }

                    void asyncWrite(const std::shared_ptr<connection::Connection> &connection,
                                    const std::shared_ptr<protocol::ClientMessage> &message) override {
                        boost::asio::async_write(*socket_,
                                                 boost::asio::buffer(message->getBuffer()->data(),
                                                                     message->getFrameLength()),
                                                 [=](const boost::system::error_code &ec, std::size_t bytesWritten) {
                                                     if (ec) {
                                                         connection->close(
                                                                 (boost::format{
                                                                         "Error %1% during message write for %2% on connection %3%"} %
                                                                  ec % *message % *connection).str());
                                                     }
                                                 });
                    }

                    void close() override {
                        boost::system::error_code ignored;
                        connectTimer.cancel(ignored);
                        socket_->lowest_layer().close(ignored);
                    }

                    virtual Address getAddress() const override {
                        return Address(socket_->lowest_layer().remote_endpoint().address().to_string(),
                                       remoteEndpoint.getPort());
                    }

                    /**
                     *
                     * This function is used to obtain the locally bound endpoint of the socket.
                     *
                     * @returns An address that represents the local endpoint of the socket.
                     */
                    virtual std::unique_ptr<Address> localSocketAddress() const override {
                        boost::system::error_code ec;
                        boost::asio::ip::basic_endpoint<boost::asio::ip::tcp> localEndpoint = socket_->lowest_layer().local_endpoint(
                                ec);
                        if (ec) {
                            return std::unique_ptr<Address>();
                        }
                        return std::unique_ptr<Address>(
                                new Address(localEndpoint.address().to_string(), localEndpoint.port()));
                    }

                    const Address &getRemoteEndpoint() const override {
                        return remoteEndpoint;
                    }

                protected:
                    void setSocketOptions(const client::config::SocketOptions &socketOptions) {
                        auto &lowestLayer = socket_->lowest_layer();

                        lowestLayer.set_option(boost::asio::ip::tcp::no_delay(socketOptions.isTcpNoDelay()));

                        lowestLayer.set_option(boost::asio::socket_base::keep_alive(socketOptions.isKeepAlive()));

                        lowestLayer.set_option(boost::asio::socket_base::reuse_address(socketOptions.isReuseAddress()));

                        int lingerSeconds = socketOptions.getLingerSeconds();
                        if (lingerSeconds > 0) {
                            lowestLayer.set_option(boost::asio::socket_base::linger(true, lingerSeconds));
                        }

                        int bufferSize = socketOptions.getBufferSizeInBytes();
                        if (bufferSize > 0) {
                            lowestLayer.set_option(boost::asio::socket_base::receive_buffer_size(bufferSize));
                            lowestLayer.set_option(boost::asio::socket_base::send_buffer_size(bufferSize));
                        }

                        // SO_NOSIGPIPE seems to be internally handled by asio on connect and accept. no such option
                        // is defined at the api, hence not setting this option
                    }

                    void do_read(const std::shared_ptr<connection::Connection> &connection) {
                        using namespace boost::asio;
                        using namespace boost::asio::ip;

                        socket_->async_read_some(buffer(connection->readHandler.byteBuffer.ix(),
                                                        connection->readHandler.byteBuffer.remaining()),
                                                 [=](const boost::system::error_code &ec, std::size_t bytesRead) {
                                                     if (ec) {
                                                         connection->close(
                                                                 (boost::format("Socket read error. %1% for %2%") % ec %
                                                                  (*connection)).str());
                                                         return;
                                                     }

                                                     connection->readHandler.byteBuffer.safeIncrementPosition(
                                                             bytesRead);

                                                     connection->readHandler.handle();

                                                     do_read(connection);
                                                 });
                    }

                    virtual void async_handle_connect(const std::shared_ptr<connection::Connection> &connection,
                                                      const std::shared_ptr<connection::AuthenticationFuture> &authFuture) {
                        setSocketOptions(socketOptions);

                        static const std::string PROTOCOL_TYPE_BYTES("CB2");
                        async_write(*socket_, boost::asio::buffer(PROTOCOL_TYPE_BYTES),
                                    [=](const boost::system::error_code &ec, size_t bytesWritten) {
                                        if (ec) {
                                            authFuture->onFailure(
                                                    std::make_shared<exception::IOException>(
                                                            "Connection::do_connect",
                                                            (boost::format(
                                                                    "Write error for initial protocol bytes %1%. %2% for %3%") %
                                                             PROTOCOL_TYPE_BYTES % ec %
                                                             (*connection)).str()));
                                            return;
                                        }

                                        do_read(connection);

                                        connection->authenticate();
                                    });
                    }

                    client::config::SocketOptions &socketOptions;
                    std::unique_ptr<T> socket_;
                    Address remoteEndpoint;
                    boost::asio::io_context &io;
                    boost::asio::ip::tcp::resolver resolver;
                    boost::asio::steady_timer connectTimer;
                    std::chrono::milliseconds connectTimeoutMillis;
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
