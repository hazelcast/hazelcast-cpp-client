//
// Created by sancar koyunlu on 8/27/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "map/ClientMapTest.h"
#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/client/EntryAdapter.h"
#include "HazelcastServerFactory.h"
#include "serialization/Employee.h"
#include "TestHelperFunctions.h"


namespace hazelcast {
    namespace client {
        namespace test {
            using namespace iTest;

            ClientMapTest::ClientMapTest(HazelcastServerFactory& hazelcastInstanceFactory)
            : iTestFixture<ClientMapTest>("ClientMapTest")
            , instance(hazelcastInstanceFactory)
            , instance2(hazelcastInstanceFactory)
            , client(new HazelcastClient(clientConfig.addAddress(Address(HOST, 5701))))
            , imap(new IMap<string, string>(client->getMap<string, string>("clientMapTest"))) {
            }


            ClientMapTest::~ClientMapTest() {
            }

            void ClientMapTest::addTests() {
                addTest(&ClientMapTest::testContains, "testContains");
                addTest(&ClientMapTest::testGet, "testGet");
                addTest(&ClientMapTest::testRemoveAndDelete, "testRemoveAndDelete");
                addTest(&ClientMapTest::testRemoveIfSame, "testRemoveIfSame");
                addTest(&ClientMapTest::testGetAllPutAll, "testGetAllPutAll");
                addTest(&ClientMapTest::testTryPutRemove, "testTryPutRemove");
                addTest(&ClientMapTest::testPutTtl, "testPutTtl");
                addTest(&ClientMapTest::testPutIfAbsent, "testPutIfAbsent");
                addTest(&ClientMapTest::testPutIfAbsentTtl, "testPutIfAbsentTtl");
                addTest(&ClientMapTest::testSet, "testSet");
                addTest(&ClientMapTest::testLock, "testLock");
                addTest(&ClientMapTest::testLockTtl, "testLockTtl");
                addTest(&ClientMapTest::testLockTtl2, "testLockTtl2");
                addTest(&ClientMapTest::testTryLock, "testTryLock");
                addTest(&ClientMapTest::testForceUnlock, "testForceUnlock");
                addTest(&ClientMapTest::testValues, "testValues");
                addTest(&ClientMapTest::testReplace, "testReplace");
                addTest(&ClientMapTest::testPredicateListenerWithPortableKey, "testPredicateListenerWithPortableKey");
                addTest(&ClientMapTest::testListener, "testListener");
                addTest(&ClientMapTest::testEvictAllEvent, "testEvictAllEvent");
                addTest(&ClientMapTest::testClearEvent, "testClearEvent");
                addTest(&ClientMapTest::testBasicPredicate, "testBasicPredicate");
                addTest(&ClientMapTest::testIssue537, "testIssue537");
                addTest(&ClientMapTest::testMapWithPortable, "testMapWithPortable");
                addTest(&ClientMapTest::testMapStoreRelatedRequests, "testMapStoreRelatedRequests");
                addTest(&ClientMapTest::testKeySetAndValuesWithPredicates, "testKeySetAndValuesWithPredicates");
            }

            void ClientMapTest::beforeClass() {
            }

            void ClientMapTest::afterClass() {
                client.reset();
                instance.shutdown();
                instance2.shutdown();
            }

            void ClientMapTest::beforeTest() {
                imap->clear();
            }

            void ClientMapTest::afterTest() {
                imap->clear();
            }

            void ClientMapTest::fillMap() {
                for (int i = 0; i < 10; i++) {
                    string key = "key";
                    key += util::IOUtil::to_string(i);
                    string value = "value";
                    value += util::IOUtil::to_string(i);
                    imap->put(key, value);
                }
            }

            class SampleEntryListener : public EntryAdapter<std::string, std::string> {
            public:
                SampleEntryListener(util::CountDownLatch& addLatch, util::CountDownLatch& removeLatch, util::CountDownLatch& updateLatch, util::CountDownLatch& evictLatch)
                : addLatch(addLatch)
                , removeLatch(removeLatch)
                , updateLatch(updateLatch)
                , evictLatch(evictLatch) {
                }

                void entryAdded(const EntryEvent<std::string, std::string>& event) {
                    addLatch.countDown();
                }

                void entryRemoved(const EntryEvent<std::string, std::string>& event) {
                    removeLatch.countDown();
                }

                void entryUpdated(const EntryEvent<std::string, std::string>& event) {
                    updateLatch.countDown();
                }

                void entryEvicted(const EntryEvent<std::string, std::string>& event) {
                    evictLatch.countDown();
                }

