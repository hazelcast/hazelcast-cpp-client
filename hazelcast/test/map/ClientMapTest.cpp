//
// Created by sancar koyunlu on 8/27/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "ClientMapTest.h"
#include "IMap.h"
#include "HazelcastClient.h"


namespace hazelcast {
    namespace client {
        namespace test {
            using namespace iTest;

            ClientMapTest::ClientMapTest(HazelcastInstanceFactory& hazelcastInstanceFactory)
            :hazelcastInstanceFactory(hazelcastInstanceFactory)
            , client(new HazelcastClient(clientConfig.addAddress(Address("localhost", 5701))))
            , iMap(new IMap<std::string, std::string>(client->getMap< std::string, std::string >("clientMapTest"))) {

            };

            void ClientMapTest::addTests() {
                addTest(&ClientMapTest::testGet, "testGet");
                addTest(&ClientMapTest::testRemoveAndDelete, "testRemoveAndDelete");
                addTest(&ClientMapTest::testRemoveIfSame, "testRemoveIfSame");
                addTest(&ClientMapTest::testContains, "testContains");
                addTest(&ClientMapTest::testReplace, "testReplace");
                addTest(&ClientMapTest::testPutTtl, "testPutTtl");
                addTest(&ClientMapTest::testTryPutRemove, "testTryPutRemove");
                addTest(&ClientMapTest::testAsyncPutWithTtl, "testAsyncPutWithTtl");
                addTest(&ClientMapTest::testIssue537, "testIssue537");
                addTest(&ClientMapTest::testListener, "testListener");
                addTest(&ClientMapTest::testTryLock, "testTryLock");
                addTest(&ClientMapTest::testForceUnlock, "testForceUnlock");
                addTest(&ClientMapTest::testLockTtl2, "testLockTtl2");
                addTest(&ClientMapTest::testLockTtl, "testLockTtl");
                addTest(&ClientMapTest::testLock, "testLock");
                addTest(&ClientMapTest::testBasicPredicate, "testBasicPredicate");
                addTest(&ClientMapTest::testPutTransient, "testPutTransient");
                addTest(&ClientMapTest::testSet, "testSet");
                addTest(&ClientMapTest::testAsyncGet, "testAsyncGet");
                addTest(&ClientMapTest::testAsyncPut, "testAsyncPut");
                addTest(&ClientMapTest::testAsyncRemove, "testAsyncRemove");
                addTest(&ClientMapTest::testValues, "testValues");
                addTest(&ClientMapTest::testGetAllPutAll, "testGetAllPutAll");
                addTest(&ClientMapTest::testPutIfAbsent, "testPutIfAbsent");
                addTest(&ClientMapTest::testPutIfAbsentTtl, "testPutIfAbsentTtl");

            };

            void ClientMapTest::beforeClass() {
//                ClientConfig clientConfig;
//                clientConfig.addAddress(Address("localhost", 5701));
//                client.reset(new HazelcastClient(clientConfig));
//                iMap.reset(new IMap<std::string, std::string>(client->getMap < std::string, std::string > ("clientMapTest")));

            };

            void ClientMapTest::afterClass() {
            };

            void ClientMapTest::beforeTest() {
                iMap->clear();
            };

            void ClientMapTest::afterTest() {
                iMap->clear();
            };

            void ClientMapTest::fillMap() {
                for (int i = 0; i < 10; i++) {
                    std::string key = "key";
                    key += hazelcast::util::to_string(i);
                    std::string value = "value";
                    value += hazelcast::util::to_string(i);
                    iMap->put(key, value);
                }
            }

            void ClientMapTest::testGet() {
                fillMap();
                for (int i = 0; i < 10; i++) {
                    std::string key = "key";
                    key += hazelcast::util::to_string(i);
                    std::string temp = iMap->get(key);

                    std::string value = "value";
                    value += hazelcast::util::to_string(i);
                    assertEqual(temp, value);
                }
            }

            void ClientMapTest::testRemoveAndDelete() {
                fillMap();
                std::string temp = iMap->remove("key10");
                assertEqual(temp, "");
                iMap->deleteEntry("key9");
                assertEqual(iMap->size(), 9);
                for (int i = 0; i < 9; i++) {
                    std::string key = "key";
                    key += hazelcast::util::to_string(i);
                    std::string temp = iMap->remove(key);
                    std::string value = "value";
                    value += hazelcast::util::to_string(i);
                    assertEqual(temp, value);
                }
                assertEqual(iMap->size(), 0);
            }

            void ClientMapTest::testRemoveIfSame() {
                fillMap();

                assertFalse(iMap->remove("key2", "value"));
                assertEqual(iMap->size(), 10);

                assertTrue((iMap->remove("key2", "value1")));
                assertEqual(iMap->size(), 9);

            }

            void ClientMapTest::flush() {
                //TODO mapStore
            }

            void ClientMapTest::clear() {
                iMap->clear();
            }

            void ClientMapTest::testContains() {
                fillMap();

                assertFalse(iMap->containsKey("key10"));
                assertTrue(iMap->containsKey("key1"));

                assertFalse(iMap->containsValue("value10"));
                assertTrue(iMap->containsValue("value1"));

            }

