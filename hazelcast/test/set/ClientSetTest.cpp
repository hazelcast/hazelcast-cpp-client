//
// Created by sancar koyunlu on 9/13/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "ClientSetTest.h"
#include "hazelcast/client/HazelcastClient.h"
#include "HazelcastServerFactory.h"

namespace hazelcast {
    namespace client {
        namespace test {
            using namespace iTest;

            ClientSetTest::ClientSetTest(HazelcastServerFactory &serverFactory)
            : iTestFixture<ClientSetTest>("ClientSetTest")
            , instance(serverFactory)
            , client(new HazelcastClient(clientConfig.addAddress(Address(serverFactory.getServerAddress(), 5701))))
            , set(new ISet<std::string >(client->getSet< std::string >("ClientSetTest"))) {
            }


            ClientSetTest::~ClientSetTest() {
            }

            void ClientSetTest::addTests() {
                addTest(&ClientSetTest::testAddAll, "testAddAll");
                addTest(&ClientSetTest::testAddRemove, "testAddRemove");
                addTest(&ClientSetTest::testContains, "testContains");
                addTest(&ClientSetTest::testRemoveRetainAll, "testRemoveRetainAll");
                addTest(&ClientSetTest::testListener, "testListener");

            }

            void ClientSetTest::beforeClass() {
            }

            void ClientSetTest::afterClass() {
                client.reset();
                instance.shutdown();
            }

            void ClientSetTest::beforeTest() {
            }

            void ClientSetTest::afterTest() {
                set->clear();
            }

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

            class MySetItemListener : public ItemListener<std::string> {
            public:
                MySetItemListener(hazelcast::util::CountDownLatch &latch)
                :latch(latch) {

                }

                void itemAdded(const ItemEvent<std::string>& itemEvent) {
                    latch.countDown();
                }

                void itemRemoved(const ItemEvent<std::string>& item) {
                }

            private:
                hazelcast::util::CountDownLatch &latch;
            };


            void ClientSetTest::testListener() {
                hazelcast::util::CountDownLatch latch(6);

                MySetItemListener listener(latch);
                std::string registrationId = set->addItemListener(listener, true);

                for (int i = 0; i < 5; i++) {
                    set->add(std::string("item") + hazelcast::util::IOUtil::to_string(i));
                }
                set->add("done");

                assertTrue(latch.await(20 ));

                assertTrue(set->removeItemListener(registrationId));
            }

        }
    }
}

