//
// Created by sancar koyunlu on 9/13/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "ClientSetTest.h"
#include "hazelcast/client/HazelcastClient.h"
#include "HazelcastInstanceFactory.h"
#include "hazelcast/util/CountDownLatch.h"

namespace hazelcast {
    namespace client {
        namespace test {
            using namespace iTest;

            ClientSetTest::ClientSetTest(HazelcastInstanceFactory& hazelcastInstanceFactory)
            :hazelcastInstanceFactory(hazelcastInstanceFactory)
            , instance(hazelcastInstanceFactory)
            , client(new HazelcastClient(clientConfig.addAddress(Address(HOST, 5701))))
            , set(new ISet<std::string >(client->getSet< std::string >("ClientSetTest"))) {
            };


            ClientSetTest::~ClientSetTest() {
            };

            void ClientSetTest::addTests() {
                addTest(&ClientSetTest::testAddAll, "testAddAll");
                addTest(&ClientSetTest::testAddRemove, "testAddRemove");
                addTest(&ClientSetTest::testContains, "testContains");
                addTest(&ClientSetTest::testRemoveRetainAll, "testRemoveRetainAll");
                addTest(&ClientSetTest::testListener, "testListener");

            };

            void ClientSetTest::beforeClass() {
            };

            void ClientSetTest::afterClass() {
                client.reset();
                instance.shutdown();
            };

            void ClientSetTest::beforeTest() {
            };

            void ClientSetTest::afterTest() {
                set->clear();
            };

            void ClientSetTest::testAddAll() {
                std::vector<std::string> l;
                l.push_back("item1");
                l.push_back("item2");

                assertTrue(set->addAll(l));
                assertEqual(2, set->size());

                assertFalse(set->addAll(l));
                assertEqual(2, set->size());

            }

            void ClientSetTest::testAddRemove() {
                assertTrue(set->add("item1"));
                assertTrue(set->add("item2"));
                assertTrue(set->add("item3"));
                assertEqual(3, set->size());

                assertFalse(set->add("item3"));
                assertEqual(3, set->size());


                assertFalse(set->remove("item4"));
                assertTrue(set->remove("item3"));

            }

            void ClientSetTest::testContains() {
                assertTrue(set->add("item1"));
                assertTrue(set->add("item2"));
                assertTrue(set->add("item3"));
                assertTrue(set->add("item4"));

                assertFalse(set->contains("item5"));
                assertTrue(set->contains("item2"));

                std::vector<std::string> l;
                l.push_back("item6");
                l.push_back("item3");

                assertFalse(set->containsAll(l));
                assertTrue(set->add("item6"));
                assertTrue(set->containsAll(l));
            }


            void ClientSetTest::testRemoveRetainAll() {
                assertTrue(set->add("item1"));
                assertTrue(set->add("item2"));
                assertTrue(set->add("item3"));
                assertTrue(set->add("item4"));

                std::vector<std::string> l;
                l.push_back("item4");
                l.push_back("item3");

                assertTrue(set->removeAll(l));
                assertEqual(2, set->size());
                assertFalse(set->removeAll(l));
                assertEqual(2, set->size());

                l.clear();
                l.push_back("item1");
                l.push_back("item2");
                assertFalse(set->retainAll(l));
                assertEqual(2, set->size());

                l.clear();
                assertTrue(set->retainAll(l));
                assertEqual(0, set->size());

            }

            class MySetItemListener {
            public:
                MySetItemListener(util::CountDownLatch& latch)
                :latch(latch) {

                }

                void itemAdded(impl::ItemEvent<std::string> itemEvent) {
                    latch.countDown();
                }

                void itemRemoved(impl::ItemEvent<std::string> item) {
                }

            private:
                util::CountDownLatch& latch;
            };

            void listenerTestThread(ISet<std::string> *set) {
                for (int i = 0; i < 5; i++) {
                    set->add(std::string("item") + util::to_string(i));
                }
                set->add("done");
            }

            void ClientSetTest::testListener() {
//                util::CountDownLatch latch(6);
//
//                MySetItemListener listener(latch);
//                long registrationId = set->addItemListener(listener, true);
//                boost::thread t(listenerTestThread, set.get());
//                assertTrue(latch.await(20 * 1000));
//
//                set->removeItemListener(registrationId);
            }

        }
    }
}