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
// Created by sancar koyunlu on 8/27/13.

#include "hazelcast/client/query/OrPredicate.h"
#include "hazelcast/client/query/RegexPredicate.h"
#include "hazelcast/client/query/PagingPredicate.h"
#include "hazelcast/client/query/QueryConstants.h"
#include "hazelcast/client/query/NotPredicate.h"
#include "hazelcast/client/query/InstanceOfPredicate.h"
#include "hazelcast/client/query/NotEqualPredicate.h"
#include "hazelcast/client/query/InPredicate.h"
#include "hazelcast/client/query/ILikePredicate.h"
#include "hazelcast/client/query/LikePredicate.h"
#include "hazelcast/client/query/GreaterLessPredicate.h"
#include "hazelcast/client/query/AndPredicate.h"
#include "hazelcast/client/query/BetweenPredicate.h"
#include "hazelcast/client/query/EqualPredicate.h"
#include "hazelcast/client/query/TruePredicate.h"
#include "hazelcast/client/query/FalsePredicate.h"
#include "hazelcast/client/query/SqlPredicate.h"
#include "hazelcast/util/Util.h"
#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/client/adaptor/RawPointerMap.h"
#include "hazelcast/client/EntryAdapter.h"
#include "hazelcast/client/serialization/IdentifiedDataSerializable.h"
#include "hazelcast/client/ClientConfig.h"

#include "HazelcastServerFactory.h"
#include "serialization/Employee.h"
#include "TestHelperFunctions.h"
#include "ClientTestSupport.h"
#include "HazelcastServer.h"


namespace hazelcast {
    namespace client {
        namespace test {
            namespace adaptor {
                class RawPointerMapTest : public ClientTestSupport {
                public:
                    RawPointerMapTest() : instance(*g_srvFactory), instance2(*g_srvFactory), client(getNewClient()),
                                          originalMap(client->getMap<std::string, std::string>("RawPointerMapTest")),
                                          imap(new hazelcast::client::adaptor::RawPointerMap<std::string, std::string>(originalMap)) {
                    }

                    void fillMap() {
                        for (int i = 0; i < 10; i++) {
                            std::string key = "key";
                            key += util::IOUtil::to_string(i);
                            std::string value = "value";
                            value += util::IOUtil::to_string(i);
                            imap->put(key, value);
                        }
                    }
                protected:
                    HazelcastServer instance;
                    HazelcastServer instance2;
                    ClientConfig clientConfig;
                    std::auto_ptr<HazelcastClient> client;
                    IMap<std::string, std::string> originalMap;
                    std::auto_ptr<hazelcast::client::adaptor::RawPointerMap<std::string, std::string> > imap;
                };

                class SampleEntryListener : public EntryAdapter<std::string, std::string> {
                public:
                    SampleEntryListener(util::CountDownLatch &addLatch, util::CountDownLatch &removeLatch,
                                        util::CountDownLatch &updateLatch, util::CountDownLatch &evictLatch)
                            : addLatch(addLatch), removeLatch(removeLatch), updateLatch(updateLatch),
                              evictLatch(evictLatch) {
                    }

                    void entryAdded(const EntryEvent<std::string, std::string> &event) {
                        addLatch.countDown();
                    }

                    void entryRemoved(const EntryEvent<std::string, std::string> &event) {
                        removeLatch.countDown();
                    }

                    void entryUpdated(const EntryEvent<std::string, std::string> &event) {
                        updateLatch.countDown();
                    }

                    void entryEvicted(const EntryEvent<std::string, std::string> &event) {
                        evictLatch.countDown();
                    }

                private:
                    util::CountDownLatch &addLatch;
                    util::CountDownLatch &removeLatch;
                    util::CountDownLatch &updateLatch;
                    util::CountDownLatch &evictLatch;
                };

                class MyListener : public EntryAdapter<std::string, std::string> {
                public:
                    MyListener(util::CountDownLatch &latch, util::CountDownLatch &nullLatch)
                            : latch(latch), nullLatch(nullLatch) {
                    }

                    void entryAdded(const EntryEvent<std::string, std::string> &event) {
                        latch.countDown();
                    }

                    void entryEvicted(const EntryEvent<std::string, std::string> &event) {
                        const std::string &oldValue = event.getOldValue();
                        if (oldValue.compare("")) {
                            nullLatch.countDown();
                        }
                        latch.countDown();
                    }

                private:
                    util::CountDownLatch &latch;
                    util::CountDownLatch &nullLatch;
                };

                class ClearListener : public EntryAdapter<std::string, std::string> {
                public:
                    ClearListener(util::CountDownLatch &latch) : latch(latch) {
                    }

                    void mapCleared(const MapEvent &event) {
                        latch.countDown();
                    }

                private:
                    util::CountDownLatch &latch;
                };

                class EvictListener : public EntryAdapter<std::string, std::string> {
                public:
                    EvictListener(util::CountDownLatch &latch) : latch(latch) {
                    }

                    void mapEvicted(const MapEvent &event) {
                        latch.countDown();
                    }

                private:
                    util::CountDownLatch &latch;
                };

                TEST_F(RawPointerMapTest, testIssue537) {
                    util::CountDownLatch latch(2);
                    util::CountDownLatch nullLatch(1);
                    MyListener myListener(latch, nullLatch);
                    std::string id = imap->addEntryListener(myListener, true);

                    imap->put("key1", "value1", 2 * 1000);

                    ASSERT_TRUE(latch.await(10));
                    ASSERT_TRUE(nullLatch.await(1));

                    ASSERT_TRUE(imap->removeEntryListener(id));

                    imap->put("key2", "value2");
                    ASSERT_EQ(1, imap->size());
                }

                TEST_F(RawPointerMapTest, testContains) {
                    fillMap();

                    ASSERT_FALSE(imap->containsKey("key10"));
                    ASSERT_TRUE(imap->containsKey("key1"));

                    ASSERT_FALSE(imap->containsValue("value10"));
                    ASSERT_TRUE(imap->containsValue("value1"));

                }

                TEST_F(RawPointerMapTest, testGet) {
                    fillMap();
                    for (int i = 0; i < 10; i++) {
                        std::string key = "key";
                        key += util::IOUtil::to_string(i);
                        std::auto_ptr<std::string> temp = imap->get(key);

                        std::string value = "value";
                        value += util::IOUtil::to_string(i);
                        ASSERT_EQ(*temp, value);
                    }
                }

                TEST_F(RawPointerMapTest, testRemoveAndDelete) {
                    fillMap();
                    std::auto_ptr<std::string> temp = imap->remove("key10");
                    ASSERT_EQ(temp.get(), (std::string *) NULL);
                    imap->deleteEntry("key9");
                    ASSERT_EQ(imap->size(), 9);
                    for (int i = 0; i < 9; i++) {
                        std::string key = "key";
                        key += util::IOUtil::to_string(i);
                        std::auto_ptr<std::string> temp2 = imap->remove(key);
                        std::string value = "value";
                        value += util::IOUtil::to_string(i);
                        ASSERT_EQ(*temp2, value);
                    }
                    ASSERT_EQ(imap->size(), 0);
                }

                TEST_F(RawPointerMapTest, testRemoveIfSame) {
                    fillMap();

                    ASSERT_FALSE(imap->remove("key2", "value"));
                    ASSERT_EQ(10, imap->size());

                    ASSERT_TRUE((imap->remove("key2", "value2")));
                    ASSERT_EQ(9, imap->size());

                }

                TEST_F(RawPointerMapTest, testGetAllPutAll) {

                    std::map<std::string, std::string> mapTemp;

                    for (int i = 0; i < 100; i++) {
                        mapTemp[util::IOUtil::to_string(i)] = util::IOUtil::to_string(i);
                    }
                    imap->putAll(mapTemp);
                    ASSERT_EQ(imap->size(), 100);

                    for (int i = 0; i < 100; i++) {
                        std::string expected = util::IOUtil::to_string(i);
                        std::auto_ptr<std::string> actual = imap->get(util::IOUtil::to_string(i));
                        ASSERT_EQ(expected, *actual);
                    }

                    std::set<std::string> tempSet;
                    tempSet.insert(util::IOUtil::to_string(1));
                    tempSet.insert(util::IOUtil::to_string(3));

                    std::auto_ptr<hazelcast::client::EntryArray<std::string, std::string> > m2 = imap->getAll(tempSet);

                    ASSERT_EQ(2U, m2->size());
                    std::auto_ptr<std::string> key1 = m2->releaseKey(0);
                    ASSERT_NE((std::string *)NULL, key1.get());
                    std::auto_ptr<std::string> value1 = m2->releaseValue(0);
                    ASSERT_NE((std::string *)NULL, value1.get());
                    ASSERT_EQ(*key1, *value1);
                    ASSERT_TRUE(*key1 == "1" || *key1 == "3" );

                    std::pair<const std::string *, const std::string *> entry = (*m2)[1];
                    ASSERT_NE((std::string *)NULL, entry.first);
                    ASSERT_NE((std::string *)NULL, entry.second);
                    ASSERT_EQ(*entry.first, *entry.second);
                    ASSERT_TRUE(*entry.first == "1" || *entry.first == "3" );
                    ASSERT_NE(*key1, *entry.first);
                }