            private:
                util::CountDownLatch& addLatch;
                util::CountDownLatch& removeLatch;
                util::CountDownLatch& updateLatch;
                util::CountDownLatch& evictLatch;
            };

            class MyListener : public EntryAdapter<std::string, std::string> {
            public:
                MyListener(util::CountDownLatch& latch, util::CountDownLatch& nullLatch)
                : latch(latch), nullLatch(nullLatch) {
                }

                void entryAdded(const EntryEvent<std::string, std::string>& event) {
                    latch.countDown();
                }

                void entryEvicted(const EntryEvent<std::string, std::string>& event) {

                    const string& oldValue = event.getOldValue();

                    if (oldValue.compare("")) {
                        nullLatch.countDown();
                    }

                    latch.countDown();
                }

            private:
                util::CountDownLatch& latch;
                util::CountDownLatch& nullLatch;
            };


            class ClearListener : public EntryAdapter<std::string, std::string> {
            public:
                ClearListener(util::CountDownLatch& latch) : latch(latch) {

                }

                void mapCleared(const IMapEvent& event) {
                    latch.countDown();
                }

            private:
                util::CountDownLatch& latch;
            };

            class EvictListener : public EntryAdapter<std::string, std::string> {
            public:
                EvictListener(util::CountDownLatch& latch) : latch(latch) {

                }

                void mapEvicted(const IMapEvent& event) {
                    latch.countDown();
                }

            private:
                util::CountDownLatch& latch;
            };

            void ClientMapTest::testIssue537() {
                util::CountDownLatch latch(2);
                util::CountDownLatch nullLatch(1);
                MyListener myListener(latch, nullLatch);
                std::string id = imap->addEntryListener(myListener, true);

                imap->put("key1", "value1", 2 * 1000);

                assertTrue(latch.await(10));
                assertTrue(nullLatch.await(1));

                assertTrue(imap->removeEntryListener(id));

                imap->put("key2", "value2");
                assertEqual(1, imap->size());
            }

            void ClientMapTest::testContains() {
                fillMap();

                assertFalse(imap->containsKey("key10"));
                assertTrue(imap->containsKey("key1"));

                assertFalse(imap->containsValue("value10"));
                assertTrue(imap->containsValue("value1"));

            }

            void ClientMapTest::testGet() {
                fillMap();
                for (int i = 0; i < 10; i++) {
                    string key = "key";
                    key += util::IOUtil::to_string(i);
                    boost::shared_ptr<string> temp = imap->get(key);

                    string value = "value";
                    value += util::IOUtil::to_string(i);
                    assertEqual(*temp, value);
                }
            }

            void ClientMapTest::testRemoveAndDelete() {
                fillMap();
                boost::shared_ptr<string> temp = imap->remove("key10");
                assertNull(temp.get());
                imap->deleteEntry("key9");
                assertEqual(imap->size(), 9);
                for (int i = 0; i < 9; i++) {
                    string key = "key";
                    key += util::IOUtil::to_string(i);
                    boost::shared_ptr<string> temp2 = imap->remove(key);
                    string value = "value";
                    value += util::IOUtil::to_string(i);
                    assertEqual(*temp2, value);
                }
                assertEqual(imap->size(), 0);
            }

            void ClientMapTest::testRemoveIfSame() {
                fillMap();

                assertFalse(imap->remove("key2", "value"));
                assertEqual(10, imap->size());

                assertTrue((imap->remove("key2", "value2")));
                assertEqual(9, imap->size());

            }

            void ClientMapTest::testGetAllPutAll() {

                std::map<std::string, std::string> mapTemp;

                for (int i = 0; i < 100; i++) {
                    mapTemp[util::IOUtil::to_string(i)] = util::IOUtil::to_string(i);
                }
                imap->putAll(mapTemp);
                assertEqual(imap->size(), 100);

                for (int i = 0; i < 100; i++) {
                    string expected = util::IOUtil::to_string(i);
                    boost::shared_ptr<string> actual = imap->get(util::IOUtil::to_string(i));
                    assertEqual(expected, *actual);
                }

                std::set<std::string> tempSet;
                tempSet.insert(util::IOUtil::to_string(1));
                tempSet.insert(util::IOUtil::to_string(3));

                std::map<std::string, std::string> m2 = imap->getAll(tempSet);

                assertEqual(2U, m2.size());
                assertEqual(m2[util::IOUtil::to_string(1)], "1");
                assertEqual(m2[util::IOUtil::to_string(3)], "3");

            }

