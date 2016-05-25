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
#include "hazelcast/client/EntryAdapter.h"
#include "hazelcast/client/EntryEvent.h"

#include "HazelcastServerFactory.h"
#include "serialization/Employee.h"
#include "TestHelperFunctions.h"
#include "ClientTestSupport.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/IMap.h"
#include "HazelcastServer.h"

namespace hazelcast {
    namespace client {
        namespace test {
            class ClientMapTest : public ClientTestSupport {
            protected:
                class MapGetInterceptor : public serialization::IdentifiedDataSerializable {
                public:
                    MapGetInterceptor(const std::string &prefix) : prefix(std::auto_ptr<std::string>(new std::string(prefix))) { }

                    virtual int getFactoryId() const {
                        return 666;
                    }

                    virtual int getClassId() const {
                        return 6;
                    }

                    virtual void writeData(serialization::ObjectDataOutput &writer) const {
                        writer.writeUTF(prefix.get());
                    }

                    virtual void readData(serialization::ObjectDataInput &reader) {
                        prefix = reader.readUTF();
                    }
                private:
                    std::auto_ptr<std::string> prefix;
                };

                virtual void TearDown() {
                    // clear maps
                    employees->clear();
                    intMap->clear();
                    imap->clear();
                }

                static void SetUpTestCase() {
                    instance = new HazelcastServer(*g_srvFactory);
                    instance2 = new HazelcastServer(*g_srvFactory);
                    clientConfig = new ClientConfig();
                    clientConfig->addAddress(Address(g_srvFactory->getServerAddress(), 5701));
                    client = new HazelcastClient(*clientConfig);
                    imap = new IMap<std::string, std::string>(client->getMap<std::string, std::string>("clientMapTest"));
                    intMap = new IMap<int, int>(client->getMap<int, int>("IntMap"));
                    employees = new IMap<int, Employee>(client->getMap<int, Employee>("EmployeesMap"));
                }

                static void TearDownTestCase() {
                    delete employees;
                    delete intMap;
                    delete imap;
                    delete client;
                    delete clientConfig;
                    delete instance2;
                    delete instance;

                    employees = NULL;
                    intMap = NULL;
                    imap = NULL;
                    client = NULL;
                    clientConfig = NULL;
                    instance2 = NULL;
                    instance = NULL;
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

                static HazelcastServer *instance;
                static HazelcastServer *instance2;
                static ClientConfig *clientConfig;
                static HazelcastClient *client;
                static IMap<std::string, std::string> *imap;
                static IMap<int, int> *intMap;
                static IMap<int, Employee> *employees;
            };

            HazelcastServer *ClientMapTest::instance = NULL;
            HazelcastServer *ClientMapTest::instance2 = NULL;
            ClientConfig *ClientMapTest::clientConfig = NULL;
            HazelcastClient *ClientMapTest::client = NULL;
            IMap<std::string, std::string> *ClientMapTest::imap = NULL;
            IMap<int, int> *ClientMapTest::intMap = NULL;
            IMap<int, Employee> *ClientMapTest::employees = NULL;

            void tryPutThread(util::ThreadArgs &args) {
                util::CountDownLatch *latch = (util::CountDownLatch *) args.arg0;
                IMap<std::string, std::string> *imap = (IMap<std::string, std::string> *) args.arg1;
                bool result = imap->tryPut("key1", "value3", 1 * 1000);
                if (!result) {
                    latch->countDown();
                }
            }

            void tryRemoveThread(util::ThreadArgs &args) {
                util::CountDownLatch *latch = (util::CountDownLatch *) args.arg0;
                IMap<std::string, std::string> *imap = (IMap<std::string, std::string> *) args.arg1;
                bool result = imap->tryRemove("key2", 1 * 1000);
                if (!result) {
                    latch->countDown();
                }
            }

            void testLockThread(util::ThreadArgs &args) {
                util::CountDownLatch *latch = (util::CountDownLatch *) args.arg0;
                IMap<std::string, std::string> *imap = (IMap<std::string, std::string> *) args.arg1;
                imap->tryPut("key1", "value2", 1);
                latch->countDown();
            }

            void testLockTTLThread(util::ThreadArgs &args) {
                util::CountDownLatch *latch = (util::CountDownLatch *) args.arg0;
                IMap<std::string, std::string> *imap = (IMap<std::string, std::string> *) args.arg1;
                imap->tryPut("key1", "value2", 5 * 1000);
                latch->countDown();
            }

            void testLockTTL2Thread(util::ThreadArgs &args) {
                util::CountDownLatch *latch = (util::CountDownLatch *) args.arg0;
                IMap<std::string, std::string> *imap = (IMap<std::string, std::string> *) args.arg1;
                if (!imap->tryLock("key1")) {
                    latch->countDown();
                }
                if (imap->tryLock("key1", 5 * 1000)) {
                    latch->countDown();
                }
            }

            void testMapTryLockThread1(util::ThreadArgs &args) {
                util::CountDownLatch *latch = (util::CountDownLatch *) args.arg0;
                IMap<std::string, std::string> *imap = (IMap<std::string, std::string> *) args.arg1;
                if (!imap->tryLock("key1", 2)) {
                    latch->countDown();
                }
            }

            void testMapTryLockThread2(util::ThreadArgs &args) {
                util::CountDownLatch *latch = (util::CountDownLatch *) args.arg0;
                IMap<std::string, std::string> *imap = (IMap<std::string, std::string> *) args.arg1;
                if (imap->tryLock("key1", 20 * 1000)) {
                    latch->countDown();
                }
            }

            void testMapForceUnlockThread(util::ThreadArgs &args) {
                util::CountDownLatch *latch = (util::CountDownLatch *) args.arg0;
                IMap<std::string, std::string> *imap = (IMap<std::string, std::string> *) args.arg1;
                imap->forceUnlock("key1");
                latch->countDown();
            }

            template <typename K, typename V>
            class CountdownListener : public EntryAdapter<K, V> {
            public:
                CountdownListener(util::CountDownLatch &addLatch, util::CountDownLatch &removeLatch,
                                    util::CountDownLatch &updateLatch, util::CountDownLatch &evictLatch)
                        : addLatch(addLatch), removeLatch(removeLatch), updateLatch(updateLatch),
                          evictLatch(evictLatch) {
                }

                void entryAdded(const EntryEvent<K, V> &event) {
                    std::ostringstream out;
                    out << "[entryAdded] " << event.getKey();
                    util::ILogger::getLogger().info(out.str());

                    addLatch.countDown();
                }

                void entryRemoved(const EntryEvent<K, V> &event) {
                    std::ostringstream out;
                    out << "[entryRemoved] Key:" << event.getKey();
                    util::ILogger::getLogger().info(out.str());

                    removeLatch.countDown();
                }

                void entryUpdated(const EntryEvent<K, V> &event) {
                    std::ostringstream out;
                    out << "[entryUpdated] Key:" << event.getKey();
                    util::ILogger::getLogger().info(out.str());

                    updateLatch.countDown();
                }