                void tryPutThread(util::ThreadArgs &args) {
                    util::CountDownLatch *latch = (util::CountDownLatch *) args.arg0;
                    hazelcast::client::adaptor::RawPointerMap<std::string, std::string> *imap = (hazelcast::client::adaptor::RawPointerMap<std::string, std::string> *) args.arg1;
                    bool result = imap->tryPut("key1", "value3", 1 * 1000);
                    if (!result) {
                        latch->countDown();
                    }
                }

                void tryRemoveThread(util::ThreadArgs &args) {
                    util::CountDownLatch *latch = (util::CountDownLatch *) args.arg0;
                    hazelcast::client::adaptor::RawPointerMap<std::string, std::string> *imap = (hazelcast::client::adaptor::RawPointerMap<std::string, std::string> *) args.arg1;
                    bool result = imap->tryRemove("key2", 1 * 1000);
                    if (!result) {
                        latch->countDown();
                    }
                }

                TEST_F(RawPointerMapTest, testTryPutRemove) {

                    ASSERT_TRUE(imap->tryPut("key1", "value1", 1 * 1000));
                    ASSERT_TRUE(imap->tryPut("key2", "value2", 1 * 1000));
                    imap->lock("key1");
                    imap->lock("key2");

                    util::CountDownLatch latch(2);

                    util::Thread t1(tryPutThread, &latch, imap.get());
                    util::Thread t2(tryRemoveThread, &latch, imap.get());

                    ASSERT_TRUE(latch.await(20));
                    ASSERT_EQ("value1", *(imap->get("key1")));
                    ASSERT_EQ("value2", *(imap->get("key2")));
                    imap->forceUnlock("key1");
                    imap->forceUnlock("key2");
                }

                TEST_F(RawPointerMapTest, testPutTtl) {
                    util::CountDownLatch dummy(10);
                    util::CountDownLatch evict(1);
                    SampleEntryListener sampleEntryListener(dummy, dummy, dummy, evict);
                    std::string id = imap->addEntryListener(sampleEntryListener, false);

                    imap->put("key1", "value1", 2000);
                    std::auto_ptr<std::string> temp = imap->get("key1");
                    ASSERT_EQ(*temp, "value1");
                    ASSERT_TRUE(evict.await(20 * 1000));
                    std::auto_ptr<std::string> temp2 = imap->get("key1");
                    ASSERT_EQ(temp2.get(), (std::string *) NULL);

                    ASSERT_TRUE(imap->removeEntryListener(id));
                }

                TEST_F(RawPointerMapTest, testPutIfAbsent) {
                    std::auto_ptr<std::string> o = imap->putIfAbsent("key1", "value1");
                    ASSERT_EQ(o.get(), (std::string *) NULL);
                    ASSERT_EQ("value1", *(imap->putIfAbsent("key1", "value3")));
                }

                TEST_F(RawPointerMapTest, testPutIfAbsentTtl) {
                    ASSERT_EQ(imap->putIfAbsent("key1", "value1", 1000).get(), (std::string *) NULL);
                    ASSERT_EQ("value1", *(imap->putIfAbsent("key1", "value3", 1000)));

                    ASSERT_NULL_EVENTUALLY(imap->putIfAbsent("key1", "value3", 1000).get());
                    ASSERT_EQ("value3", *(imap->putIfAbsent("key1", "value4", 1000)));
                }

                TEST_F(RawPointerMapTest, testSet) {
                    imap->set("key1", "value1");
                    ASSERT_EQ("value1", *(imap->get("key1")));

                    imap->set("key1", "value2");
                    ASSERT_EQ("value2", *(imap->get("key1")));

                    imap->set("key1", "value3", 1000);
                    ASSERT_EQ("value3", *(imap->get("key1")));

                    ASSERT_NULL_EVENTUALLY(imap->get("key1").get());
                }

                void testLockThread(util::ThreadArgs &args) {
                    util::CountDownLatch *latch = (util::CountDownLatch *) args.arg0;
                    hazelcast::client::adaptor::RawPointerMap<std::string, std::string> *imap = (hazelcast::client::adaptor::RawPointerMap<std::string, std::string> *) args.arg1;
                    imap->tryPut("key1", "value2", 1);
                    latch->countDown();
                }

                TEST_F(RawPointerMapTest, testLock) {
                    imap->put("key1", "value1");
                    ASSERT_EQ("value1", *(imap->get("key1")));
                    imap->lock("key1");
                    util::CountDownLatch latch(1);
                    util::Thread t1(testLockThread, &latch, imap.get());
                    ASSERT_TRUE(latch.await(5));
                    ASSERT_EQ("value1", *(imap->get("key1")));
                    imap->forceUnlock("key1");

                }

                void testLockTTLThread(util::ThreadArgs &args) {
                    util::CountDownLatch *latch = (util::CountDownLatch *) args.arg0;
                    hazelcast::client::adaptor::RawPointerMap<std::string, std::string> *imap = (hazelcast::client::adaptor::RawPointerMap<std::string, std::string> *) args.arg1;
                    imap->tryPut("key1", "value2", 5 * 1000);
                    latch->countDown();
                }

                TEST_F(RawPointerMapTest, testLockTtl) {
                    imap->put("key1", "value1");
                    ASSERT_EQ("value1", *(imap->get("key1")));
                    imap->lock("key1", 2 * 1000);
                    util::CountDownLatch latch(1);
                    util::Thread t1(testLockTTLThread, &latch, imap.get());
                    ASSERT_TRUE(latch.await(10));
                    ASSERT_FALSE(imap->isLocked("key1"));
                    ASSERT_EQ("value2", *(imap->get("key1")));
                    imap->forceUnlock("key1");

                }

                void testLockTTL2Thread(util::ThreadArgs &args) {
                    util::CountDownLatch *latch = (util::CountDownLatch *) args.arg0;
                    hazelcast::client::adaptor::RawPointerMap<std::string, std::string> *imap = (hazelcast::client::adaptor::RawPointerMap<std::string, std::string> *) args.arg1;
                    if (!imap->tryLock("key1")) {
                        latch->countDown();
                    }
                    if (imap->tryLock("key1", 5 * 1000)) {
                        latch->countDown();
                    }
                }

                TEST_F(RawPointerMapTest, testLockTtl2) {
                    imap->lock("key1", 3 * 1000);
                    util::CountDownLatch latch(2);
                    util::Thread t1(testLockTTL2Thread, &latch, imap.get());
                    ASSERT_TRUE(latch.await(10));
                    imap->forceUnlock("key1");

                }

                void testMapTryLockThread1(util::ThreadArgs &args) {
                    util::CountDownLatch *latch = (util::CountDownLatch *) args.arg0;
                    hazelcast::client::adaptor::RawPointerMap<std::string, std::string> *imap = (hazelcast::client::adaptor::RawPointerMap<std::string, std::string> *) args.arg1;
                    if (!imap->tryLock("key1", 2)) {
                        latch->countDown();
                    }
                }

                void testMapTryLockThread2(util::ThreadArgs &args) {
                    util::CountDownLatch *latch = (util::CountDownLatch *) args.arg0;
                    hazelcast::client::adaptor::RawPointerMap<std::string, std::string> *imap = (hazelcast::client::adaptor::RawPointerMap<std::string, std::string> *) args.arg1;
                    if (imap->tryLock("key1", 20 * 1000)) {
                        latch->countDown();
                    }
                }

                TEST_F(RawPointerMapTest, testTryLock) {

                    ASSERT_TRUE(imap->tryLock("key1", 2 * 1000));
                    util::CountDownLatch latch(1);
                    util::Thread t1(testMapTryLockThread1, &latch, imap.get());

                    ASSERT_TRUE(latch.await(100));

                    ASSERT_TRUE(imap->isLocked("key1"));

                    util::CountDownLatch latch2(1);
                    util::Thread t2(testMapTryLockThread2, &latch2, imap.get());

                    util::sleep(1);
                    imap->unlock("key1");
                    ASSERT_TRUE(latch2.await(100));
                    ASSERT_TRUE(imap->isLocked("key1"));
                    imap->forceUnlock("key1");

                }

                void testMapForceUnlockThread(util::ThreadArgs &args) {
                    util::CountDownLatch *latch = (util::CountDownLatch *) args.arg0;
                    hazelcast::client::adaptor::RawPointerMap<std::string, std::string> *imap = (hazelcast::client::adaptor::RawPointerMap<std::string, std::string> *) args.arg1;
                    imap->forceUnlock("key1");
                    latch->countDown();
                }

                TEST_F(RawPointerMapTest, testForceUnlock) {
                    imap->lock("key1");
                    util::CountDownLatch latch(1);
                    util::Thread t2(testMapForceUnlockThread, &latch, imap.get());
                    ASSERT_TRUE(latch.await(100));
                    t2.join();
                    ASSERT_FALSE(imap->isLocked("key1"));

                }

