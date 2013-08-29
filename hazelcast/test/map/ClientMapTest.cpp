//
// Created by sancar koyunlu on 8/27/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "ClientMapTest.h"
#include "HazelcastClient.h"
#include "HazelcastInstanceFactory.h"
#include "CountDownLatch.h"


namespace hazelcast {
    namespace client {
        namespace test {
            using namespace iTest;

            ClientMapTest::ClientMapTest(HazelcastInstanceFactory& hazelcastInstanceFactory)
            :hazelcastInstanceFactory(hazelcastInstanceFactory)
            , instance(hazelcastInstanceFactory.newHazelcastInstance())
            , client(new HazelcastClient(clientConfig.addAddress(Address("localhost", 5701))))
            , imap(new IMap<std::string, std::string>(client->getMap< std::string, std::string >("clientMapTest"))) {

            };


            ClientMapTest::~ClientMapTest() {
                instance.shutdown();
            };

            void ClientMapTest::addTests() {
                addTest(&ClientMapTest::testIssue537, "testIssue537");
                addTest(&ClientMapTest::testContains, "testContains");
                addTest(&ClientMapTest::testGet, "testGet");
                addTest(&ClientMapTest::testRemoveAndDelete, "testRemoveAndDelete");
                addTest(&ClientMapTest::testRemoveIfSame, "testRemoveIfSame");
                addTest(&ClientMapTest::testGetAllPutAll, "testGetAllPutAll");
                addTest(&ClientMapTest::testAsyncGet, "testAsyncGet");
                addTest(&ClientMapTest::testAsyncPut, "testAsyncPut");
                addTest(&ClientMapTest::testAsyncPutWithTtl, "testAsyncPutWithTtl");
                addTest(&ClientMapTest::testAsyncRemove, "testAsyncRemove");
                addTest(&ClientMapTest::testTryPutRemove, "testTryPutRemove");
                addTest(&ClientMapTest::testPutTtl, "testPutTtl");
                addTest(&ClientMapTest::testPutIfAbsent, "testPutIfAbsent");
                addTest(&ClientMapTest::testPutIfAbsentTtl, "testPutIfAbsentTtl");
                addTest(&ClientMapTest::testSet, "testSet");
                addTest(&ClientMapTest::testPutTransient, "testPutTransient");
                addTest(&ClientMapTest::testLock, "testLock");
                addTest(&ClientMapTest::testLockTtl, "testLockTtl");
                addTest(&ClientMapTest::testLockTtl2, "testLockTtl2");
                addTest(&ClientMapTest::testTryLock, "testTryLock");
                addTest(&ClientMapTest::testForceUnlock, "testForceUnlock");
                addTest(&ClientMapTest::testValues, "testValues");
                addTest(&ClientMapTest::testReplace, "testReplace");
                addTest(&ClientMapTest::testListener, "testListener");
                addTest(&ClientMapTest::testBasicPredicate, "testBasicPredicate");

            };

            void ClientMapTest::beforeClass() {
            };

            void ClientMapTest::afterClass() {
            };

            void ClientMapTest::beforeTest() {
                imap->clear();
            };

            void ClientMapTest::afterTest() {
                imap->clear();
            };

            void ClientMapTest::fillMap() {
                for (int i = 0; i < 10; i++) {
                    std::string key = "key";
                    key += util::to_string(i);
                    std::string value = "value";
                    value += util::to_string(i);
                    imap->put(key, value);
                }
            }

            class MyListener {
            public:
                MyListener(util::CountDownLatch& latch, util::CountDownLatch& nullLatch)
                :latch(latch)
                , nullLatch(nullLatch) {
                };

                void entryAdded() {
                };

                void entryRemoved(impl::EntryEvent<std::string, std::string>& event) {
                }

                void entryUpdated(impl::EntryEvent<std::string, std::string>& event) {
                }

                void entryEvicted(impl::EntryEvent<std::string, std::string>& event) {
                    const std::string & value = event.getValue();
                    const std::string & oldValue = event.getOldValue();
                    if (value.compare("")) {
                        nullLatch.countDown();
                    }
                    if (oldValue.compare("")) {
                        nullLatch.countDown();
                    }
                    latch.countDown();
                }

            private:
                util::CountDownLatch& nullLatch;
                util::CountDownLatch& latch;
            };

            void ClientMapTest::testIssue537() {
                util::CountDownLatch latch(2);
                util::CountDownLatch nullLatch(2);
                MyListener myListener(latch, nullLatch);
//                long id = imap->addEntryListener(myListener, true);

                imap->put("key1", "value1", 2 * 1000);

                assertTrue(latch.await(10 * 1000));
                assertTrue(nullLatch.await(1000));

//                imap->removeEntryListener(id);

                imap->put("key2", "value2");
                // This method contains CountDownLatch
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
                    std::string key = "key";
                    key += util::to_string(i);
                    std::string temp = imap->get(key);

                    std::string value = "value";
                    value += util::to_string(i);
                    assertEqual(temp, value);
                }
            }

            void ClientMapTest::testRemoveAndDelete() {
                fillMap();
                std::string temp = imap->remove("key10");
                assertEqual(temp, "");
                imap->deleteEntry("key9");
                assertEqual(imap->size(), 9);
                for (int i = 0; i < 9; i++) {
                    std::string key = "key";
                    key += util::to_string(i);
                    std::string temp = imap->remove(key);
                    std::string value = "value";
                    value += util::to_string(i);
                    assertEqual(temp, value);
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
                    mapTemp[util::to_string(i)] = util::to_string(i);
                }
                imap->putAll(mapTemp);
                assertEqual(imap->size(), 100);

                for (int i = 0; i < 100; i++) {
                    string expected = util::to_string(i);
                    string actual = imap->get(util::to_string(i));
                    assertEqual(expected, actual);
                }

                std::set<std::string> tempSet;
                tempSet.insert(util::to_string(1));
                tempSet.insert(util::to_string(3));

                std::map<std::string, std::string> m2 = imap->getAll(tempSet);

                assertEqual(m2.size(), 2);
                assertEqual(m2[util::to_string(1)], "1");
                assertEqual(m2[util::to_string(3)], "3");

            }