            void tryPutThread(util::ThreadArgs& args) {
                util::CountDownLatch *latch = (util::CountDownLatch *)args.arg0;
                IMap<std::string, std::string> *imap = (IMap<std::string, std::string> *)args.arg1;
                bool result = imap->tryPut("key1", "value3", 1 * 1000);
                if (!result) {
                    latch->countDown();
                }
            }

            void tryRemoveThread(util::ThreadArgs& args) {
                util::CountDownLatch *latch = (util::CountDownLatch *)args.arg0;
                IMap<std::string, std::string> *imap = (IMap<std::string, std::string> *)args.arg1;
                bool result = imap->tryRemove("key2", 1 * 1000);
                if (!result) {
                    latch->countDown();
                }
            }

            void ClientMapTest::testTryPutRemove() {

                assertTrue(imap->tryPut("key1", "value1", 1 * 1000));
                assertTrue(imap->tryPut("key2", "value2", 1 * 1000));
                imap->lock("key1");
                imap->lock("key2");

                util::CountDownLatch latch(2);

                util::Thread t1(tryPutThread, &latch, imap.get());
                util::Thread t2(tryRemoveThread, &latch, imap.get());

                assertTrue(latch.await(20));
                assertEqual("value1", *(imap->get("key1")));
                assertEqual("value2", *(imap->get("key2")));
                imap->forceUnlock("key1");
                imap->forceUnlock("key2");
            }

            void ClientMapTest::testPutTtl() {
                util::CountDownLatch dummy(10);
                util::CountDownLatch evict(1);
                SampleEntryListener sampleEntryListener(dummy, dummy, dummy, evict);
                std::string id = imap->addEntryListener(sampleEntryListener, false);

                imap->put("key1", "value1", 2000);
                boost::shared_ptr<std::string> temp = imap->get("key1");
                assertEqual(*temp, "value1");
                assertTrue(evict.await(20 * 1000));
                boost::shared_ptr<std::string> temp2 = imap->get("key1");
                assertNull(temp2.get());

                assertTrue(imap->removeEntryListener(id));
            }

            void ClientMapTest::testPutIfAbsent() {
                boost::shared_ptr<std::string> o = imap->putIfAbsent("key1", "value1");
                assertNull(o.get());
                assertEqual("value1", *(imap->putIfAbsent("key1", "value3")));
            }

            void ClientMapTest::testPutIfAbsentTtl() {
                assertNull(imap->putIfAbsent("key1", "value1", 1000).get());
                assertEqual("value1", *(imap->putIfAbsent("key1", "value3", 1000)));
                ASSERT_EVENTUALLY(assertNull, imap->putIfAbsent("key1", "value3", 1000).get());
                assertEqual("value3", *(imap->putIfAbsent("key1", "value4", 1000)));
            }

            void ClientMapTest::testSet() {
                imap->set("key1", "value1");
                assertEqual("value1", *(imap->get("key1")));

                imap->set("key1", "value2");
                assertEqual("value2", *(imap->get("key1")));

                imap->set("key1", "value3", 1000);
                assertEqual("value3", *(imap->get("key1")));

                ASSERT_EVENTUALLY(assertNull, imap->get("key1").get());
            }

            void testLockThread(util::ThreadArgs& args) {
                util::CountDownLatch *latch = (util::CountDownLatch *)args.arg0;
                IMap<std::string, std::string> *imap = (IMap<std::string, std::string> *)args.arg1;
                imap->tryPut("key1", "value2", 1);
                latch->countDown();
            }

            void ClientMapTest::testLock() {
                imap->put("key1", "value1");
                assertEqual("value1", *(imap->get("key1")));
                imap->lock("key1");
                util::CountDownLatch latch(1);
                util::Thread t1(testLockThread, &latch, imap.get());
                assertTrue(latch.await(5));
                assertEqual("value1", *(imap->get("key1")));
                imap->forceUnlock("key1");

            }

            void testLockTTLThread(util::ThreadArgs& args) {
                util::CountDownLatch *latch = (util::CountDownLatch *)args.arg0;
                IMap<std::string, std::string> *imap = (IMap<std::string, std::string> *)args.arg1;
                imap->tryPut("key1", "value2", 5 * 1000);
                latch->countDown();
            }

            void ClientMapTest::testLockTtl() {
                imap->put("key1", "value1");
                assertEqual("value1", *(imap->get("key1")));
                imap->lock("key1", 2 * 1000);
                util::CountDownLatch latch(1);
                util::Thread t1(testLockTTLThread, &latch, imap.get());
                assertTrue(latch.await(10));
                assertFalse(imap->isLocked("key1"));
                assertEqual("value2", *(imap->get("key1")));
                imap->forceUnlock("key1");

            }