                TEST_F(RawPointerMapTest, testValues) {

                    fillMap();
                    query::SqlPredicate predicate("this == value1");
                    std::auto_ptr<hazelcast::client::DataArray<std::string> > tempVector = imap->values(predicate);
                    ASSERT_EQ(1U, tempVector->size());

                    ASSERT_EQ("value1", *tempVector->get(0));
                }

                TEST_F(RawPointerMapTest, testValuesWithPredicate) {
                    IMap<int, int> basicIntMap = client->getMap<int, int>("testValuesWithPredicateIntMap");
                    client::adaptor::RawPointerMap<int, int> intMap(basicIntMap);
                    const int numItems = 20;
                    for (int i = 0; i < numItems; ++i) {
                        intMap.put(i, 2 * i);
                    }

                    std::auto_ptr<DataArray<int> > values = intMap.values();
                    ASSERT_EQ(numItems, (int)values->size());
                    std::vector<int> actualValues;
                    for (int i = 0; i < (int)values->size(); ++i) {
                        ASSERT_NE((const int *)NULL, values->get(i));
                        actualValues.push_back(*values->get(i));
                    }
                    std::sort(actualValues.begin(), actualValues.end());
                    for (int i = 0; i < numItems; ++i) {
                        ASSERT_EQ(2 * i, actualValues[i]);
                    }

                    // EqualPredicate
                    // key == 5
                    values = intMap.values(query::EqualPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5));
                    ASSERT_EQ(1, (int)values->size());
                    ASSERT_NE((const int *)NULL, (*values)[0]);
                    ASSERT_EQ(2 * 5, *((*values)[0]));

                    // value == 8
                    values = intMap.values(query::EqualPredicate<int>(query::QueryConstants::getValueAttributeName(), 8));
                    ASSERT_EQ(1, (int)values->size());
                    ASSERT_NE((const int *)NULL, (*values)[0]);
                    ASSERT_EQ(8, *((*values)[0]));

                    // key == numItems
                    values = intMap.values(query::EqualPredicate<int>(query::QueryConstants::getKeyAttributeName(), numItems));
                    ASSERT_EQ(0, (int)values->size());

                    // NotEqual Predicate
                    // key != 5
                    values = intMap.values(query::NotEqualPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5));
                    ASSERT_EQ(numItems - 1, (int)values->size());
                    actualValues.clear();
                    for (int i = 0; i < (int)values->size(); ++i) {
                        ASSERT_NE((const int *)NULL, values->get(i));
                        actualValues.push_back(*values->get(i));
                    }
                    std::sort(actualValues.begin(), actualValues.end());
                    for (int i = 0; i < numItems - 1; ++i) {
                        if (i >= 5) {
                            ASSERT_EQ(2 * (i + 1), actualValues[i]);
                        } else {
                            ASSERT_EQ(2 * i, actualValues[i]);
                        }
                    }

                    // this(value) != 8
                    values = intMap.values(query::NotEqualPredicate<int>(query::QueryConstants::getValueAttributeName(), 8));
                    ASSERT_EQ(numItems - 1, (int)values->size());
                    actualValues.clear();
                    for (int i = 0; i < (int)values->size(); ++i) {
                        ASSERT_NE((const int *)NULL, values->get(i));
                        actualValues.push_back(*values->get(i));
                    }
                    std::sort(actualValues.begin(), actualValues.end());
                    for (int i = 0; i < numItems - 1; ++i) {
                        if (i >= 4) {
                            ASSERT_EQ(2 * (i + 1), actualValues[i]);
                        } else {
                            ASSERT_EQ(2 * i, actualValues[i]);
                        }
                    }

                    // TruePredicate
                    values = intMap.values(query::TruePredicate());
                    ASSERT_EQ(numItems, (int)values->size());
                    actualValues.clear();
                    for (int i = 0; i < (int)values->size(); ++i) {
                        ASSERT_NE((const int *)NULL, values->get(i));
                        actualValues.push_back(*values->get(i));
                    }
                    std::sort(actualValues.begin(), actualValues.end());
                    for (int i = 0; i < numItems; ++i) {
                        ASSERT_EQ(2 * i, actualValues[i]);
                    }

                    // FalsePredicate
                    values = intMap.values(query::FalsePredicate());
                    ASSERT_EQ(0, (int)values->size());

                    // BetweenPredicate
                    // 5 <= key <= 10
                    values = intMap.values(query::BetweenPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5, 10));
                    ASSERT_EQ(6, (int)values->size());
                    actualValues.clear();
                    for (int i = 0; i < (int)values->size(); ++i) {
                        ASSERT_NE((const int *)NULL, values->get(i));
                        actualValues.push_back(*values->get(i));
                    }
                    std::sort(actualValues.begin(), actualValues.end());
                    for (int i = 0; i < 6; ++i) {
                        ASSERT_EQ(2 * (i + 5), actualValues[i]);
                    }

                    // 20 <= key <=30
                    values = intMap.values(query::BetweenPredicate<int>(query::QueryConstants::getKeyAttributeName(), 20, 30));
                    ASSERT_EQ(0, (int)values->size());

                    // GreaterLessPredicate
                    // value <= 10
                    values = intMap.values(
                            query::GreaterLessPredicate<int>(query::QueryConstants::getValueAttributeName(), 10, true, true));
                    ASSERT_EQ(6, (int)values->size());
                    actualValues.clear();
                    for (int i = 0; i < (int)values->size(); ++i) {
                        ASSERT_NE((const int *)NULL, values->get(i));
                        actualValues.push_back(*values->get(i));
                    }
                    std::sort(actualValues.begin(), actualValues.end());
                    for (int i = 0; i < 6; ++i) {
                        ASSERT_EQ(2 * i, actualValues[i]);
                    }

                    // key < 7
                    values = intMap.values(
                            query::GreaterLessPredicate<int>(query::QueryConstants::getKeyAttributeName(), 7, false, true));
                    ASSERT_EQ(7, (int)values->size());
                    actualValues.clear();
                    for (int i = 0; i < (int)values->size(); ++i) {
                        ASSERT_NE((const int *)NULL, values->get(i));
                        actualValues.push_back(*values->get(i));
                    }
                    std::sort(actualValues.begin(), actualValues.end());
                    for (int i = 0; i < 7; ++i) {
                        ASSERT_EQ(2 * i, actualValues[i]);
                    }

                    // value >= 15
                    values = intMap.values(
                            query::GreaterLessPredicate<int>(query::QueryConstants::getValueAttributeName(), 15, true, false));
                    ASSERT_EQ(12, (int)values->size());
                    actualValues.clear();
                    for (int i = 0; i < (int)values->size(); ++i) {
                        ASSERT_NE((const int *)NULL, values->get(i));
                        actualValues.push_back(*values->get(i));
                    }
                    std::sort(actualValues.begin(), actualValues.end());
                    for (int i = 0; i < 12; ++i) {
                        ASSERT_EQ(2 * (i + 8), actualValues[i]);
                    }

                    // key > 5
                    values = intMap.values(
                            query::GreaterLessPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5, false, false));
                    ASSERT_EQ(14, (int)values->size());
                    actualValues.clear();
                    for (int i = 0; i < (int)values->size(); ++i) {
                        ASSERT_NE((const int *)NULL, values->get(i));
                        actualValues.push_back(*values->get(i));
                    }
                    std::sort(actualValues.begin(), actualValues.end());
                    for (int i = 0; i < 14; ++i) {
                        ASSERT_EQ(2 * (i + 6), actualValues[i]);
                    }

                    // InPredicate
                    // key in {4, 10, 19}
                    std::vector<int> inVals(3);
                    inVals[0] = 4;
                    inVals[1] = 10;
                    inVals[2] = 19;
                    values = intMap.values(query::InPredicate<int>(query::QueryConstants::getKeyAttributeName(), inVals));
                    ASSERT_EQ(3, (int)values->size());
                    actualValues.clear();
                    for (int i = 0; i < (int)values->size(); ++i) {
                        ASSERT_NE((const int *)NULL, values->get(i));
                        actualValues.push_back(*values->get(i));
                    }
                    std::sort(actualValues.begin(), actualValues.end());
                    ASSERT_EQ(2 * 4, actualValues[0]);
                    ASSERT_EQ(2 * 10, actualValues[1]);
                    ASSERT_EQ(2 * 19, actualValues[2]);

                    // value in {4, 10, 19}
                    values = intMap.values(query::InPredicate<int>(query::QueryConstants::getValueAttributeName(), inVals));
                    ASSERT_EQ(2, (int)values->size());
                    actualValues.clear();
                    for (int i = 0; i < (int)values->size(); ++i) {
                        ASSERT_NE((const int *)NULL, values->get(i));
                        actualValues.push_back(*values->get(i));
                    }
                    std::sort(actualValues.begin(), actualValues.end());
                    ASSERT_EQ(4, actualValues[0]);
                    ASSERT_EQ(10, actualValues[1]);

