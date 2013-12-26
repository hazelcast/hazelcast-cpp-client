//
// Created by sancar koyunlu on 9/13/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "list/ClientListTest.h"
#include "hazelcast/client/HazelcastClient.h"
#include "HazelcastInstanceFactory.h"
#include "hazelcast/util/CountDownLatch.h"

namespace hazelcast {
    namespace client {
        namespace test {
            using namespace iTest;

            ClientListTest::ClientListTest(HazelcastInstanceFactory &hazelcastInstanceFactory)
            :hazelcastInstanceFactory(hazelcastInstanceFactory)
            , instance(hazelcastInstanceFactory)
            , client(new HazelcastClient(clientConfig.addAddress(Address(HOST, 5701))))
            , list(new IList<std::string>(client->getList< std::string >("ClientListTest"))) {
            };


            ClientListTest::~ClientListTest() {
            };

            void ClientListTest::addTests() {
                addTest(&ClientListTest::testAddAll, "testAddAll");
                addTest(&ClientListTest::testAddSetRemove, "testAddSetRemove");
                addTest(&ClientListTest::testIndexOf, "testIndexOf");
                addTest(&ClientListTest::testToArray, "testToArray");
                addTest(&ClientListTest::testContains, "testContains");
                addTest(&ClientListTest::testRemoveRetainAll, "removeRetainAll");
                addTest(&ClientListTest::testListener, "testListener");

            };

            void ClientListTest::beforeClass() {
            };

            void ClientListTest::afterClass() {
                client.reset();
                instance.shutdown();
            };

            void ClientListTest::beforeTest() {

            };

            void ClientListTest::afterTest() {
                list->clear();
            };

            void ClientListTest::testAddAll() {

                std::vector<std::string> l;
                l.push_back("item1");
                l.push_back("item2");
                assertTrue(list->addAll(l));

                assertTrue(list->addAll(1, l));
                assertEqual(4, list->size());

                assertEqual("item1", *(list->get(0)));
                assertEqual("item1", *(list->get(1)));
                assertEqual("item2", *(list->get(2)));
                assertEqual("item2", *(list->get(3)));
            }

            void ClientListTest::testAddSetRemove() {
                assertTrue(list->add("item1"));
                assertTrue(list->add("item2"));
                list->add(0, "item3");
                assertEqual(3, list->size());
                boost::shared_ptr<std::string> temp = list->set(2, "item4");
                assertEqual("item2", *temp);

                assertEqual(3, list->size());
                assertEqual("item3", *(list->get(0)));
                assertEqual("item1", *(list->get(1)));
                assertEqual("item4", *(list->get(2)));

                assertFalse(list->remove("item2"));
                assertTrue(list->remove("item3"));

                temp = list->remove(1);
                assertEqual("item4", *temp);

                assertEqual(1, list->size());
                assertEqual("item1", *(list->get(0)));
            }

            void ClientListTest::testIndexOf() {
                assertTrue(list->add("item1"));
                assertTrue(list->add("item2"));
                assertTrue(list->add("item1"));
                assertTrue(list->add("item4"));

                assertEqual(-1, list->indexOf("item5"));
                assertEqual(0, list->indexOf("item1"));

                assertEqual(-1, list->lastIndexOf("item6"));
                assertEqual(2, list->lastIndexOf("item1"));
            }

            void ClientListTest::testToArray() {
                assertTrue(list->add("item1"));
                assertTrue(list->add("item2"));
                assertTrue(list->add("item1"));
                assertTrue(list->add("item4"));

                std::vector<std::string> ar = list->toArray();

                assertEqual("item1", ar[0]);
                assertEqual("item2", ar[1]);
                assertEqual("item1", ar[2]);
                assertEqual("item4", ar[3]);

                std::vector<string> arr2 = list->subList(1, 3);

                assertEqual(2, arr2.size());
                assertEqual("item2", arr2[0]);
                assertEqual("item1", arr2[1]);
            }

            void ClientListTest::testContains() {
                assertTrue(list->add("item1"));
                assertTrue(list->add("item2"));
                assertTrue(list->add("item1"));
                assertTrue(list->add("item4"));

                assertFalse(list->contains("item3"));
                assertTrue(list->contains("item2"));

                std::vector<std::string> l;
                l.push_back("item4");
                l.push_back("item3");

                assertFalse(list->containsAll(l));
                assertTrue(list->add("item3"));
                assertTrue(list->containsAll(l));
            }

            void ClientListTest::testRemoveRetainAll() {
                assertTrue(list->add("item1"));
                assertTrue(list->add("item2"));
                assertTrue(list->add("item1"));
                assertTrue(list->add("item4"));

                std::vector<std::string> l;
                l.push_back("item4");
                l.push_back("item3");

                assertTrue(list->removeAll(l));
                assertEqual(3, list->size());
                assertFalse(list->removeAll(l));
                assertEqual(3, list->size());

                l.clear();
                l.push_back("item1");
                l.push_back("item2");
                assertFalse(list->retainAll(l));
                assertEqual(3, list->size());

                l.clear();
                assertTrue(list->retainAll(l));
                assertEqual(0, list->size());

            }

            class MyListItemListener {
            public:
                MyListItemListener(util::CountDownLatch &latch)
                :latch(latch) {

                }

                void itemAdded(impl::ItemEvent<std::string> itemEvent) {
                    latch.countDown();
                }

                void itemRemoved(impl::ItemEvent<std::string> item) {
                }

            private:
                util::CountDownLatch &latch;
            };

            void listenerTestThread(IList<std::string> *list) {
                try {
                    for (int i = 0; i < 5; i++) {
                        list->add(std::string("item") + util::to_string(i));
                    }
                    list->add("done");
                } catch(std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    std::cerr.flush();
                } catch(...) {
                    std::cerr << "unexpected error at ClientListTest::listenerTestThread" << std::endl;
                }
            }

            void ClientListTest::testListener() {
//                util::CountDownLatch latch(6);
//
//                MyListItemListener listener(latch);
//                long registrationId = list->addItemListener(listener, true);
//                boost::thread t(listenerTestThread, list.get());
//                assertTrue(latch.await(20 * 1000));
//
//                list->removeItemListener(registrationId);
            }

        }
    }
}
