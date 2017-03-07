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
                SSLSocket::SSLSocket(const client::Address &address, asio::io_service &ioSrv,
                                     asio::ssl::context &context)
                        : remoteEndpoint(address), isOpen(true), ioService(ioSrv), sslContext(context) {
                    socket = std::auto_ptr<asio::ssl::stream<asio::ip::tcp::socket> >(
                            new asio::ssl::stream<asio::ip::tcp::socket>(ioService, sslContext));
                }

                SSLSocket::~SSLSocket() {
                    close();
                }

                int SSLSocket::connect(int timeoutInMillis) {
                    try {
                        asio::ip::tcp::resolver resolver(ioService);
                        std::ostringstream out;
                        out << remoteEndpoint.getPort();
                        asio::ip::tcp::resolver::query query(remoteEndpoint.getHost(), out.str());
                        asio::ip::tcp::resolver::iterator iterator = resolver.resolve(query);

                        asio::connect(socket->lowest_layer(), iterator);

                        asio::error_code error;
                        socket->handshake(asio::ssl::stream<asio::ip::tcp::socket>::client, error);
                        if (error) {
                            throw exception::IOException("SSLSocket::connect", error.message());
                        }
                        // set the socket as blocking by default
                        socket->lowest_layer().non_blocking(false);
                    } catch (std::exception &e) {
                        throw exception::IOException("SSLSocket::connect", e.what());
                    }

                    return 0;
                }

                void SSLSocket::setBlocking(bool blocking) {
                    socket->lowest_layer().non_blocking(!blocking);
                }

                int SSLSocket::send(const void *buffer, int len) const {
                    try {
                        asio::error_code error;
                        size_t size = socket->write_some(asio::buffer(buffer, len), error);
                        if (error) {
                            throw client::exception::IOException("SSLSocket::send", error.message());
                        }
                        return (int) size;
                    } catch (std::exception &e) {
                        throw client::exception::IOException("SSLSocket::send", e.what());
                    }
                }

                int SSLSocket::receive(void *buffer, int len, int flag) const {
                    asio::error_code error;

                    size_t size = asio::read(*socket, asio::buffer(buffer, (size_t) len), asio::transfer_all(), error);

                    handleError("SSLSocket::receive", error);

                    return (int) size;
                }

                int SSLSocket::receiveBlocking(void *buffer, int len, int flag) const {
                    try {
                        asio::error_code error;

                        size_t size = asio::read(*socket, asio::buffer(buffer, (size_t) len), error);

                        if (error) {
                            throw client::exception::IOException("SSLSocket::receive", error.message());
                        }

                        return (int) size;
                    } catch (std::exception &e) {
                        throw client::exception::IOException("SSLSocket::send", e.what());
                    }
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

                void SSLSocket::handleError(const std::string &source, const asio::error_code &error) const {
                    if (error) {
                        int errorNumber = error.value();

                        #if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
                        if (WSAEWOULDBLOCK != errorNumber && WSAEINPROGRESS != errorNumber && WSAEALREADY != errorNumber) {
                        #else
                        if (EINPROGRESS != errorNumber && EALREADY != errorNumber && EAGAIN != errorNumber) {
                            #endif

                            throw client::exception::IOException(source, error.message());
                        }
                    }
                }


            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif // HZ_BUILD_WITH_SSL
