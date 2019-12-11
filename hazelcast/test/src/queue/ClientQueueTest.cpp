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

#include "hazelcast/util/Util.h"
#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/client/ItemListener.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/IQueue.h"

namespace hazelcast {
    namespace client {
        namespace test {
            class ClientQueueTest : public ClientTestSupport {
            protected:
                virtual void TearDown() {
                    q->clear();
                }

                static void SetUpTestCase() {
                    instance = new HazelcastServer(*g_srvFactory);
                    client = new HazelcastClient(getConfig());
                    q = new IQueue<std::string>(client->getQueue<std::string>("MyQueue"));
                }

                static void TearDownTestCase() {
                    delete q;
                    delete client;
                    delete instance;

                    q = NULL;
                    client = NULL;
                    instance = NULL;
                }

                static HazelcastServer *instance;
                static HazelcastClient *client;
                static IQueue<std::string> *q;
            };
            
            HazelcastServer *ClientQueueTest::instance = NULL;
            HazelcastClient *ClientQueueTest::client = NULL;
            IQueue<std::string> *ClientQueueTest::q = NULL;

            class QueueTestItemListener : public ItemListener<std::string> {
            public:
                QueueTestItemListener(util::CountDownLatch &latch)
                        : latch(latch) {

                }

                void itemAdded(const ItemEvent<std::string> &itemEvent) {
                    latch.countDown();
                }

                void itemRemoved(const ItemEvent<std::string> &item) {
                }

            private:
                util::CountDownLatch &latch;
            };

            TEST_F(ClientQueueTest, testListener) {
                ASSERT_EQ(0, q->size());

                util::CountDownLatch latch(5);

                QueueTestItemListener listener(latch);
                std::string id = q->addItemListener(listener, true);

                util::sleep(1);

                for (int i = 0; i < 5; i++) {
                    ASSERT_TRUE(q->offer(std::string("event_item") + util::IOUtil::to_string(i)));
                }

                ASSERT_TRUE(latch.await(5));
                ASSERT_TRUE(q->removeItemListener(id));

                // added for test coverage
                ASSERT_NO_THROW(q->destroy());
            }

            void testOfferPollThread2(util::ThreadArgs &args) {
                IQueue<std::string> *q = (IQueue<std::string> *) args.arg0;
                util::sleep(2);
                q->offer("item1");
            }

            TEST_F(ClientQueueTest, testOfferPoll) {
                for (int i = 0; i < 10; i++) {
                    bool result = q->offer("item");
                    ASSERT_TRUE(result);
                }
                ASSERT_EQ(10, q->size());
                q->poll();
                bool result = q->offer("item", 5);
                ASSERT_TRUE(result);

                for (int i = 0; i < 10; i++) {
                    ASSERT_NE(q->poll().get(), (std::string *) NULL);
                }
                ASSERT_EQ(0, q->size());

                util::StartedThread t2(testOfferPollThread2, q);

                boost::shared_ptr<std::string> item = q->poll(30 * 1000);
                ASSERT_NE(item.get(), (std::string *) NULL);
                ASSERT_EQ("item1", *item);
                t2.join();
            }

            TEST_F(ClientQueueTest, testPeek) {
                ASSERT_TRUE(q->offer("peek 1"));
                ASSERT_TRUE(q->offer("peek 2"));
                ASSERT_TRUE(q->offer("peek 3"));

                boost::shared_ptr<std::string> item = q->peek();
                ASSERT_NE((std::string *)NULL, item.get());
                ASSERT_EQ("peek 1", *item);
            }
            
            TEST_F(ClientQueueTest, testTake) {
                q->put("peek 1");
                ASSERT_TRUE(q->offer("peek 2"));
                ASSERT_TRUE(q->offer("peek 3"));

                boost::shared_ptr<std::string> item = q->take();
                ASSERT_NE((std::string *)NULL, item.get());
                ASSERT_EQ("peek 1", *item);

                item = q->take();
                ASSERT_NE((std::string *)NULL, item.get());
                ASSERT_EQ("peek 2", *item);

                item = q->take();
                ASSERT_NE((std::string *)NULL, item.get());
                ASSERT_EQ("peek 3", *item);

                ASSERT_TRUE(q->isEmpty());

                // start a thread to insert an item
                util::StartedThread t2(testOfferPollThread2, q);

                item = q->take();  //  should block till it gets an item
                ASSERT_NE((std::string *)NULL, item.get());
                ASSERT_EQ("item1", *item);

                t2.join();
            }

            TEST_F(ClientQueueTest, testRemainingCapacity) {
                int capacity = q->remainingCapacity();
                ASSERT_TRUE(capacity > 10000);
                q->offer("item");
                ASSERT_EQ(capacity - 1, q->remainingCapacity());
            }