                    // InstanceOfPredicate
                    // value instanceof Integer
                    values = intMap.values(query::InstanceOfPredicate("java.lang.Integer"));
                    ASSERT_EQ(20, (int)values->size());
                    actualValues.clear();
                    for (int i = 0; i < (int)values->size(); ++i) {
                        ASSERT_NE((const int *)NULL, values->get(i));
                        actualValues.push_back(*values->get(i));
                    }
                    std::sort(actualValues.begin(), actualValues.end());
                    for (int i = 0; i < numItems; ++i) {
                        ASSERT_EQ(2 * i, actualValues[i]);
                    }

                    values = intMap.values(query::InstanceOfPredicate("java.lang.String"));
                    ASSERT_EQ(0, (int)values->size());

                    // NotPredicate
                    // !(5 <= key <= 10)
                    std::auto_ptr<query::Predicate> bp = std::auto_ptr<query::Predicate>(new query::BetweenPredicate<int>(
                            query::QueryConstants::getKeyAttributeName(), 5, 10));
                    query::NotPredicate notPredicate(bp);
                    values = intMap.values(notPredicate);
                    ASSERT_EQ(14, (int)values->size());
                    actualValues.clear();
                    for (int i = 0; i < (int)values->size(); ++i) {
                        ASSERT_NE((const int *)NULL, values->get(i));
                        actualValues.push_back(*values->get(i));
                    }
                    std::sort(actualValues.begin(), actualValues.end());
                    for (int i = 0; i < 14; ++i) {
                        if (i >= 5) {
                            ASSERT_EQ(2 * (i + 6), actualValues[i]);
                        } else {
                            ASSERT_EQ(2 * i, actualValues[i]);
                        }
                    }