            void testLockTTL2Thread(util::ThreadArgs& args) {
                util::CountDownLatch *latch = (util::CountDownLatch *)args.arg0;
                IMap<std::string, std::string> *imap = (IMap<std::string, std::string> *)args.arg1;
                if (!imap->tryLock("key1")) {
                    latch->countDown();
                }
                if (imap->tryLock("key1", 5 * 1000)) {
                    latch->countDown();
                }
            }

            void ClientMapTest::testLockTtl2() {
                imap->lock("key1", 3 * 1000);
                util::CountDownLatch latch(2);
                util::Thread t1(testLockTTL2Thread, &latch, imap.get());
                assertTrue(latch.await(10));
                imap->forceUnlock("key1");

            }

            void testMapTryLockThread1(util::ThreadArgs& args) {
                util::CountDownLatch *latch = (util::CountDownLatch *)args.arg0;
                IMap<std::string, std::string> *imap = (IMap<std::string, std::string> *)args.arg1;
                if (!imap->tryLock("key1", 2)) {
                    latch->countDown();
                }
            }

            void testMapTryLockThread2(util::ThreadArgs& args) {
                util::CountDownLatch *latch = (util::CountDownLatch *)args.arg0;
                IMap<std::string, std::string> *imap = (IMap<std::string, std::string> *)args.arg1;
                if (imap->tryLock("key1", 20 * 1000)) {
                    latch->countDown();
                }
            }

            void ClientMapTest::testTryLock() {

                assertTrue(imap->tryLock("key1", 2 * 1000), "1");
                util::CountDownLatch latch(1);
                util::Thread t1(testMapTryLockThread1, &latch, imap.get());

                assertTrue(latch.await(100), "2");

                assertTrue(imap->isLocked("key1"), "3");

                util::CountDownLatch latch2(1);
                util::Thread t2(testMapTryLockThread2, &latch2, imap.get());

                util::sleep(1);
                imap->unlock("key1");
                assertTrue(latch2.await(100), "4");
                assertTrue(imap->isLocked("key1"), "5");
                imap->forceUnlock("key1");

            }

            void testMapForceUnlockThread(util::ThreadArgs& args) {
                util::CountDownLatch *latch = (util::CountDownLatch *)args.arg0;
                IMap<std::string, std::string> *imap = (IMap<std::string, std::string> *)args.arg1;
                imap->forceUnlock("key1");
                latch->countDown();
            }

            void ClientMapTest::testForceUnlock() {
                imap->lock("key1");
                util::CountDownLatch latch(1);
                util::Thread t2(testMapForceUnlockThread, &latch, imap.get());
                assertTrue(latch.await(100));
                t2.join();
                assertFalse(imap->isLocked("key1"));

            }

            void ClientMapTest::testValues() {

                fillMap();
                std::vector<std::string> tempVector;
                tempVector = imap->values("this == value1");
                assertEqual(1U, tempVector.size());

                std::vector<std::string>::iterator it = tempVector.begin();
                assertEqual("value1", *it);
            }

            void ClientMapTest::testReplace() {
                boost::shared_ptr<std::string> temp = imap->replace("key1", "value");
                assertNull(temp.get());

                std::string tempKey = "key1";
                std::string tempValue = "value1";
                imap->put(tempKey, tempValue);

                assertEqual("value1", *(imap->replace("key1", "value2")));
                assertEqual("value2", *(imap->get("key1")));

                assertEqual(false, imap->replace("key1", "value1", "value3"));
                assertEqual("value2", *(imap->get("key1")));

                assertEqual(true, imap->replace("key1", "value2", "value3"));
                assertEqual("value3", *(imap->get("key1")));
            }

            class SampleEntryListenerForPortableKey : public EntryAdapter<Employee, int> {
            public:
                SampleEntryListenerForPortableKey(util::CountDownLatch& latch, util::AtomicInt& atomicInteger)
                : latch(latch), atomicInteger(atomicInteger) {

                }

                void entryAdded(const EntryEvent<Employee, int>& event) {
                    ++atomicInteger;
                    latch.countDown();
                }

            private:
                util::CountDownLatch& latch;
                util::AtomicInt& atomicInteger;
            };


            void ClientMapTest::testPredicateListenerWithPortableKey() {
                IMap<Employee, int> tradeMap = client->getMap<Employee, int>("tradeMap");
                util::CountDownLatch countDownLatch(1);
                util::AtomicInt atomicInteger(0);
                SampleEntryListenerForPortableKey listener(countDownLatch, atomicInteger);
                Employee key("a", 1);
                std::string id = tradeMap.addEntryListener(listener, key, true);
                Employee key2("a", 2);
                tradeMap.put(key2, 1);
                tradeMap.put(key, 3);
                assertTrue(countDownLatch.await(5));
                assertEqual(1, (int)atomicInteger);

                assertTrue(tradeMap.removeEntryListener(id));
            }

