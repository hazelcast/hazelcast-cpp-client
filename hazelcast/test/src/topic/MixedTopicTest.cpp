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

#include <HazelcastServer.h>
#include "ClientTestSupport.h"

#include "hazelcast/client/HazelcastClient.h"

namespace hazelcast {
    namespace client {

        class HazelcastClient;

        namespace test {
            class MixedTopicTest : public ClientTestSupport {
            public:
                MixedTopicTest();

            protected:
                HazelcastServer instance;
                ClientConfig clientConfig;
                HazelcastClient client;
                mixedtype::ITopic topic;
            };


            MixedTopicTest::MixedTopicTest()
                    : instance(*g_srvFactory), client(getNewClient()),
                      topic(client.toMixedType().getTopic("MixedTopicTest")) {
            }

            class MyMessageListener : public mixedtype::topic::MessageListener {
            public:
                MyMessageListener(util::CountDownLatch &latch)
                        : latch(latch) {
                }

                virtual void onMessage(std::unique_ptr<topic::Message<TypedData> > &&message) {
                    latch.countDown();
                }

            private:
                util::CountDownLatch &latch;
            };

            TEST_F(MixedTopicTest, testTopicListeners) {
                util::CountDownLatch latch(10);
                MyMessageListener listener(latch);
                std::string id = topic.addMessageListener(listener);

                for (int i = 0; i < 10; i++) {
                    topic.publish<std::string>(std::string("naber") + util::IOUtil::to_string(i));
                }
                ASSERT_TRUE(latch.await(20));
                topic.removeMessageListener(id);
            }
        }
    }
}

