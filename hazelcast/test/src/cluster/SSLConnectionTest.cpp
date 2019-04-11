/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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
/**
 * This has to be the first include, so that Python.h is the first include. Otherwise, compilation warning such as
 * "_POSIX_C_SOURCE" redefined occurs.
 */
#include "HazelcastServerFactory.h"
#include "ClientTestSupport.h"
#include "HazelcastServer.h"

#ifdef HZ_BUILD_WITH_SSL

#include "hazelcast/client/internal/socket/SSLSocket.h"
#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/client/connection/ClientConnectionManagerImpl.h"

namespace hazelcast {
    namespace client {
        namespace test {
            class SSLConnectionTest : public ClientTestSupport {
            protected:
                static void SetUpTestCase() {
                    sslFactory = new HazelcastServerFactory(getSslFilePath());
                    instance = new HazelcastServer(*sslFactory);
                }

                static void TearDownTestCase() {
                    delete instance;
                    delete sslFactory;
                }

                std::vector<internal::socket::SSLSocket::CipherInfo> getCiphers(ClientConfig &config) {
                    HazelcastClient client(config);
                    spi::ClientContext context(client);
                    std::vector<boost::shared_ptr<connection::Connection> > conns = context.getConnectionManager().getActiveConnections();
                    EXPECT_GT(conns.size(), (size_t) 0);
                    boost::shared_ptr<connection::Connection> aConnection = conns[0];
                    internal::socket::SSLSocket &socket = (internal::socket::SSLSocket &) aConnection->getSocket();
                    return socket.getCiphers();
                }

                static HazelcastServer *instance;
                static HazelcastServerFactory *sslFactory;
            };

            HazelcastServer *SSLConnectionTest::instance = NULL;
            HazelcastServerFactory *SSLConnectionTest::sslFactory = NULL;

            TEST_F(SSLConnectionTest, testSslSocketTimeoutToOutsideNetwork) {
                ClientConfig config;
                config.getNetworkConfig().setConnectionAttemptPeriod(1000).setConnectionTimeout(2000).addAddress(
                        Address("8.8.8.8", 5701)).getSSLConfig().setEnabled(true);
                ASSERT_THROW(HazelcastClient client(config), exception::IllegalStateException);
            }

            TEST_F(SSLConnectionTest, testSSLWrongCAFilePath) {
                ClientConfig config = getConfig();
                config.getNetworkConfig().getSSLConfig().setEnabled(true).addVerifyFile("abc");
                ASSERT_THROW(HazelcastClient client(config), exception::IllegalStateException);
            }

            TEST_F(SSLConnectionTest, testSSLIncorrectVerifyFile) {
                ClientConfig config = getConfig();
                // The contents of wrong_public_key.crt key is incorrect
                config.getNetworkConfig().getSSLConfig().setEnabled(true).addVerifyFile(
                        "hazelcast/test/resources/wrong_public_key.crt");
                ASSERT_THROW(HazelcastClient client(config), exception::IllegalStateException);
            }


            TEST_F(SSLConnectionTest, testConnectWhenNoVerifyFileTrustedCertificateServer) {
                // The server is started with the letsencrypt.jks which is from a trusted authority
                ClientConfig config = getConfig();
                config.getNetworkConfig().getSSLConfig().setEnabled(true);
                HazelcastClient client(config);
            }

            TEST_F(SSLConnectionTest, testExcludedCipher) {
                ClientConfig config = getConfig();
                config.getNetworkConfig().getSSLConfig().setEnabled(true);
                HazelcastClient client(config);
            }
        }
    }
}
#endif //#ifdef HZ_BUILD_WITH_SSL



