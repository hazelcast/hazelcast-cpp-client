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
#include "hazelcast/client/internal/socket/SocketFactory.h"
#include "hazelcast/client/config/SSLConfig.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/config/ClientNetworkConfig.h"
#include "hazelcast/util/ILogger.h"
#include "hazelcast/client/Socket.h"
#include "hazelcast/client/internal/socket/TcpSocket.h"
#include "hazelcast/client/spi/ClientContext.h"

#ifdef HZ_BUILD_WITH_SSL
#include "hazelcast/client/internal/socket/SSLSocket.h"
#endif // HZ_BUILD_WITH_SSL

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4996) //for strerror	
#endif

namespace hazelcast {
    namespace client {
        namespace internal {
            namespace socket {
                SocketFactory::SocketFactory(spi::ClientContext &clientContext) : clientContext(clientContext) {
                }

                bool SocketFactory::start() {
                    #ifdef HZ_BUILD_WITH_SSL
                    const config::SSLConfig &sslConfig = clientContext.getClientConfig().getNetworkConfig().getSSLConfig();
                    if (sslConfig.isEnabled()) {
                        sslContext = std::auto_ptr<asio::ssl::context>(new asio::ssl::context(
                                (asio::ssl::context_base::method) sslConfig.getProtocol()));

                        const std::vector<std::string> &verifyFiles = sslConfig.getVerifyFiles();
                        bool success = true;
                        for (std::vector<std::string>::const_iterator it = verifyFiles.begin(); it != verifyFiles.end();
                             ++it) {
                            asio::error_code ec;
                            sslContext->load_verify_file(*it, ec);
                            if (ec) {
                                util::ILogger::getLogger().warning(
                                        std::string("SocketFactory::start: Failed to load CA "
                                                            "verify file at ") + *it + " "
                                        + ec.message());
                                success = false;
                            }
                        }

                        if (!success) {
                            sslContext.reset();
                            util::ILogger::getLogger().warning("SocketFactory::start: Failed to load one or more "
                                                                       "configured CA verify files (PEM files). Please "
                                                                       "correct the files and retry.");
                            return false;
                        }

                        // set cipher list if the list is set
                        const std::string &cipherList = sslConfig.getCipherList();
                        if (!cipherList.empty()) {
                            if (!SSL_CTX_set_cipher_list(sslContext->native_handle(), cipherList.c_str())) {
                                util::ILogger::getLogger().warning(
                                        std::string("SocketFactory::start: Could not load any "
                                                            "of the ciphers in the config provided "
                                                            "ciphers:") + cipherList);
                                return false;
                            }
                        }

                        ioService = std::auto_ptr<asio::io_service>(new asio::io_service);
                    }
                    #else
                    (void) clientContext;
                    #endif
                    
                    return true;
                }

                std::auto_ptr<Socket> SocketFactory::create(const Address &address) const {
                    #ifdef HZ_BUILD_WITH_SSL
                    if (sslContext.get()) {
                        return std::auto_ptr<Socket>(new internal::socket::SSLSocket(address, *ioService, *sslContext));
                    }
                    #endif
                    return std::auto_ptr<Socket>(new internal::socket::TcpSocket(address));
                }
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
