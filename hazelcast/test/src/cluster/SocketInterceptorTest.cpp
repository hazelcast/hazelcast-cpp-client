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
#include "hazelcast/util/CountDownLatch.h"
#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/client/SocketInterceptor.h"
#include "hazelcast/client/Socket.h"

namespace hazelcast {
    namespace client {
        namespace test {
            class SocketInterceptorTest : public ClientTestSupport {
            };

            class MySocketInterceptor : public SocketInterceptor {
            public:
                MySocketInterceptor(util::CountDownLatch &latch) : interceptorLatch(latch) {
                }
                
                void onConnect(const hazelcast::client::Socket &connectedSocket) {
                    ASSERT_EQ("127.0.0.1", connectedSocket.getAddress().getHost());
                    ASSERT_NE(0, connectedSocket.getAddress().getPort());
                    interceptorLatch.countDown();
                }

            private:
                util::CountDownLatch &interceptorLatch;
            };

            #ifdef HZ_BUILD_WITH_SSL
            TEST_F(SocketInterceptorTest, interceptSSLBasic) {
                HazelcastServerFactory sslFactory(getSslFilePath());
                HazelcastServer instance(sslFactory);
                std::auto_ptr<ClientConfig> config = getConfig();
                util::CountDownLatch interceptorLatch(1);
                MySocketInterceptor interceptor(interceptorLatch);
                config->setSocketInterceptor(&interceptor);
                config::SSLConfig sslConfig;
                sslConfig.setEnabled(true).addVerifyFile(getCAFilePath());
                config->getNetworkConfig().setSSLConfig(sslConfig);
                HazelcastClient client(*config);
                interceptorLatch.await(2);
            }
            #endif

            TEST_F(SocketInterceptorTest, interceptBasic) {
                HazelcastServer instance(*g_srvFactory);
                std::auto_ptr<ClientConfig> config = getConfig();
                util::CountDownLatch interceptorLatch(1);
                MySocketInterceptor interceptor(interceptorLatch);
                config->setSocketInterceptor(&interceptor);
                HazelcastClient client(*config);
                interceptorLatch.await(2);
            }
        }
    }
}