                void entryEvicted(const EntryEvent<K, V> &event) {
                    std::ostringstream out;
                    out << "[entryEvicted] Key:" << event.getKey();
                    util::ILogger::getLogger().info(out.str());

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

            TEST_F(ClientMapTest, testIssue537) {
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

            TEST_F(ClientMapTest, testContains) {
                fillMap();

                ASSERT_FALSE(imap->containsKey("key10"));
                ASSERT_TRUE(imap->containsKey("key1"));

                ASSERT_FALSE(imap->containsValue("value10"));
                ASSERT_TRUE(imap->containsValue("value1"));

            }

            TEST_F(ClientMapTest, testGet) {
                fillMap();
                for (int i = 0; i < 10; i++) {
                    std::string key = "key";
                    key += util::IOUtil::to_string(i);
                    boost::shared_ptr<std::string> temp = imap->get(key);

                    std::string value = "value";
                    value += util::IOUtil::to_string(i);
                    ASSERT_EQ(*temp, value);
                }
            }

            TEST_F(ClientMapTest, testRemoveAndDelete) {
                fillMap();
                boost::shared_ptr<std::string> temp = imap->remove("key10");
                ASSERT_EQ(temp.get(), (std::string *) NULL);
                imap->deleteEntry("key9");
                ASSERT_EQ(imap->size(), 9);
                for (int i = 0; i < 9; i++) {
                    std::string key = "key";
                    key += util::IOUtil::to_string(i);
                    boost::shared_ptr<std::string> temp2 = imap->remove(key);
                    std::string value = "value";
                    value += util::IOUtil::to_string(i);
                    ASSERT_EQ(*temp2, value);
                }
                ASSERT_EQ(imap->size(), 0);
            }

            TEST_F(ClientMapTest, testRemoveIfSame) {
                fillMap();

                ASSERT_FALSE(imap->remove("key2", "value"));
                ASSERT_EQ(10, imap->size());

                ASSERT_TRUE((imap->remove("key2", "value2")));
                ASSERT_EQ(9, imap->size());

            }

            TEST_F(ClientMapTest, testGetAllPutAll) {

                std::map<std::string, std::string> mapTemp;

                for (int i = 0; i < 100; i++) {
                    mapTemp[util::IOUtil::to_string(i)] = util::IOUtil::to_string(i);
                }
                imap->putAll(mapTemp);
                ASSERT_EQ(imap->size(), 100);

                for (int i = 0; i < 100; i++) {
                    std::string expected = util::IOUtil::to_string(i);
                    boost::shared_ptr<std::string> actual = imap->get(util::IOUtil::to_string(i));
                    ASSERT_EQ(expected, *actual);
                }

                std::set<std::string> tempSet;
                tempSet.insert(util::IOUtil::to_string(1));
                tempSet.insert(util::IOUtil::to_string(3));

                std::map<std::string, std::string> m2 = imap->getAll(tempSet);

                ASSERT_EQ(2U, m2.size());
                ASSERT_EQ(m2[util::IOUtil::to_string(1)], "1");
                ASSERT_EQ(m2[util::IOUtil::to_string(3)], "3");

            }

            TEST_F(ClientMapTest, testTryPutRemove) {

                ASSERT_TRUE(imap->tryPut("key1", "value1", 1 * 1000));
                ASSERT_TRUE(imap->tryPut("key2", "value2", 1 * 1000));
                imap->lock("key1");
                imap->lock("key2");

                util::CountDownLatch latch(2);

                util::Thread t1(tryPutThread, &latch, imap);
                util::Thread t2(tryRemoveThread, &latch, imap);

                ASSERT_TRUE(latch.await(20));
                ASSERT_EQ("value1", *(imap->get("key1")));
                ASSERT_EQ("value2", *(imap->get("key2")));
                imap->forceUnlock("key1");
                imap->forceUnlock("key2");
            }

            TEST_F(ClientMapTest, testPutTtl) {
                util::CountDownLatch dummy(10);
                util::CountDownLatch evict(1);
                CountdownListener<std::string, std::string> sampleEntryListener(dummy, dummy, dummy, evict);
                std::string id = imap->addEntryListener(sampleEntryListener, false);

                imap->put("key1", "value1", 2000);
                boost::shared_ptr<std::string> temp = imap->get("key1");
                ASSERT_EQ(*temp, "value1");
                ASSERT_TRUE(evict.await(20 * 1000));
                boost::shared_ptr<std::string> temp2 = imap->get("key1");
                ASSERT_EQ(temp2.get(), (std::string *) NULL);

                ASSERT_TRUE(imap->removeEntryListener(id));
            }

            TEST_F(ClientMapTest, testPutIfAbsent) {
                boost::shared_ptr<std::string> o = imap->putIfAbsent("key1", "value1");
                ASSERT_EQ(o.get(), (std::string *) NULL);
                ASSERT_EQ("value1", *(imap->putIfAbsent("key1", "value3")));
            }

            TEST_F(ClientMapTest, testPutIfAbsentTtl) {
                ASSERT_EQ(imap->putIfAbsent("key1", "value1", 1000).get(), (std::string *) NULL);
                ASSERT_EQ("value1", *(imap->putIfAbsent("key1", "value3", 1000)));

                ASSERT_NULL_EVENTUALLY(imap->putIfAbsent("key1", "value3", 1000).get());
                ASSERT_EQ("value3", *(imap->putIfAbsent("key1", "value4", 1000)));
            }

            TEST_F(ClientMapTest, testSet) {
                imap->set("key1", "value1");
                ASSERT_EQ("value1", *(imap->get("key1")));

                imap->set("key1", "value2");
                ASSERT_EQ("value2", *(imap->get("key1")));

                imap->set("key1", "value3", 1000);
                ASSERT_EQ("value3", *(imap->get("key1")));

                ASSERT_NULL_EVENTUALLY(imap->get("key1").get());
            }

            TEST_F(ClientMapTest, testLock) {
                imap->put("key1", "value1");
                ASSERT_EQ("value1", *(imap->get("key1")));
                imap->lock("key1");
                util::CountDownLatch latch(1);
                util::Thread t1(testLockThread, &latch, imap);
                ASSERT_TRUE(latch.await(5));
                ASSERT_EQ("value1", *(imap->get("key1")));
                imap->forceUnlock("key1");

            }

            TEST_F(ClientMapTest, testLockTtl) {
                imap->put("key1", "value1");
                ASSERT_EQ("value1", *(imap->get("key1")));
                imap->lock("key1", 2 * 1000);
                util::CountDownLatch latch(1);
                util::Thread t1(testLockTTLThread, &latch, imap);
                ASSERT_TRUE(latch.await(10));
                ASSERT_FALSE(imap->isLocked("key1"));
                ASSERT_EQ("value2", *(imap->get("key1")));
                imap->forceUnlock("key1");

            }

            TEST_F(ClientMapTest, testLockTtl2) {
                imap->lock("key1", 3 * 1000);
                util::CountDownLatch latch(2);
                util::Thread t1(testLockTTL2Thread, &latch, imap);
                ASSERT_TRUE(latch.await(10));
                imap->forceUnlock("key1");

            }

            TEST_F(ClientMapTest, testTryLock) {

                ASSERT_TRUE(imap->tryLock("key1", 2 * 1000));
                util::CountDownLatch latch(1);
                util::Thread t1(testMapTryLockThread1, &latch, imap);

                ASSERT_TRUE(latch.await(100));

                ASSERT_TRUE(imap->isLocked("key1"));

                util::CountDownLatch latch2(1);
                util::Thread t2(testMapTryLockThread2, &latch2, imap);

                util::sleep(1);
                imap->unlock("key1");
                ASSERT_TRUE(latch2.await(100));
                ASSERT_TRUE(imap->isLocked("key1"));
                imap->forceUnlock("key1");

            }

            TEST_F(ClientMapTest, testForceUnlock) {
                imap->lock("key1");
                util::CountDownLatch latch(1);
                util::Thread t2(testMapForceUnlockThread, &latch, imap);
                ASSERT_TRUE(latch.await(100));
                t2.join();
                ASSERT_FALSE(imap->isLocked("key1"));

            }

            TEST_F(ClientMapTest, testValues) {
                fillMap();
                std::vector<std::string> tempVector;
                query::SqlPredicate predicate("this == value1");
                tempVector = imap->values(predicate);
                ASSERT_EQ(1U, tempVector.size());

                std::vector<std::string>::iterator it = tempVector.begin();
                ASSERT_EQ("value1", *it);
            }

            TEST_F(ClientMapTest, testValuesWithPredicate) {
                const int numItems = 20;
                for (int i = 0; i < numItems; ++i) {
                    intMap->put(i, 2 * i);
                }

                std::vector<int> values = intMap->values();
                ASSERT_EQ(numItems, (int)values.size());
                std::sort(values.begin(), values.end());
                for (int i = 0; i < numItems; ++i) {
                    ASSERT_EQ(2 * i, values[i]);
                }

                // EqualPredicate
                // key == 5
                values = intMap->values(query::EqualPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5));
                ASSERT_EQ(1, (int)values.size());
                ASSERT_EQ(2 * 5, values[0]);

                // value == 8
                values = intMap->values(query::EqualPredicate<int>(query::QueryConstants::getValueAttributeName(), 8));
                ASSERT_EQ(1, (int)values.size());
                ASSERT_EQ(8, values[0]);

                // key == numItems
                values = intMap->values(query::EqualPredicate<int>(query::QueryConstants::getKeyAttributeName(), numItems));
                ASSERT_EQ(0, (int)values.size());

                // NotEqual Predicate
                // key != 5
                values = intMap->values(query::NotEqualPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5));
                ASSERT_EQ(numItems - 1, (int)values.size());
                std::sort(values.begin(), values.end());
                for (int i = 0; i < numItems - 1; ++i) {
                    if (i >= 5) {
                        ASSERT_EQ(2 * (i + 1), values[i]);
                    } else {
                        ASSERT_EQ(2 * i, values[i]);
                    }
                }

                // this(value) != 8
                values = intMap->values(query::NotEqualPredicate<int>(query::QueryConstants::getValueAttributeName(), 8));
                ASSERT_EQ(numItems - 1, (int)values.size());
                std::sort(values.begin(), values.end());
                for (int i = 0; i < numItems - 1; ++i) {
                    if (i >= 4) {
                        ASSERT_EQ(2 * (i + 1), values[i]);
                    } else {
                        ASSERT_EQ(2 * i, values[i]);
                    }
                }

                // TruePredicate
                values = intMap->values(query::TruePredicate());
                ASSERT_EQ(numItems, (int)values.size());
                std::sort(values.begin(), values.end());
                for (int i = 0; i < numItems; ++i) {
                    ASSERT_EQ(2 * i, values[i]);
                }

                // FalsePredicate
                values = intMap->values(query::FalsePredicate());
                ASSERT_EQ(0, (int)values.size());

                // BetweenPredicate
                // 5 <= key <= 10
                values = intMap->values(query::BetweenPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5, 10));
                std::sort(values.begin(), values.end());
                ASSERT_EQ(6, (int)values.size());
                for (int i = 0; i < 6; ++i) {
                    ASSERT_EQ(2 * (i + 5), values[i]);
                }

                // 20 <= key <=30
                values = intMap->values(query::BetweenPredicate<int>(query::QueryConstants::getKeyAttributeName(), 20, 30));
                ASSERT_EQ(0, (int)values.size());

                // GreaterLessPredicate
                // value <= 10
                values = intMap->values(
                        query::GreaterLessPredicate<int>(query::QueryConstants::getValueAttributeName(), 10, true, true));
                ASSERT_EQ(6, (int)values.size());
                std::sort(values.begin(), values.end());
                for (int i = 0; i < 6; ++i) {
                    ASSERT_EQ(2 * i, values[i]);
                }

                // key < 7
                values = intMap->values(
                        query::GreaterLessPredicate<int>(query::QueryConstants::getKeyAttributeName(), 7, false, true));
                ASSERT_EQ(7, (int)values.size());
                std::sort(values.begin(), values.end());
                for (int i = 0; i < 7; ++i) {
                    ASSERT_EQ(2 * i, values[i]);
                }

                // value >= 15
                values = intMap->values(
                        query::GreaterLessPredicate<int>(query::QueryConstants::getValueAttributeName(), 15, true, false));
                ASSERT_EQ(12, (int)values.size());
                std::sort(values.begin(), values.end());
                for (int i = 0; i < 12; ++i) {
                    ASSERT_EQ(2 * (i + 8), values[i]);
                }

                // key > 5
                values = intMap->values(
                        query::GreaterLessPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5, false, false));
                ASSERT_EQ(14, (int)values.size());
                std::sort(values.begin(), values.end());
                for (int i = 0; i < 14; ++i) {
                    ASSERT_EQ(2 * (i + 6), values[i]);
                }

                // InPredicate
                // key in {4, 10, 19}
                std::vector<int> inVals(3);
                inVals[0] = 4;
                inVals[1] = 10;
                inVals[2] = 19;
                values = intMap->values(query::InPredicate<int>(query::QueryConstants::getKeyAttributeName(), inVals));
                ASSERT_EQ(3, (int)values.size());
                std::sort(values.begin(), values.end());
                ASSERT_EQ(2 * 4, values[0]);
                ASSERT_EQ(2 * 10, values[1]);
                ASSERT_EQ(2 * 19, values[2]);

                // value in {4, 10, 19}
                values = intMap->values(query::InPredicate<int>(query::QueryConstants::getValueAttributeName(), inVals));
                ASSERT_EQ(2, (int)values.size());
                std::sort(values.begin(), values.end());
                ASSERT_EQ(4, values[0]);
                ASSERT_EQ(10, values[1]);

                // InstanceOfPredicate
                // value instanceof Integer
                values = intMap->values(query::InstanceOfPredicate("java.lang.Integer"));
                ASSERT_EQ(20, (int)values.size());
                std::sort(values.begin(), values.end());
                for (int i = 0; i < numItems; ++i) {
                    ASSERT_EQ(2 * i, values[i]);
                }

                values = intMap->values(query::InstanceOfPredicate("java.lang.String"));
                ASSERT_EQ(0, (int)values.size());

                // NotPredicate
                // !(5 <= key <= 10)
                std::auto_ptr<query::Predicate> bp = std::auto_ptr<query::Predicate>(new query::BetweenPredicate<int>(
                        query::QueryConstants::getKeyAttributeName(), 5, 10));
                query::NotPredicate notPredicate(bp);
                values = intMap->values(notPredicate);
                ASSERT_EQ(14, (int)values.size());
                std::sort(values.begin(), values.end());
                for (int i = 0; i < 14; ++i) {
                    if (i >= 5) {
                        ASSERT_EQ(2 * (i + 6), values[i]);
                    } else {
                        ASSERT_EQ(2 * i, values[i]);
                    }
                }

                // AndPredicate
                // 5 <= key <= 10 AND Values in {4, 10, 19} = values {4, 10}
                bp = std::auto_ptr<query::Predicate>(
                        new query::BetweenPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5, 10));
                std::auto_ptr<query::Predicate> inPred = std::auto_ptr<query::Predicate>(
                        new query::InPredicate<int>(query::QueryConstants::getValueAttributeName(), inVals));
                values = intMap->values(query::AndPredicate().add(bp).add(inPred));
                ASSERT_EQ(1, (int)values.size());
                std::sort(values.begin(), values.end());
                ASSERT_EQ(10, values[0]);

                // OrPredicate
                // 5 <= key <= 10 OR Values in {4, 10, 19} = values {4, 10, 12, 14, 16, 18, 20}
                bp = std::auto_ptr<query::Predicate>(
                        new query::BetweenPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5, 10));
                inPred = std::auto_ptr<query::Predicate>(
                        new query::InPredicate<int>(query::QueryConstants::getValueAttributeName(), inVals));
                values = intMap->values(query::OrPredicate().add(bp).add(inPred));
                ASSERT_EQ(7, (int)values.size());
                std::sort(values.begin(), values.end());
                ASSERT_EQ(4, values[0]);
                ASSERT_EQ(10, values[1]);
                ASSERT_EQ(12, values[2]);
                ASSERT_EQ(14, values[3]);
                ASSERT_EQ(16, values[4]);
                ASSERT_EQ(18, values[5]);
                ASSERT_EQ(20, values[6]);

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
                std::vector<std::string> strValues = imap->values(
                        query::LikePredicate(query::QueryConstants::getValueAttributeName(), "value1"));
                ASSERT_EQ(1, (int)strValues.size());
                ASSERT_EQ("value1", strValues[0]);

                // ILikePredicate
                // value ILIKE "%VALue%1%" : {"myvalue_111_test", "value1", "value10", "value11"}
                strValues = imap->values(
                        query::ILikePredicate(query::QueryConstants::getValueAttributeName(), "%VALue%1%"));
                ASSERT_EQ(4, (int)strValues.size());
                std::sort(strValues.begin(), strValues.end());
                ASSERT_EQ("myvalue_111_test", strValues[0]);
                ASSERT_EQ("value1", strValues[1]);
                ASSERT_EQ("value10", strValues[2]);
                ASSERT_EQ("value11", strValues[3]);

