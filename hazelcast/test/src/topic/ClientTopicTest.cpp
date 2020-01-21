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

#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/ITopic.h"
#include "hazelcast/client/HazelcastClient.h"

namespace hazelcast {
    namespace client {

        namespace test {
            class ClientTopicTest : public ClientTestSupport {
            public:
                ClientTopicTest();

            protected:
                HazelcastServer instance;
                ClientConfig clientConfig;
                HazelcastClient client;
                ITopic<std::string> topic;
            };

            ClientTopicTest::ClientTopicTest() : instance(*g_srvFactory), client(getNewClient()),
                                                 topic(client.getTopic<std::string>("ClientTopicTest")) {
            }

            class MyMessageListener : public topic::MessageListener<std::string> {
            public:
                MyMessageListener(util::CountDownLatch &latch)
                :latch(latch) {
                }

                void onMessage(std::auto_ptr<topic::Message<std::string> > message) {
                    latch.countDown();
                }
            private:
                util::CountDownLatch &latch;
            };

            TEST_F(ClientTopicTest, testTopicListeners) {
                util::CountDownLatch latch(10);
                MyMessageListener listener(latch);
                std::string id = topic.addMessageListener(listener);

                for (int i = 0; i < 10; i++) {
                    topic.publish(std::string("naber") + util::IOUtil::to_string(i));
                }
                ASSERT_TRUE(latch.await(20 ));
                topic.removeMessageListener(id);
            }
        }
    }
}

