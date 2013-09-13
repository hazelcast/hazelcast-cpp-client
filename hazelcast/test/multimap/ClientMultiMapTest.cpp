//
// Created by sancar koyunlu on 8/27/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "ClientMultiMapTest.h"
#include "HazelcastClient.h"
#include "HazelcastInstanceFactory.h"
#include "CountDownLatch.h"


namespace hazelcast {
    namespace client {
        namespace test {
            using namespace iTest;

            ClientMultiMapTest::ClientMultiMapTest(HazelcastInstanceFactory& hazelcastInstanceFactory)
            :hazelcastInstanceFactory(hazelcastInstanceFactory)
            , instance(hazelcastInstanceFactory)
            , client(new HazelcastClient(clientConfig.addAddress(Address("localhost", 5701))))
            , mm(new MultiMap<std::string, std::string>(client->getMultiMap< std::string, std::string >("ClientMultiMapTest"))) {
            };


            ClientMultiMapTest::~ClientMultiMapTest() {
            };

            void ClientMultiMapTest::addTests() {
                addTest(&ClientMultiMapTest::testPutGetRemove, "testPutGetRemove");
                addTest(&ClientMultiMapTest::testKeySetEntrySetAndValues, "testKeySetEntrySetAndValues");
                addTest(&ClientMultiMapTest::testContains, "testContains");
                addTest(&ClientMultiMapTest::testListener, "testListener");
                addTest(&ClientMultiMapTest::testLock, "testLock");
                addTest(&ClientMultiMapTest::testLockTtl, "testLockTtl");
                addTest(&ClientMultiMapTest::testTryLock, "testTryLock");
                addTest(&ClientMultiMapTest::testForceUnlock, "testForceUnlock");

            };

            void ClientMultiMapTest::beforeClass() {
            };

            void ClientMultiMapTest::afterClass() {
                client.reset();
                instance.shutdown();
            };

            void ClientMultiMapTest::beforeTest() {
                mm->clear();
            };

            void ClientMultiMapTest::afterTest() {
                mm->clear();
            };

            void ClientMultiMapTest::testPutGetRemove() {
                assertTrue(mm->put("key1", "value1"));
                assertTrue(mm->put("key1", "value2"));
                assertTrue(mm->put("key1", "value3"));

                assertTrue(mm->put("key2", "value4"));
                assertTrue(mm->put("key2", "value5"));

                assertEqual(3, mm->valueCount("key1"));
                assertEqual(2, mm->valueCount("key2"));
                assertEqual(5, mm->size());

                std::vector<string> coll = mm->get("key1");
                assertEqual(3, coll.size());

                coll = mm->remove("key2");
                assertEqual(2, coll.size());
                assertEqual(0, mm->valueCount("key2"));
                assertEqual(0, mm->get("key2").size());

                assertFalse(mm->remove("key1", "value4"));
                assertEqual(3, mm->size());

                assertTrue(mm->remove("key1", "value2"));
                assertEqual(2, mm->size());

                assertTrue(mm->remove("key1", "value1"));
                assertEqual(1, mm->size());
                assertEqual("value3", mm->get("key1")[0]);
            }


            void ClientMultiMapTest::testKeySetEntrySetAndValues() {
                assertTrue(mm->put("key1", "value1"));
                assertTrue(mm->put("key1", "value2"));
                assertTrue(mm->put("key1", "value3"));

                assertTrue(mm->put("key2", "value4"));
                assertTrue(mm->put("key2", "value5"));


                assertEqual(2, mm->keySet().size());
                assertEqual(5, mm->values().size());
                assertEqual(5, mm->entrySet().size());
            }


            void ClientMultiMapTest::testContains() {
                assertTrue(mm->put("key1", "value1"));
                assertTrue(mm->put("key1", "value2"));
                assertTrue(mm->put("key1", "value3"));

                assertTrue(mm->put("key2", "value4"));
                assertTrue(mm->put("key2", "value5"));

                assertFalse(mm->containsKey("key3"));
                assertTrue(mm->containsKey("key1"));

                assertFalse(mm->containsValue("value6"));
                assertTrue(mm->containsValue("value4"));

                assertFalse(mm->containsEntry("key1", "value4"));
                assertFalse(mm->containsEntry("key2", "value3"));
                assertTrue(mm->containsEntry("key1", "value1"));
                assertTrue(mm->containsEntry("key2", "value5"));
            }

            class MyMultiMapListener {
            public:
                MyMultiMapListener(util::CountDownLatch& addedLatch, util::CountDownLatch& removedLatch)
                :addedLatch(addedLatch)
                , removedLatch(removedLatch) {
                };

                void entryAdded(impl::EntryEvent<std::string, std::string>& event) {
                    addedLatch.countDown();
                };

