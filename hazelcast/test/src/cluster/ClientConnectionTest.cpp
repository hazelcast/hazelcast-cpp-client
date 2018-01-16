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
#endif

#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/client/connection/ConnectionManager.h"
#include "ClientTestSupport.h"
#include "HazelcastServer.h"
#include "HazelcastServerFactory.h"

namespace hazelcast {
    namespace client {
        namespace test {
            class ClientConnectionTest : public ClientTestSupport {
            protected:
                #ifdef HZ_BUILD_WITH_SSL
                std::vector<internal::socket::SSLSocket::CipherInfo> getCiphers(ClientConfig &config) {
                    HazelcastClient client(config);
                    spi::ClientContext context(client);
                    std::vector<boost::shared_ptr<connection::Connection> > conns = context.getConnectionManager().getConnections();
                    EXPECT_GT(conns.size(), (size_t) 0);
                    boost::shared_ptr<connection::Connection> aConnection = conns[0];
                    internal::socket::SSLSocket &socket = (internal::socket::SSLSocket &) aConnection->getSocket();
                    return socket.getCiphers();
                }
                #endif
            };

            TEST_F(ClientConnectionTest, testTcpSocketTimeoutToOutsideNetwork) {
                HazelcastServer instance(*g_srvFactory);
                ClientConfig config;
                config.addAddress(Address("8.8.8.8", 5701));
                ASSERT_THROW(HazelcastClient client(config), exception::IllegalStateException);
            }

            #ifdef HZ_BUILD_WITH_SSL
            TEST_F(ClientConnectionTest, testSslSocketTimeoutToOutsideNetwork) {
                HazelcastServerFactory sslFactory(getSslFilePath());
                HazelcastServer instance(sslFactory);
                ClientConfig config;
                config.addAddress(Address("8.8.8.8", 5701));
                config.getNetworkConfig().getSSLConfig().setEnabled(true).addVerifyFile(getCAFilePath());
                ASSERT_THROW(HazelcastClient client(config), exception::IllegalStateException);
            }

            TEST_F(ClientConnectionTest, testSSLWrongCAFilePath) {
                HazelcastServerFactory sslFactory(getSslFilePath());
                HazelcastServer instance(sslFactory);
                std::auto_ptr<ClientConfig> config = getConfig();
                config->getNetworkConfig().getSSLConfig().setEnabled(true).addVerifyFile("abc");
                ASSERT_THROW(HazelcastClient client(*config), exception::IllegalStateException);
            }

            TEST_F(ClientConnectionTest, testExcludedCipher) {
                HazelcastServerFactory sslFactory(getSslFilePath());
                HazelcastServer instance(sslFactory);

                std::auto_ptr<ClientConfig> config = getConfig();
                config->getNetworkConfig().getSSLConfig().setEnabled(true).addVerifyFile(getCAFilePath()).setCipherList(
                        "HIGH");
                std::vector<internal::socket::SSLSocket::CipherInfo> supportedCiphers = getCiphers(*config);

                std::string unsupportedCipher = supportedCiphers[0].name;
                config = getConfig();
                config->getNetworkConfig().getSSLConfig().setEnabled(true).addVerifyFile(getCAFilePath()).
                        setCipherList(std::string("HIGH:!") + unsupportedCipher);
                
                std::vector<internal::socket::SSLSocket::CipherInfo> newCiphers = getCiphers(*config);

                ASSERT_EQ(supportedCiphers.size() - 1, newCiphers.size());

                for (std::vector<internal::socket::SSLSocket::CipherInfo>::const_iterator it = newCiphers.begin();
                        it != newCiphers.end();++it) {
                    ASSERT_NE(unsupportedCipher, it->name);
                }
            }
            #endif
        }
    }
}