            void ClientMapTest::testListener() {
                util::CountDownLatch latch1Add(5);
                util::CountDownLatch latch1Remove(2);
                util::CountDownLatch dummy(10);
                util::CountDownLatch latch2Add(1);
                util::CountDownLatch latch2Remove(1);

                SampleEntryListener listener1(latch1Add, latch1Remove, dummy, dummy);
                SampleEntryListener listener2(latch2Add, latch2Remove, dummy, dummy);

                std::string listener1ID = imap->addEntryListener(listener1, false);
                std::string listener2ID = imap->addEntryListener(listener2, "key3", true);

                util::sleep(2);

                imap->put("key1", "value1");
                imap->put("key2", "value2");
                imap->put("key3", "value3");
                imap->put("key4", "value4");
                imap->put("key5", "value5");

                imap->remove("key1");
                imap->remove("key3");

                assertTrue(latch1Add.await(10));
                assertTrue(latch1Remove.await(10));
                assertTrue(latch2Add.await(5));
                assertTrue(latch2Remove.await(5));

                assertTrue(imap->removeEntryListener(listener1ID));
                assertTrue(imap->removeEntryListener(listener2ID));
            }

            void ClientMapTest::testClearEvent() {
                util::CountDownLatch latch(1);
                ClearListener clearListener(latch);
                imap->addEntryListener(clearListener, false);
                imap->clear();
                assertTrue(latch.await(120));
            }

            void ClientMapTest::testEvictAllEvent() {
                util::CountDownLatch latch(1);
                EvictListener evictListener(latch);
                imap->addEntryListener(evictListener, false);
                imap->evictAll();
                assertTrue(latch.await(120));
            }


            void ClientMapTest::testBasicPredicate() {

                fillMap();
                std::vector<std::string> tempVector;
                tempVector = imap->values("this = 'value1'");

                assertEqual("value1", tempVector[0]);

                std::vector<std::string> tempVector2;
                tempVector2 = imap->keySet("this = 'value1'");

                std::vector<std::string>::iterator it2 = tempVector2.begin();
                assertEqual("key1", *it2);


                std::vector<std::pair<std::string, std::string> > tempVector3;
                tempVector3 = imap->entrySet("this == value1");

                std::vector<std::pair<std::string, std::string> >::iterator it3 = tempVector3.begin();
                assertEqual("key1", (*it3).first);
                assertEqual("value1", (*it3).second);

            }

            void ClientMapTest::testKeySetAndValuesWithPredicates() {
                std::string name = "testKeysetAndValuesWithPredicates";
                IMap<Employee, Employee> map = client->getMap<Employee, Employee>(name);

                Employee emp1("abc-123-xvz", 34);
                Employee emp2("abc-123-xvz", 20);

                map.put(emp1, emp1);
                assertNull(map.put(emp2, emp2).get());
                assertEqual(2, (int)map.size());
                assertEqual(2, (int)map.keySet().size());
                assertEqual(0, (int)map.keySet("a = 10").size());
                assertEqual(0, (int)map.values("a = 10").size());
                assertEqual(2, (int)map.keySet("a >= 10").size());
                assertEqual(2, (int)map.values("a >= 10").size());
                assertEqual(2, (int)map.size());
                assertEqual(2, (int)map.values().size());


            }

            void ClientMapTest::testMapWithPortable() {
                IMap<int, Employee> employees = client->getMap<int, Employee>("employees");
                boost::shared_ptr<Employee> n1 = employees.get(1);
                assertNull(n1.get());
                Employee employee("sancar", 24);
                boost::shared_ptr<Employee> ptr = employees.put(1, employee);
                assertNull(ptr.get());
                assertFalse(employees.isEmpty());
                EntryView<int, Employee> view = employees.getEntryView(1);
                assertEqual(view.value, employee);
                assertEqual(view.key, 1);

                employees.addIndex("a", true);
                employees.addIndex("n", false);
            }


            void ClientMapTest::testMapStoreRelatedRequests() {
                imap->putTransient("ali", "veli", 1100);
                imap->flush();
                assertEqual(1, imap->size());
                assertFalse(imap->evict("deli"));
                assertTrue(imap->evict("ali"));
                assertNull(imap->get("ali").get());
            }

        }
    }
}