            void ClientMapTest::testAsyncGet() {
                fillMap();
                Future<std::string> f = imap->getAsync("key1");
                std::string o;
                FutureStatus status = f.wait_for(0);
                assertEqual(FutureStatus::TIMEOUT, status);

                o = f.get();
                assertEqual("value1", o);
            }

            void ClientMapTest::testAsyncPut() {
                fillMap();
                Future<string> f = imap->putAsync("key3", "value");
                FutureStatus status = f.wait_for(0);
                assertEqual(FutureStatus::TIMEOUT, status);
                std::string & o = f.get();
                assertEqual("value3", o);
                assertEqual("value", imap->get("key3"));

            }

            void ClientMapTest::testAsyncPutWithTtl() {

                // This method contains CountDownLatch
                assertTrue(false);

            }

            void ClientMapTest::testAsyncRemove() {
                fillMap();
                Future<string> f = imap->removeAsync("key4");
                FutureStatus status = f.wait_for(0);
                assertEqual(FutureStatus::TIMEOUT, status);
                std::string & o = f.get();
                assertEqual("value4", o);
                assertEqual(9, imap->size());

            }

            void ClientMapTest::testTryPutRemove() {

                // This method contains CountDownLatch
                assertTrue(false);
            }

            void ClientMapTest::testPutTtl() {
                imap->put("key1", "value1", 1000);
                std::string temp = imap->get("key1");
                assertEqual(temp, "");
                boost::this_thread::sleep(boost::posix_time::milliseconds(2000));
                std::string temp2 = imap->get("key1");
                assertEqual(temp2, "");
            }

            void ClientMapTest::testPutIfAbsent() {
                std::string o = imap->putIfAbsent("key1", "value1");
                assertEqual("", o);
                assertEqual("value1", imap->putIfAbsent("key1", "value3"));
            }

            void ClientMapTest::testPutIfAbsentTtl() {
                assertEqual("", imap->putIfAbsent("key1", "value1", 1000));
                assertEqual("value1", imap->putIfAbsent("key1", "value3", 1000));
                boost::this_thread::sleep(boost::posix_time::seconds(2));
                assertEqual("", imap->putIfAbsent("key1", "value3", 1000));
                assertEqual("value3", imap->putIfAbsent("key1", "value4", 1000));
                boost::this_thread::sleep(boost::posix_time::seconds(2));
            }

            void ClientMapTest::testSet() {
                imap->set("key1", "value1");
                assertEqual("value1", imap->get("key1"));

                imap->set("key1", "value2");
                assertEqual("value2", imap->get("key1"));

                imap->set("key1", "value3", 1000);
                assertEqual("value3", imap->get("key1"));

                boost::this_thread::sleep(boost::posix_time::seconds(2));
                assertEqual(imap->get("key1"), "");

            }


            void ClientMapTest::testPutTransient() {
                //TODO mapstore
                assertTrue(false);

            }

            void ClientMapTest::testLock() {

                // This method contains CountDownLatch
                assertTrue(false);

            }


            void ClientMapTest::testLockTtl() {

                // This method contains CountDownLatch
                assertTrue(false);

            }

            void ClientMapTest::testLockTtl2() {

                // This method contains CountDownLatch
                assertTrue(false);

            }

            void ClientMapTest::testTryLock() {

                // This method contains CountDownLatch
                assertTrue(false);

            }

            void ClientMapTest::testForceUnlock() {

                // This method contains CountDownLatch
                assertTrue(false);

            }

            void ClientMapTest::testValues() {

                fillMap();
                vector<std::string> tempVector;
                tempVector = imap->values("this , value1");
                assertEqual(1, tempVector.size());

                vector<std::string>::iterator it = tempVector.begin();
                assertEqual("value1", *it);
            }

            void ClientMapTest::testReplace() {
                std::string temp = imap->replace("key1", "value");
                assertEqual(temp, "");

                std::string tempKey = "key1";
                std::string tempValue = "value1";
                imap->put(tempKey, tempValue);

                assertEqual("value1", imap->replace("key1", "value2"));
                assertEqual("value2", imap->get("key1"));

                assertEqual(false, imap->replace("key1", "value1", "value3"));
                assertEqual("value2", imap->get("key1"));

                assertEqual(true, imap->replace("key1", "value2", "value3"));
                assertEqual("value3", imap->get("key1"));
            }

            void ClientMapTest::testListener() {

                // This method contains CountDownLatch
                assertTrue(false);

            }

            void ClientMapTest::testBasicPredicate() {

                fillMap();
                vector<std::string> tempVector;
                tempVector = imap->values("this , value1");

                vector<std::string>::iterator it = tempVector.begin();
                assertEqual("value1", *it);

                vector<std::string> tempVector2;
                tempVector2 = imap->keySet("this , value1");

                vector<std::string>::iterator it2 = tempVector2.begin();
                assertEqual("key1", *it2);


                std::vector<std::pair<std::string, std::string> > tempVector3;
                tempVector3 = imap->entrySet("this , value1");

                std::vector<std::pair<std::string, std::string> > ::iterator it3 = tempVector3.begin();
                assertEqual("key1", (*it3).first);
                assertEqual("value1", (*it3).second);

            }
        }
    }
}
