/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
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
                protected:
                    class MapGetInterceptor : public serialization::IdentifiedDataSerializable {
                    public:
                        MapGetInterceptor(const std::string &prefix) : prefix(
                                std::auto_ptr<std::string>(new std::string(prefix))) {}

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

                    /**
                     * This processor validates that the string value for the entry is the same as the test string
                     * "xyzä123 イロハニホヘト チリヌルヲ ワカヨタレソ ツネナラム"
                     */
                    class UTFValueValidatorProcessor : public serialization::IdentifiedDataSerializable {
                    public:
                        virtual int getFactoryId() const {
                            return 666;
                        }

                        virtual int getClassId() const {
                            return 9;
                        }

                        virtual void writeData(serialization::ObjectDataOutput &writer) const {
                        }

                        virtual void readData(serialization::ObjectDataInput &reader) {
                        }
                    };

                    virtual void TearDown() {
                        // clear maps
                        intMap->clear();
                        employees->clear();
                        imap->clear();
                    }

                    static void SetUpTestCase() {
                        #ifdef HZ_BUILD_WITH_SSL
                        instance = new HazelcastServer(*g_srvFactory, true);
                        instance2 = new HazelcastServer(*g_srvFactory, true);
                        #else
                        instance = new HazelcastServer(*g_srvFactory);
                        instance2 = new HazelcastServer(*g_srvFactory);
                        #endif

                        clientConfig = getConfig().release();
                        #ifdef HZ_BUILD_WITH_SSL
                        config::SSLConfig sslConfig;
                        sslConfig.setEnabled(true).setProtocol(config::tlsv1).addVerifyFile(getCAFilePath());
                        clientConfig->getNetworkConfig().setSSLConfig(sslConfig);
                        #endif // HZ_BUILD_WITH_SSL
                        client = new HazelcastClient(*clientConfig);
                        legacyMap = new IMap<std::string, std::string>(
                                client->getMap<std::string, std::string>("RawPointerMapTest"));
                        imap = new client::adaptor::RawPointerMap<std::string, std::string>(*legacyMap);
                        legacyEmployees = new IMap<int, Employee>(client->getMap<int, Employee>("EmployeesMap"));
                        employees = new client::adaptor::RawPointerMap<int, Employee>(*legacyEmployees);
                        legacyIntMap = new IMap<int, int>(client->getMap<int, int>("legacyIntMap"));
                        intMap = new client::adaptor::RawPointerMap<int, int>(*legacyIntMap);
                    }

                    static void TearDownTestCase() {
                        delete intMap;
                        delete legacyIntMap;
                        delete employees;
                        delete legacyEmployees;
                        delete imap;
                        delete legacyMap;
                        delete client;
                        delete clientConfig;
                        delete instance2;
                        delete instance;

                        intMap = NULL;
                        legacyIntMap = NULL;
                        employees = NULL;
                        legacyEmployees = NULL;
                        imap = NULL;
                        legacyMap = NULL;
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
                    static client::adaptor::RawPointerMap<std::string, std::string> *imap;
                    static IMap<std::string, std::string> *legacyMap;
                    static IMap<int, Employee> *legacyEmployees;
                    static client::adaptor::RawPointerMap<int, Employee> *employees;
                    static IMap<int, int> *legacyIntMap;
                    static client::adaptor::RawPointerMap<int, int> *intMap;
                };

                HazelcastServer *RawPointerMapTest::instance = NULL;
                HazelcastServer *RawPointerMapTest::instance2 = NULL;
                ClientConfig *RawPointerMapTest::clientConfig = NULL;
                HazelcastClient *RawPointerMapTest::client = NULL;
                IMap<std::string, std::string> *RawPointerMapTest::legacyMap = NULL;
                client::adaptor::RawPointerMap<std::string, std::string> *RawPointerMapTest::imap = NULL;
                IMap<int, Employee> *RawPointerMapTest::legacyEmployees = NULL;
                client::adaptor::RawPointerMap<int, Employee> *RawPointerMapTest::employees = NULL;
                IMap<int, int> *RawPointerMapTest::legacyIntMap = NULL;
                client::adaptor::RawPointerMap<int, int> *RawPointerMapTest::intMap = NULL;

                void tryPutThread(util::ThreadArgs &args) {
                    util::CountDownLatch *latch = (util::CountDownLatch *) args.arg0;
                    client::adaptor::RawPointerMap<std::string, std::string> *imap = (client::adaptor::RawPointerMap<std::string, std::string> *) args.arg1;
                    bool result = imap->tryPut("key1", "value3", 1 * 1000);
                    if (!result) {
                        latch->countDown();
                    }
                }

                void tryRemoveThread(util::ThreadArgs &args) {
                    util::CountDownLatch *latch = (util::CountDownLatch *) args.arg0;
                    client::adaptor::RawPointerMap<std::string, std::string> *imap = (client::adaptor::RawPointerMap<std::string, std::string> *) args.arg1;
                    bool result = imap->tryRemove("key2", 1 * 1000);
                    if (!result) {
                        latch->countDown();
                    }
                }

                void testLockThread(util::ThreadArgs &args) {
                    util::CountDownLatch *latch = (util::CountDownLatch *) args.arg0;
                    client::adaptor::RawPointerMap<std::string, std::string> *imap = (client::adaptor::RawPointerMap<std::string, std::string> *) args.arg1;
                    imap->tryPut("key1", "value2", 1);
                    latch->countDown();
                }

                void testLockTTLThread(util::ThreadArgs &args) {
                    util::CountDownLatch *latch = (util::CountDownLatch *) args.arg0;
                    client::adaptor::RawPointerMap<std::string, std::string> *imap = (client::adaptor::RawPointerMap<std::string, std::string> *) args.arg1;
                    imap->tryPut("key1", "value2", 5 * 1000);
                    latch->countDown();
                }

                void testLockTTL2Thread(util::ThreadArgs &args) {
                    util::CountDownLatch *latch = (util::CountDownLatch *) args.arg0;
                    client::adaptor::RawPointerMap<std::string, std::string> *imap = (client::adaptor::RawPointerMap<std::string, std::string> *) args.arg1;
                    if (!imap->tryLock("key1")) {
                        latch->countDown();
                    }
                    if (imap->tryLock("key1", 5 * 1000)) {
                        latch->countDown();
                    }
                }

                void testMapTryLockThread1(util::ThreadArgs &args) {
                    util::CountDownLatch *latch = (util::CountDownLatch *) args.arg0;
                    client::adaptor::RawPointerMap<std::string, std::string> *imap = (client::adaptor::RawPointerMap<std::string, std::string> *) args.arg1;
                    if (!imap->tryLock("key1", 2)) {
                        latch->countDown();
                    }
                }

                void testMapTryLockThread2(util::ThreadArgs &args) {
                    util::CountDownLatch *latch = (util::CountDownLatch *) args.arg0;
                    client::adaptor::RawPointerMap<std::string, std::string> *imap = (client::adaptor::RawPointerMap<std::string, std::string> *) args.arg1;
                    if (imap->tryLock("key1", 20 * 1000)) {
                        latch->countDown();
                    }
                }

                void testMapForceUnlockThread(util::ThreadArgs &args) {
                    util::CountDownLatch *latch = (util::CountDownLatch *) args.arg0;
                    client::adaptor::RawPointerMap<std::string, std::string> *imap = (client::adaptor::RawPointerMap<std::string, std::string> *) args.arg1;
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
                    virtual int getFactoryId() const {
                        return 666;
                    }

                    /**
                     * @return class id
                     */
                    virtual int getClassId() const {
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


                class WaitMultiplierProcessor : public serialization::IdentifiedDataSerializable {
                public:
                    WaitMultiplierProcessor(int waitTime, int multiplier)
                            : waiTimeInMillis(waitTime), multiplier(multiplier) { }

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
                        return 8;
                    }

                    /**
                     * Defines how this class will be written.
                     * @param writer ObjectDataOutput
                     */
                    void writeData(serialization::ObjectDataOutput &writer) const {
                        writer.writeInt(waiTimeInMillis);
                        writer.writeInt(multiplier);
                    }

                    /**
                     *Defines how this class will be read.
                     * @param reader ObjectDataInput
                     */
                    void readData(serialization::ObjectDataInput &reader) {
                        waiTimeInMillis = reader.readInt();
                        multiplier = reader.readInt();
                    }

                    int getMultiplier() const {
                        return multiplier;
                    }

                private:
                    int waiTimeInMillis;
                    int multiplier;
                };

                class EntryMultiplierWithNullableResult : public EntryMultiplier {
                public:
                    EntryMultiplierWithNullableResult(int multiplier) : EntryMultiplier(multiplier) { }

                    virtual int getFactoryId() const {
                        return 666;
                    }

                    virtual int getClassId() const {
                        return 7;
                    }
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

                TEST_F(RawPointerMapTest, testRemoveAll) {
                    fillMap();

                    imap->removeAll(
                            query::EqualPredicate<std::string>(query::QueryConstants::getKeyAttributeName(), "key5"));

                    std::auto_ptr<std::string> value = imap->get("key5");

                    ASSERT_NULL("key5 should not exist", value.get(), std::string);

                    query::LikePredicate likeAllValues(query::QueryConstants::getValueAttributeName(), "value%");

                    imap->removeAll(likeAllValues);

                    ASSERT_TRUE(imap->isEmpty());
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
                    ASSERT_NE((std::string *) NULL, key1.get());
                    std::auto_ptr<std::string> value1 = m2->releaseValue(0);
                    ASSERT_NE((std::string *) NULL, value1.get());
                    ASSERT_EQ(*key1, *value1);
                    ASSERT_TRUE(*key1 == "1" || *key1 == "3");

                    std::pair<const std::string *, const std::string *> entry = (*m2)[1];
                    ASSERT_NE((std::string *) NULL, entry.first);
                    ASSERT_NE((std::string *) NULL, entry.second);
                    ASSERT_EQ(*entry.first, *entry.second);
                    ASSERT_TRUE(*entry.first == "1" || *entry.first == "3");
                    ASSERT_NE(*key1, *entry.first);
                }

                TEST_F(RawPointerMapTest, testTryPutRemove) {
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

                TEST_F(RawPointerMapTest, testGetEntryViewForNonExistentData) {
                    std::auto_ptr<MapEntryView<std::string, std::string> > view = imap->getEntryView("non-existent");

                    ASSERT_EQ((MapEntryView<std::string, std::string> *) NULL, view.get());

                    // put an entry that will expire in 1 milliseconds
                    imap->put("short_entry", "short living value", 1);

                    util::sleepmillis(500);

                    view = imap->getEntryView("short_entry");

                    ASSERT_EQ((MapEntryView<std::string, std::string> *) NULL, view.get());
                }

                TEST_F(RawPointerMapTest, testPutTtl) {
                    util::CountDownLatch dummy(10);
                    util::CountDownLatch evict(1);
                    CountdownListener<std::string, std::string> sampleEntryListener(dummy, dummy, dummy, evict);
                    std::string id = imap->addEntryListener(sampleEntryListener, false);

                    imap->put("key1", "value1", 2000);
                    std::auto_ptr<std::string> temp = imap->get("key1");
                    ASSERT_EQ(*temp, "value1");
                    util::sleep(2);
                    // trigger eviction
                    std::auto_ptr<std::string> temp2 = imap->get("key1");
                    ASSERT_EQ(temp2.get(), (std::string *) NULL);
                    ASSERT_TRUE(evict.await(20));

                    ASSERT_TRUE(imap->removeEntryListener(id));
                }

                TEST_F(RawPointerMapTest, testPutConfigTtl) {
                    IMap<std::string, std::string> oneSecMap = client->getMap<std::string, std::string>(
                            "OneSecondTtlMap");
                    hazelcast::client::adaptor::RawPointerMap<std::string, std::string> map(oneSecMap);
                    util::CountDownLatch dummy(10);
                    util::CountDownLatch evict(1);
                    CountdownListener<std::string, std::string> sampleEntryListener(dummy, dummy, dummy, evict);
                    std::string id = map.addEntryListener(sampleEntryListener, false);

                    map.put("key1", "value1");
                    std::auto_ptr<std::string> temp = map.get("key1");
                    ASSERT_EQ(*temp, "value1");
                    util::sleep(2);
                    // trigger eviction
                    std::auto_ptr<std::string> temp2 = map.get("key1");
                    ASSERT_EQ(temp2.get(), (std::string *) NULL);
                    ASSERT_TRUE(evict.await(5));

                    ASSERT_TRUE(map.removeEntryListener(id));
                }

                TEST_F(RawPointerMapTest, testPutIfAbsent) {
                    std::auto_ptr<std::string> o = imap->putIfAbsent("key1", "value1");
                    ASSERT_EQ(o.get(), (std::string *) NULL);
                    ASSERT_EQ("value1", *(imap->putIfAbsent("key1", "value3")));
                }

                TEST_F(RawPointerMapTest, testPutIfAbsentTtl) {
                    ASSERT_EQ(imap->putIfAbsent("key1", "value1", 1000).get(), (std::string *) NULL);
                    ASSERT_EQ("value1", *(imap->putIfAbsent("key1", "value3", 1000)));

                    ASSERT_NULL_EVENTUALLY(imap->putIfAbsent("key1", "value3", 1000).get(), std::string);
                    ASSERT_EQ("value3", *(imap->putIfAbsent("key1", "value4", 1000)));
                }

                TEST_F(RawPointerMapTest, testSet) {
                    imap->set("key1", "value1");
                    ASSERT_EQ("value1", *(imap->get("key1")));

                    imap->set("key1", "value2");
                    ASSERT_EQ("value2", *(imap->get("key1")));

                    imap->set("key1", "value3", 1000);
                    ASSERT_EQ("value3", *(imap->get("key1")));

                    ASSERT_NULL_EVENTUALLY(imap->get("key1").get(), std::string);
                }

                TEST_F(RawPointerMapTest, testSetTtl) {
                    IMap<std::string, std::string> oneSecMap = client->getMap<std::string, std::string>(
                            "OneSecondTtlMap");
                    hazelcast::client::adaptor::RawPointerMap<std::string, std::string> map(oneSecMap);
                    util::CountDownLatch dummy(10);
                    util::CountDownLatch evict(1);
                    CountdownListener<std::string, std::string> sampleEntryListener(dummy, dummy, dummy, evict);
                    std::string id = map.addEntryListener(sampleEntryListener, false);

                    map.set("key1", "value1", 1000);
                    std::auto_ptr<std::string> temp = map.get("key1");
                    ASSERT_EQ(*temp, "value1");
                    util::sleep(2);
                    // trigger eviction
                    std::auto_ptr<std::string> temp2 = map.get("key1");
                    ASSERT_EQ(temp2.get(), (std::string *) NULL);
                    ASSERT_TRUE(evict.await(5));

                    ASSERT_TRUE(map.removeEntryListener(id));
                }

                TEST_F(RawPointerMapTest, testSetConfigTtl) {
                    IMap<std::string, std::string> oneSecMap = client->getMap<std::string, std::string>(
                            "OneSecondTtlMap");
                    hazelcast::client::adaptor::RawPointerMap<std::string, std::string> map(oneSecMap);
                    util::CountDownLatch dummy(10);
                    util::CountDownLatch evict(1);
                    CountdownListener<std::string, std::string> sampleEntryListener(dummy, dummy, dummy, evict);
                    std::string id = map.addEntryListener(sampleEntryListener, false);

                    map.set("key1", "value1");
                    std::auto_ptr<std::string> temp = map.get("key1");
                    ASSERT_EQ(*temp, "value1");
                    util::sleep(2);
                    // trigger eviction
                    std::auto_ptr<std::string> temp2 = map.get("key1");
                    ASSERT_EQ(temp2.get(), (std::string *) NULL);
                    ASSERT_TRUE(evict.await(5));

                    ASSERT_TRUE(map.removeEntryListener(id));
                }

                TEST_F(RawPointerMapTest, testLock) {
                    imap->put("key1", "value1");
                    ASSERT_EQ("value1", *(imap->get("key1")));
                    imap->lock("key1");
                    util::CountDownLatch latch(1);
                    util::Thread t1(testLockThread, &latch, imap);
                    ASSERT_TRUE(latch.await(5));
                    ASSERT_EQ("value1", *(imap->get("key1")));
                    imap->forceUnlock("key1");

                }

                TEST_F(RawPointerMapTest, testLockTtl) {
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

                TEST_F(RawPointerMapTest, testLockTtl2) {
                    imap->lock("key1", 3 * 1000);
                    util::CountDownLatch latch(2);
                    util::Thread t1(testLockTTL2Thread, &latch, imap);
                    ASSERT_TRUE(latch.await(10));
                    imap->forceUnlock("key1");

                }

                TEST_F(RawPointerMapTest, testTryLock) {

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

                TEST_F(RawPointerMapTest, testForceUnlock) {
                    imap->lock("key1");
                    util::CountDownLatch latch(1);
                    util::Thread t2(testMapForceUnlockThread, &latch, imap);
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
                    const int numItems = 20;
                    for (int i = 0; i < numItems; ++i) {
                        intMap->put(i, 2 * i);
                    }

                    std::auto_ptr<DataArray<int> > values = intMap->values();
                    ASSERT_EQ(numItems, (int) values->size());
                    std::vector<int> actualValues;
                    for (int i = 0; i < (int) values->size(); ++i) {
                        ASSERT_NE((const int *) NULL, values->get(i));
                        actualValues.push_back(*values->get(i));
                    }
                    std::sort(actualValues.begin(), actualValues.end());
                    for (int i = 0; i < numItems; ++i) {
                        ASSERT_EQ(2 * i, actualValues[i]);
                    }

                    // EqualPredicate
                    // key == 5
                    values = intMap->values(
                            query::EqualPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5));
                    ASSERT_EQ(1, (int) values->size());
                    ASSERT_NE((const int *) NULL, (*values)[0]);
                    ASSERT_EQ(2 * 5, *((*values)[0]));

                    // value == 8
                    values = intMap->values(
                            query::EqualPredicate<int>(query::QueryConstants::getValueAttributeName(), 8));
                    ASSERT_EQ(1, (int) values->size());
                    ASSERT_NE((const int *) NULL, (*values)[0]);
                    ASSERT_EQ(8, *((*values)[0]));

                    // key == numItems
                    values = intMap->values(
                            query::EqualPredicate<int>(query::QueryConstants::getKeyAttributeName(), numItems));
                    ASSERT_EQ(0, (int) values->size());

                    // NotEqual Predicate
                    // key != 5
                    values = intMap->values(
                            query::NotEqualPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5));
                    ASSERT_EQ(numItems - 1, (int) values->size());
                    actualValues.clear();
                    for (int i = 0; i < (int) values->size(); ++i) {
                        ASSERT_NE((const int *) NULL, values->get(i));
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
                    values = intMap->values(
                            query::NotEqualPredicate<int>(query::QueryConstants::getValueAttributeName(), 8));
                    ASSERT_EQ(numItems - 1, (int) values->size());
                    actualValues.clear();
                    for (int i = 0; i < (int) values->size(); ++i) {
                        ASSERT_NE((const int *) NULL, values->get(i));
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
                    values = intMap->values(query::TruePredicate());
                    ASSERT_EQ(numItems, (int) values->size());
                    actualValues.clear();
                    for (int i = 0; i < (int) values->size(); ++i) {
                        ASSERT_NE((const int *) NULL, values->get(i));
                        actualValues.push_back(*values->get(i));
                    }
                    std::sort(actualValues.begin(), actualValues.end());
                    for (int i = 0; i < numItems; ++i) {
                        ASSERT_EQ(2 * i, actualValues[i]);
                    }

                    // FalsePredicate
                    values = intMap->values(query::FalsePredicate());
                    ASSERT_EQ(0, (int) values->size());

                    // BetweenPredicate
                    // 5 <= key <= 10
                    values = intMap->values(
                            query::BetweenPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5, 10));
                    ASSERT_EQ(6, (int) values->size());
                    actualValues.clear();
                    for (int i = 0; i < (int) values->size(); ++i) {
                        ASSERT_NE((const int *) NULL, values->get(i));
                        actualValues.push_back(*values->get(i));
                    }
                    std::sort(actualValues.begin(), actualValues.end());
                    for (int i = 0; i < 6; ++i) {
                        ASSERT_EQ(2 * (i + 5), actualValues[i]);
                    }

                    // 20 <= key <=30
                    values = intMap->values(
                            query::BetweenPredicate<int>(query::QueryConstants::getKeyAttributeName(), 20, 30));
                    ASSERT_EQ(0, (int) values->size());

                    // GreaterLessPredicate
                    // value <= 10
                    values = intMap->values(
                            query::GreaterLessPredicate<int>(query::QueryConstants::getValueAttributeName(), 10, true,
                                                             true));
                    ASSERT_EQ(6, (int) values->size());
                    actualValues.clear();
                    for (int i = 0; i < (int) values->size(); ++i) {
                        ASSERT_NE((const int *) NULL, values->get(i));
                        actualValues.push_back(*values->get(i));
                    }
                    std::sort(actualValues.begin(), actualValues.end());
                    for (int i = 0; i < 6; ++i) {
                        ASSERT_EQ(2 * i, actualValues[i]);
                    }

                    // key < 7
                    values = intMap->values(
                            query::GreaterLessPredicate<int>(query::QueryConstants::getKeyAttributeName(), 7, false,
                                                             true));
                    ASSERT_EQ(7, (int) values->size());
                    actualValues.clear();
                    for (int i = 0; i < (int) values->size(); ++i) {
                        ASSERT_NE((const int *) NULL, values->get(i));
                        actualValues.push_back(*values->get(i));
                    }
                    std::sort(actualValues.begin(), actualValues.end());
                    for (int i = 0; i < 7; ++i) {
                        ASSERT_EQ(2 * i, actualValues[i]);
                    }

                    // value >= 15
                    values = intMap->values(
                            query::GreaterLessPredicate<int>(query::QueryConstants::getValueAttributeName(), 15, true,
                                                             false));
                    ASSERT_EQ(12, (int) values->size());
                    actualValues.clear();
                    for (int i = 0; i < (int) values->size(); ++i) {
                        ASSERT_NE((const int *) NULL, values->get(i));
                        actualValues.push_back(*values->get(i));
                    }
                    std::sort(actualValues.begin(), actualValues.end());
                    for (int i = 0; i < 12; ++i) {
                        ASSERT_EQ(2 * (i + 8), actualValues[i]);
                    }

                    // key > 5
                    values = intMap->values(
                            query::GreaterLessPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5, false,
                                                             false));
                    ASSERT_EQ(14, (int) values->size());
                    actualValues.clear();
                    for (int i = 0; i < (int) values->size(); ++i) {
                        ASSERT_NE((const int *) NULL, values->get(i));
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
                    values = intMap->values(
                            query::InPredicate<int>(query::QueryConstants::getKeyAttributeName(), inVals));
                    ASSERT_EQ(3, (int) values->size());
                    actualValues.clear();
                    for (int i = 0; i < (int) values->size(); ++i) {
                        ASSERT_NE((const int *) NULL, values->get(i));
                        actualValues.push_back(*values->get(i));
                    }
                    std::sort(actualValues.begin(), actualValues.end());
                    ASSERT_EQ(2 * 4, actualValues[0]);
                    ASSERT_EQ(2 * 10, actualValues[1]);
                    ASSERT_EQ(2 * 19, actualValues[2]);

                    // value in {4, 10, 19}
                    values = intMap->values(
                            query::InPredicate<int>(query::QueryConstants::getValueAttributeName(), inVals));
                    ASSERT_EQ(2, (int) values->size());
                    actualValues.clear();
                    for (int i = 0; i < (int) values->size(); ++i) {
                        ASSERT_NE((const int *) NULL, values->get(i));
                        actualValues.push_back(*values->get(i));
                    }
                    std::sort(actualValues.begin(), actualValues.end());
                    ASSERT_EQ(4, actualValues[0]);
                    ASSERT_EQ(10, actualValues[1]);

                    // InstanceOfPredicate
                    // value instanceof Integer
                    values = intMap->values(query::InstanceOfPredicate("java.lang.Integer"));
                    ASSERT_EQ(20, (int) values->size());
                    actualValues.clear();
                    for (int i = 0; i < (int) values->size(); ++i) {
                        ASSERT_NE((const int *) NULL, values->get(i));
                        actualValues.push_back(*values->get(i));
                    }
                    std::sort(actualValues.begin(), actualValues.end());
                    for (int i = 0; i < numItems; ++i) {
                        ASSERT_EQ(2 * i, actualValues[i]);
                    }

                    values = intMap->values(query::InstanceOfPredicate("java.lang.String"));
                    ASSERT_EQ(0, (int) values->size());

                    // NotPredicate
                    // !(5 <= key <= 10)
                    std::auto_ptr<query::Predicate> bp = std::auto_ptr<query::Predicate>(
                            new query::BetweenPredicate<int>(
                                    query::QueryConstants::getKeyAttributeName(), 5, 10));
                    query::NotPredicate notPredicate(bp);
                    values = intMap->values(notPredicate);
                    ASSERT_EQ(14, (int) values->size());
                    actualValues.clear();
                    for (int i = 0; i < (int) values->size(); ++i) {
                        ASSERT_NE((const int *) NULL, values->get(i));
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
                    values = intMap->values(query::AndPredicate().add(bp).add(inPred));
                    ASSERT_EQ(1, (int) values->size());
                    ASSERT_EQ(10, *(values->release(0)));

                    // OrPredicate
                    // 5 <= key <= 10 OR Values in {4, 10, 19} = values {4, 10, 12, 14, 16, 18, 20}
                    bp = std::auto_ptr<query::Predicate>(
                            new query::BetweenPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5, 10));
                    inPred = std::auto_ptr<query::Predicate>(
                            new query::InPredicate<int>(query::QueryConstants::getValueAttributeName(), inVals));
                    values = intMap->values(query::OrPredicate().add(bp).add(inPred));
                    ASSERT_EQ(7, (int) values->size());
                    actualValues.clear();
                    for (int i = 0; i < (int) values->size(); ++i) {
                        ASSERT_NE((const int *) NULL, values->get(i));
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
                    std::auto_ptr<DataArray<std::string> > strValues = imap->keySet(
                            query::LikePredicate(query::QueryConstants::getValueAttributeName(), "value1"));
                    ASSERT_EQ(1, (int) strValues->size());
                    ASSERT_NE((const std::string *) NULL, strValues->get(0));
                    ASSERT_EQ("key1", *strValues->get(0));

                    // ILikePredicate
                    // value ILIKE "%VALue%1%" : {"myvalue_111_test", "value1", "value10", "value11"}
                    strValues = imap->keySet(
                            query::ILikePredicate(query::QueryConstants::getValueAttributeName(), "%VALue%1%"));
                    ASSERT_EQ(4, (int) strValues->size());
                    std::vector<std::string> actualStrs;
                    for (int i = 0; i < (int) strValues->size(); ++i) {
                        ASSERT_NE((const std::string *) NULL, strValues->get(i));
                        actualStrs.push_back(*strValues->get(i));
                    }
                    std::sort(actualStrs.begin(), actualStrs.end());
                    ASSERT_EQ("key1", actualStrs[0]);
                    ASSERT_EQ("key10", actualStrs[1]);
                    ASSERT_EQ("key11", actualStrs[2]);
                    ASSERT_EQ("key_111_test", actualStrs[3]);

                    // value ILIKE "%VAL%2%" : {"myvalue_22_test", "value2"}
                    strValues = imap->keySet(
                            query::ILikePredicate(query::QueryConstants::getValueAttributeName(), "%VAL%2%"));
                    ASSERT_EQ(2, (int) strValues->size());
                    actualStrs.clear();
                    for (int i = 0; i < (int) strValues->size(); ++i) {
                        ASSERT_NE((const std::string *) NULL, strValues->get(i));
                        actualStrs.push_back(*strValues->get(i));
                    }
                    std::sort(actualStrs.begin(), actualStrs.end());
                    ASSERT_EQ("key2", actualStrs[0]);
                    ASSERT_EQ("key_22_test", actualStrs[1]);

                    // SqlPredicate
                    // __key BETWEEN 4 and 7 : {4, 5, 6, 7} -> {8, 10, 12, 14}
                    char sql[100];
                    util::snprintf(sql, 50, "%s BETWEEN 4 and 7", query::QueryConstants::getKeyAttributeName());
                    values = intMap->values(query::SqlPredicate(sql));
                    ASSERT_EQ(4, (int) values->size());
                    actualValues.clear();
                    for (int i = 0; i < (int) values->size(); ++i) {
                        ASSERT_NE((const int *) NULL, values->get(i));
                        actualValues.push_back(*values->get(i));
                    }
                    std::sort(actualValues.begin(), actualValues.end());
                    for (int i = 0; i < 4; ++i) {
                        ASSERT_EQ(2 * (i + 4), actualValues[i]);
                    }

                    // RegexPredicate
                    // value matches the regex ".*value.*2.*" : {myvalue_22_test, value2}
                    strValues = imap->keySet(
                            query::RegexPredicate(query::QueryConstants::getValueAttributeName(), ".*value.*2.*"));
                    ASSERT_EQ(2, (int) strValues->size());
                    actualStrs.clear();
                    for (int i = 0; i < (int) strValues->size(); ++i) {
                        ASSERT_NE((const std::string *) NULL, strValues->get(i));
                        actualStrs.push_back(*strValues->get(i));
                    }
                    std::sort(actualStrs.begin(), actualStrs.end());
                    ASSERT_EQ("key2", actualStrs[0]);
                    ASSERT_EQ("key_22_test", actualStrs[1]);
                }

                TEST_F(RawPointerMapTest, testValuesWithPagingPredicate) {
                    int predSize = 5;
                    const int totalEntries = 25;

                    for (int i = 0; i < totalEntries; ++i) {
                        intMap->put(i, i);
                    }

                    query::PagingPredicate<int, int> predicate((size_t) predSize);

                    std::auto_ptr<DataArray<int> > values = intMap->values(predicate);
                    ASSERT_EQ(predSize, (int) values->size());
                    for (int i = 0; i < predSize; ++i) {
                        ASSERT_NE((const int *) NULL, values->get(i));
                        ASSERT_EQ(i, *values->get(i));
                    }

                    values = intMap->values(predicate);
                    ASSERT_EQ(predSize, (int) values->size());
                    for (int i = 0; i < predSize; ++i) {
                        ASSERT_NE((const int *) NULL, values->get(i));
                        ASSERT_EQ(i, *values->get(i));
                    }

                    predicate.nextPage();
                    values = intMap->values(predicate);
                    ASSERT_EQ(predSize, (int) values->size());

                    for (int i = 0; i < predSize; ++i) {
                        ASSERT_NE((const int *) NULL, values->get(i));
                        ASSERT_EQ(predSize + i, *values->get(i));
                    }

                    const std::pair<int *, int *> *anchor = predicate.getAnchor();
                    ASSERT_NE((const std::pair<int *, int *> *) NULL, anchor);
                    ASSERT_NE((int *) NULL, anchor->first);
                    ASSERT_NE((int *) NULL, anchor->second);
                    ASSERT_EQ(9, *anchor->first);
                    ASSERT_EQ(9, *anchor->second);

                    ASSERT_EQ(1, (int) predicate.getPage());

                    predicate.setPage(4);

                    values = intMap->values(predicate);
                    ASSERT_EQ(predSize, (int) values->size());

                    for (int i = 0; i < predSize; ++i) {
                        ASSERT_NE((const int *) NULL, values->get(i));
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
                    ASSERT_EQ(3, (int) anchorEntry->first);
                    ASSERT_EQ(19, *anchorEntry->second.first);
                    ASSERT_EQ(19, *anchorEntry->second.second);

                    predicate.nextPage();
                    values = intMap->values(predicate);
                    ASSERT_EQ(0, (int) values->size());

                    predicate.setPage(0);
                    values = intMap->values(predicate);
                    ASSERT_EQ(predSize, (int) values->size());
                    for (int i = 0; i < predSize; ++i) {
                        ASSERT_NE((const int *) NULL, values->get(i));
                        ASSERT_EQ(i, *values->get(i));
                    }

                    predicate.previousPage();
                    ASSERT_EQ(0, (int) predicate.getPage());

                    predicate.setPage(5);
                    values = intMap->values(predicate);
                    ASSERT_EQ(0, (int) values->size());

                    predicate.setPage(3);
                    values = intMap->values(predicate);
                    ASSERT_EQ(predSize, (int) values->size());
                    for (int i = 0; i < predSize; ++i) {
                        ASSERT_NE((const int *) NULL, values->get(i));
                        ASSERT_EQ(3 * predSize + i, *values->get(i));
                    }

                    predicate.previousPage();
                    values = intMap->values(predicate);
                    ASSERT_EQ(predSize, (int) values->size());
                    for (int i = 0; i < predSize; ++i) {
                        ASSERT_NE((const int *) NULL, values->get(i));
                        ASSERT_EQ(2 * predSize + i, *values->get(i));
                    }

                    // test PagingPredicate with inner predicate (value < 10)
                    std::auto_ptr<query::Predicate> lessThanTenPredicate(std::auto_ptr<query::Predicate>(
                            new query::GreaterLessPredicate<int>(query::QueryConstants::getValueAttributeName(), 9,
                                                                 false,
                                                                 true)));
                    query::PagingPredicate<int, int> predicate2(lessThanTenPredicate, 5);
                    values = intMap->values(predicate2);
                    ASSERT_EQ(predSize, (int) values->size());
                    for (int i = 0; i < predSize; ++i) {
                        ASSERT_NE((const int *) NULL, values->get(i));
                        ASSERT_EQ(i, *values->get(i));
                    }

                    predicate2.nextPage();
                    // match values 5,6, 7, 8
                    values = intMap->values(predicate2);
                    ASSERT_EQ(predSize - 1, (int) values->size());
                    for (int i = 0; i < predSize - 1; ++i) {
                        ASSERT_NE((const int *) NULL, values->get(i));
                        ASSERT_EQ(predSize + i, *values->get(i));
                    }

                    predicate2.nextPage();
                    values = intMap->values(predicate2);
                    ASSERT_EQ(0, (int) values->size());

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
                            std::auto_ptr<query::EntryComparator<int, Employee> >(new EmployeeEntryComparator()),
                            (size_t) predSize);
                    std::auto_ptr<DataArray<Employee> > result = employees->values(predicate3);
                    ASSERT_EQ(2, (int) result->size());
                    ASSERT_NE((const Employee *) NULL, (*result)[0]);
                    ASSERT_NE((const Employee *) NULL, (*result)[1]);
                    ASSERT_EQ(empl6, *((*result)[0]));
                    ASSERT_EQ(empl2, *result->get(1));

                    predicate3.nextPage();
                    result = employees->values(predicate3);
                    ASSERT_EQ(2, (int) result->size());
                    ASSERT_NE((const Employee *) NULL, (*result)[0]);
                    ASSERT_NE((const Employee *) NULL, (*result)[1]);
                    ASSERT_EQ(empl3, *((*result)[0]));
                    ASSERT_EQ(empl4, *result->get(1));
                }

                TEST_F(RawPointerMapTest, testKeySetWithPredicate) {
                    const int numItems = 20;
                    for (int i = 0; i < numItems; ++i) {
                        intMap->put(i, 2 * i);
                    }

                    std::auto_ptr<DataArray<int> > values = intMap->keySet();
                    ASSERT_EQ(numItems, (int) values->size());
                    std::vector<int> actualValues;
                    for (int i = 0; i < (int) values->size(); ++i) {
                        ASSERT_NE((const int *) NULL, values->get(i));
                        actualValues.push_back(*values->get(i));
                    }
                    std::sort(actualValues.begin(), actualValues.end());
                    for (int i = 0; i < numItems; ++i) {
                        ASSERT_EQ(i, actualValues[i]);
                    }

                    // EqualPredicate
                    // key == 5
                    values = intMap->keySet(
                            query::EqualPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5));
                    ASSERT_EQ(1, (int) values->size());
                    ASSERT_NE((const int *) NULL, (*values)[0]);
                    ASSERT_EQ(5, *((*values)[0]));

                    // value == 8
                    values = intMap->keySet(
                            query::EqualPredicate<int>(query::QueryConstants::getValueAttributeName(), 8));
                    ASSERT_EQ(1, (int) values->size());
                    ASSERT_NE((const int *) NULL, (*values)[0]);
                    ASSERT_EQ(4, *((*values)[0]));

                    // key == numItems
                    values = intMap->keySet(
                            query::EqualPredicate<int>(query::QueryConstants::getKeyAttributeName(), numItems));
                    ASSERT_EQ(0, (int) values->size());

                    // NotEqual Predicate
                    // key != 5
                    values = intMap->keySet(
                            query::NotEqualPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5));
                    ASSERT_EQ(numItems - 1, (int) values->size());
                    actualValues.clear();
                    for (int i = 0; i < (int) values->size(); ++i) {
                        ASSERT_NE((const int *) NULL, values->get(i));
                        actualValues.push_back(*values->get(i));
                    }
                    std::sort(actualValues.begin(), actualValues.end());
                    for (int i = 0; i < numItems - 1; ++i) {
                        if (i >= 5) {
                            ASSERT_EQ((i + 1), actualValues[i]);
                        } else {
                            ASSERT_EQ(i, actualValues[i]);
                        }
                    }

                    // this(value) != 8
                    values = intMap->keySet(
                            query::NotEqualPredicate<int>(query::QueryConstants::getValueAttributeName(), 8));
                    ASSERT_EQ(numItems - 1, (int) values->size());
                    actualValues.clear();
                    for (int i = 0; i < (int) values->size(); ++i) {
                        ASSERT_NE((const int *) NULL, values->get(i));
                        actualValues.push_back(*values->get(i));
                    }
                    std::sort(actualValues.begin(), actualValues.end());
                    for (int i = 0; i < numItems - 1; ++i) {
                        if (i >= 4) {
                            ASSERT_EQ((i + 1), actualValues[i]);
                        } else {
                            ASSERT_EQ(i, actualValues[i]);
                        }
                    }

                    // TruePredicate
                    values = intMap->keySet(query::TruePredicate());
                    ASSERT_EQ(numItems, (int) values->size());
                    actualValues.clear();
                    for (int i = 0; i < (int) values->size(); ++i) {
                        ASSERT_NE((const int *) NULL, values->get(i));
                        actualValues.push_back(*values->get(i));
                    }
                    std::sort(actualValues.begin(), actualValues.end());
                    for (int i = 0; i < numItems; ++i) {
                        ASSERT_EQ(i, actualValues[i]);
                    }

                    // FalsePredicate
                    values = intMap->keySet(query::FalsePredicate());
                    ASSERT_EQ(0, (int) values->size());

                    // BetweenPredicate
                    // 5 <= key <= 10
                    values = intMap->keySet(
                            query::BetweenPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5, 10));
                    ASSERT_EQ(6, (int) values->size());
                    actualValues.clear();
                    for (int i = 0; i < (int) values->size(); ++i) {
                        ASSERT_NE((const int *) NULL, values->get(i));
                        actualValues.push_back(*values->get(i));
                    }
                    std::sort(actualValues.begin(), actualValues.end());
                    for (int i = 0; i < 6; ++i) {
                        ASSERT_EQ((i + 5), actualValues[i]);
                    }

                    // 20 <= key <=30
                    values = intMap->keySet(
                            query::BetweenPredicate<int>(query::QueryConstants::getKeyAttributeName(), 20, 30));
                    ASSERT_EQ(0, (int) values->size());

                    // GreaterLessPredicate
                    // value <= 10
                    values = intMap->keySet(
                            query::GreaterLessPredicate<int>(query::QueryConstants::getValueAttributeName(), 10, true,
                                                             true));
                    ASSERT_EQ(6, (int) values->size());
                    actualValues.clear();
                    for (int i = 0; i < (int) values->size(); ++i) {
                        ASSERT_NE((const int *) NULL, values->get(i));
                        actualValues.push_back(*values->get(i));
                    }
                    std::sort(actualValues.begin(), actualValues.end());
                    for (int i = 0; i < 6; ++i) {
                        ASSERT_EQ(i, actualValues[i]);
                    }

                    // key < 7
                    values = intMap->keySet(
                            query::GreaterLessPredicate<int>(query::QueryConstants::getKeyAttributeName(), 7, false,
                                                             true));
                    ASSERT_EQ(7, (int) values->size());
                    actualValues.clear();
                    for (int i = 0; i < (int) values->size(); ++i) {
                        ASSERT_NE((const int *) NULL, values->get(i));
                        actualValues.push_back(*values->get(i));
                    }
                    std::sort(actualValues.begin(), actualValues.end());
                    for (int i = 0; i < 7; ++i) {
                        ASSERT_EQ(i, actualValues[i]);
                    }

                    // value >= 15
                    values = intMap->keySet(
                            query::GreaterLessPredicate<int>(query::QueryConstants::getValueAttributeName(), 15, true,
                                                             false));
                    ASSERT_EQ(12, (int) values->size());
                    actualValues.clear();
                    for (int i = 0; i < (int) values->size(); ++i) {
                        ASSERT_NE((const int *) NULL, values->get(i));
                        actualValues.push_back(*values->get(i));
                    }
                    std::sort(actualValues.begin(), actualValues.end());
                    for (int i = 0; i < 12; ++i) {
                        ASSERT_EQ((i + 8), actualValues[i]);
                    }

                    // key > 5
                    values = intMap->keySet(
                            query::GreaterLessPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5, false,
                                                             false));
                    ASSERT_EQ(14, (int) values->size());
                    actualValues.clear();
                    for (int i = 0; i < (int) values->size(); ++i) {
                        ASSERT_NE((const int *) NULL, values->get(i));
                        actualValues.push_back(*values->get(i));
                    }
                    std::sort(actualValues.begin(), actualValues.end());
                    for (int i = 0; i < 14; ++i) {
                        ASSERT_EQ((i + 6), actualValues[i]);
                    }

                    // InPredicate
                    // key in {4, 10, 19}
                    std::vector<int> inVals(3);
                    inVals[0] = 4;
                    inVals[1] = 10;
                    inVals[2] = 19;
                    values = intMap->keySet(
                            query::InPredicate<int>(query::QueryConstants::getKeyAttributeName(), inVals));
                    ASSERT_EQ(3, (int) values->size());
                    actualValues.clear();
                    for (int i = 0; i < (int) values->size(); ++i) {
                        ASSERT_NE((const int *) NULL, values->get(i));
                        actualValues.push_back(*values->get(i));
                    }
                    std::sort(actualValues.begin(), actualValues.end());
                    ASSERT_EQ(4, actualValues[0]);
                    ASSERT_EQ(10, actualValues[1]);
                    ASSERT_EQ(19, actualValues[2]);

                    // value in {4, 10, 19}
                    values = intMap->keySet(
                            query::InPredicate<int>(query::QueryConstants::getValueAttributeName(), inVals));
                    ASSERT_EQ(2, (int) values->size());
                    actualValues.clear();
                    for (int i = 0; i < (int) values->size(); ++i) {
                        ASSERT_NE((const int *) NULL, values->get(i));
                        actualValues.push_back(*values->get(i));
                    }
                    std::sort(actualValues.begin(), actualValues.end());
                    ASSERT_EQ(2, actualValues[0]);
                    ASSERT_EQ(5, actualValues[1]);

                    // InstanceOfPredicate
                    // value instanceof Integer
                    values = intMap->keySet(query::InstanceOfPredicate("java.lang.Integer"));
                    ASSERT_EQ(20, (int) values->size());
                    actualValues.clear();
                    for (int i = 0; i < (int) values->size(); ++i) {
                        ASSERT_NE((const int *) NULL, values->get(i));
                        actualValues.push_back(*values->get(i));
                    }
                    std::sort(actualValues.begin(), actualValues.end());
                    for (int i = 0; i < numItems; ++i) {
                        ASSERT_EQ(i, actualValues[i]);
                    }

                    values = intMap->keySet(query::InstanceOfPredicate("java.lang.String"));
                    ASSERT_EQ(0, (int) values->size());

                    // NotPredicate
                    // !(5 <= key <= 10)
                    std::auto_ptr<query::Predicate> bp = std::auto_ptr<query::Predicate>(
                            new query::BetweenPredicate<int>(
                                    query::QueryConstants::getKeyAttributeName(), 5, 10));
                    query::NotPredicate notPredicate(bp);
                    values = intMap->keySet(notPredicate);
                    ASSERT_EQ(14, (int) values->size());
                    actualValues.clear();
                    for (int i = 0; i < (int) values->size(); ++i) {
                        ASSERT_NE((const int *) NULL, values->get(i));
                        actualValues.push_back(*values->get(i));
                    }
                    std::sort(actualValues.begin(), actualValues.end());
                    for (int i = 0; i < 14; ++i) {
                        if (i >= 5) {
                            ASSERT_EQ((i + 6), actualValues[i]);
                        } else {
                            ASSERT_EQ(i, actualValues[i]);
                        }
                    }

                    // AndPredicate
                    // 5 <= key <= 10 AND Values in {4, 10, 19} = values {4, 10}
                    bp = std::auto_ptr<query::Predicate>(
                            new query::BetweenPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5, 10));
                    std::auto_ptr<query::Predicate> inPred = std::auto_ptr<query::Predicate>(
                            new query::InPredicate<int>(query::QueryConstants::getValueAttributeName(), inVals));
                    values = intMap->keySet(query::AndPredicate().add(bp).add(inPred));
                    ASSERT_EQ(1, (int) values->size());
                    ASSERT_EQ(5, *(values->release(0)));

                    // OrPredicate
                    // 5 <= key <= 10 OR Values in {4, 10, 19} = values {4, 10, 12, 14, 16, 18, 20}
                    bp = std::auto_ptr<query::Predicate>(
                            new query::BetweenPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5, 10));
                    inPred = std::auto_ptr<query::Predicate>(
                            new query::InPredicate<int>(query::QueryConstants::getValueAttributeName(), inVals));
                    values = intMap->keySet(query::OrPredicate().add(bp).add(inPred));
                    ASSERT_EQ(7, (int) values->size());
                    actualValues.clear();
                    for (int i = 0; i < (int) values->size(); ++i) {
                        ASSERT_NE((const int *) NULL, values->get(i));
                        actualValues.push_back(*values->get(i));
                    }
                    std::sort(actualValues.begin(), actualValues.end());
                    ASSERT_EQ(2, actualValues[0]);
                    ASSERT_EQ(5, actualValues[1]);
                    ASSERT_EQ(6, actualValues[2]);
                    ASSERT_EQ(7, actualValues[3]);
                    ASSERT_EQ(8, actualValues[4]);
                    ASSERT_EQ(9, actualValues[5]);
                    ASSERT_EQ(10, actualValues[6]);

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
                    std::auto_ptr<DataArray<std::string> > strValues = imap->keySet(
                            query::LikePredicate(query::QueryConstants::getValueAttributeName(), "value1"));
                    ASSERT_EQ(1, (int) strValues->size());
                    ASSERT_NE((const std::string *) NULL, strValues->get(0));
                    ASSERT_EQ("key1", *strValues->get(0));

                    // ILikePredicate
                    // value ILIKE "%VALue%1%" : {"key_111_test", "key1", "key10", "key11"}
                    strValues = imap->keySet(
                            query::ILikePredicate(query::QueryConstants::getValueAttributeName(), "%VALue%1%"));
                    ASSERT_EQ(4, (int) strValues->size());
                    std::vector<std::string> actualStrs;
                    for (int i = 0; i < (int) strValues->size(); ++i) {
                        ASSERT_NE((const std::string *) NULL, strValues->get(i));
                        actualStrs.push_back(*strValues->get(i));
                    }
                    std::sort(actualStrs.begin(), actualStrs.end());
                    ASSERT_EQ("key1", actualStrs[0]);
                    ASSERT_EQ("key10", actualStrs[1]);
                    ASSERT_EQ("key11", actualStrs[2]);
                    ASSERT_EQ("key_111_test", actualStrs[3]);

                    // value ILIKE "%VAL%2%" : {"key_22_test", "key2"}
                    strValues = imap->keySet(
                            query::ILikePredicate(query::QueryConstants::getValueAttributeName(), "%VAL%2%"));
                    ASSERT_EQ(2, (int) strValues->size());
                    actualStrs.clear();
                    for (int i = 0; i < (int) strValues->size(); ++i) {
                        ASSERT_NE((const std::string *) NULL, strValues->get(i));
                        actualStrs.push_back(*strValues->get(i));
                    }
                    std::sort(actualStrs.begin(), actualStrs.end());
                    ASSERT_EQ("key2", actualStrs[0]);
                    ASSERT_EQ("key_22_test", actualStrs[1]);

                    // SqlPredicate
                    // __key BETWEEN 4 and 7 : {4, 5, 6, 7} -> {8, 10, 12, 14}
                    char sql[100];
                    util::snprintf(sql, 50, "%s BETWEEN 4 and 7", query::QueryConstants::getKeyAttributeName());
                    values = intMap->keySet(query::SqlPredicate(sql));
                    ASSERT_EQ(4, (int) values->size());
                    actualValues.clear();
                    for (int i = 0; i < (int) values->size(); ++i) {
                        ASSERT_NE((const int *) NULL, values->get(i));
                        actualValues.push_back(*values->get(i));
                    }
                    std::sort(actualValues.begin(), actualValues.end());
                    for (int i = 0; i < 4; ++i) {
                        ASSERT_EQ(i + 4, actualValues[i]);
                    }

                    // RegexPredicate
                    // value matches the regex ".*value.*2.*" : {myvalue_22_test, value2}
                    strValues = imap->keySet(
                            query::RegexPredicate(query::QueryConstants::getValueAttributeName(), ".*value.*2.*"));
                    ASSERT_EQ(2, (int) strValues->size());
                    actualStrs.clear();
                    for (int i = 0; i < (int) strValues->size(); ++i) {
                        ASSERT_NE((const std::string *) NULL, strValues->get(i));
                        actualStrs.push_back(*strValues->get(i));
                    }
                    std::sort(actualStrs.begin(), actualStrs.end());
                    ASSERT_EQ("key2", actualStrs[0]);
                    ASSERT_EQ("key_22_test", actualStrs[1]);
                }

                TEST_F(RawPointerMapTest, testKeySetWithPagingPredicate) {
                    int predSize = 5;
                    const int totalEntries = 25;

                    for (int i = 0; i < totalEntries; ++i) {
                        intMap->put(i, i);
                    }

                    query::PagingPredicate<int, int> predicate((size_t) predSize);

                    std::auto_ptr<DataArray<int> > values = intMap->keySet(predicate);
                    ASSERT_EQ(predSize, (int) values->size());
                    for (int i = 0; i < predSize; ++i) {
                        ASSERT_NE((const int *) NULL, values->get(i));
                        ASSERT_EQ(i, *values->get(i));
                    }

                    values = intMap->keySet(predicate);
                    ASSERT_EQ(predSize, (int) values->size());
                    for (int i = 0; i < predSize; ++i) {
                        ASSERT_NE((const int *) NULL, values->get(i));
                        ASSERT_EQ(i, *values->get(i));
                    }

                    predicate.nextPage();
                    values = intMap->keySet(predicate);
                    ASSERT_EQ(predSize, (int) values->size());

                    for (int i = 0; i < predSize; ++i) {
                        ASSERT_NE((const int *) NULL, values->get(i));
                        ASSERT_EQ(predSize + i, *values->get(i));
                    }

                    const std::pair<int *, int *> *anchor = predicate.getAnchor();
                    ASSERT_NE((const std::pair<int *, int *> *) NULL, anchor);
                    ASSERT_NE((int *) NULL, anchor->first);
                    ASSERT_EQ(9, *anchor->first);

                    ASSERT_EQ(1, (int) predicate.getPage());

                    predicate.setPage(4);

                    values = intMap->keySet(predicate);
                    ASSERT_EQ(predSize, (int) values->size());

                    for (int i = 0; i < predSize; ++i) {
                        ASSERT_NE((const int *) NULL, values->get(i));
                        ASSERT_EQ(predSize * 4 + i, *values->get(i));
                    }

                    anchor = predicate.getAnchor();
                    ASSERT_NE((const std::pair<int *, int *> *) NULL, anchor);
                    ASSERT_NE((int *) NULL, anchor->first);
                    ASSERT_EQ(24, *anchor->first);

                    const std::pair<size_t, std::pair<int *, int *> > *anchorEntry = predicate.getNearestAnchorEntry();
                    ASSERT_NE((const std::pair<size_t, std::pair<int *, int *> > *) NULL, anchorEntry);
                    ASSERT_EQ(3, (int) anchorEntry->first);

                    predicate.nextPage();
                    values = intMap->keySet(predicate);
                    ASSERT_EQ(0, (int) values->size());

                    predicate.setPage(0);
                    values = intMap->keySet(predicate);
                    ASSERT_EQ(predSize, (int) values->size());
                    for (int i = 0; i < predSize; ++i) {
                        ASSERT_NE((const int *) NULL, values->get(i));
                        ASSERT_EQ(i, *values->get(i));
                    }

                    predicate.previousPage();
                    ASSERT_EQ(0, (int) predicate.getPage());

                    predicate.setPage(5);
                    values = intMap->keySet(predicate);
                    ASSERT_EQ(0, (int) values->size());

                    predicate.setPage(3);
                    values = intMap->keySet(predicate);
                    ASSERT_EQ(predSize, (int) values->size());
                    for (int i = 0; i < predSize; ++i) {
                        ASSERT_NE((const int *) NULL, values->get(i));
                        ASSERT_EQ(3 * predSize + i, *values->get(i));
                    }

                    predicate.previousPage();
                    values = intMap->keySet(predicate);
                    ASSERT_EQ(predSize, (int) values->size());
                    for (int i = 0; i < predSize; ++i) {
                        const int *value = values->get(i);
                        ASSERT_NE((const int *) NULL, value);
                        ASSERT_EQ(2 * predSize + i, *value);
                    }

                    // test PagingPredicate with inner predicate (value < 10)
                    std::auto_ptr<query::Predicate> lessThanTenPredicate(std::auto_ptr<query::Predicate>(
                            new query::GreaterLessPredicate<int>(query::QueryConstants::getValueAttributeName(), 9,
                                                                 false,
                                                                 true)));
                    query::PagingPredicate<int, int> predicate2(lessThanTenPredicate, 5);
                    values = intMap->keySet(predicate2);
                    ASSERT_EQ(predSize, (int) values->size());
                    for (int i = 0; i < predSize; ++i) {
                        ASSERT_NE((const int *) NULL, values->get(i));
                        ASSERT_EQ(i, *values->get(i));
                    }

                    predicate2.nextPage();
                    // match values 5,6, 7, 8
                    values = intMap->keySet(predicate2);
                    ASSERT_EQ(predSize - 1, (int) values->size());
                    for (int i = 0; i < predSize - 1; ++i) {
                        ASSERT_NE((const int *) NULL, values->get(i));
                        ASSERT_EQ(predSize + i, *values->get(i));
                    }

                    predicate2.nextPage();
                    values = intMap->keySet(predicate2);
                    ASSERT_EQ(0, (int) values->size());

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
                            std::auto_ptr<query::EntryComparator<int, Employee> >(new EmployeeEntryKeyComparator()),
                            (size_t) predSize);
                    std::auto_ptr<DataArray<int> > result = employees->keySet(predicate3);
                    // since keyset result only returns keys from the server, no ordering based on the value but ordered based on the keys
                    ASSERT_EQ(2, (int) result->size());
                    ASSERT_NE((const int *) NULL, (*result)[0]);
                    ASSERT_NE((const int *) NULL, (*result)[1]);
                    ASSERT_EQ(3, *((*result)[0]));
                    ASSERT_EQ(4, *result->get(1));

                    predicate3.nextPage();
                    result = employees->keySet(predicate3);
                    ASSERT_EQ(2, (int) result->size());
                    ASSERT_NE((const int *) NULL, (*result)[0]);
                    ASSERT_NE((const int *) NULL, (*result)[1]);
                    ASSERT_EQ(5, *((*result)[0]));
                    ASSERT_EQ(6, *result->get(1));
                }


                TEST_F(RawPointerMapTest, testEntrySetWithPredicate) {
                    const int numItems = 20;
                    std::vector<std::pair<int, int> > expected(numItems);
                    for (int i = 0; i < numItems; ++i) {
                        intMap->put(i, 2 * i);
                        expected[i] = std::pair<int, int>(i, 2 * i);
                    }

                    std::auto_ptr<EntryArray<int, int> > entries = intMap->entrySet();
                    ASSERT_EQ(numItems, (int) entries->size());
                    entries->sort(query::ENTRY);
                    for (int i = 0; i < numItems; ++i) {
                        std::pair<int, int> entry(*entries->getKey(i), *entries->getValue(i));
                        ASSERT_EQ(expected[i], entry);
                    }

                    // EqualPredicate
                    // key == 5
                    entries = intMap->entrySet(
                            query::EqualPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5));
                    ASSERT_EQ(1, (int) entries->size());
                    std::pair<int, int> entry1(*entries->getKey(0), *entries->getValue(0));
                    ASSERT_EQ(expected[5], entry1);

                    // value == 8
                    entries = intMap->entrySet(
                            query::EqualPredicate<int>(query::QueryConstants::getValueAttributeName(), 8));
                    ASSERT_EQ(1, (int) entries->size());
                    std::pair<int, int> entry2(*entries->getKey(0), *entries->getValue(0));
                    ASSERT_EQ(expected[4], entry2);

                    // key == numItems
                    entries = intMap->entrySet(
                            query::EqualPredicate<int>(query::QueryConstants::getKeyAttributeName(), numItems));
                    ASSERT_EQ(0, (int) entries->size());

                    // NotEqual Predicate
                    // key != 5
                    entries = intMap->entrySet(
                            query::NotEqualPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5));
                    ASSERT_EQ(numItems - 1, (int) entries->size());
                    entries->sort(query::ENTRY);
                    for (int i = 0; i < numItems - 1; ++i) {
                        std::pair<int, int> entry(*entries->getKey(i), *entries->getValue(i));
                        if (i >= 5) {
                            ASSERT_EQ(expected[i + 1], entry);
                        } else {
                            ASSERT_EQ(expected[i], entry);
                        }
                    }

                    // value != 8
                    entries = intMap->entrySet(
                            query::NotEqualPredicate<int>(query::QueryConstants::getValueAttributeName(), 8));
                    ASSERT_EQ(numItems - 1, (int) entries->size());
                    entries->sort(query::ENTRY);
                    for (int i = 0; i < numItems - 1; ++i) {
                        std::pair<int, int> entry(*entries->getKey(i), *entries->getValue(i));
                        if (i >= 4) {
                            ASSERT_EQ(expected[i + 1], entry);
                        } else {
                            ASSERT_EQ(expected[i], entry);
                        }
                    }

                    // TruePredicate
                    entries = intMap->entrySet(query::TruePredicate());
                    ASSERT_EQ(numItems, (int) entries->size());
                    entries->sort(query::ENTRY);
                    for (int i = 0; i < numItems; ++i) {
                        std::pair<int, int> entry(*entries->getKey(i), *entries->getValue(i));
                        ASSERT_EQ(expected[i], entry);
                    }

                    // FalsePredicate
                    entries = intMap->entrySet(query::FalsePredicate());
                    ASSERT_EQ(0, (int) entries->size());

                    // BetweenPredicate
                    // 5 <= key <= 10
                    entries = intMap->entrySet(
                            query::BetweenPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5, 10));
                    entries->sort(query::ENTRY);
                    ASSERT_EQ(6, (int) entries->size());
                    for (int i = 0; i < 6; ++i) {
                        std::pair<int, int> entry(*entries->getKey(i), *entries->getValue(i));
                        ASSERT_EQ(expected[i + 5], entry);
                    }

                    // 20 <= key <=30
                    entries = intMap->entrySet(
                            query::BetweenPredicate<int>(query::QueryConstants::getKeyAttributeName(), 20, 30));
                    ASSERT_EQ(0, (int) entries->size());

                    // GreaterLessPredicate
                    // value <= 10
                    entries = intMap->entrySet(
                            query::GreaterLessPredicate<int>(query::QueryConstants::getValueAttributeName(), 10, true,
                                                             true));
                    ASSERT_EQ(6, (int) entries->size());
                    entries->sort(query::ENTRY);
                    for (int i = 0; i < 6; ++i) {
                        std::pair<int, int> entry(*entries->getKey(i), *entries->getValue(i));
                        ASSERT_EQ(expected[i], entry);
                    }

                    // key < 7
                    entries = intMap->entrySet(
                            query::GreaterLessPredicate<int>(query::QueryConstants::getKeyAttributeName(), 7, false,
                                                             true));
                    ASSERT_EQ(7, (int) entries->size());
                    entries->sort(query::ENTRY);
                    for (int i = 0; i < 7; ++i) {
                        std::pair<int, int> entry(*entries->getKey(i), *entries->getValue(i));
                        ASSERT_EQ(expected[i], entry);
                    }

                    // value >= 15
                    entries = intMap->entrySet(
                            query::GreaterLessPredicate<int>(query::QueryConstants::getValueAttributeName(), 15, true,
                                                             false));
                    ASSERT_EQ(12, (int) entries->size());
                    entries->sort(query::ENTRY);
                    for (int i = 0; i < 12; ++i) {
                        std::pair<int, int> entry(*entries->getKey(i), *entries->getValue(i));
                        ASSERT_EQ(expected[i + 8], entry);
                    }

                    // key > 5
                    entries = intMap->entrySet(
                            query::GreaterLessPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5, false,
                                                             false));
                    ASSERT_EQ(14, (int) entries->size());
                    entries->sort(query::ENTRY);
                    for (int i = 0; i < 14; ++i) {
                        std::pair<int, int> entry(*entries->getKey(i), *entries->getValue(i));
                        ASSERT_EQ(expected[i + 6], entry);
                    }

                    // InPredicate
                    // key in {4, 10, 19}
                    std::vector<int> inVals(3);
                    inVals[0] = 4;
                    inVals[1] = 10;
                    inVals[2] = 19;
                    entries = intMap->entrySet(
                            query::InPredicate<int>(query::QueryConstants::getKeyAttributeName(), inVals));
                    ASSERT_EQ(3, (int) entries->size());
                    entries->sort(query::ENTRY);
                    {
                        std::pair<int, int> entry(*entries->getKey(0), *entries->getValue(0));
                        ASSERT_EQ(expected[4], entry);
                    }
                    {
                        std::pair<int, int> entry(*entries->getKey(1), *entries->getValue(1));
                        ASSERT_EQ(expected[10], entry);
                    }
                    {
                        std::pair<int, int> entry(*entries->getKey(2), *entries->getValue(2));
                        ASSERT_EQ(expected[19], entry);
                    }

                    // value in {4, 10, 19}
                    entries = intMap->entrySet(
                            query::InPredicate<int>(query::QueryConstants::getValueAttributeName(), inVals));
                    ASSERT_EQ(2, (int) entries->size());
                    entries->sort(query::ENTRY);
                    std::pair<int, int> entry(*entries->getKey(0), *entries->getValue(0));
                    ASSERT_EQ(expected[2], entry);
                    entry = std::pair<int, int>(*entries->getKey(1), *entries->getValue(1));
                    ASSERT_EQ(expected[5], entry);

                    // InstanceOfPredicate
                    // value instanceof Integer
                    entries = intMap->entrySet(query::InstanceOfPredicate("java.lang.Integer"));
                    ASSERT_EQ(20, (int) entries->size());
                    entries->sort(query::ENTRY);
                    for (int i = 0; i < numItems; ++i) {
                        std::pair<int, int> item(*entries->getKey(i), *entries->getValue(i));
                        ASSERT_EQ(expected[i], item);
                    }

                    entries = intMap->entrySet(query::InstanceOfPredicate("java.lang.String"));
                    ASSERT_EQ(0, (int) entries->size());

                    // NotPredicate
                    // !(5 <= key <= 10)
                    std::auto_ptr<query::Predicate> bp = std::auto_ptr<query::Predicate>(
                            new query::BetweenPredicate<int>(
                                    query::QueryConstants::getKeyAttributeName(), 5, 10));
                    query::NotPredicate notPredicate(bp);
                    entries = intMap->entrySet(notPredicate);
                    ASSERT_EQ(14, (int) entries->size());
                    entries->sort(query::ENTRY);
                    for (int i = 0; i < 14; ++i) {
                        std::pair<int, int> item(*entries->getKey(i), *entries->getValue(i));
                        if (i >= 5) {
                            ASSERT_EQ(expected[i + 6], item);
                        } else {
                            ASSERT_EQ(expected[i], item);
                        }
                    }

                    // AndPredicate
                    // 5 <= key <= 10 AND Values in {4, 10, 19} = entries {4, 10}
                    bp = std::auto_ptr<query::Predicate>(
                            new query::BetweenPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5, 10));
                    std::auto_ptr<query::Predicate> inPred = std::auto_ptr<query::Predicate>(
                            new query::InPredicate<int>(query::QueryConstants::getValueAttributeName(), inVals));
                    entries = intMap->entrySet(query::AndPredicate().add(bp).add(inPred));
                    ASSERT_EQ(1, (int) entries->size());
                    entries->sort(query::ENTRY);
                    entry = std::pair<int, int>(*entries->getKey(0), *entries->getValue(0));
                    ASSERT_EQ(expected[5], entry);

                    // OrPredicate
                    // 5 <= key <= 10 OR Values in {4, 10, 19} = entries keys {2, 5, 6, 7, 8, 9, 10}
                    bp = std::auto_ptr<query::Predicate>(
                            new query::BetweenPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5, 10));
                    inPred = std::auto_ptr<query::Predicate>(
                            new query::InPredicate<int>(query::QueryConstants::getValueAttributeName(), inVals));
                    entries = intMap->entrySet(query::OrPredicate().add(bp).add(inPred));
                    ASSERT_EQ(7, (int) entries->size());
                    entries->sort(query::ENTRY);
                    for (int i = 0; i < 7; ++i) {
                        entry = std::pair<int, int>(*entries->getKey(i), *entries->getValue(i));
                        if (i == 0) {
                            ASSERT_EQ(expected[2], entry);
                        } else {
                            ASSERT_EQ(expected[i + 4], entry);
                        }
                    }

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
                    std::auto_ptr<EntryArray<std::string, std::string> > strEntries = imap->entrySet(
                            query::LikePredicate(query::QueryConstants::getValueAttributeName(), "value1"));
                    ASSERT_EQ(1, (int) strEntries->size());
                    std::pair<std::string, std::string> strEntry(*strEntries->getKey(0), *strEntries->getValue(0));
                    ASSERT_EQ(expectedStrEntries[1], strEntry);

                    // ILikePredicate
                    // value ILIKE "%VALue%1%" : {"key_111_test", "key1", "key10", "key11"}
                    strEntries = imap->entrySet(
                            query::ILikePredicate(query::QueryConstants::getValueAttributeName(), "%VALue%1%"));
                    ASSERT_EQ(4, (int) strEntries->size());
                    strEntries->sort(query::ENTRY);
                    for (int i = 0; i < 4; ++i) {
                        strEntry = std::pair<std::string, std::string>(*strEntries->getKey(i),
                                                                       *strEntries->getValue(i));
                        if (i == 0) {
                            ASSERT_EQ(expectedStrEntries[1], strEntry);
                        } else {
                            ASSERT_EQ(expectedStrEntries[i + 9], strEntry);
                        }
                    }

                    // key ILIKE "%VAL%2%" : {"key_22_test", "key2"}
                    strEntries = imap->entrySet(
                            query::ILikePredicate(query::QueryConstants::getValueAttributeName(), "%VAL%2%"));
                    ASSERT_EQ(2, (int) strEntries->size());
                    strEntries->sort(query::ENTRY);
                    strEntry = std::pair<std::string, std::string>(*strEntries->getKey(0), *strEntries->getValue(0));
                    ASSERT_EQ(expectedStrEntries[2], strEntry);
                    strEntry = std::pair<std::string, std::string>(*strEntries->getKey(1), *strEntries->getValue(1));
                    ASSERT_EQ(expectedStrEntries[13], strEntry);

                    // SqlPredicate
                    // __key BETWEEN 4 and 7 : {4, 5, 6, 7} -> {8, 10, 12, 14}
                    char sql[100];
                    util::snprintf(sql, 50, "%s BETWEEN 4 and 7", query::QueryConstants::getKeyAttributeName());
                    entries = intMap->entrySet(query::SqlPredicate(sql));
                    ASSERT_EQ(4, (int) entries->size());
                    entries->sort(query::ENTRY);
                    for (int i = 0; i < 4; ++i) {
                        std::pair<int, int> entry(*entries->getKey(i), *entries->getValue(i));
                        ASSERT_EQ(expected[i + 4], entry);
                    }

                    // RegexPredicate
                    // value matches the regex ".*value.*2.*" : {key_22_test, value2}
                    strEntries = imap->entrySet(
                            query::RegexPredicate(query::QueryConstants::getValueAttributeName(), ".*value.*2.*"));
                    ASSERT_EQ(2, (int) strEntries->size());
                    strEntries->sort(query::ENTRY);
                    strEntry = std::pair<std::string, std::string>(*strEntries->getKey(0), *strEntries->getValue(0));
                    ASSERT_EQ(expectedStrEntries[2], strEntry);
                    strEntry = std::pair<std::string, std::string>(*strEntries->getKey(1), *strEntries->getValue(1));
                    ASSERT_EQ(expectedStrEntries[13], strEntry);
                }

                TEST_F(RawPointerMapTest, testEntrySetWithPagingPredicate) {
                    int predSize = 5;
                    const int totalEntries = 25;

                    for (int i = 0; i < totalEntries; ++i) {
                        intMap->put(i, i);
                    }

                    query::PagingPredicate<int, int> predicate((size_t) predSize);

                    std::auto_ptr<EntryArray<int, int> > values = intMap->entrySet(predicate);
                    ASSERT_EQ(predSize, (int) values->size());
                    for (int i = 0; i < predSize; ++i) {
                        std::pair<int, int> expected(i, i);
                        std::pair<int, int> actual(*values->getKey(i), *values->getValue(i));
                        ASSERT_EQ(expected, actual);
                    }

                    values = intMap->entrySet(predicate);
                    ASSERT_EQ(predSize, (int) values->size());
                    for (int i = 0; i < predSize; ++i) {
                        std::pair<int, int> expected(i, i);
                        std::pair<int, int> actual(*values->getKey(i), *values->getValue(i));
                        ASSERT_EQ(expected, actual);
                    }

                    predicate.nextPage();
                    values = intMap->entrySet(predicate);
                    ASSERT_EQ(predSize, (int) values->size());

                    for (int i = 0; i < predSize; ++i) {
                        std::pair<int, int> expected(predSize + i, predSize + i);
                        std::pair<int, int> actual(*values->getKey(i), *values->getValue(i));
                        ASSERT_EQ(expected, actual);
                    }

                    const std::pair<int *, int *> *anchor = predicate.getAnchor();
                    ASSERT_NE((const std::pair<int *, int *> *) NULL, anchor);
                    ASSERT_NE((int *) NULL, anchor->first);
                    ASSERT_NE((int *) NULL, anchor->second);
                    ASSERT_EQ(9, *anchor->first);
                    ASSERT_EQ(9, *anchor->second);

                    ASSERT_EQ(1, (int) predicate.getPage());

                    predicate.setPage(4);

                    values = intMap->entrySet(predicate);
                    ASSERT_EQ(predSize, (int) values->size());
                    for (int i = 0; i < predSize; ++i) {
                        std::pair<int, int> expected(predSize * 4 + i, predSize * 4 + i);
                        std::pair<int, int> actual(*values->getKey(i), *values->getValue(i));
                        ASSERT_EQ(expected, actual);
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
                    ASSERT_EQ(3, (int) anchorEntry->first);
                    ASSERT_EQ(19, *anchorEntry->second.first);
                    ASSERT_EQ(19, *anchorEntry->second.second);

                    predicate.nextPage();
                    values = intMap->entrySet(predicate);
                    ASSERT_EQ(0, (int) values->size());

                    predicate.setPage(0);
                    values = intMap->entrySet(predicate);
                    ASSERT_EQ(predSize, (int) values->size());
                    for (int i = 0; i < predSize; ++i) {
                        std::pair<int, int> expected(i, i);
                        std::pair<int, int> actual(*values->getKey(i), *values->getValue(i));
                        ASSERT_EQ(expected, actual);
                    }

                    predicate.previousPage();
                    ASSERT_EQ(0, (int) predicate.getPage());

                    predicate.setPage(5);
                    values = intMap->entrySet(predicate);
                    ASSERT_EQ(0, (int) values->size());

                    predicate.setPage(3);
                    values = intMap->entrySet(predicate);
                    ASSERT_EQ(predSize, (int) values->size());
                    for (int i = 0; i < predSize; ++i) {
                        std::pair<int, int> expected(3 * predSize + i, 3 * predSize + i);
                        std::pair<int, int> actual(*values->getKey(i), *values->getValue(i));
                        ASSERT_EQ(expected, actual);
                    }

                    predicate.previousPage();
                    values = intMap->entrySet(predicate);
                    ASSERT_EQ(predSize, (int) values->size());
                    for (int i = 0; i < predSize; ++i) {
                        std::pair<int, int> expected(2 * predSize + i, 2 * predSize + i);
                        std::pair<int, int> actual(*values->getKey(i), *values->getValue(i));
                        ASSERT_EQ(expected, actual);
                    }

                    // test PagingPredicate with inner predicate (value < 10)
                    std::auto_ptr<query::Predicate> lessThanTenPredicate(std::auto_ptr<query::Predicate>(
                            new query::GreaterLessPredicate<int>(query::QueryConstants::getValueAttributeName(), 9,
                                                                 false,
                                                                 true)));
                    query::PagingPredicate<int, int> predicate2(lessThanTenPredicate, 5);
                    values = intMap->entrySet(predicate2);
                    ASSERT_EQ(predSize, (int) values->size());
                    for (int i = 0; i < predSize; ++i) {
                        std::pair<int, int> expected(i, i);
                        std::pair<int, int> actual(*values->getKey(i), *values->getValue(i));
                        ASSERT_EQ(expected, actual);
                    }

                    predicate2.nextPage();
                    // match values 5,6, 7, 8
                    values = intMap->entrySet(predicate2);
                    ASSERT_EQ(predSize - 1, (int) values->size());
                    for (int i = 0; i < predSize - 1; ++i) {
                        std::pair<int, int> expected(predSize + i, predSize + i);
                        std::pair<int, int> actual(*values->getKey(i), *values->getValue(i));
                        ASSERT_EQ(expected, actual);
                    }

                    predicate2.nextPage();
                    values = intMap->entrySet(predicate2);
                    ASSERT_EQ(0, (int) values->size());

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
                            std::auto_ptr<query::EntryComparator<int, Employee> >(new EmployeeEntryComparator()),
                            (size_t) predSize);
                    std::auto_ptr<EntryArray<int, Employee> > result = employees->entrySet(predicate3);
                    ASSERT_EQ(2, (int) result->size());
                    std::pair<int, Employee> expected(8, empl6);
                    std::pair<int, Employee> actual(*result->getKey(0), *result->getValue(0));
                    ASSERT_EQ(expected, actual);
                    expected = std::pair<int, Employee>(4, empl2);
                    actual = std::pair<int, Employee>(*result->getKey(1), *result->getValue(1));
                    ASSERT_EQ(expected, actual);
                }

                TEST_F(RawPointerMapTest, testReplace) {
                    std::auto_ptr<std::string> temp = imap->replace("key1", "value");
                    ASSERT_EQ(temp.get(), (std::string *) NULL);

                    std::string tempKey = "key1";
                    std::string tempValue = "value1";
                    imap->put(tempKey, tempValue);

                    ASSERT_EQ("value1", *(imap->replace("key1", "value2")));
                    ASSERT_EQ("value2", *(imap->get("key1")));

                    ASSERT_FALSE(imap->replace("key1", "value1", "value3"));
                    ASSERT_EQ("value2", *(imap->get("key1")));

                    ASSERT_TRUE(imap->replace("key1", "value2", "value3"));
                    ASSERT_EQ("value3", *(imap->get("key1")));
                }

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

                TEST_F(RawPointerMapTest, testListenerWithTruePredicate) {
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
                    std::auto_ptr<int> value = intMap->get(1);
                    ASSERT_NE((int *) NULL, value.get());
                    ASSERT_EQ(5, *value);

                    util::CountDownLatchWaiter latches;
                    latches.add(latchAdd).add(latchRemove).add(latchUpdate).add(latchEvict);
                    ASSERT_TRUE(latches.awaitMillis(2000));

                    ASSERT_TRUE(intMap->removeEntryListener(listenerId));
                }

                TEST_F(RawPointerMapTest, testListenerWithFalsePredicate) {
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
                    std::auto_ptr<int> value = intMap->get(1);
                    ASSERT_NE((int *) NULL, value.get());
                    ASSERT_EQ(5, *value);

                    util::CountDownLatchWaiter latches;
                    latches.add(latchAdd).add(latchRemove).add(latchUpdate).add(latchEvict);
                    ASSERT_FALSE(latches.awaitMillis(2000));

                    ASSERT_TRUE(intMap->removeEntryListener(listenerId));
                }

                TEST_F(RawPointerMapTest, testListenerWithEqualPredicate) {
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
                    std::auto_ptr<int> value = intMap->get(1);
                    ASSERT_NE((int *) NULL, value.get());
                    ASSERT_EQ(5, *value);

                    util::CountDownLatchWaiter latches;
                    latches.add(latchAdd).add(latchEvict);
                    ASSERT_TRUE(latches.awaitMillis(2000));

                    latches.reset();
                    latches.add(latchUpdate).add(latchRemove);
                    ASSERT_FALSE(latches.awaitMillis(2000));

                    ASSERT_TRUE(intMap->removeEntryListener(listenerId));
                }

                TEST_F(RawPointerMapTest, testListenerWithNotEqualPredicate) {
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
                    std::auto_ptr<int> value = intMap->get(1);
                    ASSERT_NE((int *) NULL, value.get());
                    ASSERT_EQ(5, *value);

                    util::CountDownLatchWaiter latches;
                    latches.add(latchAdd).add(latchRemove).add(latchUpdate);
                    ASSERT_TRUE(latches.awaitMillis(2000));

                    latches.reset();
                    latches.add(latchEvict);
                    ASSERT_FALSE(latches.awaitMillis(2000));

                    ASSERT_TRUE(intMap->removeEntryListener(listenerId));
                }

                TEST_F(RawPointerMapTest, testListenerWithGreaterLessPredicate) {
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
                    std::auto_ptr<int> value = intMap->get(1);
                    ASSERT_NE((int *) NULL, value.get());
                    ASSERT_EQ(5, *value);

                    util::CountDownLatchWaiter latches;
                    latches.add(latchAdd).add(latchRemove).add(latchUpdate);
                    ASSERT_TRUE(latches.awaitMillis(2000));

                    ASSERT_FALSE(latchEvict.awaitMillis(2000));

                    ASSERT_TRUE(intMap->removeEntryListener(listenerId));
                }

                TEST_F(RawPointerMapTest, testListenerWithBetweenPredicate) {
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
                    std::auto_ptr<int> value = intMap->get(1);
                    ASSERT_NE((int *) NULL, value.get());
                    ASSERT_EQ(5, *value);

                    util::CountDownLatchWaiter latches;
                    latches.add(latchAdd).add(latchRemove).add(latchUpdate);
                    ASSERT_TRUE(latches.awaitMillis(2000));

                    ASSERT_FALSE(latchEvict.awaitMillis(2000));

                    ASSERT_TRUE(intMap->removeEntryListener(listenerId));
                }

                TEST_F(RawPointerMapTest, testListenerWithSqlPredicate) {
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
                    std::auto_ptr<int> value = intMap->get(1);
                    ASSERT_NE((int *) NULL, value.get());
                    ASSERT_EQ(5, *value);

                    util::CountDownLatchWaiter latches;
                    latches.add(latchAdd).add(latchUpdate);
                    ASSERT_TRUE(latches.awaitMillis(2000));

                    latches.reset();
                    latches.add(latchRemove).add(latchEvict);
                    ASSERT_FALSE(latches.awaitMillis(2000));

                    ASSERT_TRUE(intMap->removeEntryListener(listenerId));
                }

                TEST_F(RawPointerMapTest, testListenerWithRegExPredicate) {
                    util::CountDownLatch latchAdd(2);
                    util::CountDownLatch latchRemove(1);
                    util::CountDownLatch latchEvict(1);
                    util::CountDownLatch latchUpdate(1);

                    CountdownListener<std::string, std::string> listener(latchAdd, latchRemove, latchUpdate,
                                                                         latchEvict);

                    // key matches any word containing ".*met.*"
                    std::string listenerId = imap->addEntryListener(listener, query::RegexPredicate(
                            query::QueryConstants::getKeyAttributeName(), ".*met.*"), true);

                    imap->put("ilkay", "yasar");
                    imap->put("mehmet", "demir");
                    imap->put("metin", "ozen", 1000); // evict after 1 second
                    imap->put("hasan", "can");
                    imap->remove("mehmet");

                    util::sleep(2);

                    ASSERT_EQ((std::string *) NULL, imap->get("metin").get()); // trigger eviction

                    // update an entry
                    imap->set("hasan", "suphi");
                    std::auto_ptr<std::string> value = imap->get("hasan");
                    ASSERT_NE((std::string *) NULL, value.get());
                    ASSERT_EQ("suphi", *value);

                    util::CountDownLatchWaiter latches;
                    latches.add(latchAdd).add(latchRemove).add(latchEvict);
                    ASSERT_TRUE(latches.awaitMillis(2000));

                    ASSERT_FALSE(latchUpdate.awaitMillis(2000));

                    ASSERT_TRUE(imap->removeEntryListener(listenerId));
                }

                TEST_F(RawPointerMapTest, testListenerWithInstanceOfPredicate) {
                    util::CountDownLatch latchAdd(3);
                    util::CountDownLatch latchRemove(1);
                    util::CountDownLatch latchEvict(1);
                    util::CountDownLatch latchUpdate(1);

                    CountdownListener<int, int> listener(latchAdd, latchRemove, latchUpdate, latchEvict);

                    // 1 <=key <= 2
                    std::string listenerId = intMap->addEntryListener(listener,
                                                                      query::InstanceOfPredicate("java.lang.Integer"),
                                                                      false);

                    intMap->put(1, 1);
                    intMap->put(2, 2);
                    intMap->put(3, 3, 1000); // evict after 1 second
                    intMap->remove(2);

                    util::sleep(2);

                    ASSERT_EQ(NULL, intMap->get(3).get()); // trigger eviction

                    // update an entry
                    intMap->set(1, 5);
                    std::auto_ptr<int> value = intMap->get(1);
                    ASSERT_NE((int *) NULL, value.get());
                    ASSERT_EQ(5, *value);

                    util::CountDownLatchWaiter latches;
                    latches.add(latchAdd).add(latchRemove).add(latchUpdate).add(latchEvict);
                    ASSERT_TRUE(latches.awaitMillis(2000));

                    ASSERT_TRUE(intMap->removeEntryListener(listenerId));
                }

                TEST_F(RawPointerMapTest, testListenerWithNotPredicate) {
                    util::CountDownLatch latchAdd(2);
                    util::CountDownLatch latchRemove(1);
                    util::CountDownLatch latchEvict(1);
                    util::CountDownLatch latchUpdate(1);

                    CountdownListener<int, int> listener(latchAdd, latchRemove, latchUpdate, latchEvict);

                    // key >= 3
                    std::auto_ptr<query::Predicate> greaterLessPred = std::auto_ptr<query::Predicate>(
                            new query::GreaterLessPredicate<int>(query::QueryConstants::getKeyAttributeName(), 3, true,
                                                                 false));
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
                    std::auto_ptr<int> value = intMap->get(1);
                    ASSERT_NE((int *) NULL, value.get());
                    ASSERT_EQ(5, *value);

                    util::CountDownLatchWaiter latches;
                    latches.add(latchAdd).add(latchRemove).add(latchUpdate);
                    ASSERT_TRUE(latches.awaitMillis(2000));

                    latches.reset();
                    latches.add(latchEvict);
                    ASSERT_FALSE(latches.awaitMillis(1000));

                    ASSERT_TRUE(intMap->removeEntryListener(listenerId));
                }

                TEST_F(RawPointerMapTest, testListenerWithAndPredicate) {
                    util::CountDownLatch latchAdd(1);
                    util::CountDownLatch latchRemove(1);
                    util::CountDownLatch latchEvict(1);
                    util::CountDownLatch latchUpdate(1);

                    CountdownListener<int, int> listener(latchAdd, latchRemove, latchUpdate, latchEvict);

                    // key < 3
                    std::auto_ptr<query::Predicate> greaterLessPred = std::auto_ptr<query::Predicate>(
                            new query::GreaterLessPredicate<int>(query::QueryConstants::getKeyAttributeName(), 3, false,
                                                                 true));
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
                    std::auto_ptr<int> value = intMap->get(1);
                    ASSERT_NE((int *) NULL, value.get());
                    ASSERT_EQ(5, *value);

                    util::CountDownLatchWaiter latches;
                    latches.add(latchAdd).add(latchUpdate);
                    ASSERT_TRUE(latches.awaitMillis(2000));

                    latches.reset();
                    latches.add(latchEvict).add(latchRemove);
                    ASSERT_FALSE(latches.awaitMillis(1000));

                    ASSERT_TRUE(intMap->removeEntryListener(listenerId));
                }

                TEST_F(RawPointerMapTest, testListenerWithOrPredicate) {
                    util::CountDownLatch latchAdd(2);
                    util::CountDownLatch latchRemove(1);
                    util::CountDownLatch latchEvict(1);
                    util::CountDownLatch latchUpdate(1);

                    CountdownListener<int, int> listener(latchAdd, latchRemove, latchUpdate, latchEvict);

                    // key >= 3
                    std::auto_ptr<query::Predicate> greaterLessPred = std::auto_ptr<query::Predicate>(
                            new query::GreaterLessPredicate<int>(query::QueryConstants::getKeyAttributeName(), 3, true,
                                                                 false));
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
                    std::auto_ptr<int> value = intMap->get(1);
                    ASSERT_NE((int *) NULL, value.get());
                    ASSERT_EQ(5, *value);

                    util::CountDownLatchWaiter latches;
                    latches.add(latchAdd).add(latchEvict).add(latchRemove);
                    ASSERT_TRUE(latches.awaitMillis(2000));

                    ASSERT_FALSE(latchUpdate.awaitMillis(2000));

                    ASSERT_TRUE(intMap->removeEntryListener(listenerId));
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
                    ASSERT_NE((std::string *) NULL, actualVal.get());
                    ASSERT_EQ("value1", *actualVal);

                    std::auto_ptr<hazelcast::client::DataArray<std::string> > tempArray2 = imap->keySet(predicate);

                    const std::string *actual = (*tempArray2)[0];
                    ASSERT_NE((std::string *) NULL, actual);
                    ASSERT_EQ("key1", *actual);


                    std::auto_ptr<hazelcast::client::EntryArray<std::string, std::string> > tempArray3 = imap->entrySet(
                            predicate);
                    actual = tempArray3->getKey(0);
                    ASSERT_NE((std::string *) NULL, actual);
                    ASSERT_EQ("key1", *actual);

                    actual = tempArray3->getValue(0);
                    ASSERT_NE((std::string *) NULL, actual);
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
                    std::auto_ptr<Employee> n1 = employees->get(1);
                    ASSERT_EQ(n1.get(), (Employee *) NULL);
                    Employee employee("sancar", 24);
                    std::auto_ptr<Employee> ptr = employees->put(1, employee);
                    ASSERT_EQ(ptr.get(), (Employee *) NULL);
                    ASSERT_FALSE(employees->isEmpty());
                    std::auto_ptr<hazelcast::client::MapEntryView<int, Employee> > view = employees->getEntryView(1);
                    ASSERT_EQ(*(view->getValue()), employee);
                    ASSERT_EQ(*(view->getKey()), 1);

                    employees->addIndex("a", true);
                    employees->addIndex("n", false);
                }


                TEST_F(RawPointerMapTest, testMapStoreRelatedRequests) {
                    imap->putTransient("ali", "veli", 1100);
                    imap->flush();
                    ASSERT_EQ(1, imap->size());
                    ASSERT_FALSE(imap->evict("deli"));
                    ASSERT_TRUE(imap->evict("ali"));
                    ASSERT_EQ(imap->get("ali").get(), (std::string *) NULL);
                }

                TEST_F(RawPointerMapTest, testExecuteOnKey) {
                    Employee empl1("ahmet", 35);
                    Employee empl2("mehmet", 21);

                    employees->put(3, empl1);
                    employees->put(4, empl2);

                    EntryMultiplier processor(4);

                    std::auto_ptr<int> result = employees->executeOnKey<int, EntryMultiplier>(4, processor);

                    ASSERT_NE((int *) NULL, result.get());
                    ASSERT_EQ(4 * processor.getMultiplier(), *result);
                }

                TEST_F(RawPointerMapTest, testExecuteOnNonExistentKey) {
                    EntryMultiplier processor(4);

                    std::auto_ptr<int> result = employees->executeOnKey<int, EntryMultiplier>(17, processor);

                    ASSERT_NE((int *) NULL, result.get());
                    ASSERT_EQ(-1, *result);
                }

                TEST_F(RawPointerMapTest, testSubmitToKey) {
                    Employee empl1("ahmet", 35);
                    Employee empl2("mehmet", 21);

                    employees->put(3, empl1);
                    employees->put(4, empl2);

                    // Waits at the server side before running the operation
                    WaitMultiplierProcessor processor(3000, 4);

                    Future<int> initialFuture =
                            employees->submitToKey<int, WaitMultiplierProcessor>(
                                    4, processor);

                    // Should invalidate the initialFuture
                    Future<int> future = initialFuture;

                    ASSERT_FALSE(initialFuture.valid());
                    ASSERT_THROW(initialFuture.wait_for(1000), exception::FutureUninitialized);
                    ASSERT_TRUE(future.valid());

                    future_status status = future.wait_for(1 * 1000);
                    ASSERT_EQ(future_status::timeout, status);
                    ASSERT_TRUE(future.valid());

                    status = future.wait_for(3 * 1000);
                    ASSERT_EQ(future_status::ready, status);
                    std::auto_ptr<int> result = future.get();
                    ASSERT_NE((int *) NULL, result.get());
                    ASSERT_EQ(4 * processor.getMultiplier(), *result);
                    ASSERT_FALSE(future.valid());
                }

                TEST_F(RawPointerMapTest, testSubmitToKeyMultipleAsyncCalls) {
                    Employee empl1("ahmet", 35);
                    Employee empl2("mehmet", 21);

                    employees->put(3, empl1);
                    employees->put(4, empl2);

                    int waitTimeInMillis = 500;

                    // Waits at the server side before running the operation
                    WaitMultiplierProcessor processor(waitTimeInMillis, 4);

                    std::vector<Future<int> > allFutures;

                    // test putting into a vector of futures
                    Future<int> future = employees->submitToKey<int, WaitMultiplierProcessor>(
                            3, processor);
                    allFutures.push_back(future);

                    // test re-assigning a future and putting into the vector
                    future = employees->submitToKey<int, WaitMultiplierProcessor>(
                            3, processor);
                    allFutures.push_back(future);

                    // test submitting a non-existent key
                    allFutures.push_back(employees->submitToKey<int, WaitMultiplierProcessor>(
                            99, processor));

                    for (std::vector<Future<int> >::const_iterator it = allFutures.begin();
                         it != allFutures.end(); ++it) {
                        future_status status = (*it).wait_for(2 * waitTimeInMillis);
                        ASSERT_EQ(future_status::ready, status);
                    }

                    for (std::vector<Future<int> >::iterator it = allFutures.begin(); it != allFutures.end(); ++it) {
                        std::auto_ptr<int> result = (*it).get();
                        ASSERT_NE((int *) NULL, result.get());
                        ASSERT_FALSE((*it).valid());
                    }
                }

                TEST_F(RawPointerMapTest, testExecuteOnKeys) {
                    Employee empl1("ahmet", 35);
                    Employee empl2("mehmet", 21);
                    Employee empl3("deniz", 25);

                    employees->put(3, empl1);
                    employees->put(4, empl2);
                    employees->put(5, empl3);

                    EntryMultiplierWithNullableResult processor(4);

                    std::set<int> keys;
                    keys.insert(3);
                    keys.insert(5);
                    // put non existent key
                    keys.insert(999);

                    std::auto_ptr<hazelcast::client::EntryArray<int, int> > result =
                            employees->executeOnKeys<int, EntryMultiplier>(keys, processor);

                    ASSERT_EQ(2, (int) result->size());
                    ASSERT_NE((const int *) NULL, result->getKey(0));
                    ASSERT_NE((const int *) NULL, result->getKey(1));
                    int key0 = *result->getKey(0);
                    ASSERT_TRUE(3 == key0 || 5 == key0);
                    int key1 = *result->getKey(1);
                    ASSERT_TRUE(3 == key1 || 5 == key1);
                    ASSERT_NE(key0, key1);
                    ASSERT_EQ(key0 * processor.getMultiplier(), *result->getValue(0));
                    ASSERT_EQ(key1 * processor.getMultiplier(), *result->getValue(1));
                }

                TEST_F(RawPointerMapTest, testExecuteOnEntries) {
                    Employee empl1("ahmet", 35);
                    Employee empl2("mehmet", 21);
                    Employee empl3("deniz", 25);

                    employees->put(3, empl1);
                    employees->put(4, empl2);
                    employees->put(5, empl3);

                    EntryMultiplier processor(4);

                    std::auto_ptr<hazelcast::client::EntryArray<int, int> > result = employees->executeOnEntries<int, EntryMultiplier>(
                            processor);

                    ASSERT_EQ(3, (int) result->size());
                    for (size_t i = 0; i < result->size(); ++i) {
                        const int *key = result->getKey(i);
                        const int *value = result->getValue(i);
                        ASSERT_TRUE(*key == 3 || *key == 4 || *key == 5);
                        ASSERT_EQ((*key) * processor.getMultiplier(), (*value));
                    }
                }

                TEST_F(RawPointerMapTest, testExecuteOnEntriesWithTruePredicate) {
                    Employee empl1("ahmet", 35);
                    Employee empl2("mehmet", 21);
                    Employee empl3("deniz", 25);

                    employees->put(3, empl1);
                    employees->put(4, empl2);
                    employees->put(5, empl3);

                    EntryMultiplier processor(4);

                    std::auto_ptr<hazelcast::client::EntryArray<int, int> > result =
                            employees->executeOnEntries<int, EntryMultiplier>(processor, query::TruePredicate());

                    ASSERT_EQ(3, (int) result->size());
                    for (size_t i = 0; i < result->size(); ++i) {
                        const int *key = result->getKey(i);
                        const int *value = result->getValue(i);
                        ASSERT_TRUE(*key == 3 || *key == 4 || *key == 5);
                        ASSERT_EQ((*key) * processor.getMultiplier(), (*value));
                    }
                }

                TEST_F(RawPointerMapTest, testExecuteOnEntriesWithFalsePredicate) {
                    Employee empl1("ahmet", 35);
                    Employee empl2("mehmet", 21);
                    Employee empl3("deniz", 25);

                    employees->put(3, empl1);
                    employees->put(4, empl2);
                    employees->put(5, empl3);

                    EntryMultiplier processor(4);

                    std::auto_ptr<hazelcast::client::EntryArray<int, int> > result =
                            employees->executeOnEntries<int, EntryMultiplier>(processor, query::FalsePredicate());

                    ASSERT_EQ(0, (int) result->size());
                }

                TEST_F(RawPointerMapTest, testExecuteOnEntriesWithAndPredicate) {
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

                    std::auto_ptr<hazelcast::client::EntryArray<int, int> > result = employees->executeOnEntries<int, EntryMultiplier>(
                            processor, andPredicate);

                    ASSERT_EQ(1, (int) result->size());
                    ASSERT_EQ(5, *result->getKey(0));
                    ASSERT_EQ(5 * processor.getMultiplier(), *result->getValue(0));
                }

                TEST_F(RawPointerMapTest, testExecuteOnEntriesWithOrPredicate) {
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
                            std::auto_ptr<query::Predicate>(
                                    new query::GreaterLessPredicate<int>("a", 25, false, false)));

                    EntryMultiplier processor(4);

                    std::auto_ptr<hazelcast::client::EntryArray<int, int> > result = employees->executeOnEntries<int, EntryMultiplier>(
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
                    Employee empl1("ahmet", 35);
                    Employee empl2("mehmet", 21);
                    Employee empl3("deniz", 25);

                    employees->put(3, empl1);
                    employees->put(4, empl2);
                    employees->put(5, empl3);

                    EntryMultiplier processor(4);

                    std::auto_ptr<hazelcast::client::EntryArray<int, int> > result = employees->executeOnEntries<int, EntryMultiplier>(
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
                    Employee empl1("ahmet", 35);
                    Employee empl2("mehmet", 21);
                    Employee empl3("deniz", 25);

                    employees->put(3, empl1);
                    employees->put(4, empl2);
                    employees->put(5, empl3);

                    EntryMultiplier processor(4);

                    std::auto_ptr<hazelcast::client::EntryArray<int, int> > result = employees->executeOnEntries<int, EntryMultiplier>(
                            processor, query::EqualPredicate<int>("a", 25));

                    ASSERT_EQ(1, (int) result->size());
                    ASSERT_EQ(5, *result->getKey(0));
                    ASSERT_EQ(5 * processor.getMultiplier(), *result->getValue(0));
                }

                TEST_F(RawPointerMapTest, testExecuteOnEntriesWithNotEqualPredicate) {
                    Employee empl1("ahmet", 35);
                    Employee empl2("mehmet", 21);
                    Employee empl3("deniz", 25);

                    employees->put(3, empl1);
                    employees->put(4, empl2);
                    employees->put(5, empl3);

                    EntryMultiplier processor(4);

                    std::auto_ptr<hazelcast::client::EntryArray<int, int> > result = employees->executeOnEntries<int, EntryMultiplier>(
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
                    Employee empl1("ahmet", 35);
                    Employee empl2("mehmet", 21);
                    Employee empl3("deniz", 25);

                    employees->put(3, empl1);
                    employees->put(4, empl2);
                    employees->put(5, empl3);

                    EntryMultiplier processor(4);

                    std::auto_ptr<hazelcast::client::EntryArray<int, int> > result = employees->executeOnEntries<int, EntryMultiplier>(
                            processor, query::GreaterLessPredicate<int>("a", 25, false, true)); // <25 matching

                    ASSERT_EQ(1, (int) result->size());
                    ASSERT_EQ(4, *result->getKey(0));
                    ASSERT_EQ(4 * processor.getMultiplier(), *result->getValue(0));

                    result = employees->executeOnEntries<int, EntryMultiplier>(
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

                    result = employees->executeOnEntries<int, EntryMultiplier>(
                            processor, query::GreaterLessPredicate<int>("a", 25, false, false)); // >25 matching

                    ASSERT_EQ(1, (int) result->size());
                    ASSERT_EQ(3, *result->getKey(0));
                    ASSERT_EQ(3 * processor.getMultiplier(), *result->getValue(0));

                    result = employees->executeOnEntries<int, EntryMultiplier>(
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
                    Employee empl1("ahmet", 35);
                    Employee empl2("mehmet", 21);
                    Employee empl3("deniz", 25);

                    employees->put(3, empl1);
                    employees->put(4, empl2);
                    employees->put(5, empl3);

                    EntryMultiplier processor(4);

                    std::auto_ptr<hazelcast::client::EntryArray<int, int> > result = employees->executeOnEntries<int, EntryMultiplier>(
                            processor, query::LikePredicate("n", "deniz"));

                    ASSERT_EQ(1, (int) result->size());
                    ASSERT_EQ(5, *result->getKey(0));
                    ASSERT_EQ(5 * processor.getMultiplier(), *result->getValue(0));
                }

                TEST_F(RawPointerMapTest, testExecuteOnEntriesWithILikePredicate) {
                    Employee empl1("ahmet", 35);
                    Employee empl2("mehmet", 21);
                    Employee empl3("deniz", 25);

                    employees->put(3, empl1);
                    employees->put(4, empl2);
                    employees->put(5, empl3);

                    EntryMultiplier processor(4);

                    std::auto_ptr<hazelcast::client::EntryArray<int, int> > result = employees->executeOnEntries<int, EntryMultiplier>(
                            processor, query::ILikePredicate("n", "deniz"));

                    ASSERT_EQ(1, (int) result->size());
                    ASSERT_EQ(5, *result->getKey(0));
                    ASSERT_EQ(5 * processor.getMultiplier(), *result->getValue(0));
                }

                TEST_F(RawPointerMapTest, testExecuteOnEntriesWithInPredicate) {
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
                    std::auto_ptr<hazelcast::client::EntryArray<int, int> > result = employees->executeOnEntries<int, EntryMultiplier>(
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
                    Employee empl1("ahmet", 35);
                    Employee empl2("mehmet", 21);
                    Employee empl3("deniz", 25);

                    employees->put(3, empl1);
                    employees->put(4, empl2);
                    employees->put(5, empl3);

                    EntryMultiplier processor(4);
                    std::auto_ptr<hazelcast::client::EntryArray<int, int> > result = employees->executeOnEntries<int, EntryMultiplier>(
                            processor, query::InstanceOfPredicate("Employee"));

                    ASSERT_EQ(3, (int) result->size());
                }

                TEST_F(RawPointerMapTest, testExecuteOnEntriesWithNotPredicate) {
                    Employee empl1("ahmet", 35);
                    Employee empl2("mehmet", 21);
                    Employee empl3("deniz", 25);

                    employees->put(3, empl1);
                    employees->put(4, empl2);
                    employees->put(5, empl3);

                    EntryMultiplier processor(4);
                    query::NotPredicate notEqualPredicate(
                            std::auto_ptr<query::Predicate>(new query::EqualPredicate<int>("a", 25)));
                    std::auto_ptr<hazelcast::client::EntryArray<int, int> > result =
                            employees->executeOnEntries<int, EntryMultiplier>(processor, notEqualPredicate);

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
                    result = employees->executeOnEntries<int, EntryMultiplier>(processor, notFalsePredicate);

                    ASSERT_EQ(3, (int) result->size());

                    query::NotPredicate notBetweenPredicate(
                            std::auto_ptr<query::Predicate>(new query::BetweenPredicate<int>("a", 25, 35)));
                    result = employees->executeOnEntries<int, EntryMultiplier>(processor, notBetweenPredicate);

                    ASSERT_EQ(1, (int) result->size());
                    ASSERT_EQ(4, *result->getKey(0));
                    ASSERT_EQ(4 * processor.getMultiplier(), *result->getValue(0));
                }

                TEST_F(RawPointerMapTest, testExecuteOnEntriesWithRegexPredicate) {
                    Employee empl1("ahmet", 35);
                    Employee empl2("mehmet", 21);
                    Employee empl3("deniz", 25);

                    employees->put(3, empl1);
                    employees->put(4, empl2);
                    employees->put(5, empl3);

                    EntryMultiplier processor(4);

                    std::auto_ptr<hazelcast::client::EntryArray<int, int> > result = employees->executeOnEntries<int, EntryMultiplier>(
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

                TEST_F(RawPointerMapTest, testAddInterceptor) {
                    std::string prefix("My Prefix");
                    MapGetInterceptor interceptor(prefix);
                    imap->addInterceptor<MapGetInterceptor>(interceptor);

                    std::auto_ptr<std::string> val = imap->get("nonexistent");
                    ASSERT_NE((std::string *) NULL, val.get());
                    ASSERT_EQ(prefix, *val);

                    val = imap->put("key1", "value1");
                    ASSERT_EQ((std::string *) NULL, val.get());

                    val = imap->get("key1");
                    ASSERT_NE((std::string *) NULL, val.get());
                    ASSERT_EQ(prefix + "value1", *val);
                }

                TEST_F(RawPointerMapTest, testReadUTFWrittenByJava) {
                    std::string value = "xyzä123 イロハニホヘト チリヌルヲ ワカヨタレソ ツネナラム";
                    std::string key = "myutfkey";
                    IMap<std::string, std::string> map = client->getMap<std::string, std::string>(
                            "testReadUTFWrittenByJavaMap");
                    map.put(key, value);
                    UTFValueValidatorProcessor processor;
                    boost::shared_ptr<bool> result = map.executeOnKey<bool, UTFValueValidatorProcessor>(key, processor);
                    ASSERT_NOTNULL(result.get(), bool);
                    ASSERT_TRUE(*result);
                }
            }
        }
    }
}

