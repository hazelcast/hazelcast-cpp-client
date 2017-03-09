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

#include "ClientTestSupport.h"
#include "hazelcast/client/HazelcastClient.h"
#include "HazelcastServer.h"

namespace hazelcast {
    namespace client {
        namespace test {
            class ClientConnectionTest : public ClientTestSupport {
            protected:
                void testConnectionTimeout(int timeoutInMillis) {
                        //Should work without throwing exception.
                        std::auto_ptr<ClientConfig> clientConfig = getConfig();
                        clientConfig->getNetworkConfig().setConnectionTimeout(timeoutInMillis);
                        HazelcastServer instance(*g_srvFactory, true);
                        ASSERT_NO_THROW(HazelcastClient client(*clientConfig));
                    }
            };

            TEST_F(ClientConnectionTest, testTimeoutToOutsideNetwork) {
                HazelcastServer instance(*g_srvFactory, true);
                ClientConfig config;
                config.addAddress(Address("8.8.8.8", 5701));
                ASSERT_THROW(HazelcastClient client(config), exception::IllegalStateException);
            }
            
            TEST_F(ClientConnectionTest, testConnectionTimeout_withIntMax) {
                HazelcastServer instance(*g_srvFactory, true);
                ClientConfig config;
                config.addAddress(Address("8.8.8.8", 5701));
                ASSERT_THROW(HazelcastClient client(config), exception::IllegalStateException);
            }

            TEST_F(ClientConnectionTest, testSSLWrongCAFilePath) {
                HazelcastServer instance(*g_srvFactory, true);
                std::auto_ptr<ClientConfig> config = getConfig();
                config->getNetworkConfig().getSSLConfig().setEnabled(true).setCertificateAuthorityFilePath("abc");
                ASSERT_THROW(HazelcastClient client(*config), exception::IllegalStateException);
            }
        }
    }
}


