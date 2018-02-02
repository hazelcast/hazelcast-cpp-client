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
/**
 * This has to be the first include, so that Python.h is the first include. Otherwise, compilation warning such as
 * "_POSIX_C_SOURCE" redefined occurs.
 */
#include "HazelcastServerFactory.h"

#include "ClientTestSupport.h"
#include "HazelcastServer.h"

#include "hazelcast/client/mixedtype/IQueue.h"
#include "hazelcast/util/Util.h"
#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/client/ItemListener.h"
#include "hazelcast/client/ClientConfig.h"

using namespace hazelcast::client::mixedtype;

namespace hazelcast {
    namespace client {
        namespace test {
            class MixedQueueTest : public ClientTestSupport {
            protected:
                virtual void TearDown() {
                    q->clear();
                }

                static void SetUpTestCase() {
                    instance = new HazelcastServer(*g_srvFactory);
                    clientConfig = new ClientConfig();
                    client = new HazelcastClient(*clientConfig);
                    q = new mixedtype::IQueue(client->toMixedType().getQueue("MyQueue"));
                }

                static void TearDownTestCase() {
                    delete q;
                    delete client;
                    delete clientConfig;
                    delete instance;

                    q = NULL;
                    client = NULL;
                    clientConfig = NULL;
                    instance = NULL;
                }

                static void testOfferPollThread2(util::ThreadArgs &args) {
                    mixedtype::IQueue *q = (mixedtype::IQueue *) args.arg0;
                    util::sleep(2);
                    q->offer<std::string>("item1");
                    util::ILogger::getLogger().info("[testOfferPollThread2] item1 is offered");
                }

                static HazelcastServer *instance;
                static ClientConfig *clientConfig;
                static HazelcastClient *client;
                static mixedtype::IQueue *q;
            };
            
            HazelcastServer *MixedQueueTest::instance = NULL;
            ClientConfig *MixedQueueTest::clientConfig = NULL;
            HazelcastClient *MixedQueueTest::client = NULL;
            mixedtype::IQueue *MixedQueueTest::q = NULL;

            class QueueTestItemListener : public MixedItemListener {
            public:
                QueueTestItemListener(util::CountDownLatch &latch)
                        : latch(latch) {
                }

                virtual void itemAdded(const ItemEvent<TypedData> &item) {
                    latch.countDown();
                }

                virtual void itemRemoved(const ItemEvent<TypedData> &item) {
                }

            private:
                util::CountDownLatch &latch;
            };

            TEST_F(MixedQueueTest, testListener) {
                ASSERT_EQ(0, q->size());

                util::CountDownLatch latch(5);

                QueueTestItemListener listener(latch);
                std::string id = q->addItemListener(listener, true);

                util::sleep(1);

                for (int i = 0; i < 5; i++) {
                    ASSERT_TRUE(q->offer<std::string>(std::string("event_item") + util::IOUtil::to_string(i)));
                }

                ASSERT_TRUE(latch.await(5));
                ASSERT_TRUE(q->removeItemListener(id));

                // added for test coverage
                ASSERT_NO_THROW(q->destroy());
            }

            TEST_F(MixedQueueTest, testOfferPoll) {
                for (int i = 0; i < 10; i++) {
                    bool result = q->offer<std::string>("item");
                    ASSERT_TRUE(result);
                }
                ASSERT_EQ(10, q->size());
                q->poll();
                bool result = q->offer<std::string>("item", 5);
                ASSERT_TRUE(result);

                for (int i = 0; i < 10; i++) {
                    ASSERT_NE(q->poll().get<std::string>().get(), (std::string *) NULL);
                }
                ASSERT_EQ(0, q->size());

                util::Thread t2(testOfferPollThread2, q);

                std::auto_ptr<std::string> item = q->poll(30 * 1000).get<std::string>();
                ASSERT_NE(item.get(), (std::string *) NULL);
                ASSERT_EQ("item1", *item);
                t2.join();
            }

            TEST_F(MixedQueueTest, testPeek) {
                ASSERT_TRUE(q->offer<std::string>("peek 1"));
                ASSERT_TRUE(q->offer<std::string>("peek 2"));
                ASSERT_TRUE(q->offer<std::string>("peek 3"));

                std::auto_ptr<std::string> item = q->peek().get<std::string>();
                ASSERT_NE((std::string *)NULL, item.get());
                ASSERT_EQ("peek 1", *item);
            }
            
            TEST_F(MixedQueueTest, testTake) {
                ASSERT_TRUE(q->offer<std::string>("peek 1"));
                ASSERT_TRUE(q->offer<std::string>("peek 2"));
                ASSERT_TRUE(q->offer<std::string>("peek 3"));

                std::auto_ptr<std::string> item = q->take().get<std::string>();
                ASSERT_NE((std::string *)NULL, item.get());
                ASSERT_EQ("peek 1", *item);

                item = q->take().get<std::string>();
                ASSERT_NE((std::string *)NULL, item.get());
                ASSERT_EQ("peek 2", *item);

                item = q->take().get<std::string>();
                ASSERT_NE((std::string *)NULL, item.get());
                ASSERT_EQ("peek 3", *item);

                ASSERT_TRUE(q->isEmpty());

                // start a thread to insert an item
                util::Thread t2(testOfferPollThread2, q);

                item = q->take().get<std::string>();  //  should block till it gets an item
                ASSERT_NE((std::string *)NULL, item.get());
                ASSERT_EQ("item1", *item);

                t2.join();
            }

            TEST_F(MixedQueueTest, testRemainingCapacity) {
                int capacity = q->remainingCapacity();
                ASSERT_TRUE(capacity > 10000);
                q->offer<std::string>("item");
                ASSERT_EQ(capacity - 1, q->remainingCapacity());
            }