            void ClientMapTest::testReplace() {
                std::string temp = iMap->replace("key1", "value");
                assertEqual(temp, "");

                std::string tempKey = "key1";
                std::string tempValue = "value1";
                iMap->put(tempKey, tempValue);

                assertEqual("value1", iMap->replace("key1", "value2"));
                assertEqual("value2", iMap->get("key1"));

                assertEqual(false, iMap->replace("key1", "value1", "value3"));
                assertEqual("value2", iMap->get("key1"));

                assertEqual(true, iMap->replace("key1", "value2", "value3"));
                assertEqual("value3", iMap->get("key1"));
            }

            void ClientMapTest::testPutTtl() {

                iMap->put("key1", "value1", 1000);
                std::string temp = iMap->get("key1");
                assertEqual(temp, "");
                boost::this_thread::sleep(boost::posix_time::milliseconds(2000));
                std::string temp2 = iMap->get("key1");
                assertEqual(temp2, "");
            }

            void ClientMapTest::testTryPutRemove() {

                // This method contains CountDownLatch
                assertTrue(false);
            }

            void ClientMapTest::testAsyncPutWithTtl() {

                // This method contains CountDownLatch
                assertTrue(false);

            }

            void ClientMapTest::testIssue537() {

                // This method contains CountDownLatch
                assertTrue(false);

            }

            void ClientMapTest::testListener() {

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

            void ClientMapTest::testLockTtl2() {

                // This method contains CountDownLatch
                assertTrue(false);

            }

            void ClientMapTest::testLockTtl() {

                // This method contains CountDownLatch
                assertTrue(false);

            }

            void ClientMapTest::testLock() {

                // This method contains CountDownLatch
                assertTrue(false);

            }

            void ClientMapTest::testBasicPredicate() {

                fillMap();
                vector<std::string> tempVector;
                tempVector = iMap->values("this , value1");

                vector<std::string>::iterator it = tempVector.begin();
                assertEqual("value1", *it);

                vector<std::string> tempVector2;
                tempVector2 = iMap->keySet("this , value1");

                vector<std::string>::iterator it2 = tempVector2.begin();
                assertEqual("key1", *it2);


                std::vector<std::pair<std::string, std::string> > tempVector3;
                tempVector3 = iMap->entrySet("this , value1");

                std::vector<std::pair<std::string, std::string> > ::iterator it3 = tempVector3.begin();
                assertEqual("key1", (*it3).first);
                assertEqual("value1", (*it3).second);

            }

            void ClientMapTest::testPutTransient() {

                //TODO mapstore
                assertTrue(false);

            }

            void ClientMapTest::testSet() {

                iMap->set("key1", "value1");
                assertEqual("value1", iMap->get("key1"));

                iMap->set("key1", "value2");
                assertEqual("value2", iMap->get("key1"));

                iMap->set("key1", "value3", 1000);
                assertEqual("value3", iMap->get("key1"));

                boost::this_thread::sleep(boost::posix_time::milliseconds(2000));

                assertEqual(iMap->get("key1"), "");

            }

            void ClientMapTest::testAsyncGet() {

                // This method contains Async functions which is not coded yet
                assertTrue(false);

            }

            void ClientMapTest::testAsyncPut() {

                // This method contains Async functions which is not coded yet
                assertTrue(false);

            }

            void ClientMapTest::testAsyncRemove() {

                // This method contains Async functions which is not coded yet
                assertTrue(false);

            }

            void ClientMapTest::testValues() {

                fillMap();
                vector<std::string> tempVector;
                tempVector = iMap->values("this , value1");
                assertEqual(1, tempVector.size());

                vector<std::string>::iterator it = tempVector.begin();
                assertEqual("value1", *it);
            }

            void ClientMapTest::testGetAllPutAll() {

                std::map<std::string, std::string> mapTemp;

                for (int i = 0; i < 100; i++) {
                    mapTemp[hazelcast::util::to_string(i)] = hazelcast::util::to_string(i);
                }
                iMap->putAll(mapTemp);
                assertEqual(iMap->size(), 100);
                for (int i = 0; i < 100; i++) {
                    assertEqual(iMap->get(hazelcast::util::to_string(i)), hazelcast::util::to_string(i));
                }

                IMap<std::string, std::string> iMap2 = client->getMap<std::string, std::string >("dev");

                std::set<std::string> tempSet;
                tempSet.insert(hazelcast::util::to_string(1));
                tempSet.insert(hazelcast::util::to_string(3));

                iMap2.getAll(tempSet);

                assertEqual(iMap2.size(), 2);
                assertEqual(iMap2.get(hazelcast::util::to_string(1)), "1");
                assertEqual(iMap2.get(hazelcast::util::to_string(3)), "3");

            }

            void ClientMapTest::testPutIfAbsent() {

                // putIfAbsent method is not coded yet
                assertTrue(false);

            }

            void ClientMapTest::testPutIfAbsentTtl() {

                // putIfAbsent method is not coded yet
                assertTrue(false);

            }

            void ClientMapTest::destroy() {

                // waiting for framework
                assertTrue(false);

            }

        }
    }
}
