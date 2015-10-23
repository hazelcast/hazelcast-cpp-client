/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
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
#include "ClientTopicTest.h"
#include "HazelcastServerFactory.h"
#include "hazelcast/client/HazelcastClient.h"

namespace hazelcast {
    namespace client {

        class HazelcastClient;

        namespace test {
            using namespace iTest;

            ClientTopicTest::ClientTopicTest(HazelcastServerFactory &serverFactory)
            : ClientTestSupport<ClientTopicTest>("ClientTopicTest" , &serverFactory)
            , instance(serverFactory)
            , client(getNewClient())
            , topic(new ITopic<std::string>(client->getTopic<std::string>("ClientTopicTest"))) {
            }


            void ClientTopicTest::addTests() {
                addTest(&ClientTopicTest::testTopicListeners, "testTopicListeners");
            }

            void ClientTopicTest::beforeClass() {
            }

            void ClientTopicTest::afterClass() {
                client.reset();
                instance.shutdown();
            }

            void ClientTopicTest::beforeTest() {
            }

            void ClientTopicTest::afterTest() {
            }

            class MyMessageListener {
            public:
                MyMessageListener(util::CountDownLatch &latch)
                :latch(latch) {

                }

                void onMessage(topic::Message<std::string> message) {
                    latch.countDown();
                }

            private:
                util::CountDownLatch &latch;
            };

            void ClientTopicTest::testTopicListeners() {

                util::CountDownLatch latch(10);
                MyMessageListener listener(latch);
                std::string id = topic->addMessageListener(listener);

                for (int i = 0; i < 10; i++) {
                    topic->publish(std::string("naber") + util::IOUtil::to_string(i));
                }
                assertTrue(latch.await(20 ));
                topic->removeMessageListener(id);

            }
        }
    }
}