            TEST_F(MixedQueueTest, testRemove) {
                ASSERT_TRUE(q->offer<std::string>("item1"));
                ASSERT_TRUE(q->offer<std::string>("item2"));
                ASSERT_TRUE(q->offer<std::string>("item3"));

                ASSERT_FALSE(q->remove<std::string>("item4"));
                ASSERT_EQ(3, q->size());

                ASSERT_TRUE(q->remove<std::string>("item2"));

                ASSERT_EQ(2, q->size());

                ASSERT_EQ("item1", *(q->poll().get<std::string>()));
                ASSERT_EQ("item3", *(q->poll().get<std::string>()));
            }


            TEST_F(MixedQueueTest, testContains) {
                ASSERT_TRUE(q->offer<std::string>("item1"));
                ASSERT_TRUE(q->offer<std::string>("item2"));
                ASSERT_TRUE(q->offer<std::string>("item3"));
                ASSERT_TRUE(q->offer<std::string>("item4"));
                ASSERT_TRUE(q->offer<std::string>("item5"));


                ASSERT_TRUE(q->contains<std::string>("item3"));
                ASSERT_FALSE(q->contains<std::string>("item"));

                std::vector<std::string> list;
                list.push_back("item4");
                list.push_back("item2");

                ASSERT_TRUE(q->containsAll<std::string>(list));

                list.push_back("item");
                ASSERT_FALSE(q->containsAll(list));
            }

            TEST_F(MixedQueueTest, testDrain) {
                ASSERT_TRUE(q->offer<std::string>("item1"));
                ASSERT_TRUE(q->offer<std::string>("item2"));
                ASSERT_TRUE(q->offer<std::string>("item3"));
                ASSERT_TRUE(q->offer<std::string>("item4"));
                ASSERT_TRUE(q->offer<std::string>("item5"));

                std::vector<TypedData> list;
                size_t result = q->drainTo(list, 2);
                ASSERT_EQ(2U, result);
                ASSERT_EQ("item1", *list[0].get<std::string>());
                ASSERT_EQ("item2", *list[1].get<std::string>());

                std::vector<TypedData> list2;
                result = q->drainTo(list2);
                ASSERT_EQ(3U, result);
                ASSERT_EQ("item3", *list2[0].get<std::string>());
                ASSERT_EQ("item4", *list2[1].get<std::string>());
                ASSERT_EQ("item5", *list2[2].get<std::string>());

                ASSERT_TRUE(q->offer<std::string>("item1"));
                ASSERT_TRUE(q->offer<std::string>("item2"));
                ASSERT_TRUE(q->offer<std::string>("item3"));
                list2.clear();
                result = q->drainTo(list2, 5);
                ASSERT_EQ(3U, result);
                ASSERT_EQ("item1", *list2[0].get<std::string>());
                ASSERT_EQ("item2", *list2[1].get<std::string>());
                ASSERT_EQ("item3", *list2[2].get<std::string>());
            }

            TEST_F(MixedQueueTest, testToArray) {
                ASSERT_TRUE(q->offer<std::string>("item1"));
                ASSERT_TRUE(q->offer<std::string>("item2"));
                ASSERT_TRUE(q->offer<std::string>("item3"));
                ASSERT_TRUE(q->offer<std::string>("item4"));
                ASSERT_TRUE(q->offer<std::string>("item5"));

                std::vector<TypedData> array = q->toArray();
                size_t size = array.size();
                for (size_t i = 0; i < size; i++) {
                    ASSERT_EQ(std::string("item") + util::IOUtil::to_string(i + 1), *array[i].get<std::string>());
                }
            }

            TEST_F(MixedQueueTest, testAddAll) {
                std::vector<std::string> coll;
                coll.push_back("item1");
                coll.push_back("item2");
                coll.push_back("item3");
                coll.push_back("item4");

                ASSERT_TRUE(q->addAll<std::string>(coll));
                int size = q->size();
                ASSERT_EQ(size, (int) coll.size());
            }

            TEST_F(MixedQueueTest, testRemoveRetain) {
                ASSERT_TRUE(q->offer<std::string>("item1"));
                ASSERT_TRUE(q->offer<std::string>("item2"));
                ASSERT_TRUE(q->offer<std::string>("item3"));
                ASSERT_TRUE(q->offer<std::string>("item4"));
                ASSERT_TRUE(q->offer<std::string>("item5"));

                std::vector<std::string> list;
                list.push_back("item8");
                list.push_back("item9");
                ASSERT_FALSE(q->removeAll(list));
                ASSERT_EQ(5, q->size());

                list.push_back("item3");
                list.push_back("item4");
                list.push_back("item1");
                ASSERT_TRUE(q->removeAll(list));
                ASSERT_EQ(2, q->size());

                list.clear();
                list.push_back("item2");
                list.push_back("item5");
                ASSERT_FALSE(q->retainAll<std::string>(list));
                ASSERT_EQ(2, q->size());

                list.clear();
                ASSERT_TRUE(q->retainAll<std::string>(list));
                ASSERT_EQ(0, q->size());
            }

            TEST_F(MixedQueueTest, testClear) {
                ASSERT_TRUE(q->offer<std::string>("item1"));
                ASSERT_TRUE(q->offer<std::string>("item2"));
                ASSERT_TRUE(q->offer<std::string>("item3"));
                ASSERT_TRUE(q->offer<std::string>("item4"));
                ASSERT_TRUE(q->offer<std::string>("item5"));

                q->clear();

                ASSERT_EQ(0, q->size());
                ASSERT_EQ(q->poll().get<std::string>().get(), (std::string *) NULL);
            }
        }
    }
}