            TEST_F(ClientQueueTest, testRemove) {
                ASSERT_TRUE(q->offer("item1"));
                ASSERT_TRUE(q->offer("item2"));
                ASSERT_TRUE(q->offer("item3"));

                ASSERT_FALSE(q->remove("item4"));
                ASSERT_EQ(3, q->size());

                ASSERT_TRUE(q->remove("item2"));

                ASSERT_EQ(2, q->size());

                ASSERT_EQ("item1", *(q->poll()));
                ASSERT_EQ("item3", *(q->poll()));
            }


            TEST_F(ClientQueueTest, testContains) {
                ASSERT_TRUE(q->offer("item1"));
                ASSERT_TRUE(q->offer("item2"));
                ASSERT_TRUE(q->offer("item3"));
                ASSERT_TRUE(q->offer("item4"));
                ASSERT_TRUE(q->offer("item5"));


                ASSERT_TRUE(q->contains("item3"));
                ASSERT_FALSE(q->contains("item"));

                std::vector<std::string> list;
                list.push_back("item4");
                list.push_back("item2");

                ASSERT_TRUE(q->containsAll(list));

                list.push_back("item");
                ASSERT_FALSE(q->containsAll(list));
            }

            TEST_F(ClientQueueTest, testDrain) {
                ASSERT_TRUE(q->offer("item1"));
                ASSERT_TRUE(q->offer("item2"));
                ASSERT_TRUE(q->offer("item3"));
                ASSERT_TRUE(q->offer("item4"));
                ASSERT_TRUE(q->offer("item5"));

                std::vector<std::string> list;
                size_t result = q->drainTo(list, 2);
                ASSERT_EQ(2U, result);
                ASSERT_EQ("item1", list[0]);
                ASSERT_EQ("item2", list[1]);

                std::vector<std::string> list2;
                result = q->drainTo(list2);
                ASSERT_EQ(3U, result);
                ASSERT_EQ("item3", list2[0]);
                ASSERT_EQ("item4", list2[1]);
                ASSERT_EQ("item5", list2[2]);

                ASSERT_TRUE(q->offer("item1"));
                ASSERT_TRUE(q->offer("item2"));
                ASSERT_TRUE(q->offer("item3"));
                list2.clear();
                result = q->drainTo(list2, 5);
                ASSERT_EQ(3U, result);
                ASSERT_EQ("item1", list2[0]);
                ASSERT_EQ("item2", list2[1]);
                ASSERT_EQ("item3", list2[2]);
            }

            TEST_F(ClientQueueTest, testToArray) {
                ASSERT_TRUE(q->offer("item1"));
                ASSERT_TRUE(q->offer("item2"));
                ASSERT_TRUE(q->offer("item3"));
                ASSERT_TRUE(q->offer("item4"));
                ASSERT_TRUE(q->offer("item5"));

                std::vector<std::string> array = q->toArray();
                size_t size = array.size();
                for (size_t i = 0; i < size; i++) {
                    ASSERT_EQ(std::string("item") + util::IOUtil::to_string(i + 1), array[i]);
                }
            }

            TEST_F(ClientQueueTest, testAddAll) {
                std::vector<std::string> coll;
                coll.push_back("item1");
                coll.push_back("item2");
                coll.push_back("item3");
                coll.push_back("item4");

                ASSERT_TRUE(q->addAll(coll));
                int size = q->size();
                ASSERT_EQ(size, (int) coll.size());
            }

            TEST_F(ClientQueueTest, testRemoveRetain) {
                ASSERT_TRUE(q->offer("item1"));
                ASSERT_TRUE(q->offer("item2"));
                ASSERT_TRUE(q->offer("item3"));
                ASSERT_TRUE(q->offer("item4"));
                ASSERT_TRUE(q->offer("item5"));

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
                ASSERT_FALSE(q->retainAll(list));
                ASSERT_EQ(2, q->size());

                list.clear();
                ASSERT_TRUE(q->retainAll(list));
                ASSERT_EQ(0, q->size());
            }

            TEST_F(ClientQueueTest, testClear) {
                ASSERT_TRUE(q->offer("item1"));
                ASSERT_TRUE(q->offer("item2"));
                ASSERT_TRUE(q->offer("item3"));
                ASSERT_TRUE(q->offer("item4"));
                ASSERT_TRUE(q->offer("item5"));

                q->clear();

                ASSERT_EQ(0, q->size());
                ASSERT_EQ(q->poll().get(), (std::string *) NULL);
            }

            TEST_F(ClientQueueTest, testIsEmpty) {
                ASSERT_TRUE(q->isEmpty());
                ASSERT_TRUE(q->offer("item1"));
                ASSERT_FALSE(q->isEmpty());
            }

            TEST_F(ClientQueueTest, testPut) {
                q->put("item1");
                ASSERT_EQ(1, q->size());
            }
        }
    }
}

