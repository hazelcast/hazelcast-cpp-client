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
#include "HazelcastServerFactory.h"
#include "ClientTestSupport.h"
#include "HazelcastServer.h"

#include "hazelcast/client/HazelcastClient.h"

namespace hazelcast {
    namespace client {
        namespace test {
            class SocketOptionsTest : public ClientTestSupport {
            };

            TEST_F(SocketOptionsTest, testConfiguration) {
                HazelcastServer instance(*g_srvFactory);

                const int bufferSize = 2 * 1024;
                ClientConfig clientConfig;
                clientConfig.getNetworkConfig().getSocketOptions().setKeepAlive(false).setReuseAddress(
                        true).setTcpNoDelay(false).setLingerSeconds(5).setBufferSizeInBytes(bufferSize);

                HazelcastClient client(clientConfig);

                config::SocketOptions &socketOptions = client.getClientConfig().getNetworkConfig().getSocketOptions();
                ASSERT_FALSE(socketOptions.isKeepAlive());
                ASSERT_FALSE(socketOptions.isTcpNoDelay());
                ASSERT_EQ(5, socketOptions.getLingerSeconds());
                ASSERT_EQ(bufferSize, socketOptions.getBufferSizeInBytes());
            }
        }
    }
}
