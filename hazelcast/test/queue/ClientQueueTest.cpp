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
//
// Created by sancar koyunlu on 9/4/13.



#include "hazelcast/util/Util.h"
#include "queue/ClientQueueTest.h"
#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/client/ItemListener.h"
#include "HazelcastServerFactory.h"

namespace hazelcast {
    namespace client {
        namespace test {
            using namespace iTest;

            ClientQueueTest::ClientQueueTest(HazelcastServerFactory &serverFactory)
            : ClientTestSupport<ClientQueueTest>("ClientQueueTest", &serverFactory)
            , instance(serverFactory)
            , client(getNewClient())
            , q(new IQueue< std::string>(client->getQueue< std::string >("clientQueueTest"))) {

            }

            ClientQueueTest::~ClientQueueTest() {
            }

            void ClientQueueTest::addTests() {
                addTest(&ClientQueueTest::testListener, "testListener");
                addTest(&ClientQueueTest::testOfferPoll, "testOfferPoll");
                addTest(&ClientQueueTest::testRemainingCapacity, "testRemainingCapacity");
                addTest(&ClientQueueTest::testRemove, "testRemove");
                addTest(&ClientQueueTest::testContains, "testContains");
                addTest(&ClientQueueTest::testDrain, "testDrain");
                addTest(&ClientQueueTest::testToArray, "testToArray");
                addTest(&ClientQueueTest::testAddAll, "testAddAll");
                addTest(&ClientQueueTest::testRemoveRetain, "testRemoveRetain");
                addTest(&ClientQueueTest::testClear, "testClear");
            }

            void ClientQueueTest::beforeClass() {

            }

            void ClientQueueTest::afterClass() {
                client.reset();
                instance.shutdown();
            }

            void ClientQueueTest::beforeTest() {
                q->clear();
            }

            void ClientQueueTest::afterTest() {
                q->clear();
            }

            class QueueTestItemListener : public ItemListener<std::string>  {
            public:
                QueueTestItemListener(util::CountDownLatch &latch)
                :latch(latch) {

                }

                void itemAdded(const ItemEvent<std::string>& itemEvent) {
                    latch.countDown();
                }

                void itemRemoved(const ItemEvent<std::string>& item) {
                }

            private:
                util::CountDownLatch &latch;
            };

            void ClientQueueTest::testListener() {
                assertEqual(0, q->size());

                util::CountDownLatch latch(5);

                QueueTestItemListener listener(latch);
                std::string id = q->addItemListener(listener, true);

                util::sleep(1);

                for (int i = 0; i < 5; i++) {
                    assertTrue(q->offer(std::string("event_item") + util::IOUtil::to_string(i)));
                }

                assertTrue(latch.await(5));
                assertTrue(q->removeItemListener(id));
            }

            void testOfferPollThread2(util::ThreadArgs &args) {
                IQueue<std::string> *q = (IQueue<std::string> *) args.arg0;
                util::sleep(2);
                q->offer("item1");
		std::cout << "item1 is offered" << std::endl;
            }

            void ClientQueueTest::testOfferPoll() {
                for (int i = 0; i < 10; i++) {
                    bool result = q->offer("item");
                    assertTrue(result);
                }
                assertEqual(10, q->size());
                q->poll();
                bool result = q->offer("item", 5);
                assertTrue(result);

                for (int i = 0; i < 10; i++) {
                    assertNotNull(q->poll().get());
                }
                assertEqual(0, q->size());

                util::Thread t2(testOfferPollThread2, q.get());
		
		boost::shared_ptr<std::string> item = q->poll(30 * 1000);	
		assertNotNull(item.get(), "item should not be null");
                assertEqual("item1", *item, "item1 is missing");
                t2.join();
            }

            void ClientQueueTest::testRemainingCapacity() {
                int capacity = q->remainingCapacity();
                assertTrue(capacity > 10000);
                q->offer("item");
                assertEqual(capacity - 1, q->remainingCapacity());
            }


            void ClientQueueTest::testRemove() {
                assertTrue(q->offer("item1"));
                assertTrue(q->offer("item2"));
                assertTrue(q->offer("item3"));

                assertFalse(q->remove("item4"));
                assertEqual(3, q->size());

                assertTrue(q->remove("item2"));

                assertEqual(2, q->size());

                assertEqual("item1", *(q->poll()));
                assertEqual("item3", *(q->poll()));
            }


            void ClientQueueTest::testContains() {
                assertTrue(q->offer("item1"));
                assertTrue(q->offer("item2"));
                assertTrue(q->offer("item3"));
                assertTrue(q->offer("item4"));
                assertTrue(q->offer("item5"));


                assertTrue(q->contains("item3"));
                assertFalse(q->contains("item"));

                std::vector<std::string> list;
                list.push_back("item4");
                list.push_back("item2");

                assertTrue(q->containsAll(list));

                list.push_back("item");
                assertFalse(q->containsAll(list));

            }

            void ClientQueueTest::testDrain() {
                assertTrue(q->offer("item1"));
                assertTrue(q->offer("item2"));
                assertTrue(q->offer("item3"));
                assertTrue(q->offer("item4"));
                assertTrue(q->offer("item5"));

                std::vector<std::string> list;
                int result = q->drainTo(list, 2);
                assertEqual(2, result);
                assertEqual("item1", list[0]);
                assertEqual("item2", list[1]);

                std::vector<std::string> list2;
                result = q->drainTo(list2);
                assertEqual(3, result);
                assertEqual("item3", list2[0]);
                assertEqual("item4", list2[1]);
                assertEqual("item5", list2[2]);
            }

            void ClientQueueTest::testToArray() {
                assertTrue(q->offer("item1"));
                assertTrue(q->offer("item2"));
                assertTrue(q->offer("item3"));
                assertTrue(q->offer("item4"));
                assertTrue(q->offer("item5"));

                std::vector<std::string> array = q->toArray();
                int size = array.size();
                for (int i = 0; i < size; i++) {
                    assertEqual(std::string("item") + util::IOUtil::to_string(i + 1), array[i]);
                }

            }

            void ClientQueueTest::testAddAll() {
                std::vector<std::string > coll;
                coll.push_back("item1");
                coll.push_back("item2");
                coll.push_back("item3");
                coll.push_back("item4");

                assertTrue(q->addAll(coll));
                int size = q->size();
                assertEqual(size, (int) coll.size());

            }

            void ClientQueueTest::testRemoveRetain() {
                assertTrue(q->offer("item1"));
                assertTrue(q->offer("item2"));
                assertTrue(q->offer("item3"));
                assertTrue(q->offer("item4"));
                assertTrue(q->offer("item5"));

                std::vector<std::string> list;
                list.push_back("item8");
                list.push_back("item9");
                assertFalse(q->removeAll(list));
                assertEqual(5, q->size());

                list.push_back("item3");
                list.push_back("item4");
                list.push_back("item1");
                assertTrue(q->removeAll(list));
                assertEqual(2, q->size());

                list.clear();
                list.push_back("item2");
                list.push_back("item5");
                assertFalse(q->retainAll(list));
                assertEqual(2, q->size());

                list.clear();
                assertTrue(q->retainAll(list));
                assertEqual(0, q->size());
            }

            void ClientQueueTest::testClear() {
                assertTrue(q->offer("item1"));
                assertTrue(q->offer("item2"));
                assertTrue(q->offer("item3"));
                assertTrue(q->offer("item4"));
                assertTrue(q->offer("item5"));

                q->clear();

                assertEqual(0, q->size());
                assertNull(q->poll().get());

            }
        }
    }
}