                    // AndPredicate
                    // 5 <= key <= 10 AND Values in {4, 10, 19} = values {4, 10}
                    bp = std::auto_ptr<query::Predicate>(
                            new query::BetweenPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5, 10));
                    std::auto_ptr<query::Predicate> inPred = std::auto_ptr<query::Predicate>(
                            new query::InPredicate<int>(query::QueryConstants::getValueAttributeName(), inVals));
                    values = intMap.values(query::AndPredicate().add(bp).add(inPred));
                    ASSERT_EQ(1, (int)values->size());
                    ASSERT_EQ(10, *(values->release(0)));

                    // OrPredicate
                    // 5 <= key <= 10 OR Values in {4, 10, 19} = values {4, 10, 12, 14, 16, 18, 20}
                    bp = std::auto_ptr<query::Predicate>(
                            new query::BetweenPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5, 10));
                    inPred = std::auto_ptr<query::Predicate>(
                            new query::InPredicate<int>(query::QueryConstants::getValueAttributeName(), inVals));
                    values = intMap.values(query::OrPredicate().add(bp).add(inPred));
                    ASSERT_EQ(7, (int)values->size());
                    actualValues.clear();
                    for (int i = 0; i < (int)values->size(); ++i) {
                        ASSERT_NE((const int *)NULL, values->get(i));
                        actualValues.push_back(*values->get(i));
                    }
                    std::sort(actualValues.begin(), actualValues.end());
                    ASSERT_EQ(4, actualValues[0]);
                    ASSERT_EQ(10, actualValues[1]);
                    ASSERT_EQ(12, actualValues[2]);
                    ASSERT_EQ(14, actualValues[3]);
                    ASSERT_EQ(16, actualValues[4]);
                    ASSERT_EQ(18, actualValues[5]);
                    ASSERT_EQ(20, actualValues[6]);

                    for (int i = 0; i < 12; i++) {
                        std::string key = "key";
                        key += util::IOUtil::to_string(i);
                        std::string value = "value";
                        value += util::IOUtil::to_string(i);
                        imap->put(key, value);
                    }
                    imap->put("key_111_test", "myvalue_111_test");
                    imap->put("key_22_test", "myvalue_22_test");

                    // LikePredicate
                    // value LIKE "value1" : {"value1"}
                    std::auto_ptr<DataArray<std::string> > strValues = imap->values(
                                                query::LikePredicate(query::QueryConstants::getValueAttributeName(), "value1"));
                    ASSERT_EQ(1, (int)strValues->size());
                    ASSERT_NE((const std::string *)NULL, strValues->get(0));
                    ASSERT_EQ("value1", *strValues->get(0));

                    // ILikePredicate
                    // value ILIKE "%VALue%1%" : {"myvalue_111_test", "value1", "value10", "value11"}
                    strValues = imap->values(
                            query::ILikePredicate(query::QueryConstants::getValueAttributeName(), "%VALue%1%"));
                    ASSERT_EQ(4, (int)strValues->size());
                    std::vector<std::string> actualStrs;
                    for (int i = 0; i < (int)strValues->size(); ++i) {
                        ASSERT_NE((const std::string *)NULL, strValues->get(i));
                        actualStrs.push_back(*strValues->get(i));
                    }
                    std::sort(actualStrs.begin(), actualStrs.end());
                    ASSERT_EQ("myvalue_111_test", actualStrs[0]);
                    ASSERT_EQ("value1", actualStrs[1]);
                    ASSERT_EQ("value10", actualStrs[2]);
                    ASSERT_EQ("value11", actualStrs[3]);

                    // value ILIKE "%VAL%2%" : {"myvalue_22_test", "value2"}
                    strValues = imap->values(query::ILikePredicate(query::QueryConstants::getValueAttributeName(), "%VAL%2%"));
                    ASSERT_EQ(2, (int)strValues->size());
                    actualStrs.clear();
                    for (int i = 0; i < (int)strValues->size(); ++i) {
                        ASSERT_NE((const std::string *)NULL, strValues->get(i));
                        actualStrs.push_back(*strValues->get(i));
                    }
                    std::sort(actualStrs.begin(), actualStrs.end());
                    ASSERT_EQ("myvalue_22_test", actualStrs[0]);
                    ASSERT_EQ("value2", actualStrs[1]);

                    // SqlPredicate
                    // __key BETWEEN 4 and 7 : {4, 5, 6, 7} -> {8, 10, 12, 14}
                    char sql[100];
                    util::snprintf(sql, 50, "%s BETWEEN 4 and 7", query::QueryConstants::getKeyAttributeName());
                    values = intMap.values(query::SqlPredicate(sql));
                    ASSERT_EQ(4, (int)values->size());
                    actualValues.clear();
                    for (int i = 0; i < (int)values->size(); ++i) {
                        ASSERT_NE((const int *)NULL, values->get(i));
                        actualValues.push_back(*values->get(i));
                    }
                    std::sort(actualValues.begin(), actualValues.end());
                    for (int i = 0; i < 4; ++i) {
                        ASSERT_EQ(2 * (i + 4), actualValues[i]);
                    }

                    // RegexPredicate
                    // value matches the regex ".*value.*2.*" : {myvalue_22_test, value2}
                    strValues = imap->values(
                            query::RegexPredicate(query::QueryConstants::getValueAttributeName(), ".*value.*2.*"));
                    ASSERT_EQ(2, (int)strValues->size());
                    actualStrs.clear();
                    for (int i = 0; i < (int)strValues->size(); ++i) {
                        ASSERT_NE((const std::string *)NULL, strValues->get(i));
                        actualStrs.push_back(*strValues->get(i));
                    }
                    std::sort(actualStrs.begin(), actualStrs.end());
                    ASSERT_EQ("myvalue_22_test", actualStrs[0]);
                    ASSERT_EQ("value2", actualStrs[1]);
                }

                TEST_F(RawPointerMapTest, testValuesWithPagingPredicate) {
                    IMap<int, int> basicIntMap = client->getMap<int, int>("testValuesWithPredicateIntMap");
                    client::adaptor::RawPointerMap<int, int> intMap(basicIntMap);

                    int predSize = 5;
                    const int totalEntries = 25;

                    for (int i = 0; i < totalEntries; ++i) {
                        intMap.put(i, i);
                    }

                    query::PagingPredicate<int, int> predicate((size_t)predSize);

                    std::auto_ptr<DataArray<int> > values = intMap.values(predicate);
                    ASSERT_EQ(predSize, (int) values->size());
                    for (int i = 0; i < predSize; ++i) {
                        ASSERT_NE((const int *)NULL, values->get(i));
                        ASSERT_EQ(i, *values->get(i));
                    }

                    values = intMap.values(predicate);
                    ASSERT_EQ(predSize, (int) values->size());
                    for (int i = 0; i < predSize; ++i) {
                        ASSERT_NE((const int *)NULL, values->get(i));
                        ASSERT_EQ(i, *values->get(i));
                    }

                    predicate.nextPage();
                    values = intMap.values(predicate);
                    ASSERT_EQ(predSize, (int) values->size());

                    for (int i = 0; i < predSize; ++i) {
                        ASSERT_NE((const int *)NULL, values->get(i));
                        ASSERT_EQ(predSize + i, *values->get(i));
                    }

                    const std::pair<int *, int *> *anchor = predicate.getAnchor();
                    ASSERT_NE((const std::pair<int *, int *> *) NULL, anchor);
                    ASSERT_NE((int *) NULL, anchor->first);
                    ASSERT_NE((int *) NULL, anchor->second);
                    ASSERT_EQ(9, *anchor->first);
                    ASSERT_EQ(9, *anchor->second);

                    ASSERT_EQ(1, (int)predicate.getPage());

                    predicate.setPage(4);

                    values = intMap.values(predicate);
                    ASSERT_EQ(predSize, (int) values->size());

                    for (int i = 0; i < predSize; ++i) {
                        ASSERT_NE((const int *)NULL, values->get(i));
                        ASSERT_EQ(predSize * 4 + i, *values->get(i));
                    }

                    anchor = predicate.getAnchor();
                    ASSERT_NE((const std::pair<int *, int *> *) NULL, anchor);
                    ASSERT_NE((int *) NULL, anchor->first);
                    ASSERT_NE((int *) NULL, anchor->second);
                    ASSERT_EQ(24, *anchor->first);
                    ASSERT_EQ(24, *anchor->second);

                    const std::pair<size_t, std::pair<int *, int *> > *anchorEntry = predicate.getNearestAnchorEntry();
                    ASSERT_NE((const std::pair<size_t, std::pair<int *, int *> > *) NULL, anchorEntry);
                    ASSERT_NE((int *) NULL, anchorEntry->second.first);
                    ASSERT_NE((int *) NULL, anchorEntry->second.second);
                    ASSERT_EQ(3, (int)anchorEntry->first);
                    ASSERT_EQ(19, *anchorEntry->second.first);
                    ASSERT_EQ(19, *anchorEntry->second.second);

                    predicate.nextPage();
                    values = intMap.values(predicate);
                    ASSERT_EQ(0, (int) values->size());

                    predicate.setPage(0);
                    values = intMap.values(predicate);
                    ASSERT_EQ(predSize, (int) values->size());
                    for (int i = 0; i < predSize; ++i) {
                        ASSERT_NE((const int *)NULL, values->get(i));
                        ASSERT_EQ(i, *values->get(i));
                    }

                    predicate.previousPage();
                    ASSERT_EQ(0, (int)predicate.getPage());

                    predicate.setPage(5);
                    values = intMap.values(predicate);
                    ASSERT_EQ(0, (int) values->size());

                    predicate.setPage(3);
                    values = intMap.values(predicate);
                    ASSERT_EQ(predSize, (int) values->size());
                    for (int i = 0; i < predSize; ++i) {
                        ASSERT_NE((const int *)NULL, values->get(i));
                        ASSERT_EQ(3 * predSize + i, *values->get(i));
                    }

                    predicate.previousPage();
                    values = intMap.values(predicate);
                    ASSERT_EQ(predSize, (int) values->size());
                    for (int i = 0; i < predSize; ++i) {
                        ASSERT_NE((const int *)NULL, values->get(i));
                        ASSERT_EQ(2 * predSize + i, *values->get(i));
                    }

                    // test PagingPredicate with inner predicate (value < 10)
                    std::auto_ptr<query::Predicate> lessThanTenPredicate(std::auto_ptr<query::Predicate>(
                            new query::GreaterLessPredicate<int>(query::QueryConstants::getValueAttributeName(), 9, false,
                                                                 true)));
                    query::PagingPredicate<int, int> predicate2(lessThanTenPredicate, 5);
                    values = intMap.values(predicate2);
                    ASSERT_EQ(predSize, (int) values->size());
                    for (int i = 0; i < predSize; ++i) {
                        ASSERT_NE((const int *)NULL, values->get(i));
                        ASSERT_EQ(i, *values->get(i));
                    }

                    predicate2.nextPage();
                    // match values 5,6, 7, 8
                    values = intMap.values(predicate2);
                    ASSERT_EQ(predSize - 1, (int) values->size());
                    for (int i = 0; i < predSize - 1; ++i) {
                        ASSERT_NE((const int *)NULL, values->get(i));
                        ASSERT_EQ(predSize + i, *values->get(i));
                    }

                    predicate2.nextPage();
                    values = intMap.values(predicate2);
                    ASSERT_EQ(0, (int) values->size());

                    // test paging predicate with comparator
                    IMap<int, Employee> employeesOriginal = client->getMap<int, Employee>("testComplexObjectWithPagingPredicate");
                    client::adaptor::RawPointerMap<int, Employee> employees(employeesOriginal);

                    Employee empl1("ahmet", 35);
                    Employee empl2("mehmet", 21);
                    Employee empl3("deniz", 25);
                    Employee empl4("ali", 33);
                    Employee empl5("veli", 44);
                    Employee empl6("aylin", 5);

                    employees.put(3, empl1);
                    employees.put(4, empl2);
                    employees.put(5, empl3);
                    employees.put(6, empl4);
                    employees.put(7, empl5);
                    employees.put(8, empl6);

                    predSize = 2;
                    query::PagingPredicate<int, Employee> predicate3(
                            std::auto_ptr<query::EntryComparator<int, Employee> >(new EmployeeEntryComparator()), (size_t) predSize);
                    std::auto_ptr<DataArray<Employee> > result = employees.values(predicate3);
                    ASSERT_EQ(2, (int) result->size());
                    ASSERT_NE((const Employee *)NULL, (*result)[0]);
                    ASSERT_NE((const Employee *)NULL, (*result)[1]);
                    ASSERT_EQ(empl6, *((*result)[0]));
                    ASSERT_EQ(empl2, *result->get(1));

                    predicate3.nextPage();
                    result = employees.values(predicate3);
                    ASSERT_EQ(2, (int) result->size());
                    ASSERT_NE((const Employee *)NULL, (*result)[0]);
                    ASSERT_NE((const Employee *)NULL, (*result)[1]);
                    ASSERT_EQ(empl3, *((*result)[0]));
                    ASSERT_EQ(empl4, *result->get(1));
                }

                TEST_F(RawPointerMapTest, testReplace) {
                    std::auto_ptr<std::string> temp = imap->replace("key1", "value");
                    ASSERT_EQ(temp.get(), (std::string *) NULL);

                    std::string tempKey = "key1";
                    std::string tempValue = "value1";
                    imap->put(tempKey, tempValue);

                    ASSERT_EQ("value1", *(imap->replace("key1", "value2")));
                    ASSERT_EQ("value2", *(imap->get("key1")));

                    ASSERT_EQ(false, imap->replace("key1", "value1", "value3"));
                    ASSERT_EQ("value2", *(imap->get("key1")));

                    ASSERT_EQ(true, imap->replace("key1", "value2", "value3"));
                    ASSERT_EQ("value3", *(imap->get("key1")));
                }

                class SampleEntryListenerForPortableKey : public EntryAdapter<Employee, int> {
                public:
                    SampleEntryListenerForPortableKey(util::CountDownLatch &latch, util::AtomicInt &atomicInteger)
                            : latch(latch), atomicInteger(atomicInteger) {

                    }

                    void entryAdded(const EntryEvent<Employee, int> &event) {
                        ++atomicInteger;
                        latch.countDown();
                    }

                private:
                    util::CountDownLatch &latch;
                    util::AtomicInt &atomicInteger;
                };


                TEST_F(RawPointerMapTest, testPredicateListenerWithPortableKey) {
                    IMap<Employee, int> map = client->getMap<Employee, int>("tradeMap");
                    hazelcast::client::adaptor::RawPointerMap<Employee, int> tradeMap(map);
                    util::CountDownLatch countDownLatch(1);
                    util::AtomicInt atomicInteger(0);
                    SampleEntryListenerForPortableKey listener(countDownLatch, atomicInteger);
                    Employee key("a", 1);
                    std::string id = tradeMap.addEntryListener(listener, key, true);
                    Employee key2("a", 2);
                    tradeMap.put(key2, 1);
                    tradeMap.put(key, 3);
                    ASSERT_TRUE(countDownLatch.await(5));
                    ASSERT_EQ(1, (int) atomicInteger);

                    ASSERT_TRUE(tradeMap.removeEntryListener(id));
                }

                TEST_F(RawPointerMapTest, testListener) {
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

                    ASSERT_TRUE(latch1Add.await(10));
                    ASSERT_TRUE(latch1Remove.await(10));
                    ASSERT_TRUE(latch2Add.await(5));
                    ASSERT_TRUE(latch2Remove.await(5));

                    ASSERT_TRUE(imap->removeEntryListener(listener1ID));
                    ASSERT_TRUE(imap->removeEntryListener(listener2ID));

                }

                TEST_F(RawPointerMapTest, testClearEvent) {
                    util::CountDownLatch latch(1);
                    ClearListener clearListener(latch);
                    std::string listenerId = imap->addEntryListener(clearListener, false);
                    imap->put("key1", "value1");
                    imap->clear();
                    ASSERT_TRUE(latch.await(120));
                    imap->removeEntryListener(listenerId);
                }

                TEST_F(RawPointerMapTest, testEvictAllEvent) {
                    util::CountDownLatch latch(1);
                    EvictListener evictListener(latch);
                    std::string listenerId = imap->addEntryListener(evictListener, false);
                    imap->put("key1", "value1");
                    imap->evictAll();
                    ASSERT_TRUE(latch.await(120));
                    imap->removeEntryListener(listenerId);
                }

                TEST_F(RawPointerMapTest, testBasicPredicate) {
                    fillMap();

                    query::SqlPredicate predicate("this = 'value1'");
                    std::auto_ptr<hazelcast::client::DataArray<std::string> > tempArray = imap->values(predicate);

                    std::auto_ptr<std::string> actualVal = tempArray->release(0);
                    ASSERT_NE((std::string *)NULL, actualVal.get());
                    ASSERT_EQ("value1", *actualVal);

                    std::auto_ptr<hazelcast::client::DataArray<std::string> > tempArray2 = imap->keySet(predicate);

                    const std::string *actual = (*tempArray2)[0];
                    ASSERT_NE((std::string *)NULL, actual);
                    ASSERT_EQ("key1", *actual);


                    std::auto_ptr<hazelcast::client::EntryArray<std::string, std::string> > tempArray3 = imap->entrySet(predicate);
                    actual = tempArray3->getKey(0);
                    ASSERT_NE((std::string *)NULL, actual);
                    ASSERT_EQ("key1", *actual);

                    actual = tempArray3->getValue(0);
                    ASSERT_NE((std::string *)NULL, actual);
                    ASSERT_EQ("value1", *actual);
                }

                TEST_F(RawPointerMapTest, testKeySetAndValuesWithPredicates) {
                    std::string name = "testKeysetAndValuesWithPredicates";
                    IMap<Employee, Employee> mapOriginal = client->getMap<Employee, Employee>(name);
                    hazelcast::client::adaptor::RawPointerMap<Employee, Employee> map(mapOriginal);

                    Employee emp1("abc-123-xvz", 34);
                    Employee emp2("abc-123-xvz", 20);

                    map.put(emp1, emp1);
                    ASSERT_EQ(map.put(emp2, emp2).get(), (Employee *) NULL);
                    ASSERT_EQ(2, (int) map.size());
                    ASSERT_EQ(2, (int) map.keySet()->size());
                    query::SqlPredicate predicate("a = 10");
                    ASSERT_EQ(0, (int) map.keySet(predicate)->size());
                    query::SqlPredicate predicate2("a = 10");
                    ASSERT_EQ(0, (int) map.values(predicate2)->size());
                    query::SqlPredicate predicate3("a >= 10");
                    ASSERT_EQ(2, (int) map.keySet(predicate3)->size());
                    ASSERT_EQ(2, (int) map.values(predicate3)->size());
                    ASSERT_EQ(2, (int) map.size());
                    ASSERT_EQ(2, (int) map.values()->size());
                }

                TEST_F(RawPointerMapTest, testMapWithPortable) {
                    IMap<int, Employee> employeesMap = client->getMap<int, Employee>("employees");
                    hazelcast::client::adaptor::RawPointerMap<int, Employee> employees(employeesMap);
                    std::auto_ptr<Employee> n1 = employees.get(1);
                    ASSERT_EQ(n1.get(), (Employee *) NULL);
                    Employee employee("sancar", 24);
                    std::auto_ptr<Employee> ptr = employees.put(1, employee);
                    ASSERT_EQ(ptr.get(), (Employee *) NULL);
                    ASSERT_FALSE(employees.isEmpty());
                    std::auto_ptr<hazelcast::client::MapEntryView<int, Employee> > view = employees.getEntryView(1);
                    ASSERT_EQ(*(view->getValue()), employee);
                    ASSERT_EQ(*(view->getKey()), 1);

                    employees.addIndex("a", true);
                    employees.addIndex("n", false);
                }


                TEST_F(RawPointerMapTest, testMapStoreRelatedRequests) {
                    imap->putTransient("ali", "veli", 1100);
                    imap->flush();
                    ASSERT_EQ(1, imap->size());
                    ASSERT_FALSE(imap->evict("deli"));
                    ASSERT_TRUE(imap->evict("ali"));
                    ASSERT_EQ(imap->get("ali").get(), (std::string *) NULL);
                }

                class EntryMultiplier : public serialization::IdentifiedDataSerializable {
                public:
                    EntryMultiplier(int multiplier) : multiplier(multiplier) { }

                    /**
                     * @return factory id
                     */
                    int getFactoryId() const {
                        return 666;
                    }

                    /**
                     * @return class id
                     */
                    int getClassId() const {
                        return 3;
                    }

                    /**
                     * Defines how this class will be written.
                     * @param writer ObjectDataOutput
                     */
                    void writeData(serialization::ObjectDataOutput &writer) const {
                        writer.writeInt(multiplier);
                    }

                    /**
                     *Defines how this class will be read.
                     * @param reader ObjectDataInput
                     */
                    void readData(serialization::ObjectDataInput &reader) {
                        multiplier = reader.readInt();
                    }

                    int getMultiplier() const {
                        return multiplier;
                    }

                private:
                    int multiplier;
                };

                TEST_F(RawPointerMapTest, testExecuteOnKey) {
                    IMap<int, Employee> employeesMap = client->getMap<int, Employee>("executeOnKey");
                    hazelcast::client::adaptor::RawPointerMap<int, Employee> employees(employeesMap);

                    Employee empl1("ahmet", 35);
                    Employee empl2("mehmet", 21);

                    employees.put(3, empl1);
                    employees.put(4, empl2);

                    EntryMultiplier processor(4);

                    std::auto_ptr<int> result = employees.executeOnKey<int, EntryMultiplier>(4, processor);

                    ASSERT_NE((int *) NULL, result.get());
                    ASSERT_EQ(4 * processor.getMultiplier(), *result);
                }

                TEST_F(RawPointerMapTest, testExecuteOnEntries) {
                    IMap<int, Employee> employeesMap = client->getMap<int, Employee>("testExecuteOnEntries");
                    hazelcast::client::adaptor::RawPointerMap<int, Employee> employees(employeesMap);

                    Employee empl1("ahmet", 35);
                    Employee empl2("mehmet", 21);
                    Employee empl3("deniz", 25);

                    employees.put(3, empl1);
                    employees.put(4, empl2);
                    employees.put(5, empl3);

                    EntryMultiplier processor(4);

                    std::auto_ptr<hazelcast::client::EntryArray<int, int> > result = employees.executeOnEntries<int, EntryMultiplier>(
                            processor);

                    ASSERT_EQ(3, (int) result->size());
                    for (size_t i = 0;i < result->size();++i) {
                        const int *key = result->getKey(i);
                        const int *value = result->getValue(i);
                        ASSERT_TRUE(*key == 3 || *key == 4 || *key == 5);
                        ASSERT_EQ((*key) * processor.getMultiplier(), (*value));
                    }
                }

                TEST_F(RawPointerMapTest, testExecuteOnEntriesWithTruePredicate) {
                    IMap<int, Employee> employeesMap = client->getMap<int, Employee>("executeOnKey");
                    hazelcast::client::adaptor::RawPointerMap<int, Employee> employees(employeesMap);

                    Employee empl1("ahmet", 35);
                    Employee empl2("mehmet", 21);
                    Employee empl3("deniz", 25);

                    employees.put(3, empl1);
                    employees.put(4, empl2);
                    employees.put(5, empl3);

                    EntryMultiplier processor(4);

                    std::auto_ptr<hazelcast::client::EntryArray<int, int> > result =
                            employees.executeOnEntries<int, EntryMultiplier>(processor, query::TruePredicate());

                    ASSERT_EQ(3, (int) result->size());
                    for (size_t i = 0;i < result->size();++i) {
                        const int *key = result->getKey(i);
                        const int *value = result->getValue(i);
                        ASSERT_TRUE(*key == 3 || *key == 4 || *key == 5);
                        ASSERT_EQ((*key) * processor.getMultiplier(), (*value));
                    }
                }

                TEST_F(RawPointerMapTest, testExecuteOnEntriesWithFalsePredicate) {
                    IMap<int, Employee> employeesMap = client->getMap<int, Employee>("testExecuteOnEntries");
                    hazelcast::client::adaptor::RawPointerMap<int, Employee> employees(employeesMap);

                    Employee empl1("ahmet", 35);
                    Employee empl2("mehmet", 21);
                    Employee empl3("deniz", 25);

                    employees.put(3, empl1);
                    employees.put(4, empl2);
                    employees.put(5, empl3);

                    EntryMultiplier processor(4);

                    std::auto_ptr<hazelcast::client::EntryArray<int, int> > result =
                            employees.executeOnEntries<int, EntryMultiplier>(processor, query::FalsePredicate());

                    ASSERT_EQ(0, (int) result->size());
                }

                TEST_F(RawPointerMapTest, testExecuteOnEntriesWithAndPredicate) {
                    IMap<int, Employee> employeesMap = client->getMap<int, Employee>("testExecuteOnEntries");
                    hazelcast::client::adaptor::RawPointerMap<int, Employee> employees(employeesMap);

                    Employee empl1("ahmet", 35);
                    Employee empl2("mehmet", 21);
                    Employee empl3("deniz", 25);

                    employees.put(3, empl1);
                    employees.put(4, empl2);
                    employees.put(5, empl3);

                    query::AndPredicate andPredicate;
                    /* 25 <= age <= 35 AND age = 35 */
                    andPredicate.add(
                            std::auto_ptr<query::Predicate>(new query::BetweenPredicate<int>("a", 25, 35))).add(
                            std::auto_ptr<query::Predicate>(
                                    new query::NotPredicate(
                                            std::auto_ptr<query::Predicate>(new query::EqualPredicate<int>("a", 35)))));

                    EntryMultiplier processor(4);

                    std::auto_ptr<hazelcast::client::EntryArray<int, int> > result = employees.executeOnEntries<int, EntryMultiplier>(
                            processor, andPredicate);

                    ASSERT_EQ(1, (int) result->size());
                    ASSERT_EQ(5, *result->getKey(0));
                    ASSERT_EQ(5 * processor.getMultiplier(), *result->getValue(0));
                }

                TEST_F(RawPointerMapTest, testExecuteOnEntriesWithOrPredicate) {
                    IMap<int, Employee> employeesMap = client->getMap<int, Employee>("testExecuteOnEntries");
                    hazelcast::client::adaptor::RawPointerMap<int, Employee> employees(employeesMap);

                    Employee empl1("ahmet", 35);
                    Employee empl2("mehmet", 21);
                    Employee empl3("deniz", 25);

                    employees.put(3, empl1);
                    employees.put(4, empl2);
                    employees.put(5, empl3);

                    query::OrPredicate orPredicate;
                    /* age == 21 OR age > 25 */
                    orPredicate.add(
                            std::auto_ptr<query::Predicate>(new query::EqualPredicate<int>("a", 21))).add(
                            std::auto_ptr<query::Predicate>(new query::GreaterLessPredicate<int>("a", 25, false, false)));

                    EntryMultiplier processor(4);

                    std::auto_ptr<hazelcast::client::EntryArray<int, int> > result = employees.executeOnEntries<int, EntryMultiplier>(
                            processor, orPredicate);

                    ASSERT_EQ(2, (int) result->size());
                    if (3 == *result->getKey(0)) {
                        ASSERT_EQ(3 * processor.getMultiplier(), *result->getValue(0));
                        ASSERT_EQ(4, *result->getKey(1));
                        ASSERT_EQ(4 * processor.getMultiplier(), *result->getValue(1));
                    } else {
                        ASSERT_EQ(4, *result->getKey(0));
                        ASSERT_EQ(4 * processor.getMultiplier(), *result->getValue(0));
                        ASSERT_EQ(3, *result->getKey(1));
                        ASSERT_EQ(3 * processor.getMultiplier(), *result->getValue(1));
                    }
                }

                TEST_F(RawPointerMapTest, testExecuteOnEntriesWithBetweenPredicate) {
                    IMap<int, Employee> employeesMap = client->getMap<int, Employee>("testExecuteOnEntries");
                    hazelcast::client::adaptor::RawPointerMap<int, Employee> employees(employeesMap);

                    Employee empl1("ahmet", 35);
                    Employee empl2("mehmet", 21);
                    Employee empl3("deniz", 25);

                    employees.put(3, empl1);
                    employees.put(4, empl2);
                    employees.put(5, empl3);

                    EntryMultiplier processor(4);

                    std::auto_ptr<hazelcast::client::EntryArray<int, int> > result = employees.executeOnEntries<int, EntryMultiplier>(
                            processor, query::BetweenPredicate<int>("a", 25, 35));

                    ASSERT_EQ(2, (int) result->size());
                    if (3 == *result->getKey(0)) {
                        ASSERT_EQ(3 * processor.getMultiplier(), *result->getValue(0));
                        ASSERT_EQ(5, *result->getKey(1));
                        ASSERT_EQ(5 * processor.getMultiplier(), *result->getValue(1));
                    } else {
                        ASSERT_EQ(5, *result->getKey(0));
                        ASSERT_EQ(5 * processor.getMultiplier(), *result->getValue(0));
                        ASSERT_EQ(3, *result->getKey(1));
                        ASSERT_EQ(3 * processor.getMultiplier(), *result->getValue(1));
                    }
                }

                TEST_F(RawPointerMapTest, testExecuteOnEntriesWithEqualPredicate) {
                    IMap<int, Employee> employeesMap = client->getMap<int, Employee>("testExecuteOnEntries");
                    hazelcast::client::adaptor::RawPointerMap<int, Employee> employees(employeesMap);

                    Employee empl1("ahmet", 35);
                    Employee empl2("mehmet", 21);
                    Employee empl3("deniz", 25);

                    employees.put(3, empl1);
                    employees.put(4, empl2);
                    employees.put(5, empl3);

                    EntryMultiplier processor(4);

                    std::auto_ptr<hazelcast::client::EntryArray<int, int> > result = employees.executeOnEntries<int, EntryMultiplier>(
                            processor, query::EqualPredicate<int>("a", 25));

                    ASSERT_EQ(1, (int) result->size());
                    ASSERT_EQ(5, *result->getKey(0));
                    ASSERT_EQ(5 * processor.getMultiplier(), *result->getValue(0));
                }

                TEST_F(RawPointerMapTest, testExecuteOnEntriesWithNotEqualPredicate) {
                    IMap<int, Employee> employeesMap = client->getMap<int, Employee>("testExecuteOnEntries");
                    hazelcast::client::adaptor::RawPointerMap<int, Employee> employees(employeesMap);

                    Employee empl1("ahmet", 35);
                    Employee empl2("mehmet", 21);
                    Employee empl3("deniz", 25);

                    employees.put(3, empl1);
                    employees.put(4, empl2);
                    employees.put(5, empl3);

                    EntryMultiplier processor(4);

                    std::auto_ptr<hazelcast::client::EntryArray<int, int> > result = employees.executeOnEntries<int, EntryMultiplier>(
                            processor, query::NotEqualPredicate<int>("a", 25));

                    ASSERT_EQ(2, (int) result->size());
                    if (3 == *result->getKey(0)) {
                        ASSERT_EQ(3 * processor.getMultiplier(), *result->getValue(0));
                        ASSERT_EQ(4, *result->getKey(1));
                        ASSERT_EQ(4 * processor.getMultiplier(), *result->getValue(1));
                    } else {
                        ASSERT_EQ(4, *result->getKey(0));
                        ASSERT_EQ(4 * processor.getMultiplier(), *result->getValue(0));
                        ASSERT_EQ(3, *result->getKey(1));
                        ASSERT_EQ(3 * processor.getMultiplier(), *result->getValue(1));
                    }
                }

                TEST_F(RawPointerMapTest, testExecuteOnEntriesWithGreaterLessPredicate) {
                    IMap<int, Employee> employeesMap = client->getMap<int, Employee>("testExecuteOnEntries");
                    hazelcast::client::adaptor::RawPointerMap<int, Employee> employees(employeesMap);

                    Employee empl1("ahmet", 35);
                    Employee empl2("mehmet", 21);
                    Employee empl3("deniz", 25);

                    employees.put(3, empl1);
                    employees.put(4, empl2);
                    employees.put(5, empl3);

                    EntryMultiplier processor(4);

                    std::auto_ptr<hazelcast::client::EntryArray<int, int> > result = employees.executeOnEntries<int, EntryMultiplier>(
                            processor, query::GreaterLessPredicate<int>("a", 25, false, true)); // <25 matching

                    ASSERT_EQ(1, (int) result->size());
                    ASSERT_EQ(4, *result->getKey(0));
                    ASSERT_EQ(4 * processor.getMultiplier(), *result->getValue(0));

                    result = employees.executeOnEntries<int, EntryMultiplier>(
                            processor, query::GreaterLessPredicate<int>("a", 25, true, true)); // <=25 matching

                    ASSERT_EQ(2, (int) result->size());
                    if (4 == *result->getKey(0)) {
                        ASSERT_EQ(4 * processor.getMultiplier(), *result->getValue(0));
                        ASSERT_EQ(5, *result->getKey(1));
                        ASSERT_EQ(5 * processor.getMultiplier(), *result->getValue(1));
                    } else {
                        ASSERT_EQ(5, *result->getKey(0));
                        ASSERT_EQ(5 * processor.getMultiplier(), *result->getValue(0));
                        ASSERT_EQ(4, *result->getKey(1));
                        ASSERT_EQ(4 * processor.getMultiplier(), *result->getValue(1));
                    }

                    result = employees.executeOnEntries<int, EntryMultiplier>(
                            processor, query::GreaterLessPredicate<int>("a", 25, false, false)); // >25 matching

                    ASSERT_EQ(1, (int) result->size());
                    ASSERT_EQ(3, *result->getKey(0));
                    ASSERT_EQ(3 * processor.getMultiplier(), *result->getValue(0));

                    result = employees.executeOnEntries<int, EntryMultiplier>(
                            processor, query::GreaterLessPredicate<int>("a", 25, true, false)); // >=25 matching

                    ASSERT_EQ(2, (int) result->size());
                    if (3 == *result->getKey(0)) {
                        ASSERT_EQ(3 * processor.getMultiplier(), *result->getValue(0));
                        ASSERT_EQ(5, *result->getKey(1));
                        ASSERT_EQ(5 * processor.getMultiplier(), *result->getValue(1));
                    } else {
                        ASSERT_EQ(5, *result->getKey(0));
                        ASSERT_EQ(5 * processor.getMultiplier(), *result->getValue(0));
                        ASSERT_EQ(3, *result->getKey(1));
                        ASSERT_EQ(3 * processor.getMultiplier(), *result->getValue(1));
                    }
                }

                TEST_F(RawPointerMapTest, testExecuteOnEntriesWithLikePredicate) {
                    IMap<int, Employee> employeesMap = client->getMap<int, Employee>("testExecuteOnEntries");
                    hazelcast::client::adaptor::RawPointerMap<int, Employee> employees(employeesMap);

                    Employee empl1("ahmet", 35);
                    Employee empl2("mehmet", 21);
                    Employee empl3("deniz", 25);

                    employees.put(3, empl1);
                    employees.put(4, empl2);
                    employees.put(5, empl3);

                    EntryMultiplier processor(4);

                    std::auto_ptr<hazelcast::client::EntryArray<int, int> > result = employees.executeOnEntries<int, EntryMultiplier>(
                            processor, query::LikePredicate("n", "deniz"));

                    ASSERT_EQ(1, (int) result->size());
                    ASSERT_EQ(5, *result->getKey(0));
                    ASSERT_EQ(5 * processor.getMultiplier(), *result->getValue(0));
                }

                TEST_F(RawPointerMapTest, testExecuteOnEntriesWithILikePredicate) {
                    IMap<int, Employee> employeesMap = client->getMap<int, Employee>("testExecuteOnEntries");
                    hazelcast::client::adaptor::RawPointerMap<int, Employee> employees(employeesMap);


                    Employee empl1("ahmet", 35);
                    Employee empl2("mehmet", 21);
                    Employee empl3("deniz", 25);

                    employees.put(3, empl1);
                    employees.put(4, empl2);
                    employees.put(5, empl3);

                    EntryMultiplier processor(4);

                    std::auto_ptr<hazelcast::client::EntryArray<int, int> > result = employees.executeOnEntries<int, EntryMultiplier>(
                            processor, query::ILikePredicate("n", "deniz"));

                    ASSERT_EQ(1, (int) result->size());
                    ASSERT_EQ(5, *result->getKey(0));
                    ASSERT_EQ(5 * processor.getMultiplier(), *result->getValue(0));
                }

                TEST_F(RawPointerMapTest, testExecuteOnEntriesWithInPredicate) {
                    IMap<int, Employee> employeesMap = client->getMap<int, Employee>("testExecuteOnEntries");
                    hazelcast::client::adaptor::RawPointerMap<int, Employee> employees(employeesMap);

                    Employee empl1("ahmet", 35);
                    Employee empl2("mehmet", 21);
                    Employee empl3("deniz", 25);

                    employees.put(3, empl1);
                    employees.put(4, empl2);
                    employees.put(5, empl3);

                    EntryMultiplier processor(4);

                    std::vector<std::string> values;
                    values.push_back("ahmet");
                    query::InPredicate<std::string> predicate("n", values);
                    predicate.add("mehmet");
                    std::auto_ptr<hazelcast::client::EntryArray<int, int> > result = employees.executeOnEntries<int, EntryMultiplier>(
                            processor, predicate);

                    ASSERT_EQ(2, (int) result->size());
                    if (3 == *result->getKey(0)) {
                        ASSERT_EQ(3 * processor.getMultiplier(), *result->getValue(0));
                        ASSERT_EQ(4, *result->getKey(1));
                        ASSERT_EQ(4 * processor.getMultiplier(), *result->getValue(1));
                    } else {
                        ASSERT_EQ(4, *result->getKey(0));
                        ASSERT_EQ(4 * processor.getMultiplier(), *result->getValue(0));
                        ASSERT_EQ(3, *result->getKey(1));
                        ASSERT_EQ(3 * processor.getMultiplier(), *result->getValue(1));
                    }
                }

                TEST_F(RawPointerMapTest, testExecuteOnEntriesWithInstanceOfPredicate) {
                    IMap<int, Employee> employeesMap = client->getMap<int, Employee>("testExecuteOnEntries");
                    hazelcast::client::adaptor::RawPointerMap<int, Employee> employees(employeesMap);

                    Employee empl1("ahmet", 35);
                    Employee empl2("mehmet", 21);
                    Employee empl3("deniz", 25);

                    employees.put(3, empl1);
                    employees.put(4, empl2);
                    employees.put(5, empl3);

                    EntryMultiplier processor(4);
                    std::auto_ptr<hazelcast::client::EntryArray<int, int> > result = employees.executeOnEntries<int, EntryMultiplier>(
                            processor, query::InstanceOfPredicate("Employee"));

                    ASSERT_EQ(3, (int) result->size());
                }

                TEST_F(RawPointerMapTest, testExecuteOnEntriesWithNotPredicate) {
                    IMap<int, Employee> employeesMap = client->getMap<int, Employee>("testExecuteOnEntries");
                    hazelcast::client::adaptor::RawPointerMap<int, Employee> employees(employeesMap);

                    Employee empl1("ahmet", 35);
                    Employee empl2("mehmet", 21);
                    Employee empl3("deniz", 25);

                    employees.put(3, empl1);
                    employees.put(4, empl2);
                    employees.put(5, empl3);

                    EntryMultiplier processor(4);
                    query::NotPredicate notEqualPredicate(std::auto_ptr<query::Predicate>(new query::EqualPredicate<int>("a", 25)));
                    std::auto_ptr<hazelcast::client::EntryArray<int, int> > result =
                            employees.executeOnEntries<int, EntryMultiplier>(processor, notEqualPredicate);

                    ASSERT_EQ(2, (int) result->size());
                    if (3 == *result->getKey(0)) {
                        ASSERT_EQ(3 * processor.getMultiplier(), *result->getValue(0));
                        ASSERT_EQ(4, *result->getKey(1));
                        ASSERT_EQ(4 * processor.getMultiplier(), *result->getValue(1));
                    } else {
                        ASSERT_EQ(4, *result->getKey(0));
                        ASSERT_EQ(4 * processor.getMultiplier(), *result->getValue(0));
                        ASSERT_EQ(3, *result->getKey(1));
                        ASSERT_EQ(3 * processor.getMultiplier(), *result->getValue(1));
                    }

                    query::NotPredicate notFalsePredicate(std::auto_ptr<query::Predicate>(new query::FalsePredicate()));
                    result = employees.executeOnEntries<int, EntryMultiplier>(processor, notFalsePredicate);

                    ASSERT_EQ(3, (int) result->size());

                    query::NotPredicate notBetweenPredicate(
                            std::auto_ptr<query::Predicate>(new query::BetweenPredicate<int>("a", 25, 35)));
                    result = employees.executeOnEntries<int, EntryMultiplier>(processor, notBetweenPredicate);

                    ASSERT_EQ(1, (int) result->size());
                    ASSERT_EQ(4, *result->getKey(0));
                    ASSERT_EQ(4 * processor.getMultiplier(), *result->getValue(0));
                }

                TEST_F(RawPointerMapTest, testExecuteOnEntriesWithRegexPredicate) {
                    IMap<int, Employee> employeesMap = client->getMap<int, Employee>("testExecuteOnEntries");
                    hazelcast::client::adaptor::RawPointerMap<int, Employee> employees(employeesMap);

                    Employee empl1("ahmet", 35);
                    Employee empl2("mehmet", 21);
                    Employee empl3("deniz", 25);

                    employees.put(3, empl1);
                    employees.put(4, empl2);
                    employees.put(5, empl3);

                    EntryMultiplier processor(4);

                    std::auto_ptr<hazelcast::client::EntryArray<int, int> > result = employees.executeOnEntries<int, EntryMultiplier>(
                            processor, query::RegexPredicate("n", ".*met"));

                    ASSERT_EQ(2, (int) result->size());
                    if (3 == *result->getKey(0)) {
                        ASSERT_EQ(3 * processor.getMultiplier(), *result->getValue(0));
                        ASSERT_EQ(4, *result->getKey(1));
                        ASSERT_EQ(4 * processor.getMultiplier(), *result->getValue(1));
                    } else {
                        ASSERT_EQ(4, *result->getKey(0));
                        ASSERT_EQ(4 * processor.getMultiplier(), *result->getValue(0));
                        ASSERT_EQ(3, *result->getKey(1));
                        ASSERT_EQ(3 * processor.getMultiplier(), *result->getValue(1));
                    }
                }
                
            }
        }
    }
}

