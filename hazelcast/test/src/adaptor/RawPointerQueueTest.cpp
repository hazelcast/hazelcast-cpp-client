/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
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
//
// Created by ihsan demir on 21/3/16.
/**
 * This has to be the first include, so that Python.h is the first include. Otherwise, compilation warning such as
 * "_POSIX_C_SOURCE" redefined occurs.
 */
#include "HazelcastServerFactory.h"

#include "HazelcastServer.h"
#include "ClientTestSupport.h"

#include "hazelcast/util/Util.h"
#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/client/ItemListener.h"
#include "hazelcast/client/adaptor/RawPointerQueue.h"

namespace hazelcast {
    namespace client {
        namespace test {
            namespace adaptor {
                class RawPointerQueueTest : public ClientTestSupport {
                protected:
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

                    virtual void TearDown() {
                        q->clear();
                    }

                    static void SetUpTestCase() {
                        instance = new HazelcastServer(*g_srvFactory);
                        clientConfig = new ClientConfig();
                        clientConfig->addAddress(Address(g_srvFactory->getServerAddress(), 5701));
                        client = new HazelcastClient(*clientConfig);
                        legacy = new IQueue<std::string>(client->getQueue<std::string>("MyQueue"));
                        q = new client::adaptor::RawPointerQueue<std::string>(*legacy);
                    }

                    static void TearDownTestCase() {
                        delete q;
                        delete legacy;
                        delete client;
                        delete clientConfig;
                        delete instance;

                        q = NULL;
                        legacy = NULL;
                        client = NULL;
                        clientConfig = NULL;
                        instance = NULL;
                    }

                    static HazelcastServer *instance;
                    static ClientConfig *clientConfig;
                    static HazelcastClient *client;
                    static IQueue<std::string> *legacy;
                    static client::adaptor::RawPointerQueue<std::string> *q;
                };

                HazelcastServer *RawPointerQueueTest::instance = NULL;
                ClientConfig *RawPointerQueueTest::clientConfig = NULL;
                HazelcastClient *RawPointerQueueTest::client = NULL;
                IQueue<std::string> *RawPointerQueueTest::legacy = NULL;
                client::adaptor::RawPointerQueue<std::string> *RawPointerQueueTest::q = NULL;

                TEST_F(RawPointerQueueTest, testListener) {
                    ASSERT_EQ(0, q->size());

                    util::CountDownLatch latch(5);

                    QueueTestItemListener qener(latch);
                    std::string id = q->addItemListener(qener, true);

                    util::sleep(1);

                    for (int i = 0; i < 5; i++) {
                        ASSERT_TRUE(q->offer(std::string("event_item") + util::IOUtil::to_string(i)));
                    }

                    ASSERT_TRUE(latch.await(5));
                    ASSERT_TRUE(q->removeItemListener(id));
                }

                void testOfferPollThread2(util::ThreadArgs &args) {
                    client::adaptor::RawPointerQueue<std::string> *q = (client::adaptor::RawPointerQueue<std::string> *) args.arg0;
                    util::sleep(2);
                    q->offer("item1");
                    util::ILogger::getLogger().info("[testOfferPollThread2] item1 is offered");
                }

                TEST_F(RawPointerQueueTest, testOfferPoll) {
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

                    util::Thread t2(testOfferPollThread2, q);

                    std::auto_ptr<std::string> item = q->poll(30 * 1000);
                    ASSERT_NE(item.get(), (std::string *) NULL);
                    ASSERT_EQ("item1", *item);
                    t2.join();
                }

                TEST_F(RawPointerQueueTest, testRemainingCapacity) {
                    int capacity = q->remainingCapacity();
                    ASSERT_TRUE(capacity > 10000);
                    q->offer("item");
                    ASSERT_EQ(capacity - 1, q->remainingCapacity());
                }

                TEST_F(RawPointerQueueTest, testPeek) {
                    ASSERT_TRUE(q->offer("peek 1"));
                    ASSERT_TRUE(q->offer("peek 2"));
                    ASSERT_TRUE(q->offer("peek 3"));

                    std::auto_ptr<std::string> item = q->peek();
                    ASSERT_NE((std::string *)NULL, item.get());
                    ASSERT_EQ("peek 1", *item);
                }