                // value ILIKE "%VAL%2%" : {"myvalue_22_test", "value2"}
                strValues = imap->values(query::ILikePredicate(query::QueryConstants::getValueAttributeName(), "%VAL%2%"));
                ASSERT_EQ(2, (int)strValues.size());
                std::sort(strValues.begin(), strValues.end());
                ASSERT_EQ("myvalue_22_test", strValues[0]);
                ASSERT_EQ("value2", strValues[1]);

                // SqlPredicate
                // __key BETWEEN 4 and 7 : {4, 5, 6, 7} -> {8, 10, 12, 14}
                char sql[100];
                util::snprintf(sql, 50, "%s BETWEEN 4 and 7", query::QueryConstants::getKeyAttributeName());
                values = intMap->values(query::SqlPredicate(sql));
                ASSERT_EQ(4, (int)values.size());
                std::sort(values.begin(), values.end());
                for (int i = 0; i < 4; ++i) {
                    ASSERT_EQ(2 * (i + 4), values[i]);
                }

                // RegexPredicate
                // value matches the regex ".*value.*2.*" : {myvalue_22_test, value2}
                strValues = imap->values(
                        query::RegexPredicate(query::QueryConstants::getValueAttributeName(), ".*value.*2.*"));
                ASSERT_EQ(2, (int)strValues.size());
                std::sort(strValues.begin(), strValues.end());
                ASSERT_EQ("myvalue_22_test", strValues[0]);
                ASSERT_EQ("value2", strValues[1]);
            }

            TEST_F(ClientMapTest, testValuesWithPagingPredicate) {
                int predSize = 5;
                const int totalEntries = 25;

                for (int i = 0; i < totalEntries; ++i) {
                    intMap->put(i, i);
                }

                query::PagingPredicate<int, int> predicate((size_t)predSize);

                std::vector<int> values = intMap->values(predicate);
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    ASSERT_EQ(i, values[i]);
                }

