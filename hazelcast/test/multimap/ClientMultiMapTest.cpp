//
// Created by sancar koyunlu on 8/27/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "multimap/ClientMultiMapTest.h"
#include "hazelcast/client/HazelcastClient.h"
#include "HazelcastServerFactory.h"

namespace hazelcast {
    namespace client {
        namespace test {
            using namespace iTest;

            ClientMultiMapTest::ClientMultiMapTest(HazelcastServerFactory& hazelcastInstanceFactory)
            : iTestFixture<ClientMultiMapTest>("ClientMultiMapTest")
            , instance(hazelcastInstanceFactory)
            , client(new HazelcastClient(clientConfig.addAddress(Address(HOST, 5701))))
            , mm(new MultiMap<std::string, std::string>(client->getMultiMap<std::string, std::string>("ClientMultiMapTest"))) {
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

                std::vector<std::string> coll = mm->get("key1");
                assertEqual(3, (int)coll.size());

                coll = mm->remove("key2");
                assertEqual(2, (int)coll.size());
                assertEqual(0, mm->valueCount("key2"));
                assertEqual(0, (int)mm->get("key2").size());

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


                assertEqual(2, (int)mm->keySet().size());
                assertEqual(5, (int)mm->values().size());
                assertEqual(5, (int)mm->entrySet().size());
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
                : addedLatch(addedLatch), removedLatch(removedLatch) {
                };

                void entryAdded(EntryEvent<std::string, std::string>& event) {
                    addedLatch.countDown();
                };

                void entryRemoved(EntryEvent<std::string, std::string>& event) {
                    removedLatch.countDown();
                }

                void entryUpdated(EntryEvent<std::string, std::string>& event) {
                }

                void entryEvicted(EntryEvent<std::string, std::string>& event) {
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

                std::string id1 = mm->addEntryListener(listener1, true);
                std::string id2 = mm->addEntryListener(listener2, "key3", true);

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

                assertTrue(latch1Add.await(20), "a");
                assertTrue(latch1Remove.await(20), "b");

                assertTrue(latch2Add.await(20), "c");
                assertTrue(latch2Remove.await(20), "d");

                assertTrue(mm->removeEntryListener(id1));
                assertTrue(mm->removeEntryListener(id2));

            }

            void lockThread(util::ThreadArgs& args) {
                MultiMap<std::string, std::string> *mm = (MultiMap<std::string, std::string> *)args.arg0;
                util::CountDownLatch *latch = (util::CountDownLatch *)args.arg1;
                if (!mm->tryLock("key1")) {
                    latch->countDown();
                }
            }

            void ClientMultiMapTest::testLock() {
                mm->lock("key1");
                util::CountDownLatch latch(1);
                util::Thread t(lockThread, mm.get(), &latch);
                assertTrue(latch.await(5));
                mm->forceUnlock("key1");
            }

            void lockTtlThread(util::ThreadArgs& args) {
                MultiMap<std::string, std::string> *mm = (MultiMap<std::string, std::string> *)args.arg0;
                util::CountDownLatch *latch = (util::CountDownLatch *)args.arg1;

                if (!mm->tryLock("key1")) {
                    latch->countDown();
                }

                if (mm->tryLock("key1", 5 * 1000)) {
                    latch->countDown();
                }
            }

            void ClientMultiMapTest::testLockTtl() {
                mm->lock("key1", 3 * 1000);
                util::CountDownLatch latch(2);
                util::Thread t(lockTtlThread, mm.get(), &latch);
                assertTrue(latch.await(10));
                mm->forceUnlock("key1");
            }


            void tryLockThread(util::ThreadArgs& args) {
                MultiMap<std::string, std::string> *mm = (MultiMap<std::string, std::string> *)args.arg0;
                util::CountDownLatch *latch = (util::CountDownLatch *)args.arg1;
                try {
                    if (!mm->tryLock("key1", 2)) {
                        latch->countDown();
                    }
                } catch (...) {
                    std::cerr << "Unexpected exception at ClientMultiMapTest tryLockThread" << std::endl;
                }
            }

            void tryLockThread2(util::ThreadArgs& args) {
                MultiMap<std::string, std::string> *mm = (MultiMap<std::string, std::string> *)args.arg0;
                util::CountDownLatch *latch = (util::CountDownLatch *)args.arg1;
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
                util::Thread t(tryLockThread, mm.get(), &latch);
                assertTrue(latch.await(100));
                assertTrue(mm->isLocked("key1"));

                util::CountDownLatch latch2(1);
                util::Thread t2(tryLockThread2, mm.get(), &latch2);

                util::sleep(1);
                mm->unlock("key1");
                assertTrue(latch2.await(100));
                assertTrue(mm->isLocked("key1"));
                mm->forceUnlock("key1");
            }

            void forceUnlockThread(util::ThreadArgs& args) {
                MultiMap<std::string, std::string> *mm = (MultiMap<std::string, std::string> *)args.arg0;
                util::CountDownLatch *latch = (util::CountDownLatch *)args.arg1;
                mm->forceUnlock("key1");
                latch->countDown();
            }

            void ClientMultiMapTest::testForceUnlock() {
                mm->lock("key1");
                util::CountDownLatch latch(1);
                util::Thread t(forceUnlockThread, mm.get(), &latch);
                assertTrue(latch.await(100));
                assertFalse(mm->isLocked("key1"));
            }
        }
    }
}