                TEST_F(RawPointerQueueTest, testTake) {
                    ASSERT_TRUE(q->offer("peek 1"));
                    ASSERT_TRUE(q->offer("peek 2"));
                    ASSERT_TRUE(q->offer("peek 3"));

                    std::auto_ptr<std::string> item = q->take();
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
                    util::Thread t2(testOfferPollThread2, q);

                    item = q->take();  //  should block till it gets an item
                    ASSERT_NE((std::string *)NULL, item.get());
                    ASSERT_EQ("item1", *item);

                    t2.join();
                }

                TEST_F(RawPointerQueueTest, testRemove) {
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


                TEST_F(RawPointerQueueTest, testContains) {
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

                TEST_F(RawPointerQueueTest, testDrain) {
                    ASSERT_TRUE(q->offer("item1"));
                    ASSERT_TRUE(q->offer("item2"));
                    ASSERT_TRUE(q->offer("item3"));
                    ASSERT_TRUE(q->offer("item4"));
                    ASSERT_TRUE(q->offer("item5"));

                    std::auto_ptr<client::DataArray<std::string> > list = q->drainTo(2);
                    ASSERT_EQ((size_t)2U, list->size());
                    ASSERT_NE((std::string *)NULL, list->get(0));
                    ASSERT_NE((std::string *)NULL, list->get(1));
                    ASSERT_EQ("item1", *list->get(0));
                    ASSERT_EQ("item2", *list->get(1));

                    list = q->drainTo();
                    ASSERT_EQ((size_t)3U, list->size());
                    ASSERT_NE((std::string *)NULL, list->get(0));
                    ASSERT_NE((std::string *)NULL, list->get(1));
                    ASSERT_NE((std::string *)NULL, list->get(2));
                    ASSERT_EQ("item3", *list->get(0));
                    ASSERT_EQ("item4", *list->get(1));
                    ASSERT_EQ("item5", *list->get(2));

                    ASSERT_TRUE(q->offer("item1"));
                    ASSERT_TRUE(q->offer("item2"));
                    ASSERT_TRUE(q->offer("item3"));
                    list = q->drainTo(5);
                    ASSERT_EQ((size_t)3U, list->size());
                    ASSERT_NE((std::string *)NULL, list->get(0));
                    ASSERT_NE((std::string *)NULL, list->get(1));
                    ASSERT_NE((std::string *)NULL, list->get(2));
                    ASSERT_EQ("item1", *list->get(0));
                    ASSERT_EQ("item2", *list->get(1));
                    ASSERT_EQ("item3", *list->get(2));
                }

                TEST_F(RawPointerQueueTest, testToArray) {
                    ASSERT_TRUE(q->offer("item1"));
                    ASSERT_TRUE(q->offer("item2"));
                    ASSERT_TRUE(q->offer("item3"));
                    ASSERT_TRUE(q->offer("item4"));
                    ASSERT_TRUE(q->offer("item5"));

                    std::auto_ptr<client::DataArray<std::string> > array = q->toArray();
                    size_t size = array->size();
                    ASSERT_EQ(5U, size);
                    for (size_t i = 0; i < size; i++) {
                        const std::string *item = (*array)[i];
                        ASSERT_NE((std::string *)NULL, item);
                        ASSERT_EQ(std::string("item") + util::IOUtil::to_string(i + 1), *item);
                    }
                }

                TEST_F(RawPointerQueueTest, testAddAll) {
                    std::vector<std::string> coll;
                    coll.push_back("item1");
                    coll.push_back("item2");
                    coll.push_back("item3");
                    coll.push_back("item4");

                    ASSERT_TRUE(q->addAll(coll));
                    int size = q->size();
                    ASSERT_EQ(size, (int) coll.size());
                }

                TEST_F(RawPointerQueueTest, testRemoveRetain) {
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

                TEST_F(RawPointerQueueTest, testClear) {
                    ASSERT_TRUE(q->offer("item1"));
                    ASSERT_TRUE(q->offer("item2"));
                    ASSERT_TRUE(q->offer("item3"));
                    ASSERT_TRUE(q->offer("item4"));
                    ASSERT_TRUE(q->offer("item5"));

                    q->clear();

                    ASSERT_EQ(0, q->size());
                    ASSERT_EQ(q->poll().get(), (std::string *) NULL);
                }
            }
        }
    }
}