                void entryRemoved(impl::EntryEvent<std::string, std::string>& event) {
                    removedLatch.countDown();
                }

                void entryUpdated(impl::EntryEvent<std::string, std::string>& event) {
                }

                void entryEvicted(impl::EntryEvent<std::string, std::string>& event) {
                }

            private:
                util::CountDownLatch& addedLatch;
                util::CountDownLatch& removedLatch;
            };

            void ClientMultiMapTest::testListener() {
                util::CountDownLatch latch1Add(8);
                util::CountDownLatch latch1Remove(4);

                util::CountDownLatch latch2Add(3);
                util::CountDownLatch latch2Remove(3);

                MyMultiMapListener listener1(latch1Add, latch1Remove);
                MyMultiMapListener listener2(latch2Add, latch2Remove);

                long id1 = mm->addEntryListener(listener1, true);
                long id2 = mm->addEntryListener(listener2, "key3", true);

                mm->put("key1", "value1");
                mm->put("key1", "value2");
                mm->put("key1", "value3");
                mm->put("key2", "value4");
                mm->put("key2", "value5");

                mm->remove("key1", "value2");

                mm->put("key3", "value6");
                mm->put("key3", "value7");
                mm->put("key3", "value8");

                mm->remove("key3");

                assertTrue(latch1Add.await(20 * 1000), "a");
                assertTrue(latch1Remove.await(20 * 1000), "b");

                assertTrue(latch2Add.await(20 * 1000), "c");
                assertTrue(latch2Remove.await(20 * 1000), "d");

                mm->removeEntryListener(id1);
                mm->removeEntryListener(id2);

            }

            void lockThread(MultiMap<std::string, std::string> *mm, util::CountDownLatch *latch) {
                if (!mm->tryLock("key1")) {
                    latch->countDown();
                }
            }

            void ClientMultiMapTest::testLock() {
                mm->lock("key1");
                util::CountDownLatch latch(1);
                boost::thread t(boost::bind(lockThread, mm.get(), &latch));
                assertTrue(latch.await(5 * 1000));
                mm->forceUnlock("key1");
            }

            void lockTtlThread(MultiMap<std::string, std::string> *mm, util::CountDownLatch *latch) {
                if (!mm->tryLock("key1")) {
                    latch->countDown();
                }
                try {
                    if (mm->tryLock("key1", 5 * 1000)) {
                        latch->countDown();
                    }
                } catch (...) {
                    std::cerr << "Unexpected exception at ClientMultiMapTest lockThread2" << std::endl;
                }
            }

            void ClientMultiMapTest::testLockTtl() {
                mm->lock("key1", 3 * 1000);
                util::CountDownLatch latch(2);
                boost::thread t(lockTtlThread, mm.get(), &latch);
                assertTrue(latch.await(10 * 1000));
                mm->forceUnlock("key1");
            }


            void tryLockThread(MultiMap<std::string, std::string> *mm, util::CountDownLatch *latch) {
                try {
                    if (!mm->tryLock("key1", 2 * 1000)) {
                        latch->countDown();
                    }
                }catch (...) {
                    std::cerr << "Unexpected exception at ClientMultiMapTest tryLockThread" <<
                            std::endl;
                }
            }

            void tryLockThread2(MultiMap<std::string, std::string> *mm, util::CountDownLatch *latch) {
                try {
                    if (mm->tryLock("key1", 20 * 1000)) {
                        latch->countDown();
                    }
                } catch (...) {
                    std::cerr << "Unexpected exception at ClientMultiMapTest lockThread2" << std::endl;
                }
            }

            void ClientMultiMapTest::testTryLock() {
                assertTrue(mm->tryLock("key1", 2 * 1000));
                util::CountDownLatch latch(1);
                boost::thread t(boost::bind(tryLockThread, mm.get(), &latch));
                assertTrue(latch.await(100 * 1000));
                assertTrue(mm->isLocked("key1"));

                util::CountDownLatch latch2(1);
                boost::thread t2(boost::bind(tryLockThread2, mm.get(), &latch2));

                boost::this_thread::sleep(boost::posix_time::seconds(1));
                mm->unlock("key1");
                assertTrue(latch2.await(100 * 1000));
                assertTrue(mm->isLocked("key1"));
                mm->forceUnlock("key1");
            }

            void forceUnlockThread(MultiMap<std::string, std::string> *mm, util::CountDownLatch *latch) {
                mm->forceUnlock("key1");
                latch->countDown();
            }

            void ClientMultiMapTest::testForceUnlock() {
                mm->lock("key1");
                util::CountDownLatch latch(1);
                boost::thread t(boost::bind(forceUnlockThread, mm.get(), &latch));
                assertTrue(latch.await(100 * 1000));
                assertFalse(mm->isLocked("key1"));
            }
        }
    }
}