                values = intMap->values(predicate);
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    ASSERT_EQ(i, values[i]);
                }

                predicate.nextPage();
                values = intMap->values(predicate);
                ASSERT_EQ(predSize, (int) values.size());

                for (int i = 0; i < predSize; ++i) {
                    ASSERT_EQ(predSize + i, values[i]);
                }

                const std::pair<int *, int *> *anchor = predicate.getAnchor();
                ASSERT_NE((const std::pair<int *, int *> *) NULL, anchor);
                ASSERT_NE((int *) NULL, anchor->first);
                ASSERT_NE((int *) NULL, anchor->second);
                ASSERT_EQ(9, *anchor->first);
                ASSERT_EQ(9, *anchor->second);

                ASSERT_EQ(1, (int)predicate.getPage());

                predicate.setPage(4);

                values = intMap->values(predicate);
                ASSERT_EQ(predSize, (int) values.size());

                for (int i = 0; i < predSize; ++i) {
                    ASSERT_EQ(predSize * 4 + i, values[i]);
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
                values = intMap->values(predicate);
                ASSERT_EQ(0, (int) values.size());

                predicate.setPage(0);
                values = intMap->values(predicate);
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    ASSERT_EQ(i, values[i]);
                }

                predicate.previousPage();
                ASSERT_EQ(0, (int)predicate.getPage());

                predicate.setPage(5);
                values = intMap->values(predicate);
                ASSERT_EQ(0, (int) values.size());

                predicate.setPage(3);
                values = intMap->values(predicate);
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    ASSERT_EQ(3 * predSize + i, values[i]);
                }

                predicate.previousPage();
                values = intMap->values(predicate);
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    ASSERT_EQ(2 * predSize + i, values[i]);
                }

                // test PagingPredicate with inner predicate (value < 10)
                std::auto_ptr<query::Predicate> lessThanTenPredicate(std::auto_ptr<query::Predicate>(
                        new query::GreaterLessPredicate<int>(query::QueryConstants::getValueAttributeName(), 9, false,
                                                             true)));
                query::PagingPredicate<int, int> predicate2(lessThanTenPredicate, 5);
                values = intMap->values(predicate2);
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    ASSERT_EQ(i, values[i]);
                }

                predicate2.nextPage();
                // match values 5,6, 7, 8
                values = intMap->values(predicate2);
                ASSERT_EQ(predSize - 1, (int) values.size());
                for (int i = 0; i < predSize - 1; ++i) {
                    ASSERT_EQ(predSize + i, values[i]);
                }

                predicate2.nextPage();
                values = intMap->values(predicate2);
                ASSERT_EQ(0, (int) values.size());

                // test paging predicate with comparator
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);
                Employee empl3("deniz", 25);
                Employee empl4("ali", 33);
                Employee empl5("veli", 44);
                Employee empl6("aylin", 5);

                employees->put(3, empl1);
                employees->put(4, empl2);
                employees->put(5, empl3);
                employees->put(6, empl4);
                employees->put(7, empl5);
                employees->put(8, empl6);

                predSize = 2;
                query::PagingPredicate<int, Employee> predicate3(
                        std::auto_ptr<query::EntryComparator<int, Employee> >(new EmployeeEntryComparator()), (size_t) predSize);
                std::vector<Employee> result = employees->values(predicate3);
                ASSERT_EQ(2, (int) result.size());
                ASSERT_EQ(empl6, result[0]);
                ASSERT_EQ(empl2, result[1]);

                predicate3.nextPage();
                result = employees->values(predicate3);
                ASSERT_EQ(2, (int) result.size());
                ASSERT_EQ(empl3, result[0]);
                ASSERT_EQ(empl4, result[1]);
            }

            TEST_F(ClientMapTest, testKeySetWithPredicate) {
                const int numItems = 20;
                for (int i = 0; i < numItems; ++i) {
                    intMap->put(i, 2 * i);
                }

                std::vector<int> keys = intMap->keySet();
                ASSERT_EQ(numItems, (int)keys.size());
                std::sort(keys.begin(), keys.end());
                for (int i = 0; i < numItems; ++i) {
                    ASSERT_EQ(i, keys[i]);
                }

                // EqualPredicate
                // key == 5
                keys = intMap->keySet(query::EqualPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5));
                ASSERT_EQ(1, (int)keys.size());
                ASSERT_EQ(5, keys[0]);

                // value == 8
                keys = intMap->keySet(query::EqualPredicate<int>(query::QueryConstants::getValueAttributeName(), 8));
                ASSERT_EQ(1, (int)keys.size());
                ASSERT_EQ(4, keys[0]);

                // key == numItems
                keys = intMap->keySet(query::EqualPredicate<int>(query::QueryConstants::getKeyAttributeName(), numItems));
                ASSERT_EQ(0, (int)keys.size());

                // NotEqual Predicate
                // key != 5
                keys = intMap->keySet(query::NotEqualPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5));
                ASSERT_EQ(numItems - 1, (int)keys.size());
                std::sort(keys.begin(), keys.end());
                for (int i = 0; i < numItems - 1; ++i) {
                    if (i >= 5) {
                        ASSERT_EQ(i + 1, keys[i]);
                    } else {
                        ASSERT_EQ(i, keys[i]);
                    }
                }

                // value != 8
                keys = intMap->keySet(query::NotEqualPredicate<int>(query::QueryConstants::getValueAttributeName(), 8));
                ASSERT_EQ(numItems - 1, (int)keys.size());
                std::sort(keys.begin(), keys.end());
                for (int i = 0; i < numItems - 1; ++i) {
                    if (i >= 4) {
                        ASSERT_EQ(i + 1, keys[i]);
                    } else {
                        ASSERT_EQ(i, keys[i]);
                    }
                }

                // TruePredicate
                keys = intMap->keySet(query::TruePredicate());
                ASSERT_EQ(numItems, (int)keys.size());
                std::sort(keys.begin(), keys.end());
                for (int i = 0; i < numItems; ++i) {
                    ASSERT_EQ(i, keys[i]);
                }

                // FalsePredicate
                keys = intMap->keySet(query::FalsePredicate());
                ASSERT_EQ(0, (int)keys.size());

                // BetweenPredicate
                // 5 <= key <= 10
                keys = intMap->keySet(query::BetweenPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5, 10));
                std::sort(keys.begin(), keys.end());
                ASSERT_EQ(6, (int)keys.size());
                for (int i = 0; i < 6; ++i) {
                    ASSERT_EQ((i + 5), keys[i]);
                }

                // 20 <= key <=30
                keys = intMap->keySet(query::BetweenPredicate<int>(query::QueryConstants::getKeyAttributeName(), 20, 30));
                ASSERT_EQ(0, (int)keys.size());

                // GreaterLessPredicate
                // value <= 10
                keys = intMap->keySet(
                        query::GreaterLessPredicate<int>(query::QueryConstants::getValueAttributeName(), 10, true, true));
                ASSERT_EQ(6, (int)keys.size());
                std::sort(keys.begin(), keys.end());
                for (int i = 0; i < 6; ++i) {
                    ASSERT_EQ(i, keys[i]);
                }

                // key < 7
                keys = intMap->keySet(
                        query::GreaterLessPredicate<int>(query::QueryConstants::getKeyAttributeName(), 7, false, true));
                ASSERT_EQ(7, (int)keys.size());
                std::sort(keys.begin(), keys.end());
                for (int i = 0; i < 7; ++i) {
                    ASSERT_EQ(i, keys[i]);
                }

                // value >= 15
                keys = intMap->keySet(
                        query::GreaterLessPredicate<int>(query::QueryConstants::getValueAttributeName(), 15, true, false));
                ASSERT_EQ(12, (int)keys.size());
                std::sort(keys.begin(), keys.end());
                for (int i = 0; i < 12; ++i) {
                    ASSERT_EQ(i + 8, keys[i]);
                }

                // key > 5
                keys = intMap->keySet(
                        query::GreaterLessPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5, false, false));
                ASSERT_EQ(14, (int)keys.size());
                std::sort(keys.begin(), keys.end());
                for (int i = 0; i < 14; ++i) {
                    ASSERT_EQ(i + 6, keys[i]);
                }

                // InPredicate
                // key in {4, 10, 19}
                std::vector<int> inVals(3);
                inVals[0] = 4;
                inVals[1] = 10;
                inVals[2] = 19;
                keys = intMap->keySet(query::InPredicate<int>(query::QueryConstants::getKeyAttributeName(), inVals));
                ASSERT_EQ(3, (int)keys.size());
                std::sort(keys.begin(), keys.end());
                ASSERT_EQ(4, keys[0]);
                ASSERT_EQ(10, keys[1]);
                ASSERT_EQ(19, keys[2]);

                // value in {4, 10, 19}
                keys = intMap->keySet(query::InPredicate<int>(query::QueryConstants::getValueAttributeName(), inVals));
                ASSERT_EQ(2, (int)keys.size());
                std::sort(keys.begin(), keys.end());
                ASSERT_EQ(2, keys[0]);
                ASSERT_EQ(5, keys[1]);

                // InstanceOfPredicate
                // value instanceof Integer
                keys = intMap->keySet(query::InstanceOfPredicate("java.lang.Integer"));
                ASSERT_EQ(20, (int)keys.size());
                std::sort(keys.begin(), keys.end());
                for (int i = 0; i < numItems; ++i) {
                    ASSERT_EQ(i, keys[i]);
                }

                keys = intMap->keySet(query::InstanceOfPredicate("java.lang.String"));
                ASSERT_EQ(0, (int)keys.size());

                // NotPredicate
                // !(5 <= key <= 10)
                std::auto_ptr<query::Predicate> bp = std::auto_ptr<query::Predicate>(new query::BetweenPredicate<int>(
                        query::QueryConstants::getKeyAttributeName(), 5, 10));
                query::NotPredicate notPredicate(bp);
                keys = intMap->keySet(notPredicate);
                ASSERT_EQ(14, (int)keys.size());
                std::sort(keys.begin(), keys.end());
                for (int i = 0; i < 14; ++i) {
                    if (i >= 5) {
                        ASSERT_EQ(i + 6, keys[i]);
                    } else {
                        ASSERT_EQ(i, keys[i]);
                    }
                }

                // AndPredicate
                // 5 <= key <= 10 AND Values in {4, 10, 19} = keys {4, 10}
                bp = std::auto_ptr<query::Predicate>(
                        new query::BetweenPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5, 10));
                std::auto_ptr<query::Predicate> inPred = std::auto_ptr<query::Predicate>(
                        new query::InPredicate<int>(query::QueryConstants::getValueAttributeName(), inVals));
                keys = intMap->keySet(query::AndPredicate().add(bp).add(inPred));
                ASSERT_EQ(1, (int)keys.size());
                std::sort(keys.begin(), keys.end());
                ASSERT_EQ(5, keys[0]);

                // OrPredicate
                // 5 <= key <= 10 OR Values in {4, 10, 19} = keys {2, 5, 6, 7, 8, 9, 10}
                bp = std::auto_ptr<query::Predicate>(
                        new query::BetweenPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5, 10));
                inPred = std::auto_ptr<query::Predicate>(
                        new query::InPredicate<int>(query::QueryConstants::getValueAttributeName(), inVals));
                keys = intMap->keySet(query::OrPredicate().add(bp).add(inPred));
                ASSERT_EQ(7, (int)keys.size());
                std::sort(keys.begin(), keys.end());
                ASSERT_EQ(2, keys[0]);
                ASSERT_EQ(5, keys[1]);
                ASSERT_EQ(6, keys[2]);
                ASSERT_EQ(7, keys[3]);
                ASSERT_EQ(8, keys[4]);
                ASSERT_EQ(9, keys[5]);
                ASSERT_EQ(10, keys[6]);

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
                std::vector<std::string> strKeys = imap->keySet(
                        query::LikePredicate(query::QueryConstants::getValueAttributeName(), "value1"));
                ASSERT_EQ(1, (int)strKeys.size());
                ASSERT_EQ("key1", strKeys[0]);

                // ILikePredicate
                // value ILIKE "%VALue%1%" : {"key_111_test", "key1", "key10", "key11"}
                strKeys = imap->keySet(query::ILikePredicate(query::QueryConstants::getValueAttributeName(), "%VALue%1%"));
                ASSERT_EQ(4, (int)strKeys.size());
                std::sort(strKeys.begin(), strKeys.end());
                ASSERT_EQ("key1", strKeys[0]);
                ASSERT_EQ("key10", strKeys[1]);
                ASSERT_EQ("key11", strKeys[2]);
                ASSERT_EQ("key_111_test", strKeys[3]);

                // key ILIKE "%VAL%2%" : {"key_22_test", "key2"}
                strKeys = imap->keySet(query::ILikePredicate(query::QueryConstants::getValueAttributeName(), "%VAL%2%"));
                ASSERT_EQ(2, (int)strKeys.size());
                std::sort(strKeys.begin(), strKeys.end());
                ASSERT_EQ("key2", strKeys[0]);
                ASSERT_EQ("key_22_test", strKeys[1]);

                // SqlPredicate
                // __key BETWEEN 4 and 7 : {4, 5, 6, 7} -> {8, 10, 12, 14}
                char sql[100];
                util::snprintf(sql, 50, "%s BETWEEN 4 and 7", query::QueryConstants::getKeyAttributeName());
                keys = intMap->keySet(query::SqlPredicate(sql));
                ASSERT_EQ(4, (int)keys.size());
                std::sort(keys.begin(), keys.end());
                for (int i = 0; i < 4; ++i) {
                    ASSERT_EQ(i + 4, keys[i]);
                }

                // RegexPredicate
                // value matches the regex ".*value.*2.*" : {key_22_test, value2}
                strKeys = imap->keySet(
                        query::RegexPredicate(query::QueryConstants::getValueAttributeName(), ".*value.*2.*"));
                ASSERT_EQ(2, (int)strKeys.size());
                std::sort(strKeys.begin(), strKeys.end());
                ASSERT_EQ("key2", strKeys[0]);
                ASSERT_EQ("key_22_test", strKeys[1]);
            }

            TEST_F(ClientMapTest, testKeySetWithPagingPredicate) {
                int predSize = 5;
                const int totalEntries = 25;

                for (int i = 0; i < totalEntries; ++i) {
                    intMap->put(i, i);
                }

                query::PagingPredicate<int, int> predicate((size_t)predSize);

                std::vector<int> values = intMap->keySet(predicate);
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    ASSERT_EQ(i, values[i]);
                }

                values = intMap->keySet(predicate);
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    ASSERT_EQ(i, values[i]);
                }

                predicate.nextPage();
                values = intMap->keySet(predicate);
                ASSERT_EQ(predSize, (int) values.size());

                for (int i = 0; i < predSize; ++i) {
                    ASSERT_EQ(predSize + i, values[i]);
                }

                const std::pair<int *, int *> *anchor = predicate.getAnchor();
                ASSERT_NE((const std::pair<int *, int *> *) NULL, anchor);
                ASSERT_NE((int *) NULL, anchor->first);
                ASSERT_EQ(9, *anchor->first);

                ASSERT_EQ(1, (int)predicate.getPage());

                predicate.setPage(4);

                values = intMap->keySet(predicate);
                ASSERT_EQ(predSize, (int) values.size());

                for (int i = 0; i < predSize; ++i) {
                    ASSERT_EQ(predSize * 4 + i, values[i]);
                }

                anchor = predicate.getAnchor();
                ASSERT_NE((const std::pair<int *, int *> *) NULL, anchor);
                ASSERT_NE((int *) NULL, anchor->first);
                ASSERT_EQ(24, *anchor->first);

                const std::pair<size_t, std::pair<int *, int *> > *anchorEntry = predicate.getNearestAnchorEntry();
                ASSERT_NE((const std::pair<size_t, std::pair<int *, int *> > *) NULL, anchorEntry);
                ASSERT_NE((int *) NULL, anchorEntry->second.first);
                ASSERT_EQ(3, (int)anchorEntry->first);
                ASSERT_EQ(19, *anchorEntry->second.first);

                predicate.nextPage();
                values = intMap->keySet(predicate);
                ASSERT_EQ(0, (int) values.size());

                predicate.setPage(0);
                values = intMap->keySet(predicate);
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    ASSERT_EQ(i, values[i]);
                }

                predicate.previousPage();
                ASSERT_EQ(0, (int)predicate.getPage());

                predicate.setPage(5);
                values = intMap->keySet(predicate);
                ASSERT_EQ(0, (int) values.size());

                predicate.setPage(3);
                values = intMap->keySet(predicate);
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    ASSERT_EQ(3 * predSize + i, values[i]);
                }

                predicate.previousPage();
                values = intMap->keySet(predicate);
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    ASSERT_EQ(2 * predSize + i, values[i]);
                }

                // test PagingPredicate with inner predicate (value < 10)
                std::auto_ptr<query::Predicate> lessThanTenPredicate(std::auto_ptr<query::Predicate>(
                        new query::GreaterLessPredicate<int>(query::QueryConstants::getValueAttributeName(), 9, false,
                                                             true)));
                query::PagingPredicate<int, int> predicate2(lessThanTenPredicate, 5);
                values = intMap->keySet(predicate2);
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    ASSERT_EQ(i, values[i]);
                }

                predicate2.nextPage();
                // match values 5,6, 7, 8
                values = intMap->keySet(predicate2);
                ASSERT_EQ(predSize - 1, (int) values.size());
                for (int i = 0; i < predSize - 1; ++i) {
                    ASSERT_EQ(predSize + i, values[i]);
                }

                predicate2.nextPage();
                values = intMap->keySet(predicate2);
                ASSERT_EQ(0, (int) values.size());

                // test paging predicate with comparator
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);
                Employee empl3("deniz", 25);
                Employee empl4("ali", 33);
                Employee empl5("veli", 44);
                Employee empl6("aylin", 5);

                employees->put(3, empl1);
                employees->put(4, empl2);
                employees->put(5, empl3);
                employees->put(6, empl4);
                employees->put(7, empl5);
                employees->put(8, empl6);

                predSize = 2;
                query::PagingPredicate<int, Employee> predicate3(
                        std::auto_ptr<query::EntryComparator<int, Employee> >(new EmployeeEntryKeyComparator()), (size_t) predSize);
                std::vector<int> result = employees->keySet(predicate3);
                // since keyset result only returns keys from the server, no ordering based on the value but ordered based on the keys
                ASSERT_EQ(2, (int) result.size());
                ASSERT_EQ(3, result[0]);
                ASSERT_EQ(4, result[1]);

                predicate3.nextPage();
                result = employees->keySet(predicate3);
                ASSERT_EQ(2, (int) result.size());
                ASSERT_EQ(5, result[0]);
                ASSERT_EQ(6, result[1]);
            }

            TEST_F(ClientMapTest, testEntrySetWithPredicate) {
                const int numItems = 20;
                std::vector<std::pair<int, int> > expected(numItems);
                for (int i = 0; i < numItems; ++i) {
                    intMap->put(i, 2 * i);
                    expected[i] = std::pair<int, int>(i, 2 * i);
                }

                std::vector<std::pair<int, int> > entries = intMap->entrySet();
                ASSERT_EQ(numItems, (int)entries.size());
                std::sort(entries.begin(), entries.end());
                for (int i = 0; i < numItems; ++i) {
                    ASSERT_EQ(expected[i], entries[i]);
                }

                // EqualPredicate
                // key == 5
                entries = intMap->entrySet(query::EqualPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5));
                ASSERT_EQ(1, (int)entries.size());
                ASSERT_EQ(expected[5], entries[0]);

                // value == 8
                entries = intMap->entrySet(query::EqualPredicate<int>(query::QueryConstants::getValueAttributeName(), 8));
                ASSERT_EQ(1, (int)entries.size());
                ASSERT_EQ(expected[4], entries[0]);

                // key == numItems
                entries = intMap->entrySet(
                        query::EqualPredicate<int>(query::QueryConstants::getKeyAttributeName(), numItems));
                ASSERT_EQ(0, (int)entries.size());

                // NotEqual Predicate
                // key != 5
                entries = intMap->entrySet(query::NotEqualPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5));
                ASSERT_EQ(numItems - 1, (int)entries.size());
                std::sort(entries.begin(), entries.end());
                for (int i = 0; i < numItems - 1; ++i) {
                    if (i >= 5) {
                        ASSERT_EQ(expected[i + 1], entries[i]);
                    } else {
                        ASSERT_EQ(expected[i], entries[i]);
                    }
                }

                // value != 8
                entries = intMap->entrySet(query::NotEqualPredicate<int>(query::QueryConstants::getValueAttributeName(), 8));
                ASSERT_EQ(numItems - 1, (int)entries.size());
                std::sort(entries.begin(), entries.end());
                for (int i = 0; i < numItems - 1; ++i) {
                    if (i >= 4) {
                        ASSERT_EQ(expected[i + 1], entries[i]);
                    } else {
                        ASSERT_EQ(expected[i], entries[i]);
                    }
                }

                // TruePredicate
                entries = intMap->entrySet(query::TruePredicate());
                ASSERT_EQ(numItems, (int)entries.size());
                std::sort(entries.begin(), entries.end());
                for (int i = 0; i < numItems; ++i) {
                    ASSERT_EQ(expected[i], entries[i]);
                }

                // FalsePredicate
                entries = intMap->entrySet(query::FalsePredicate());
                ASSERT_EQ(0, (int)entries.size());

                // BetweenPredicate
                // 5 <= key <= 10
                entries = intMap->entrySet(
                        query::BetweenPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5, 10));
                std::sort(entries.begin(), entries.end());
                ASSERT_EQ(6, (int)entries.size());
                for (int i = 0; i < 6; ++i) {
                    ASSERT_EQ(expected[i + 5], entries[i]);
                }

                // 20 <= key <=30
                entries = intMap->entrySet(
                        query::BetweenPredicate<int>(query::QueryConstants::getKeyAttributeName(), 20, 30));
                ASSERT_EQ(0, (int)entries.size());

                // GreaterLessPredicate
                // value <= 10
                entries = intMap->entrySet(
                        query::GreaterLessPredicate<int>(query::QueryConstants::getValueAttributeName(), 10, true, true));
                ASSERT_EQ(6, (int)entries.size());
                std::sort(entries.begin(), entries.end());
                for (int i = 0; i < 6; ++i) {
                    ASSERT_EQ(expected[i], entries[i]);
                }

                // key < 7
                entries = intMap->entrySet(
                        query::GreaterLessPredicate<int>(query::QueryConstants::getKeyAttributeName(), 7, false, true));
                ASSERT_EQ(7, (int)entries.size());
                std::sort(entries.begin(), entries.end());
                for (int i = 0; i < 7; ++i) {
                    ASSERT_EQ(expected[i], entries[i]);
                }

                // value >= 15
                entries = intMap->entrySet(
                        query::GreaterLessPredicate<int>(query::QueryConstants::getValueAttributeName(), 15, true, false));
                ASSERT_EQ(12, (int)entries.size());
                std::sort(entries.begin(), entries.end());
                for (int i = 0; i < 12; ++i) {
                    ASSERT_EQ(expected[i + 8], entries[i]);
                }

                // key > 5
                entries = intMap->entrySet(
                        query::GreaterLessPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5, false, false));
                ASSERT_EQ(14, (int)entries.size());
                std::sort(entries.begin(), entries.end());
                for (int i = 0; i < 14; ++i) {
                    ASSERT_EQ(expected[i + 6], entries[i]);
                }

                // InPredicate
                // key in {4, 10, 19}
                std::vector<int> inVals(3);
                inVals[0] = 4;
                inVals[1] = 10;
                inVals[2] = 19;
                entries = intMap->entrySet(query::InPredicate<int>(query::QueryConstants::getKeyAttributeName(), inVals));
                ASSERT_EQ(3, (int)entries.size());
                std::sort(entries.begin(), entries.end());
                ASSERT_EQ(expected[4], entries[0]);
                ASSERT_EQ(expected[10], entries[1]);
                ASSERT_EQ(expected[19], entries[2]);

                // value in {4, 10, 19}
                entries = intMap->entrySet(query::InPredicate<int>(query::QueryConstants::getValueAttributeName(), inVals));
                ASSERT_EQ(2, (int)entries.size());
                std::sort(entries.begin(), entries.end());
                ASSERT_EQ(expected[2], entries[0]);
                ASSERT_EQ(expected[5], entries[1]);

                // InstanceOfPredicate
                // value instanceof Integer
                entries = intMap->entrySet(query::InstanceOfPredicate("java.lang.Integer"));
                ASSERT_EQ(20, (int)entries.size());
                std::sort(entries.begin(), entries.end());
                for (int i = 0; i < numItems; ++i) {
                    ASSERT_EQ(expected[i], entries[i]);
                }

                entries = intMap->entrySet(query::InstanceOfPredicate("java.lang.String"));
                ASSERT_EQ(0, (int)entries.size());

                // NotPredicate
                // !(5 <= key <= 10)
                std::auto_ptr<query::Predicate> bp = std::auto_ptr<query::Predicate>(new query::BetweenPredicate<int>(
                        query::QueryConstants::getKeyAttributeName(), 5, 10));
                query::NotPredicate notPredicate(bp);
                entries = intMap->entrySet(notPredicate);
                ASSERT_EQ(14, (int)entries.size());
                std::sort(entries.begin(), entries.end());
                for (int i = 0; i < 14; ++i) {
                    if (i >= 5) {
                        ASSERT_EQ(expected[i + 6], entries[i]);
                    } else {
                        ASSERT_EQ(expected[i], entries[i]);
                    }
                }

                // AndPredicate
                // 5 <= key <= 10 AND Values in {4, 10, 19} = entries {4, 10}
                bp = std::auto_ptr<query::Predicate>(
                        new query::BetweenPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5, 10));
                std::auto_ptr<query::Predicate> inPred = std::auto_ptr<query::Predicate>(
                        new query::InPredicate<int>(query::QueryConstants::getValueAttributeName(), inVals));
                entries = intMap->entrySet(query::AndPredicate().add(bp).add(inPred));
                ASSERT_EQ(1, (int)entries.size());
                std::sort(entries.begin(), entries.end());
                ASSERT_EQ(expected[5], entries[0]);

                // OrPredicate
                // 5 <= key <= 10 OR Values in {4, 10, 19} = entries {2, 5, 6, 7, 8, 9, 10}
                bp = std::auto_ptr<query::Predicate>(
                        new query::BetweenPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5, 10));
                inPred = std::auto_ptr<query::Predicate>(
                        new query::InPredicate<int>(query::QueryConstants::getValueAttributeName(), inVals));
                entries = intMap->entrySet(query::OrPredicate().add(bp).add(inPred));
                ASSERT_EQ(7, (int)entries.size());
                std::sort(entries.begin(), entries.end());
                ASSERT_EQ(expected[2], entries[0]);
                ASSERT_EQ(expected[5], entries[1]);
                ASSERT_EQ(expected[6], entries[2]);
                ASSERT_EQ(expected[7], entries[3]);
                ASSERT_EQ(expected[8], entries[4]);
                ASSERT_EQ(expected[9], entries[5]);
                ASSERT_EQ(expected[10], entries[6]);

                std::vector<std::pair<std::string, std::string> > expectedStrEntries(14);
                for (int i = 0; i < 12; i++) {
                    std::string key = "key";
                    key += util::IOUtil::to_string(i);
                    std::string value = "value";
                    value += util::IOUtil::to_string(i);
                    imap->put(key, value);
                    expectedStrEntries[i] = std::pair<std::string, std::string>(key, value);
                }
                imap->put("key_111_test", "myvalue_111_test");
                expectedStrEntries[12] = std::pair<std::string, std::string>("key_111_test", "myvalue_111_test");
                imap->put("key_22_test", "myvalue_22_test");
                expectedStrEntries[13] = std::pair<std::string, std::string>("key_22_test", "myvalue_22_test");

                // LikePredicate
                // value LIKE "value1" : {"value1"}
                std::vector<std::pair<std::string, std::string> > strEntries = imap->entrySet(
                        query::LikePredicate(query::QueryConstants::getValueAttributeName(), "value1"));
                ASSERT_EQ(1, (int)strEntries.size());
                ASSERT_EQ(expectedStrEntries[1], strEntries[0]);

                // ILikePredicate
                // value ILIKE "%VALue%1%" : {"key_111_test", "key1", "key10", "key11"}
                strEntries = imap->entrySet(
                        query::ILikePredicate(query::QueryConstants::getValueAttributeName(), "%VALue%1%"));
                ASSERT_EQ(4, (int)strEntries.size());
                std::sort(strEntries.begin(), strEntries.end());
                ASSERT_EQ(expectedStrEntries[1], strEntries[0]);
                ASSERT_EQ(expectedStrEntries[10], strEntries[1]);
                ASSERT_EQ(expectedStrEntries[11], strEntries[2]);
                ASSERT_EQ(expectedStrEntries[12], strEntries[3]);

                // key ILIKE "%VAL%2%" : {"key_22_test", "key2"}
                strEntries = imap->entrySet(
                        query::ILikePredicate(query::QueryConstants::getValueAttributeName(), "%VAL%2%"));
                ASSERT_EQ(2, (int)strEntries.size());
                std::sort(strEntries.begin(), strEntries.end());
                ASSERT_EQ(expectedStrEntries[2], strEntries[0]);
                ASSERT_EQ(expectedStrEntries[13], strEntries[1]);

                // SqlPredicate
                // __key BETWEEN 4 and 7 : {4, 5, 6, 7} -> {8, 10, 12, 14}
                char sql[100];
                util::snprintf(sql, 50, "%s BETWEEN 4 and 7", query::QueryConstants::getKeyAttributeName());
                entries = intMap->entrySet(query::SqlPredicate(sql));
                ASSERT_EQ(4, (int)entries.size());
                std::sort(entries.begin(), entries.end());
                for (int i = 0; i < 4; ++i) {
                    ASSERT_EQ(expected[i + 4], entries[i]);
                }

                // RegexPredicate
                // value matches the regex ".*value.*2.*" : {key_22_test, value2}
                strEntries = imap->entrySet(
                        query::RegexPredicate(query::QueryConstants::getValueAttributeName(), ".*value.*2.*"));
                ASSERT_EQ(2, (int)strEntries.size());
                std::sort(strEntries.begin(), strEntries.end());
                ASSERT_EQ(expectedStrEntries[2], strEntries[0]);
                ASSERT_EQ(expectedStrEntries[13], strEntries[1]);
            }

            TEST_F(ClientMapTest, testEntrySetWithPagingPredicate) {
                int predSize = 5;
                const int totalEntries = 25;

                for (int i = 0; i < totalEntries; ++i) {
                    intMap->put(i, i);
                }

                query::PagingPredicate<int, int> predicate((size_t) predSize);

                std::vector<std::pair<int, int> > values = intMap->entrySet(predicate);
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    std::pair<int, int> value(i, i);
                    ASSERT_EQ(value, values[i]);
                }

                values = intMap->entrySet(predicate);
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    std::pair<int, int> value(i, i);
                    ASSERT_EQ(value, values[i]);
                }

                predicate.nextPage();
                values = intMap->entrySet(predicate);
                ASSERT_EQ(predSize, (int) values.size());

                for (int i = 0; i < predSize; ++i) {
                    std::pair<int, int> value(predSize + i, predSize + i);
                    ASSERT_EQ(value, values[i]);
                }

                const std::pair<int *, int *> *anchor = predicate.getAnchor();
                ASSERT_NE((const std::pair<int *, int *> *) NULL, anchor);
                ASSERT_NE((int *) NULL, anchor->first);
                ASSERT_NE((int *) NULL, anchor->second);
                ASSERT_EQ(9, *anchor->first);
                ASSERT_EQ(9, *anchor->second);

                ASSERT_EQ(1, (int)predicate.getPage());

                predicate.setPage(4);

                values = intMap->entrySet(predicate);
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    std::pair<int, int> value(predSize * 4 + i, predSize * 4 + i);
                    ASSERT_EQ(value, values[i]);
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
                values = intMap->entrySet(predicate);
                ASSERT_EQ(0, (int) values.size());

                predicate.setPage(0);
                values = intMap->entrySet(predicate);
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    std::pair<int, int> value(i, i);
                    ASSERT_EQ(value, values[i]);
                }

                predicate.previousPage();
                ASSERT_EQ(0, (int)predicate.getPage());

                predicate.setPage(5);
                values = intMap->entrySet(predicate);
                ASSERT_EQ(0, (int) values.size());

                predicate.setPage(3);
                values = intMap->entrySet(predicate);
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    std::pair<int, int> value(3 * predSize + i, 3 * predSize + i);
                    ASSERT_EQ(value, values[i]);
                }

                predicate.previousPage();
                values = intMap->entrySet(predicate);
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    std::pair<int, int> value(2 * predSize + i, 2 * predSize + i);
                    ASSERT_EQ(value, values[i]);
                }

                // test PagingPredicate with inner predicate (value < 10)
                std::auto_ptr<query::Predicate> lessThanTenPredicate(std::auto_ptr<query::Predicate>(
                        new query::GreaterLessPredicate<int>(query::QueryConstants::getValueAttributeName(), 9, false,
                                                             true)));
                query::PagingPredicate<int, int> predicate2(lessThanTenPredicate, 5);
                values = intMap->entrySet(predicate2);
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    std::pair<int, int> value(i, i);
                    ASSERT_EQ(value, values[i]);
                }

                predicate2.nextPage();
                // match values 5,6, 7, 8
                values = intMap->entrySet(predicate2);
                ASSERT_EQ(predSize - 1, (int) values.size());
                for (int i = 0; i < predSize - 1; ++i) {
                    std::pair<int, int> value(predSize + i, predSize + i);
                    ASSERT_EQ(value, values[i]);
                }

                predicate2.nextPage();
                values = intMap->entrySet(predicate2);
                ASSERT_EQ(0, (int) values.size());

                // test paging predicate with comparator
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);
                Employee empl3("deniz", 25);
                Employee empl4("ali", 33);
                Employee empl5("veli", 44);
                Employee empl6("aylin", 5);

                employees->put(3, empl1);
                employees->put(4, empl2);
                employees->put(5, empl3);
                employees->put(6, empl4);
                employees->put(7, empl5);
                employees->put(8, empl6);

                predSize = 2;
                query::PagingPredicate<int, Employee> predicate3(
                        std::auto_ptr<query::EntryComparator<int, Employee> >(new EmployeeEntryComparator()), (size_t) predSize);
                std::vector<std::pair<int, Employee> > result = employees->entrySet(predicate3);
                ASSERT_EQ(2, (int) result.size());
                std::pair<int, Employee> value(8, empl6);
                ASSERT_EQ(value, result[0]);
                value = std::pair<int, Employee>(4, empl2);
                ASSERT_EQ(value, result[1]);

                predicate3.nextPage();
                result = employees->entrySet(predicate3);
                ASSERT_EQ(2, (int) result.size());
                value = std::pair<int, Employee>(5, empl3);
                ASSERT_EQ(value, result[0]);
                value = std::pair<int, Employee>(6, empl4);
                ASSERT_EQ(value, result[1]);
            }

            TEST_F(ClientMapTest, testReplace) {
                boost::shared_ptr<std::string> temp = imap->replace("key1", "value");
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

            TEST_F(ClientMapTest, testListenerWithPortableKey) {
                IMap<Employee, int> tradeMap = client->getMap<Employee, int>("tradeMap");
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

            TEST_F(ClientMapTest, testListener) {
                util::CountDownLatch latch1Add(5);
                util::CountDownLatch latch1Remove(2);
                util::CountDownLatch dummy(10);
                util::CountDownLatch latch2Add(1);
                util::CountDownLatch latch2Remove(1);

                CountdownListener<std::string, std::string> listener1(latch1Add, latch1Remove, dummy, dummy);
                CountdownListener<std::string, std::string> listener2(latch2Add, latch2Remove, dummy, dummy);

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

            TEST_F(ClientMapTest, testListenerWithTruePredicate) {
                util::CountDownLatch latchAdd(3);
                util::CountDownLatch latchRemove(1);
                util::CountDownLatch latchEvict(1);
                util::CountDownLatch latchUpdate(1);

                CountdownListener<int, int> listener(latchAdd, latchRemove, latchUpdate, latchEvict);

                std::string listenerId = intMap->addEntryListener(listener, query::TruePredicate(), false);

                intMap->put(1, 1);
                intMap->put(2, 2);
                intMap->put(3, 3, 1000); // evict after 1 second
                intMap->remove(2);

                util::sleep(2);
                
                ASSERT_EQ(NULL, intMap->get(3).get()); // trigger eviction

                // update an entry
                intMap->set(1, 5);
                boost::shared_ptr<int> value = intMap->get(1);
                ASSERT_NE((int *)NULL, value.get());
                ASSERT_EQ(5, *value);

                util::CountDownLatchWaiter latches;
                latches.add(latchAdd).add(latchRemove).add(latchUpdate).add(latchEvict);
                ASSERT_TRUE(latches.awaitMillis(2000));

                ASSERT_TRUE(intMap->removeEntryListener(listenerId));
            }

            TEST_F(ClientMapTest, testListenerWithFalsePredicate) {
                util::CountDownLatch latchAdd(3);
                util::CountDownLatch latchRemove(1);
                util::CountDownLatch latchEvict(1);
                util::CountDownLatch latchUpdate(1);

                CountdownListener<int, int> listener(latchAdd, latchRemove, latchUpdate, latchEvict);

                std::string listenerId = intMap->addEntryListener(listener, query::FalsePredicate(), false);

                intMap->put(1, 1);
                intMap->put(2, 2);
                intMap->put(3, 3, 1000); // evict after 1 second
                intMap->remove(2);

                util::sleep(2);

                ASSERT_EQ(NULL, intMap->get(3).get()); // trigger eviction

                // update an entry
                intMap->set(1, 5);
                boost::shared_ptr<int> value = intMap->get(1);
                ASSERT_NE((int *)NULL, value.get());
                ASSERT_EQ(5, *value);

                util::CountDownLatchWaiter latches;
                latches.add(latchAdd).add(latchRemove).add(latchUpdate).add(latchEvict);
                ASSERT_FALSE(latches.awaitMillis(2000));

                ASSERT_TRUE(intMap->removeEntryListener(listenerId));
            }

            TEST_F(ClientMapTest, testListenerWithEqualPredicate) {
                util::CountDownLatch latchAdd(1);
                util::CountDownLatch latchRemove(1);
                util::CountDownLatch latchEvict(1);
                util::CountDownLatch latchUpdate(1);

                CountdownListener<int, int> listener(latchAdd, latchRemove, latchUpdate, latchEvict);

                std::string listenerId = intMap->addEntryListener(listener, query::EqualPredicate<int>(
                        query::QueryConstants::getKeyAttributeName(), 3), true);

                intMap->put(1, 1);
                intMap->put(2, 2);
                intMap->put(3, 3, 1000); // evict after 1 second
                intMap->remove(2);

                util::sleep(2);

                ASSERT_EQ(NULL, intMap->get(3).get()); // trigger eviction

                // update an entry
                intMap->set(1, 5);
                boost::shared_ptr<int> value = intMap->get(1);
                ASSERT_NE((int *)NULL, value.get());
                ASSERT_EQ(5, *value);

                util::CountDownLatchWaiter latches;
                latches.add(latchAdd).add(latchEvict);
                ASSERT_TRUE(latches.awaitMillis(2000));

                latches.reset();
                latches.add(latchUpdate).add(latchRemove);
                ASSERT_FALSE(latches.awaitMillis(2000));

                ASSERT_TRUE(intMap->removeEntryListener(listenerId));
            }

            TEST_F(ClientMapTest, testListenerWithNotEqualPredicate) {
                util::CountDownLatch latchAdd(2);
                util::CountDownLatch latchRemove(1);
                util::CountDownLatch latchEvict(1);
                util::CountDownLatch latchUpdate(1);

                CountdownListener<int, int> listener(latchAdd, latchRemove, latchUpdate, latchEvict);

                std::string listenerId = intMap->addEntryListener(listener, query::NotEqualPredicate<int>(
                        query::QueryConstants::getKeyAttributeName(), 3), true);

                intMap->put(1, 1);
                intMap->put(2, 2);
                intMap->put(3, 3, 1000); // evict after 1 second
                intMap->remove(2);

                util::sleep(2);

                ASSERT_EQ(NULL, intMap->get(3).get()); // trigger eviction

                // update an entry
                intMap->set(1, 5);
                boost::shared_ptr<int> value = intMap->get(1);
                ASSERT_NE((int *)NULL, value.get());
                ASSERT_EQ(5, *value);

                util::CountDownLatchWaiter latches;
                latches.add(latchAdd).add(latchRemove).add(latchUpdate);
                ASSERT_TRUE(latches.awaitMillis(2000));

                latches.reset();
                latches.add(latchEvict);
                ASSERT_FALSE(latches.awaitMillis(2000));

                ASSERT_TRUE(intMap->removeEntryListener(listenerId));
            }

            TEST_F(ClientMapTest, testListenerWithGreaterLessPredicate) {
                util::CountDownLatch latchAdd(2);
                util::CountDownLatch latchRemove(1);
                util::CountDownLatch latchEvict(1);
                util::CountDownLatch latchUpdate(1);

                CountdownListener<int, int> listener(latchAdd, latchRemove, latchUpdate, latchEvict);

                // key <= 2
                std::string listenerId = intMap->addEntryListener(listener, query::GreaterLessPredicate<int>(
                        query::QueryConstants::getKeyAttributeName(), 2, true, true), false);

                intMap->put(1, 1);
                intMap->put(2, 2);
                intMap->put(3, 3, 1000); // evict after 1 second
                intMap->remove(2);

                util::sleep(2);

                ASSERT_EQ(NULL, intMap->get(3).get()); // trigger eviction

                // update an entry
                intMap->set(1, 5);
                boost::shared_ptr<int> value = intMap->get(1);
                ASSERT_NE((int *)NULL, value.get());
                ASSERT_EQ(5, *value);

                util::CountDownLatchWaiter latches;
                latches.add(latchAdd).add(latchRemove).add(latchUpdate);
                ASSERT_TRUE(latches.awaitMillis(2000));

                ASSERT_FALSE(latchEvict.awaitMillis(2000));

                ASSERT_TRUE(intMap->removeEntryListener(listenerId));
            }

            TEST_F(ClientMapTest, testListenerWithBetweenPredicate) {
                util::CountDownLatch latchAdd(2);
                util::CountDownLatch latchRemove(1);
                util::CountDownLatch latchEvict(1);
                util::CountDownLatch latchUpdate(1);

                CountdownListener<int, int> listener(latchAdd, latchRemove, latchUpdate, latchEvict);

                // 1 <=key <= 2
                std::string listenerId = intMap->addEntryListener(listener, query::BetweenPredicate<int>(
                        query::QueryConstants::getKeyAttributeName(), 1, 2), true);

                intMap->put(1, 1);
                intMap->put(2, 2);
                intMap->put(3, 3, 1000); // evict after 1 second
                intMap->remove(2);

                util::sleep(2);

                ASSERT_EQ(NULL, intMap->get(3).get()); // trigger eviction

                // update an entry
                intMap->set(1, 5);
                boost::shared_ptr<int> value = intMap->get(1);
                ASSERT_NE((int *)NULL, value.get());
                ASSERT_EQ(5, *value);

                util::CountDownLatchWaiter latches;
                latches.add(latchAdd).add(latchRemove).add(latchUpdate);
                ASSERT_TRUE(latches.awaitMillis(2000));

                ASSERT_FALSE(latchEvict.awaitMillis(2000));

                ASSERT_TRUE(intMap->removeEntryListener(listenerId));
            }

            TEST_F(ClientMapTest, testListenerWithSqlPredicate) {
                util::CountDownLatch latchAdd(1);
                util::CountDownLatch latchRemove(1);
                util::CountDownLatch latchEvict(1);
                util::CountDownLatch latchUpdate(1);

                CountdownListener<int, int> listener(latchAdd, latchRemove, latchUpdate, latchEvict);

                // 1 <=key <= 2
                std::string listenerId = intMap->addEntryListener(listener, query::SqlPredicate("__key < 2"), true);

                intMap->put(1, 1);
                intMap->put(2, 2);
                intMap->put(3, 3, 1000); // evict after 1 second
                intMap->remove(2);

                util::sleep(2);

                ASSERT_EQ(NULL, intMap->get(3).get()); // trigger eviction

                // update an entry
                intMap->set(1, 5);
                boost::shared_ptr<int> value = intMap->get(1);
                ASSERT_NE((int *)NULL, value.get());
                ASSERT_EQ(5, *value);

                util::CountDownLatchWaiter latches;
                latches.add(latchAdd).add(latchUpdate);
                ASSERT_TRUE(latches.awaitMillis(2000));

                latches.reset();
                latches.add(latchRemove).add(latchEvict);
                ASSERT_FALSE(latches.awaitMillis(2000));

                ASSERT_TRUE(intMap->removeEntryListener(listenerId));
            }

            TEST_F(ClientMapTest, testListenerWithRegExPredicate) {
                util::CountDownLatch latchAdd(2);
                util::CountDownLatch latchRemove(1);
                util::CountDownLatch latchEvict(1);
                util::CountDownLatch latchUpdate(1);

                CountdownListener<std::string, std::string> listener(latchAdd, latchRemove, latchUpdate, latchEvict);

                // key matches any word containing ".*met.*"
                std::string listenerId = imap->addEntryListener(listener, query::RegexPredicate(query::QueryConstants::getKeyAttributeName(), ".*met.*"), true);

                imap->put("ilkay", "yasar");
                imap->put("mehmet", "demir");
                imap->put("metin", "ozen", 1000); // evict after 1 second
                imap->put("hasan", "can");
                imap->remove("mehmet");

                util::sleep(2);

                ASSERT_EQ((std::string *)NULL, imap->get("metin").get()); // trigger eviction

                // update an entry
                imap->set("hasan", "suphi");
                boost::shared_ptr<std::string> value = imap->get("hasan");
                ASSERT_NE((std::string *)NULL, value.get());
                ASSERT_EQ("suphi", *value);

                util::CountDownLatchWaiter latches;
                latches.add(latchAdd).add(latchRemove).add(latchEvict);
                ASSERT_TRUE(latches.awaitMillis(2000));

                ASSERT_FALSE(latchUpdate.awaitMillis(2000));

                ASSERT_TRUE(imap->removeEntryListener(listenerId));
            }

            TEST_F(ClientMapTest, testListenerWithInstanceOfPredicate) {
                util::CountDownLatch latchAdd(3);
                util::CountDownLatch latchRemove(1);
                util::CountDownLatch latchEvict(1);
                util::CountDownLatch latchUpdate(1);

                CountdownListener<int, int> listener(latchAdd, latchRemove, latchUpdate, latchEvict);

                // 1 <=key <= 2
                std::string listenerId = intMap->addEntryListener(listener, query::InstanceOfPredicate("java.lang.Integer"), false);

                intMap->put(1, 1);
                intMap->put(2, 2);
                intMap->put(3, 3, 1000); // evict after 1 second
                intMap->remove(2);

                util::sleep(2);

                ASSERT_EQ(NULL, intMap->get(3).get()); // trigger eviction

                // update an entry
                intMap->set(1, 5);
                boost::shared_ptr<int> value = intMap->get(1);
                ASSERT_NE((int *)NULL, value.get());
                ASSERT_EQ(5, *value);

                util::CountDownLatchWaiter latches;
                latches.add(latchAdd).add(latchRemove).add(latchUpdate).add(latchEvict);
                ASSERT_TRUE(latches.awaitMillis(2000));

                ASSERT_TRUE(intMap->removeEntryListener(listenerId));
            }

            TEST_F(ClientMapTest, testListenerWithNotPredicate) {
                util::CountDownLatch latchAdd(2);
                util::CountDownLatch latchRemove(1);
                util::CountDownLatch latchEvict(1);
                util::CountDownLatch latchUpdate(1);

                CountdownListener<int, int> listener(latchAdd, latchRemove, latchUpdate, latchEvict);

                // key >= 3
                std::auto_ptr<query::Predicate> greaterLessPred = std::auto_ptr<query::Predicate>(new query::GreaterLessPredicate<int>(query::QueryConstants::getKeyAttributeName(), 3, true, false));
                query::NotPredicate notPredicate(greaterLessPred);
                std::string listenerId = intMap->addEntryListener(listener, notPredicate, false);

                intMap->put(1, 1);
                intMap->put(2, 2);
                intMap->put(3, 3, 1000); // evict after 1 second
                intMap->remove(2);

                util::sleep(2);

                ASSERT_EQ(NULL, intMap->get(3).get()); // trigger eviction

                // update an entry
                intMap->set(1, 5);
                boost::shared_ptr<int> value = intMap->get(1);
                ASSERT_NE((int *)NULL, value.get());
                ASSERT_EQ(5, *value);

                util::CountDownLatchWaiter latches;
                latches.add(latchAdd).add(latchRemove).add(latchUpdate);
                ASSERT_TRUE(latches.awaitMillis(2000));

                latches.reset();
                latches.add(latchEvict);
                ASSERT_FALSE(latches.awaitMillis(1000));

                ASSERT_TRUE(intMap->removeEntryListener(listenerId));
            }

            TEST_F(ClientMapTest, testListenerWithAndPredicate) {
                util::CountDownLatch latchAdd(1);
                util::CountDownLatch latchRemove(1);
                util::CountDownLatch latchEvict(1);
                util::CountDownLatch latchUpdate(1);

                CountdownListener<int, int> listener(latchAdd, latchRemove, latchUpdate, latchEvict);

                // key < 3
                std::auto_ptr<query::Predicate> greaterLessPred = std::auto_ptr<query::Predicate>(
                        new query::GreaterLessPredicate<int>(query::QueryConstants::getKeyAttributeName(), 3, false, true));
                // value == 1
                std::auto_ptr<query::Predicate> equalPred = std::auto_ptr<query::Predicate>(
                        new query::EqualPredicate<int>(query::QueryConstants::getKeyAttributeName(), 1));
                query::AndPredicate predicate;
                // key < 3 AND key == 1 --> (1, 1)
                predicate.add(greaterLessPred).add(equalPred);
                std::string listenerId = intMap->addEntryListener(listener, predicate, false);

                intMap->put(1, 1);
                intMap->put(2, 2);
                intMap->put(3, 3, 1000); // evict after 1 second
                intMap->remove(2);

                util::sleep(2);

                ASSERT_EQ(NULL, intMap->get(3).get()); // trigger eviction

                // update an entry
                intMap->set(1, 5);
                boost::shared_ptr<int> value = intMap->get(1);
                ASSERT_NE((int *)NULL, value.get());
                ASSERT_EQ(5, *value);

                util::CountDownLatchWaiter latches;
                latches.add(latchAdd).add(latchUpdate);
                ASSERT_TRUE(latches.awaitMillis(2000));

                latches.reset();
                latches.add(latchEvict).add(latchRemove);
                ASSERT_FALSE(latches.awaitMillis(1000));

                ASSERT_TRUE(intMap->removeEntryListener(listenerId));
            }

            TEST_F(ClientMapTest, testListenerWithOrPredicate) {
                util::CountDownLatch latchAdd(2);
                util::CountDownLatch latchRemove(1);
                util::CountDownLatch latchEvict(1);
                util::CountDownLatch latchUpdate(1);

                CountdownListener<int, int> listener(latchAdd, latchRemove, latchUpdate, latchEvict);

                // key >= 3
                std::auto_ptr<query::Predicate> greaterLessPred = std::auto_ptr<query::Predicate>(
                        new query::GreaterLessPredicate<int>(query::QueryConstants::getKeyAttributeName(), 3, true, false));
                // value == 1
                std::auto_ptr<query::Predicate> equalPred = std::auto_ptr<query::Predicate>(
                        new query::EqualPredicate<int>(query::QueryConstants::getValueAttributeName(), 2));
                query::OrPredicate predicate;
                // key >= 3 OR value == 2 --> (1, 1), (2, 2)
                predicate.add(greaterLessPred).add(equalPred);
                std::string listenerId = intMap->addEntryListener(listener, predicate, true);

                intMap->put(1, 1);
                intMap->put(2, 2);
                intMap->put(3, 3, 1000); // evict after 1 second
                intMap->remove(2);

                util::sleep(2);

                ASSERT_EQ(NULL, intMap->get(3).get()); // trigger eviction

                // update an entry
                intMap->set(1, 5);
                boost::shared_ptr<int> value = intMap->get(1);
                ASSERT_NE((int *)NULL, value.get());
                ASSERT_EQ(5, *value);

                util::CountDownLatchWaiter latches;
                latches.add(latchAdd).add(latchEvict).add(latchRemove);
                ASSERT_TRUE(latches.awaitMillis(2000));

                ASSERT_FALSE(latchUpdate.awaitMillis(2000));

                ASSERT_TRUE(intMap->removeEntryListener(listenerId));
            }

            TEST_F(ClientMapTest, testClearEvent) {
                util::CountDownLatch latch(1);
                ClearListener clearListener(latch);
                std::string listenerId = imap->addEntryListener(clearListener, false);
                imap->put("key1", "value1");
                imap->clear();
                ASSERT_TRUE(latch.await(120));
                imap->removeEntryListener(listenerId);
            }

            TEST_F(ClientMapTest, testEvictAllEvent) {
                util::CountDownLatch latch(1);
                EvictListener evictListener(latch);
                std::string listenerId = imap->addEntryListener(evictListener, false);
                imap->put("key1", "value1");
                imap->evictAll();
                ASSERT_TRUE(latch.await(120));
                imap->removeEntryListener(listenerId);
            }

            TEST_F(ClientMapTest, testMapWithPortable) {
                boost::shared_ptr<Employee> n1 = employees->get(1);
                ASSERT_EQ(n1.get(), (Employee *) NULL);
                Employee employee("sancar", 24);
                boost::shared_ptr<Employee> ptr = employees->put(1, employee);
                ASSERT_EQ(ptr.get(), (Employee *) NULL);
                ASSERT_FALSE(employees->isEmpty());
                EntryView<int, Employee> view = employees->getEntryView(1);
                ASSERT_EQ(view.value, employee);
                ASSERT_EQ(view.key, 1);

                employees->addIndex("a", true);
                employees->addIndex("n", false);
            }

            TEST_F(ClientMapTest, testMapStoreRelatedRequests) {
                imap->putTransient("ali", "veli", 1100);
                imap->flush();
                ASSERT_EQ(1, imap->size());
                ASSERT_FALSE(imap->evict("deli"));
                ASSERT_TRUE(imap->evict("ali"));
                ASSERT_EQ(imap->get("ali").get(), (std::string *) NULL);
            }

            TEST_F(ClientMapTest, testExecuteOnKey) {
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);

                employees->put(3, empl1);
                employees->put(4, empl2);

                EntryMultiplier processor(4);

                boost::shared_ptr<int> result = employees->executeOnKey<int, EntryMultiplier>(4, processor);

                ASSERT_NE((int *) NULL, result.get());
                ASSERT_EQ(4 * processor.getMultiplier(), *result);
            }

            TEST_F(ClientMapTest, testExecuteOnEntries) {
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);
                Employee empl3("deniz", 25);

                employees->put(3, empl1);
                employees->put(4, empl2);
                employees->put(5, empl3);

                EntryMultiplier processor(4);

                std::map<int, boost::shared_ptr<int> > result = employees->executeOnEntries<int, EntryMultiplier>(
                        processor);

                ASSERT_EQ(3, (int) result.size());
                ASSERT_EQ(true, (result.end() != result.find(3)));
                ASSERT_EQ(true, (result.end() != result.find(4)));
                ASSERT_EQ(true, (result.end() != result.find(5)));
                ASSERT_EQ(3 * processor.getMultiplier(), *result[3]);
                ASSERT_EQ(4 * processor.getMultiplier(), *result[4]);
                ASSERT_EQ(5 * processor.getMultiplier(), *result[5]);
            }

            TEST_F(ClientMapTest, testExecuteOnEntriesWithTruePredicate) {
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);
                Employee empl3("deniz", 25);

                employees->put(3, empl1);
                employees->put(4, empl2);
                employees->put(5, empl3);

                EntryMultiplier processor(4);

                std::map<int, boost::shared_ptr<int> > result = employees->executeOnEntries<int, EntryMultiplier>(
                        processor, query::TruePredicate());

                ASSERT_EQ(3, (int) result.size());
                ASSERT_EQ(true, (result.end() != result.find(3)));
                ASSERT_EQ(true, (result.end() != result.find(4)));
                ASSERT_EQ(true, (result.end() != result.find(5)));
                ASSERT_EQ(3 * processor.getMultiplier(), *result[3]);
                ASSERT_EQ(4 * processor.getMultiplier(), *result[4]);
                ASSERT_EQ(5 * processor.getMultiplier(), *result[5]);
            }

            TEST_F(ClientMapTest, testExecuteOnEntriesWithFalsePredicate) {
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);
                Employee empl3("deniz", 25);

                employees->put(3, empl1);
                employees->put(4, empl2);
                employees->put(5, empl3);

                EntryMultiplier processor(4);

                std::map<int, boost::shared_ptr<int> > result = employees->executeOnEntries<int, EntryMultiplier>(
                        processor, query::FalsePredicate());

                ASSERT_EQ(0, (int) result.size());
            }

            TEST_F(ClientMapTest, testExecuteOnEntriesWithAndPredicate) {
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);
                Employee empl3("deniz", 25);

                employees->put(3, empl1);
                employees->put(4, empl2);
                employees->put(5, empl3);

                query::AndPredicate andPredicate;
                /* 25 <= age <= 35 AND age = 35 */
                andPredicate.add(
                        std::auto_ptr<query::Predicate>(new query::BetweenPredicate<int>("a", 25, 35))).add(
                        std::auto_ptr<query::Predicate>(
                                new query::NotPredicate(
                                        std::auto_ptr<query::Predicate>(new query::EqualPredicate<int>("a", 35)))));

                EntryMultiplier processor(4);

                std::map<int, boost::shared_ptr<int> > result = employees->executeOnEntries<int, EntryMultiplier>(
                        processor, andPredicate);

                ASSERT_EQ(1, (int) result.size());
                ASSERT_EQ(true, (result.end() != result.find(5)));
                ASSERT_EQ(5 * processor.getMultiplier(), *result[5]);
            }

            TEST_F(ClientMapTest, testExecuteOnEntriesWithOrPredicate) {
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);
                Employee empl3("deniz", 25);

                employees->put(3, empl1);
                employees->put(4, empl2);
                employees->put(5, empl3);

                query::OrPredicate orPredicate;
                /* age == 21 OR age > 25 */
                orPredicate.add(
                        std::auto_ptr<query::Predicate>(new query::EqualPredicate<int>("a", 21))).add(
                        std::auto_ptr<query::Predicate>(new query::GreaterLessPredicate<int>("a", 25, false, false)));

                EntryMultiplier processor(4);

                std::map<int, boost::shared_ptr<int> > result = employees->executeOnEntries<int, EntryMultiplier>(
                        processor, orPredicate);

                ASSERT_EQ(2, (int) result.size());
                ASSERT_EQ(true, (result.end() != result.find(3)));
                ASSERT_EQ(true, (result.end() != result.find(4)));
                ASSERT_EQ(3 * processor.getMultiplier(), *result[3]);
                ASSERT_EQ(4 * processor.getMultiplier(), *result[4]);
            }

            TEST_F(ClientMapTest, testExecuteOnEntriesWithBetweenPredicate) {
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);
                Employee empl3("deniz", 25);

                employees->put(3, empl1);
                employees->put(4, empl2);
                employees->put(5, empl3);

                EntryMultiplier processor(4);

                std::map<int, boost::shared_ptr<int> > result = employees->executeOnEntries<int, EntryMultiplier>(
                        processor, query::BetweenPredicate<int>("a", 25, 35));

                ASSERT_EQ(2, (int) result.size());
                ASSERT_EQ(true, (result.end() != result.find(3)));
                ASSERT_EQ(true, (result.end() != result.find(5)));
                ASSERT_EQ(3 * processor.getMultiplier(), *result[3]);
                ASSERT_EQ(5 * processor.getMultiplier(), *result[5]);
            }

            TEST_F(ClientMapTest, testExecuteOnEntriesWithEqualPredicate) {
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);
                Employee empl3("deniz", 25);

                employees->put(3, empl1);
                employees->put(4, empl2);
                employees->put(5, empl3);

                EntryMultiplier processor(4);

                std::map<int, boost::shared_ptr<int> > result = employees->executeOnEntries<int, EntryMultiplier>(
                        processor, query::EqualPredicate<int>("a", 25));

                ASSERT_EQ(1, (int) result.size());
                ASSERT_EQ(true, (result.end() != result.find(5)));

                result = employees->executeOnEntries<int, EntryMultiplier>(
                        processor, query::EqualPredicate<int>("a", 10));

                ASSERT_EQ(0, (int) result.size());
            }

            TEST_F(ClientMapTest, testExecuteOnEntriesWithNotEqualPredicate) {
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);
                Employee empl3("deniz", 25);

                employees->put(3, empl1);
                employees->put(4, empl2);
                employees->put(5, empl3);

                EntryMultiplier processor(4);

                std::map<int, boost::shared_ptr<int> > result = employees->executeOnEntries<int, EntryMultiplier>(
                        processor, query::NotEqualPredicate<int>("a", 25));

                ASSERT_EQ(2, (int) result.size());
                ASSERT_EQ(true, (result.end() != result.find(3)));
                ASSERT_EQ(true, (result.end() != result.find(4)));
            }

            TEST_F(ClientMapTest, testExecuteOnEntriesWithGreaterLessPredicate) {
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);
                Employee empl3("deniz", 25);

                employees->put(3, empl1);
                employees->put(4, empl2);
                employees->put(5, empl3);

                EntryMultiplier processor(4);

                std::map<int, boost::shared_ptr<int> > result = employees->executeOnEntries<int, EntryMultiplier>(
                        processor, query::GreaterLessPredicate<int>("a", 25, false, true)); // <25 matching

                ASSERT_EQ(1, (int) result.size());
                ASSERT_EQ(true, (result.end() != result.find(4)));

                result = employees->executeOnEntries<int, EntryMultiplier>(
                        processor, query::GreaterLessPredicate<int>("a", 25, true, true)); // <=25 matching

                ASSERT_EQ(2, (int) result.size());
                ASSERT_EQ(true, (result.end() != result.find(4)));
                ASSERT_EQ(true, (result.end() != result.find(5)));

                result = employees->executeOnEntries<int, EntryMultiplier>(
                        processor, query::GreaterLessPredicate<int>("a", 25, false, false)); // >25 matching

                ASSERT_EQ(1, (int) result.size());
                ASSERT_EQ(true, (result.end() != result.find(3)));

                result = employees->executeOnEntries<int, EntryMultiplier>(
                        processor, query::GreaterLessPredicate<int>("a", 25, true, false)); // >=25 matching

                ASSERT_EQ(2, (int) result.size());
                ASSERT_EQ(true, (result.end() != result.find(3)));
                ASSERT_EQ(true, (result.end() != result.find(5)));
            }

            TEST_F(ClientMapTest, testExecuteOnEntriesWithLikePredicate) {
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);
                Employee empl3("deniz", 25);

                employees->put(3, empl1);
                employees->put(4, empl2);
                employees->put(5, empl3);

                EntryMultiplier processor(4);

                std::map<int, boost::shared_ptr<int> > result = employees->executeOnEntries<int, EntryMultiplier>(
                        processor, query::LikePredicate("n", "deniz"));

                ASSERT_EQ(1, (int) result.size());
                ASSERT_EQ(true, (result.end() != result.find(5)));
            }

            TEST_F(ClientMapTest, testExecuteOnEntriesWithILikePredicate) {
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);
                Employee empl3("deniz", 25);

                employees->put(3, empl1);
                employees->put(4, empl2);
                employees->put(5, empl3);

                EntryMultiplier processor(4);

                std::map<int, boost::shared_ptr<int> > result = employees->executeOnEntries<int, EntryMultiplier>(
                        processor, query::ILikePredicate("n", "deniz"));

                ASSERT_EQ(1, (int) result.size());
                ASSERT_EQ(true, (result.end() != result.find(5)));
            }

            TEST_F(ClientMapTest, testExecuteOnEntriesWithInPredicate) {
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);
                Employee empl3("deniz", 25);

                employees->put(3, empl1);
                employees->put(4, empl2);
                employees->put(5, empl3);

                EntryMultiplier processor(4);

                std::vector<std::string> values;
                values.push_back("ahmet");
                query::InPredicate<std::string> predicate("n", values);
                predicate.add("mehmet");
                std::map<int, boost::shared_ptr<int> > result = employees->executeOnEntries<int, EntryMultiplier>(
                        processor, predicate);

                ASSERT_EQ(2, (int) result.size());
                ASSERT_EQ(true, (result.end() != result.find(3)));
                ASSERT_EQ(true, (result.end() != result.find(4)));
            }

            TEST_F(ClientMapTest, testExecuteOnEntriesWithInstanceOfPredicate) {
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);
                Employee empl3("deniz", 25);

                employees->put(3, empl1);
                employees->put(4, empl2);
                employees->put(5, empl3);

                EntryMultiplier processor(4);
                std::map<int, boost::shared_ptr<int> > result = employees->executeOnEntries<int, EntryMultiplier>(
                        processor, query::InstanceOfPredicate("Employee"));

                ASSERT_EQ(3, (int) result.size());
                ASSERT_EQ(true, (result.end() != result.find(3)));
                ASSERT_EQ(true, (result.end() != result.find(4)));
                ASSERT_EQ(true, (result.end() != result.find(5)));
            }

            TEST_F(ClientMapTest, testExecuteOnEntriesWithNotPredicate) {
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);
                Employee empl3("deniz", 25);

                employees->put(3, empl1);
                employees->put(4, empl2);
                employees->put(5, empl3);

                EntryMultiplier processor(4);
                std::auto_ptr<query::Predicate> eqPredicate(new query::EqualPredicate<int>("a", 25));
                query::NotPredicate notPredicate(eqPredicate);
                std::map<int, boost::shared_ptr<int> > result = employees->executeOnEntries<int, EntryMultiplier>(
                        processor, notPredicate);

                ASSERT_EQ(2, (int) result.size());
                ASSERT_EQ(true, (result.end() != result.find(3)));
                ASSERT_EQ(true, (result.end() != result.find(4)));

                query::NotPredicate notFalsePredicate(std::auto_ptr<query::Predicate>(new query::FalsePredicate()));
                result = employees->executeOnEntries<int, EntryMultiplier>(
                        processor, notFalsePredicate);

                ASSERT_EQ(3, (int) result.size());
                ASSERT_EQ(true, (result.end() != result.find(3)));
                ASSERT_EQ(true, (result.end() != result.find(4)));
                ASSERT_EQ(true, (result.end() != result.find(5)));

                query::NotPredicate notBetweenPredicate(std::auto_ptr<query::Predicate>(new query::BetweenPredicate<int>("a", 25, 35)));
                result = employees->executeOnEntries<int, EntryMultiplier>(processor, notBetweenPredicate);

                ASSERT_EQ(1, (int) result.size());
                ASSERT_EQ(true, (result.end() != result.find(4)));
            }

            TEST_F(ClientMapTest, testExecuteOnEntriesWithRegexPredicate) {
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);
                Employee empl3("deniz", 25);

                employees->put(3, empl1);
                employees->put(4, empl2);
                employees->put(5, empl3);

                EntryMultiplier processor(4);

                std::map<int, boost::shared_ptr<int> > result = employees->executeOnEntries<int, EntryMultiplier>(
                        processor, query::RegexPredicate("n", ".*met"));

                ASSERT_EQ(2, (int) result.size());
                ASSERT_EQ(true, (result.end() != result.find(3)));
                ASSERT_EQ(true, (result.end() != result.find(4)));
            }

            TEST_F(ClientMapTest, testAddInterceptor) {
                std::string prefix("My Prefix");
                MapGetInterceptor interceptor(prefix);
                imap->addInterceptor<MapGetInterceptor>(interceptor);

                boost::shared_ptr<std::string> val = imap->get("nonexistent");
                ASSERT_NE((std::string *)NULL, val.get());
                ASSERT_EQ(prefix, *val);

                val = imap->put("key1", "value1");
                ASSERT_EQ((std::string *)NULL, val.get());

                val = imap->get("key1");
                ASSERT_NE((std::string *)NULL, val.get());
                ASSERT_EQ(prefix + "value1", *val);
            }
        }
    }
}

