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
                    BaseSocket(boost::asio::ip::tcp::resolver &ioResolver,
                            const Address &address, client::config::SocketOptions &socketOptions,
                            boost::asio::io_context &io, std::chrono::steady_clock::duration &connectTimeoutInMillis)
                            : socketOptions(socketOptions), remoteEndpoint(address), io(io),
                              socketStrand(io), connectTimer(socketStrand),
                              connectTimeoutMillis(connectTimeoutInMillis), resolver(ioResolver), socket_(socketStrand) {
                    }

                    template<typename CONTEXT>
                    BaseSocket(boost::asio::ip::tcp::resolver &ioResolver,
                            const Address &address, client::config::SocketOptions &socketOptions,
                            boost::asio::io_context &io, std::chrono::steady_clock::duration &connectTimeoutInMillis,
                            CONTEXT &context)
                            : socketOptions(socketOptions), remoteEndpoint(address), io(io),
                              socketStrand(io), connectTimer(socketStrand),
                              connectTimeoutMillis(connectTimeoutInMillis), resolver(ioResolver),
                              socket_(socketStrand, context) {
                    }

                    ~BaseSocket() override {
                        close();
                    }

                    void asyncStart(const std::shared_ptr<connection::Connection> connection,
                                    const std::shared_ptr<connection::AuthenticationFuture> authFuture) override {
                        using namespace boost::asio;
                        using namespace boost::asio::ip;

                        connectTimer.expires_from_now(connectTimeoutMillis);
                        connectTimer.async_wait([this, connection, authFuture](const boost::system::error_code &ec) {
                            if (ec == boost::asio::error::operation_aborted) {
                                return;
                            }
                            authFuture->onFailure(std::make_exception_ptr(exception::IOException(
                                    "Connection::asyncStart", (boost::format(
                                            "Connection establishment to server %1% timed out in %2% msecs. %3%") %
                                                               remoteEndpoint % connectTimeoutMillis.count() %
                                                               ec).str())));
                            return;
                        });
                        resolver.async_resolve(remoteEndpoint.getHost(), std::to_string(remoteEndpoint.getPort()),
                                               bind_executor(socketStrand, [=](const boost::system::error_code &ec,
                                                   tcp::resolver::results_type resolvedAddresses) {
                                                   if (ec) {
                                                       boost::system::error_code ignored;
                                                       connectTimer.cancel(ignored);
                                                       authFuture->onFailure(
                                                               std::make_exception_ptr(exception::IOException(
                                                                       "Connection::asyncStart", (boost::format(
                                                                               "Could not resolve server address %1%. %2%") %
                                                                                                  remoteEndpoint %
                                                                                                  ec).str())));
                                                       return;
                                                   }

                                                   async_connect(socket_.lowest_layer(), resolvedAddresses,
                                                                 [=](const boost::system::error_code &ec,
                                                                     const tcp::endpoint &) {
                                                                     boost::system::error_code ignored;
                                                                     connectTimer.cancel(ignored);
                                                                     if (ec) {
                                                                         authFuture->onFailure(
                                                                                 std::make_exception_ptr(
                                                                                         exception::IOException(
                                                                                                 "Connection::asyncStart",
                                                                                                 (boost::format(
                                                                                                         "Socket failed to connect to server address %1%. %2%") %
                                                                                                  remoteEndpoint %
                                                                                                  ec).str())));
                                                                         return;
                                                                     }

                                                                     this->async_handle_connect(connection, authFuture);
                                                                 });
                                               }));
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

                            boost::asio::async_write(socket_,
                                                     boost::asio::buffer(message->getBuffer().data(),
                                                                         message->getFrameLength()),
                                                     [=](const boost::system::error_code &ec,
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
                                                     });
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
                    std::unique_ptr<Address> localSocketAddress() const override {
                        boost::system::error_code ec;
                        boost::asio::ip::basic_endpoint<boost::asio::ip::tcp> localEndpoint = socket_.lowest_layer().local_endpoint(
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

                    virtual void async_handle_connect(const std::shared_ptr<connection::Connection> connection,
                                                      const std::shared_ptr<connection::AuthenticationFuture> authFuture) {
                        try {
                            setSocketOptions(socketOptions);
                        } catch (std::exception &e) {
                            std::make_exception_ptr(exception::IOException(
                                    "Connection::do_connect",
                                    (boost::format(
                                            "Failed to set socket options for %1%. %2%") % e.what() %
                                     (*connection)).str()));
                            return;
                        }

                        static const std::string PROTOCOL_TYPE_BYTES("CB2");
                        async_write(socket_, boost::asio::buffer(PROTOCOL_TYPE_BYTES),
                                    [=](const boost::system::error_code &ec, size_t bytesWritten) {
                                        if (ec) {
                                            authFuture->onFailure(
                                                    std::make_exception_ptr(exception::IOException(
                                                            "Connection::do_connect",
                                                            (boost::format(
                                                                    "Write error for initial protocol bytes %1%. %2% for %3%") %
                                                             PROTOCOL_TYPE_BYTES % ec %
                                                             (*connection)).str())));
                                            return;
                                        }

                                        do_read(connection);

                                        connection->authenticate();
                                    });
                    }

                    client::config::SocketOptions &socketOptions;
                    Address remoteEndpoint;
                    boost::asio::io_context &io;
                    boost::asio::io_context::strand socketStrand;
                    boost::asio::steady_timer connectTimer;
                    std::chrono::steady_clock::duration connectTimeoutMillis;
                    boost::asio::ip::tcp::resolver &resolver;
                    T socket_;
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
