/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
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

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#include <winsock2.h>
#endif

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
                SSLSocket::SSLSocket(const client::Address &address, asio::ssl::context &context,
                        client::config::SocketOptions &socketOptions) : remoteEndpoint(address),
                        sslContext(context), deadline(ioService), socketId(-1), socketOptions(socketOptions) {
                    socket = std::auto_ptr<asio::ssl::stream<asio::ip::tcp::socket> >(
                            new asio::ssl::stream<asio::ip::tcp::socket>(ioService, sslContext));
                }

                SSLSocket::~SSLSocket() {
                    close();
                }

                void SSLSocket::handleConnect(const asio::error_code &error) {
                    errorCode = error;
                }

                void SSLSocket::checkDeadline(const asio::error_code &ec) {
                    // The timer may return an error, e.g. operation_aborted when we cancel it. would_block is OK,
                    // since we set it at the start of the connection.
                    if (ec && ec != asio::error::would_block) {
                        return;
                    }

                    // Check whether the deadline has passed. We compare the deadline against
                    // the current time since a new asynchronous operation may have moved the
                    // deadline before this actor had a chance to run.
                    if (deadline.expires_at() <= asio::deadline_timer::traits_type::now()) {
                        // The deadline has passed. The socket is closed so that any outstanding
                        // asynchronous operations are cancelled. This allows the blocked
                        // connect(), read_line() or write_line() functions to return.
                        asio::error_code ignored_ec;
                        socket->lowest_layer().close(ignored_ec);

                        return;
                    }

                    // Put the actor back to sleep. _1 is for passing the error_code to the method.
                    deadline.async_wait(boost::bind(&SSLSocket::checkDeadline, this, _1));
                }

                int SSLSocket::connect(int timeoutInMillis) {
                    try {
                        asio::ip::tcp::resolver resolver(ioService);
                        std::ostringstream out;
                        out << remoteEndpoint.getPort();
                        asio::ip::tcp::resolver::query query(remoteEndpoint.getHost(), out.str());
                        asio::ip::tcp::resolver::iterator iterator = resolver.resolve(query);

                        deadline.expires_from_now(boost::posix_time::milliseconds(timeoutInMillis));

                        // Set up the variable that receives the result of the asynchronous
                        // operation. The error code is set to would_block to signal that the
                        // operation is incomplete. Asio guarantees that its asynchronous
                        // operations will never fail with would_block, so any other value in
                        // errorCode indicates completion.
                        errorCode = asio::error::would_block;

                        checkDeadline(errorCode);

                        // Start the asynchronous operation itself. a callback will update the ec variable when the
                        // operation completes.
                        asio::async_connect(socket->lowest_layer(), iterator, boost::bind(&SSLSocket::handleConnect,
                                                                                          this, _1));

                        // Block until the asynchronous operation has completed.
                        asio::error_code ioRunErrorCode;

                        // the restart is needed for the other connection attempts to work since the ioservice goes
                        // into the stopped state following the loop
                        ioService.restart();
                        do {
                            ioService.run_one(ioRunErrorCode);
                        } while (!ioRunErrorCode && (errorCode == asio::error::would_block));

                        // cancel the deadline timer
                        deadline.cancel();

                        // Cancel async connect operation if it is still in operation
                        socket->lowest_layer().cancel();

                        if (ioRunErrorCode) {
                            return ioRunErrorCode.value();
                        }

                        if (errorCode) {
                            return errorCode.value();
                        }

                        // Determine whether a connection was successfully established. The
                        // deadline actor may have had a chance to run and close our socket, even
                        // though the connect operation notionally succeeded. Therefore we must
                        // check whether the socket is still open before deciding if we succeeded
                        // or failed.
                        if (!socket->lowest_layer().is_open()) {
                            return asio::error::operation_aborted;
                        }

                        socket->handshake(asio::ssl::stream<asio::ip::tcp::socket>::client);

                        setSocketOptions();

                        setBlocking(false);
                        socketId = socket->lowest_layer().native_handle();
                    } catch (asio::system_error &e) {
                        return e.code().value();
                    }

                    return 0;
                }

                void SSLSocket::setBlocking(bool blocking) {
                    socket->lowest_layer().non_blocking(!blocking);
                }

                std::vector<SSLSocket::CipherInfo> SSLSocket::getCiphers() const {
                    STACK_OF(SSL_CIPHER) *ciphers = SSL_get_ciphers(socket->native_handle());
                    std::vector<CipherInfo> supportedCiphers;
                    for (int i = 0; i < sk_SSL_CIPHER_num (ciphers); ++i) {
                        struct SSLSocket::CipherInfo info;
                        SSL_CIPHER *cipher = sk_SSL_CIPHER_value (ciphers, i);
                        info.name = SSL_CIPHER_get_name(cipher);
                        info.numberOfBits = SSL_CIPHER_get_bits(cipher, 0);
                        info.version = SSL_CIPHER_get_version(cipher);
                        char descBuf[256];
                        info.description = SSL_CIPHER_description(cipher, descBuf, 256);
                        supportedCiphers.push_back(info);
                    }
                    return supportedCiphers;
                }

                int SSLSocket::send(const void *buffer, int len, int flag) {
                    size_t size = 0;
                    asio::error_code ec;

                    if (flag == MSG_WAITALL) {
                        size = asio::write(*socket, asio::buffer(buffer, (size_t) len),
                                           asio::transfer_exactly((size_t) len), ec);
                    } else {
                        size = socket->write_some(asio::buffer(buffer, (size_t) len), ec);
                    }

                    return handleError("SSLSocket::send", size, ec);
                }

                int SSLSocket::receive(void *buffer, int len, int flag) {
                    asio::error_code ec;
                    size_t size = 0;

                    ReadHandler readHandler(size, ec);
                    asio::error_code ioRunErrorCode;
                    ioService.restart();
                    if (flag == MSG_WAITALL) {
                        asio::async_read(*socket, asio::buffer(buffer, (size_t) len),
                                         asio::transfer_exactly((size_t) len), readHandler);
                        do {
                            ioService.run_one(ioRunErrorCode);
                            handleError("SSLSocket::receive", size, ec);
                        } while (!ioRunErrorCode && readHandler.getNumRead() < (size_t) len);

                        return (int) readHandler.getNumRead();
                    } else {
                        size = asio::read(*socket, asio::buffer(buffer, (size_t) len),
                                          asio::transfer_exactly((size_t) len), ec);
                    }

                    return handleError("SSLSocket::receive", size, ec);
                }

                int SSLSocket::getSocketId() const {
                    return socketId;
                }

                client::Address SSLSocket::getAddress() const {
                    asio::ip::basic_endpoint<asio::ip::tcp> remoteEndpoint = socket->lowest_layer().remote_endpoint();
                    return client::Address(remoteEndpoint.address().to_string(), remoteEndpoint.port());
                }

                std::auto_ptr<Address> SSLSocket::localSocketAddress() const {
                    asio::error_code ec;
                    asio::ip::basic_endpoint<asio::ip::tcp> localEndpoint = socket->lowest_layer().local_endpoint(ec);
                    if (ec) {
                        return std::auto_ptr<Address>();
                    }
                    return std::auto_ptr<Address>(
                            new Address(localEndpoint.address().to_string(), localEndpoint.port()));
                }

                void SSLSocket::close() {
                    asio::error_code ec;
                    // Call the non-exception throwing versions of the following method
                    socket->lowest_layer().close(ec);
                }

                int SSLSocket::handleError(const std::string &source, size_t numBytes,
                                           const asio::error_code &error) const {
                    if (error && error != asio::error::try_again && error != asio::error::would_block) {
                        throw exception::IOException(source, error.message());
                    }
                    return (int) numBytes;
                }

                void SSLSocket::setSocketOptions() {
                    asio::basic_socket<asio::ip::tcp, asio::stream_socket_service<asio::ip::tcp> > &lowestLayer =
                            socket->lowest_layer();

                    lowestLayer.set_option(asio::ip::tcp::no_delay(socketOptions.isTcpNoDelay()));

                    lowestLayer.set_option(asio::socket_base::keep_alive(socketOptions.isKeepAlive()));

                    lowestLayer.set_option(asio::socket_base::reuse_address(socketOptions.isReuseAddress()));

                    int lingerSeconds = socketOptions.getLingerSeconds();
                    if (lingerSeconds > 0) {
                        lowestLayer.set_option(asio::socket_base::linger(true, lingerSeconds));
                    }

                    int bufferSize = socketOptions.getBufferSize();
                    if (bufferSize > 0) {
                        lowestLayer.set_option(asio::socket_base::receive_buffer_size(bufferSize));
                        lowestLayer.set_option(asio::socket_base::send_buffer_size(bufferSize));
                    }

                    // SO_NOSIGPIPE seems to be internally handled by asio on connect and accept. no such option
                    // is defined at the api, hence not setting this option
                }

                SSLSocket::ReadHandler::ReadHandler(size_t &numRead, asio::error_code &ec) : numRead(numRead),
                                                                                             errorCode(ec) {}

                void SSLSocket::ReadHandler::operator()(const asio::error_code &err, std::size_t bytes_transferred) {
                    errorCode = err;
                    numRead += bytes_transferred;
                }

                size_t &SSLSocket::ReadHandler::getNumRead() const {
                    return numRead;
                }

                asio::error_code &SSLSocket::ReadHandler::getErrorCode() const {
                    return errorCode;
                }

                std::ostream &operator<<(std::ostream &out, const SSLSocket::CipherInfo &info) {
                    out << "Cipher{"
                           "Name: " << info.name <<
                        ", Bits:" << info.numberOfBits <<
                        ", Version:" << info.version <<
                        ", Description:" << info.description << "}";

                    return out;
                }
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif // HZ_BUILD_WITH_SSL
