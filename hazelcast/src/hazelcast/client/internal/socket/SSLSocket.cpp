/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
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
#ifdef HZ_BUILD_WITH_SSL

#include "hazelcast/client/internal/socket/SSLSocket.h"
#include "hazelcast/client/config/SSLConfig.h"
#include "hazelcast/client/exception/IOException.h"
#include "hazelcast/util/IOUtil.h"

#include <boost/bind.hpp>
#include <iostream>
#include <cstdlib>
#include <string.h>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4996) //for strerror
#endif

namespace hazelcast {
    namespace client {
        namespace internal {
            namespace socket {
                struct HAZELCAST_API ConnectHandler
                {
                    ConnectHandler(asio::error_code &errorCode) : ec(errorCode) {}

                    template <typename Iterator>
                    void operator()(
                            const asio::error_code& error,
                            Iterator iterator)
                    {
                        ec = error;
                        if (error) {
                            throw exception::IOException("SSLSocket::connect", error.message());
                        }
                    }

                    asio::error_code &ec;
                };

                SSLSocket::SSLSocket(const client::Address &address, asio::io_service &ioSrv,
                                     asio::ssl::context &context)
                        : remoteEndpoint(address), isOpen(true), ioService(ioSrv), sslContext(context),
                          deadline(ioSrv) {
                    socket = std::auto_ptr<asio::ssl::stream<asio::ip::tcp::socket> >(
                            new asio::ssl::stream<asio::ip::tcp::socket>(ioService, sslContext));
                }

                SSLSocket::~SSLSocket() {
                    close();
                }

                void SSLSocket::startTimer()
                {
                    // Check whether the deadline has passed. We compare the deadline against
                    // the current time since a new asynchronous operation may have moved the
                    // deadline before this actor had a chance to run.
                    if (deadline.expires_at() <= asio::deadline_timer::traits_type::now())
                    {
                        // The deadline has passed. The socket is closed so that any outstanding
                        // asynchronous operations are cancelled. This allows the blocked
                        // connect(), read_line() or write_line() functions to return.
                        asio::error_code ignored_ec;
                        socket->lowest_layer().close(ignored_ec);

                        // There is no longer an active deadline. The expiry is set to positive
                        // infinity so that the actor takes no action until a new deadline is set.
                        deadline.expires_at(boost::posix_time::pos_infin);
                    }

                    // Put the actor back to sleep.
                    deadline.async_wait(boost::bind(&SSLSocket::startTimer, this));
                }

                int SSLSocket::connect(int timeoutInMillis) {
                    try {
                        asio::ip::tcp::resolver resolver(ioService);
                        std::ostringstream out;
                        out << remoteEndpoint.getPort();
                        asio::ip::tcp::resolver::query query(remoteEndpoint.getHost(), out.str());
                        asio::ip::tcp::resolver::iterator iterator = resolver.resolve(query);

                        deadline.expires_from_now(boost::posix_time::milliseconds(timeoutInMillis));

                        startTimer();

                        // Set up the variable that receives the result of the asynchronous
                        // operation. The error code is set to would_block to signal that the
                        // operation is incomplete. Asio guarantees that its asynchronous
                        // operations will never fail with would_block, so any other value in
                        // ec indicates completion.
                        asio::error_code ec = asio::error::would_block;

                        // Start the asynchronous operation itself. a callback will update the ec variable when the
                        // operation completes.
                        ConnectHandler handler(ec);
                        asio::async_connect(socket->lowest_layer(), iterator, handler);

                        // Block until the asynchronous operation has completed.
                        do {
                            ioService.run_one();
                        } while (ec == asio::error::would_block);

                        // Determine whether a connection was successfully established. The
                        // deadline actor may have had a chance to run and close our socket, even
                        // though the connect operation notionally succeeded. Therefore we must
                        // check whether the socket is still open before deciding if we succeeded
                        // or failed.
                        if (ec || !socket->lowest_layer().is_open()) {
                            asio::system_error systemError(ec ? ec : asio::error::operation_aborted);
                            throw exception::IOException("SSLSocket::connect", systemError.what());
                        }

                        deadline.cancel();

                        socket->handshake(asio::ssl::stream<asio::ip::tcp::socket>::client);

                        int size = 32 * 1024;
                        socket->lowest_layer().set_option(asio::socket_base::receive_buffer_size(size));
                        socket->lowest_layer().set_option(asio::socket_base::send_buffer_size(size));

                        // SO_NOSIGPIPE seems to be internally handled by asio on connect and accept. no such option
                        // is defined at the api, hence not setting this option

                        // set the socket as blocking by default
                        setBlocking(true);
                    } catch (asio::system_error &e) {
                        throw exception::IOException("SSLSocket::connect", e.what());
                    }

                    return 0;
                }

                void SSLSocket::setBlocking(bool blocking) {
                    socket->lowest_layer().non_blocking(!blocking);
                }

                int SSLSocket::send(const void *buffer, int len) const {
                    size_t size = 0;
                    asio::error_code ec;

                    size = asio::write(*socket, asio::buffer(buffer, (size_t) len),
                                       asio::transfer_exactly((size_t) len), ec);

                    return handleError("SSLSocket::send", size, ec);
                }

                int SSLSocket::receive(void *buffer, int len, int flag) const {
                    size_t size = 0;
                    asio::error_code ec;

                    size = asio::read(*socket, asio::buffer(buffer, (size_t) len),
                                      asio::transfer_exactly((size_t) len), ec);

                    return handleError("SSLSocket::receive", size, ec);
                }

                int SSLSocket::getSocketId() const {
                    return socket->lowest_layer().native_handle();
                }

                void SSLSocket::setRemoteEndpoint(const client::Address &address) {
                    remoteEndpoint = address;
                }

                const client::Address &SSLSocket::getRemoteEndpoint() const {
                    return remoteEndpoint;
                }

                client::Address SSLSocket::getAddress() const {
                    asio::ip::basic_endpoint<asio::ip::tcp> local_endpoint = socket->lowest_layer().local_endpoint();
                    return client::Address(local_endpoint.address().to_string(), local_endpoint.port());
                }

                void SSLSocket::close() {
                    if (isOpen.compareAndSet(true, false)) {
                        socket->lowest_layer().close();
                    }
                }

                int SSLSocket::handleError(const std::string &source, size_t numBytes, const asio::error_code &error) const {
                    if (error && error != asio::error::try_again && error != asio::error::would_block) {
                            throw exception::IOException(source, error.message());
                    }
                    return (int) numBytes;
                }
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif // HZ_BUILD_WITH_SSL
