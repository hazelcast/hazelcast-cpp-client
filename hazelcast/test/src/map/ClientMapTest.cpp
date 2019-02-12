/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
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

/**
 * This has to be the first include, so that Python.h is the first include. Otherwise, compilation warning such as
 * "_POSIX_C_SOURCE" redefined occurs.
 */
#include "HazelcastServerFactory.h"

#include "HazelcastServer.h"
#include "serialization/Employee.h"
#include "TestHelperFunctions.h"
#include "ClientTestSupport.h"

#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/IMap.h"
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

namespace hazelcast {
    namespace client {
        namespace test {
            class PartitionAwareInt : public serialization::IdentifiedDataSerializable, public PartitionAware<int> {
            public:
                PartitionAwareInt() : partitionKey(0), actualKey(0) {}

                PartitionAwareInt(int partitionKey, int actualKey)
                        : partitionKey(partitionKey), actualKey(actualKey) {}

                virtual const int *getPartitionKey() const {
                    return &partitionKey;
                }

                int getActualKey() const {
                    return actualKey;
                }

                virtual int getFactoryId() const {
                    return 666;
                }

                virtual int getClassId() const {
                    return 9;
                }

                virtual void writeData(serialization::ObjectDataOutput &writer) const {
                    writer.writeInt(actualKey);
                }

                virtual void readData(serialization::ObjectDataInput &reader) {
                    actualKey = reader.readInt();
                }

            private:
                int partitionKey;
                int actualKey;
            };

            bool operator<(const PartitionAwareInt &lhs, const PartitionAwareInt &rhs) {
                return lhs.getActualKey() < rhs.getActualKey();
            }

            class MapClientConfig : public ClientConfig {
            public:
                static const char *intMapName;
                static const char *employeesMapName;
                static const char *imapName;
                static const std::string ONE_SECOND_MAP_NAME;

                MapClientConfig() {
                    addAddress(Address(g_srvFactory->getServerAddress(), 5701));
                }

                virtual ~MapClientConfig() {
                }
            };

            const char *MapClientConfig::intMapName = "IntMap";
            const char *MapClientConfig::employeesMapName = "EmployeesMap";
            const char *MapClientConfig::imapName = "clientMapTest";
            const std::string MapClientConfig::ONE_SECOND_MAP_NAME = "OneSecondTtlMap";

            class NearCachedDataMapClientConfig : public MapClientConfig {
            public:
                NearCachedDataMapClientConfig() {
                    addNearCacheConfig<int, int>(boost::shared_ptr<config::NearCacheConfig<int, int> >(new config::NearCacheConfig<int, int>(intMapName)));

                    addNearCacheConfig<int, Employee>(boost::shared_ptr<config::NearCacheConfig<int, Employee> >(new config::NearCacheConfig<int, Employee>(employeesMapName)));

                    addNearCacheConfig<std::string, std::string>(boost::shared_ptr<config::NearCacheConfig<std::string, std::string> >(new config::NearCacheConfig<std::string, std::string>(imapName)));

                    addNearCacheConfig<std::string, std::string>(boost::shared_ptr<config::NearCacheConfig<std::string, std::string> >(new config::NearCacheConfig<std::string, std::string>(ONE_SECOND_MAP_NAME)));
                }
            };

            class NearCachedObjectMapClientConfig : public MapClientConfig {
            public:
                NearCachedObjectMapClientConfig() {
                    addNearCacheConfig<int, int>(boost::shared_ptr<config::NearCacheConfig<int, int> >(new config::NearCacheConfig<int, int>(intMapName, config::OBJECT)));

                    addNearCacheConfig<int, Employee>(boost::shared_ptr<config::NearCacheConfig<int, Employee> >(new config::NearCacheConfig<int, Employee>(employeesMapName, config::OBJECT)));

                    addNearCacheConfig<std::string, std::string>(boost::shared_ptr<config::NearCacheConfig<std::string, std::string> >(new config::NearCacheConfig<std::string, std::string>(imapName, config::OBJECT)));

                    addNearCacheConfig<std::string, std::string>(boost::shared_ptr<config::NearCacheConfig<std::string, std::string> >(new config::NearCacheConfig<std::string, std::string>(ONE_SECOND_MAP_NAME, config::OBJECT)));
                }
            };

            template<typename CONFIGTYPE>
            class ClientMapTest : public ClientTestSupport {
            public:
                static void SetUpTestCase() {
                    instance = new HazelcastServer(*g_srvFactory);
                    instance2 = new HazelcastServer(*g_srvFactory);

                    clientConfig = new CONFIGTYPE();

                    client = new HazelcastClient(*clientConfig);
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
            protected:
                class MapGetInterceptor : public serialization::IdentifiedDataSerializable {
                public:
                    MapGetInterceptor(const std::string &prefix) : prefix(
                            std::auto_ptr<std::string>(new std::string(prefix))) { }

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

                virtual void SetUp() {
                    imap = new IMap<std::string, std::string>(
                            client->getMap<std::string, std::string>(MapClientConfig::imapName));
                    intMap = new IMap<int, int>(client->getMap<int, int>(MapClientConfig::intMapName));
                    employees = new IMap<int, Employee>(
                            client->getMap<int, Employee>(MapClientConfig::employeesMapName));
                }

                virtual void TearDown() {
                    // clear maps
                    employees->destroy();
                    intMap->destroy();
                    imap->destroy();
                    client->getMap<std::string, std::string>(MapClientConfig::ONE_SECOND_MAP_NAME).destroy();
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

                static void tryPutThread(util::ThreadArgs &args) {
                    util::CountDownLatch *latch = (util::CountDownLatch *) args.arg0;
                    IMap<std::string, std::string> *imap = (IMap<std::string, std::string> *) args.arg1;
                    bool result = imap->tryPut("key1", "value3", 1 * 1000);
                    if (!result) {
                        latch->countDown();
                    }
                }

                static void tryRemoveThread(util::ThreadArgs &args) {
                    util::CountDownLatch *latch = (util::CountDownLatch *) args.arg0;
                    IMap<std::string, std::string> *imap = (IMap<std::string, std::string> *) args.arg1;
                    bool result = imap->tryRemove("key2", 1 * 1000);
                    if (!result) {
                        latch->countDown();
                    }
                }

                static void testLockThread(util::ThreadArgs &args) {
                    util::CountDownLatch *latch = (util::CountDownLatch *) args.arg0;
                    IMap<std::string, std::string> *imap = (IMap<std::string, std::string> *) args.arg1;
                    imap->tryPut("key1", "value2", 1);
                    latch->countDown();
                }

                static void testLockTTLThread(util::ThreadArgs &args) {
                    util::CountDownLatch *latch = (util::CountDownLatch *) args.arg0;
                    IMap<std::string, std::string> *imap = (IMap<std::string, std::string> *) args.arg1;
                    imap->tryPut("key1", "value2", 5 * 1000);
                    latch->countDown();
                }

                static void testLockTTL2Thread(util::ThreadArgs &args) {
                    util::CountDownLatch *latch = (util::CountDownLatch *) args.arg0;
                    IMap<std::string, std::string> *imap = (IMap<std::string, std::string> *) args.arg1;
                    if (!imap->tryLock("key1")) {
                        latch->countDown();
                    }
                    if (imap->tryLock("key1", 5 * 1000)) {
                        latch->countDown();
                    }
                }

                static void testMapTryLockThread1(util::ThreadArgs &args) {
                    util::CountDownLatch *latch = (util::CountDownLatch *) args.arg0;
                    IMap<std::string, std::string> *imap = (IMap<std::string, std::string> *) args.arg1;
                    if (!imap->tryLock("key1", 2)) {
                        latch->countDown();
                    }
                }

                static void testMapTryLockThread2(util::ThreadArgs &args) {
                    util::CountDownLatch *latch = (util::CountDownLatch *) args.arg0;
                    IMap<std::string, std::string> *imap = (IMap<std::string, std::string> *) args.arg1;
                    if (imap->tryLock("key1", 20 * 1000)) {
                        latch->countDown();
                    }
                }

                static void testMapForceUnlockThread(util::ThreadArgs &args) {
                    util::CountDownLatch *latch = (util::CountDownLatch *) args.arg0;
                    IMap<std::string, std::string> *imap = (IMap<std::string, std::string> *) args.arg1;
                    imap->forceUnlock("key1");
                    latch->countDown();
                }

                template<typename K, typename V>
                class EvictedEntryListener : public EntryAdapter<K, V> {
                public:
                    EvictedEntryListener(const boost::shared_ptr<CountDownLatch> &evictLatch) : evictLatch(
                            evictLatch) {}

                    virtual void entryEvicted(const EntryEvent<K, V> &event) {
                        evictLatch->countDown();
                    }

                private:
                    boost::shared_ptr<util::CountDownLatch> evictLatch;
                };

                template<typename K, typename V>
                class CountdownListener : public EntryAdapter<K, V> {
                public:
                    CountdownListener(util::CountDownLatch &addLatch, util::CountDownLatch &removeLatch,
                                      util::CountDownLatch &updateLatch, util::CountDownLatch &evictLatch)
                            : addLatch(addLatch), removeLatch(removeLatch), updateLatch(updateLatch),
                              evictLatch(evictLatch) {
                    }

                    void entryAdded(const EntryEvent<K, V> &event) {
                        addLatch.countDown();
                    }

                    void entryRemoved(const EntryEvent<K, V> &event) {
                        removeLatch.countDown();
                    }

                    void entryUpdated(const EntryEvent<K, V> &event) {
                        updateLatch.countDown();
                    }

                    void entryEvicted(const EntryEvent<K, V> &event) {
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

                static HazelcastServer *instance;
                static HazelcastServer *instance2;
                static CONFIGTYPE *clientConfig;
                static HazelcastClient *client;
                static IMap<std::string, std::string> *imap;
                static IMap<int, int> *intMap;
                static IMap<int, Employee> *employees;
            };

            template<typename CONFIGTYPE>
            HazelcastServer *ClientMapTest<CONFIGTYPE>::instance = NULL;
            template<typename CONFIGTYPE>
            HazelcastServer *ClientMapTest<CONFIGTYPE>::instance2 = NULL;
            template<typename CONFIGTYPE>
            CONFIGTYPE *ClientMapTest<CONFIGTYPE>::clientConfig = NULL;
            template<typename CONFIGTYPE>
            HazelcastClient *ClientMapTest<CONFIGTYPE>::client = NULL;
            template<typename CONFIGTYPE>
            IMap<std::string, std::string> *ClientMapTest<CONFIGTYPE>::imap = NULL;
            template<typename CONFIGTYPE>
            IMap<int, int> *ClientMapTest<CONFIGTYPE>::intMap = NULL;
            template<typename CONFIGTYPE>
            IMap<int, Employee> *ClientMapTest<CONFIGTYPE>::employees = NULL;

            typedef ::testing::Types<MapClientConfig, NearCachedDataMapClientConfig, NearCachedObjectMapClientConfig> ConfigTypes;
            TYPED_TEST_CASE(ClientMapTest, ConfigTypes);

            TYPED_TEST(ClientMapTest, testIssue537) {
                util::CountDownLatch latch(2);
                util::CountDownLatch nullLatch(1);
                typename ClientMapTest<TypeParam>::MyListener myListener(latch, nullLatch);
                std::string id = ClientMapTest<TypeParam>::imap->addEntryListener(myListener, true);

                ClientMapTest<TypeParam>::imap->put("key1", "value1", 2 * 1000);

                ASSERT_TRUE(latch.await(10));
                ASSERT_TRUE(nullLatch.await(1));

                ASSERT_TRUE(ClientMapTest<TypeParam>::imap->removeEntryListener(id));

                ClientMapTest<TypeParam>::imap->put("key2", "value2");
                ASSERT_EQ(1, ClientMapTest<TypeParam>::imap->size());
            }

            TYPED_TEST(ClientMapTest, testContains) {
                ClientMapTest<TypeParam>::fillMap();

                ASSERT_FALSE(ClientMapTest<TypeParam>::imap->containsKey("key10"));
                ASSERT_TRUE(ClientMapTest<TypeParam>::imap->containsKey("key1"));

                ASSERT_FALSE(ClientMapTest<TypeParam>::imap->containsValue("value10"));
                ASSERT_TRUE(ClientMapTest<TypeParam>::imap->containsValue("value1"));
            }

            TYPED_TEST(ClientMapTest, testGet) {
                ClientMapTest<TypeParam>::fillMap();
                for (int i = 0; i < 10; i++) {
                    std::string key = "key";
                    key += util::IOUtil::to_string(i);
                    boost::shared_ptr<std::string> temp = ClientMapTest<TypeParam>::imap->get(key);

                    std::string value = "value";
                    value += util::IOUtil::to_string(i);
                    ASSERT_EQ(*temp, value);
                }
            }

            TYPED_TEST(ClientMapTest, testAsyncGet) {
                ClientMapTest<TypeParam>::fillMap();
                boost::shared_ptr<ICompletableFuture<std::string> > future = ClientMapTest<TypeParam>::imap->getAsync(
                        "key1");
                boost::shared_ptr<std::string> value = future->get();
                ASSERT_NOTNULL(value.get(), std::string);
                ASSERT_EQ("value1", *value);
            }

            TYPED_TEST(ClientMapTest, testAsyncPut) {
                ClientMapTest<TypeParam>::fillMap();
                boost::shared_ptr<ICompletableFuture<std::string> > future = ClientMapTest<TypeParam>::imap->putAsync(
                        "key3", "value");
                boost::shared_ptr<std::string> value = future->get();
                ASSERT_NOTNULL(value.get(), std::string);
                ASSERT_EQ("value3", *value);
                value = ClientMapTest<TypeParam>::imap->get("key3");
                ASSERT_NOTNULL(value.get(), std::string);
                ASSERT_EQ("value", *value);
            }

            TYPED_TEST(ClientMapTest, testAsyncPutWithTtl) {
                ClientMapTest<TypeParam>::fillMap();
                boost::shared_ptr<util::CountDownLatch> evictLatch(new util::CountDownLatch(1));
                typename ClientMapTest<TypeParam>::template EvictedEntryListener<std::string, std::string> listener(
                        evictLatch);
                std::string listenerId = ClientMapTest<TypeParam>::imap->addEntryListener(listener, true);

                boost::shared_ptr<ICompletableFuture<std::string> > future = ClientMapTest<TypeParam>::imap->putAsync(
                        "key", "value1", 3, util::concurrent::TimeUnit::SECONDS());
                boost::shared_ptr<std::string> value = future->get();
                ASSERT_NULL("no value for key should exist", value.get(), std::string);
                value = ClientMapTest<TypeParam>::imap->get("key");
                ASSERT_NOTNULL(value.get(), std::string);
                ASSERT_EQ("value1", *value);

                ASSERT_OPEN_EVENTUALLY(*evictLatch);

                monitor::impl::NearCacheStatsImpl *nearCacheStatsImpl =
                        (monitor::impl::NearCacheStatsImpl *) ClientMapTest<TypeParam>::imap->getLocalMapStats().getNearCacheStats();

                // When ttl expires at server, the server does not send near cache invalidation, hence below is for
                // non-near cache test case.
                if (!nearCacheStatsImpl) {
                    value = ClientMapTest<TypeParam>::imap->get("key");
                    ASSERT_NULL("The value for key should have expired and not exist", value.get(), std::string);
                }

                ASSERT_TRUE(ClientMapTest<TypeParam>::imap->removeEntryListener(listenerId));
            }

            TYPED_TEST(ClientMapTest, testAsyncPutWithMaxIdle) {
                ClientMapTest<TypeParam>::fillMap();
                boost::shared_ptr<util::CountDownLatch> evictLatch(new util::CountDownLatch(1));
                typename ClientMapTest<TypeParam>::template EvictedEntryListener<std::string, std::string> listener(
                        evictLatch);
                std::string listenerId = ClientMapTest<TypeParam>::imap->addEntryListener(listener, true);

                boost::shared_ptr<ICompletableFuture<std::string> > future = ClientMapTest<TypeParam>::imap->putAsync(
                        "key", "value1", 0, util::concurrent::TimeUnit::SECONDS(), 3,
                        util::concurrent::TimeUnit::SECONDS());
                boost::shared_ptr<std::string> value = future->get();
                ASSERT_NULL("no value for key should exist", value.get(), std::string);
                value = ClientMapTest<TypeParam>::imap->get("key");
                ASSERT_NOTNULL(value.get(), std::string);
                ASSERT_EQ("value1", *value);

                ASSERT_OPEN_EVENTUALLY(*evictLatch);

                monitor::impl::NearCacheStatsImpl *nearCacheStatsImpl =
                        (monitor::impl::NearCacheStatsImpl *) ClientMapTest<TypeParam>::imap->getLocalMapStats().getNearCacheStats();

                // When ttl expires at server, the server does not send near cache invalidation, hence below is for
                // non-near cache test case.
                if (!nearCacheStatsImpl) {
                    value = ClientMapTest<TypeParam>::imap->get("key");
                    ASSERT_NULL("The value for key should have expired and not exist", value.get(), std::string);
                }

                ASSERT_TRUE(ClientMapTest<TypeParam>::imap->removeEntryListener(listenerId));
            }

            TYPED_TEST(ClientMapTest, testAsyncSet) {
                ClientMapTest<TypeParam>::fillMap();
                boost::shared_ptr<ICompletableFuture<void> > future = ClientMapTest<TypeParam>::imap->setAsync("key3",
                                                                                                               "value");
                future->get();
                boost::shared_ptr<std::string> value = ClientMapTest<TypeParam>::imap->get("key3");
                ASSERT_NOTNULL(value.get(), std::string);
                ASSERT_EQ("value", *value);
            }

            TYPED_TEST(ClientMapTest, testAsyncSetWithTtl) {
                ClientMapTest<TypeParam>::fillMap();
                boost::shared_ptr<util::CountDownLatch> evictLatch(new util::CountDownLatch(1));
                typename ClientMapTest<TypeParam>::template EvictedEntryListener<std::string, std::string> listener(
                        evictLatch);
                std::string listenerId = ClientMapTest<TypeParam>::imap->addEntryListener(listener, true);

                boost::shared_ptr<ICompletableFuture<void> > future = ClientMapTest<TypeParam>::imap->setAsync("key",
                                                                                                               "value1",
                                                                                                               3,
                                                                                                               util::concurrent::TimeUnit::SECONDS());
                future->get();
                boost::shared_ptr<std::string> value = ClientMapTest<TypeParam>::imap->get("key");
                ASSERT_NOTNULL(value.get(), std::string);
                ASSERT_EQ("value1", *value);

                ASSERT_OPEN_EVENTUALLY(*evictLatch);

                monitor::impl::NearCacheStatsImpl *nearCacheStatsImpl =
                        (monitor::impl::NearCacheStatsImpl *) ClientMapTest<TypeParam>::imap->getLocalMapStats().getNearCacheStats();

                // When ttl expires at server, the server does not send near cache invalidation, hence below is for 
                // non-near cache test case.
                if (!nearCacheStatsImpl) {
                    value = ClientMapTest<TypeParam>::imap->get("key");
                    ASSERT_NULL("The value for key should have expired and not exist", value.get(), std::string);
                }

                ASSERT_TRUE(ClientMapTest<TypeParam>::imap->removeEntryListener(listenerId));
            }

            TYPED_TEST(ClientMapTest, testAsyncSetWithMaxIdle) {
                ClientMapTest<TypeParam>::fillMap();
                boost::shared_ptr<util::CountDownLatch> evictLatch(new util::CountDownLatch(1));
                typename ClientMapTest<TypeParam>::template EvictedEntryListener<std::string, std::string> listener(
                        evictLatch);
                std::string listenerId = ClientMapTest<TypeParam>::imap->addEntryListener(listener, true);

                boost::shared_ptr<ICompletableFuture<void> > future = ClientMapTest<TypeParam>::imap->setAsync("key",
                                                                                                               "value1",
                                                                                                               0,
                                                                                                               util::concurrent::TimeUnit::SECONDS(),
                                                                                                               3,
                                                                                                               util::concurrent::TimeUnit::SECONDS());
                future->get();
                boost::shared_ptr<std::string> value = ClientMapTest<TypeParam>::imap->get("key");
                ASSERT_NOTNULL(value.get(), std::string);
                ASSERT_EQ("value1", *value);

                ASSERT_OPEN_EVENTUALLY(*evictLatch);

                monitor::impl::NearCacheStatsImpl *nearCacheStatsImpl =
                        (monitor::impl::NearCacheStatsImpl *) ClientMapTest<TypeParam>::imap->getLocalMapStats().getNearCacheStats();

                // When ttl expires at server, the server does not send near cache invalidation, hence below is for 
                // non-near cache test case.
                if (!nearCacheStatsImpl) {
                    value = ClientMapTest<TypeParam>::imap->get("key");
                    ASSERT_NULL("The value for key should have expired and not exist", value.get(), std::string);
                }

                ASSERT_TRUE(ClientMapTest<TypeParam>::imap->removeEntryListener(listenerId));
            }

            TYPED_TEST(ClientMapTest, testAsyncRemove) {
                ClientMapTest<TypeParam>::fillMap();
                boost::shared_ptr<ICompletableFuture<string> > future = ClientMapTest<TypeParam>::imap->removeAsync(
                        "key4");
                future->get();
                boost::shared_ptr<std::string> value = future->get();
                ASSERT_NOTNULL(value.get(), std::string);
                ASSERT_EQ("value4", *value);
            }

            TYPED_TEST(ClientMapTest, testPartitionAwareKey) {
                int partitionKey = 5;
                int value = 25;
                IMap<PartitionAwareInt, int> map =
                        ClientMapTest<TypeParam>::client->template getMap<PartitionAwareInt, int>(MapClientConfig::intMapName);
                PartitionAwareInt partitionAwareInt(partitionKey, 7);
                map.put(partitionAwareInt, value);
                boost::shared_ptr<int> val = map.get(partitionAwareInt);
                ASSERT_NOTNULL(val.get(), int);
                ASSERT_EQ(*val, value);
            }

            TYPED_TEST(ClientMapTest, testRemoveAndDelete) {
                ClientMapTest<TypeParam>::fillMap();
                boost::shared_ptr<std::string> temp = ClientMapTest<TypeParam>::imap->remove("key10");
                ASSERT_EQ(temp.get(), (std::string *) NULL);
                ClientMapTest<TypeParam>::imap->deleteEntry("key9");
                ASSERT_EQ(ClientMapTest<TypeParam>::imap->size(), 9);
                for (int i = 0; i < 9; i++) {
                    std::string key = "key";
                    key += util::IOUtil::to_string(i);
                    boost::shared_ptr<std::string> temp2 = ClientMapTest<TypeParam>::imap->remove(key);
                    std::string value = "value";
                    value += util::IOUtil::to_string(i);
                    ASSERT_EQ(*temp2, value);
                }
                ASSERT_EQ(ClientMapTest<TypeParam>::imap->size(), 0);
            }

            TYPED_TEST(ClientMapTest, testRemoveIfSame) {
                ClientMapTest<TypeParam>::fillMap();

                ASSERT_FALSE(ClientMapTest<TypeParam>::imap->remove("key2", "value"));
                ASSERT_EQ(10, ClientMapTest<TypeParam>::imap->size());

                ASSERT_TRUE((ClientMapTest<TypeParam>::imap->remove("key2", "value2")));
                ASSERT_EQ(9, ClientMapTest<TypeParam>::imap->size());

            }

            TYPED_TEST(ClientMapTest, testRemoveAll) {
                ClientMapTest<TypeParam>::fillMap();

                ClientMapTest<TypeParam>::imap->removeAll(
                        query::EqualPredicate<std::string>(query::QueryConstants::getKeyAttributeName(), "key5"));

                boost::shared_ptr<std::string> value = ClientMapTest<TypeParam>::imap->get("key5");
                
                ASSERT_NULL("key5 should not exist", value.get(), std::string);

                ClientMapTest<TypeParam>::imap->removeAll(
                        query::LikePredicate(query::QueryConstants::getValueAttributeName(), "value%"));

                ASSERT_TRUE(ClientMapTest<TypeParam>::imap->isEmpty());
            }

            TYPED_TEST(ClientMapTest, testGetAllPutAll) {

                std::map<std::string, std::string> mapTemp;

                for (int i = 0; i < 100; i++) {
                    mapTemp[util::IOUtil::to_string(i)] = util::IOUtil::to_string(i);
                }
                ASSERT_EQ(ClientMapTest<TypeParam>::imap->size(), 0);
                ClientMapTest<TypeParam>::imap->putAll(mapTemp);
                ASSERT_EQ(ClientMapTest<TypeParam>::imap->size(), 100);

                for (int i = 0; i < 100; i++) {
                    std::string expected = util::IOUtil::to_string(i);
                    boost::shared_ptr<std::string> actual = ClientMapTest<TypeParam>::imap->get(
                            util::IOUtil::to_string(i));
                    ASSERT_EQ(expected, *actual);
                }

                std::set<std::string> tempSet;
                tempSet.insert(util::IOUtil::to_string(1));
                tempSet.insert(util::IOUtil::to_string(3));

                std::map<std::string, std::string> m2 = ClientMapTest<TypeParam>::imap->getAll(tempSet);

                ASSERT_EQ(2U, m2.size());
                ASSERT_EQ(m2[util::IOUtil::to_string(1)], "1");
                ASSERT_EQ(m2[util::IOUtil::to_string(3)], "3");

            }

            TYPED_TEST(ClientMapTest, testTryPutRemove) {
                ASSERT_TRUE(ClientMapTest<TypeParam>::imap->tryPut("key1", "value1", 1 * 1000));
                ASSERT_TRUE(ClientMapTest<TypeParam>::imap->tryPut("key2", "value2", 1 * 1000));
                ClientMapTest<TypeParam>::imap->lock("key1");
                ClientMapTest<TypeParam>::imap->lock("key2");

                util::CountDownLatch latch(2);

                util::StartedThread t1(ClientMapTest<TypeParam>::tryPutThread, &latch, ClientMapTest<TypeParam>::imap);
                util::StartedThread t2(ClientMapTest<TypeParam>::tryRemoveThread, &latch, ClientMapTest<TypeParam>::imap);

                ASSERT_TRUE(latch.await(20));
                ASSERT_EQ("value1", *(ClientMapTest<TypeParam>::imap->get("key1")));
                ASSERT_EQ("value2", *(ClientMapTest<TypeParam>::imap->get("key2")));
                ClientMapTest<TypeParam>::imap->forceUnlock("key1");
                ClientMapTest<TypeParam>::imap->forceUnlock("key2");
            }

            TYPED_TEST(ClientMapTest, testPutTtl) {
                util::CountDownLatch dummy(10);
                util::CountDownLatch evict(1);
                typename ClientMapTest<TypeParam>::template CountdownListener<std::string, std::string> sampleEntryListener(
                        dummy, dummy, dummy, evict);
                std::string id = ClientMapTest<TypeParam>::imap->addEntryListener(sampleEntryListener, false);

                IMap<std::string, std::string> &map = *ClientMapTest<TypeParam>::imap;

                monitor::impl::NearCacheStatsImpl *nearCacheStatsImpl = (monitor::impl::NearCacheStatsImpl *) map.getLocalMapStats().getNearCacheStats();

                int64_t initialInvalidationRequests = 0;
                if (nearCacheStatsImpl) {
                    initialInvalidationRequests = nearCacheStatsImpl->getInvalidationRequests();
                }

                // put will cause an invalidation event sent from the server to the client
                map.put("key1", "value1", 1000);

                // if near cache is enabled
                if (nearCacheStatsImpl) {
                    ASSERT_EQ_EVENTUALLY(initialInvalidationRequests + 1,  nearCacheStatsImpl->getInvalidationRequests());

                    // populate near cache
                    ClientMapTest<TypeParam>::imap->get("key1").get();

                    // When ttl expires at server, the server does not send near cache invalidation.
                    ASSERT_TRUE_ALL_THE_TIME((map.get("key1").get() && nearCacheStatsImpl->getInvalidationRequests() == initialInvalidationRequests + 1), 2);
                } else {
                    // trigger eviction
                    ASSERT_NULL_EVENTUALLY(map.get("key1").get(), std::string);
                }

                ASSERT_TRUE(evict.await(5));

                ASSERT_TRUE(ClientMapTest<TypeParam>::imap->removeEntryListener(id));
            }

            TYPED_TEST(ClientMapTest, testPutConfigTtl) {
                IMap<std::string, std::string> map = ClientMapTest<TypeParam>::client->template getMap<std::string, std::string>(
                        MapClientConfig::ONE_SECOND_MAP_NAME);
                util::CountDownLatch dummy(10);
                util::CountDownLatch evict(1);
                typename ClientMapTest<TypeParam>::template CountdownListener<std::string, std::string> sampleEntryListener(
                        dummy, dummy, dummy, evict);
                std::string id = map.addEntryListener(sampleEntryListener, false);

                monitor::impl::NearCacheStatsImpl *nearCacheStatsImpl = (monitor::impl::NearCacheStatsImpl *) map.getLocalMapStats().getNearCacheStats();

                int64_t initialInvalidationRequests = 0;
                if (nearCacheStatsImpl) {
                    initialInvalidationRequests = nearCacheStatsImpl->getInvalidationRequests();
                }

                // put will cause an invalidation event sent from the server to the client
                map.put("key1", "value1");

                // if near cache is enabled
                if (nearCacheStatsImpl) {
                    ASSERT_EQ_EVENTUALLY(initialInvalidationRequests + 1,  nearCacheStatsImpl->getInvalidationRequests());

                    // populate near cache
                    ClientMapTest<TypeParam>::imap->get("key1").get();

                    // When ttl expires at server, the server does not send near cache invalidation.
                    ASSERT_TRUE_ALL_THE_TIME((map.get("key1").get() && nearCacheStatsImpl->getInvalidationRequests() == initialInvalidationRequests + 1), 2);
                } else {
                    // trigger eviction
                    ASSERT_NULL_EVENTUALLY(map.get("key1").get(), std::string);
                }

                ASSERT_TRUE(evict.await(5));

                ASSERT_TRUE(map.removeEntryListener(id));
            }

            TYPED_TEST(ClientMapTest, testPutIfAbsent) {
                boost::shared_ptr<std::string> o = ClientMapTest<TypeParam>::imap->putIfAbsent("key1", "value1");
                ASSERT_EQ(o.get(), (std::string *) NULL);
                ASSERT_EQ("value1", *(ClientMapTest<TypeParam>::imap->putIfAbsent("key1", "value3")));
            }

            TYPED_TEST(ClientMapTest, testPutIfAbsentTtl) {
                ASSERT_EQ(ClientMapTest<TypeParam>::imap->putIfAbsent("key1", "value1", 1000).get(),
                          (std::string *) NULL);
                ASSERT_EQ("value1", *(ClientMapTest<TypeParam>::imap->putIfAbsent("key1", "value3", 1000)));

                ASSERT_NULL_EVENTUALLY(ClientMapTest<TypeParam>::imap->putIfAbsent("key1", "value3", 1000).get(), std::string);
                ASSERT_EQ("value3", *(ClientMapTest<TypeParam>::imap->putIfAbsent("key1", "value4", 1000)));
            }

            TYPED_TEST(ClientMapTest, testSet) {
                ClientMapTest<TypeParam>::imap->set("key1", "value1");
                ASSERT_EQ("value1", *(ClientMapTest<TypeParam>::imap->get("key1")));

                ClientMapTest<TypeParam>::imap->set("key1", "value2");
                ASSERT_EQ("value2", *(ClientMapTest<TypeParam>::imap->get("key1")));
            }

            TYPED_TEST(ClientMapTest, testSetTtl) {
                IMap<std::string, std::string> &map = *ClientMapTest<TypeParam>::imap;

                util::CountDownLatch dummy(10);
                util::CountDownLatch evict(1);
                typename ClientMapTest<TypeParam>::template CountdownListener<std::string, std::string> sampleEntryListener(
                        dummy, dummy, dummy, evict);
                std::string id = map.addEntryListener(sampleEntryListener, false);

                monitor::impl::NearCacheStatsImpl *nearCacheStatsImpl = (monitor::impl::NearCacheStatsImpl *) map.getLocalMapStats().getNearCacheStats();

                int64_t initialInvalidationRequests = 0;
                if (nearCacheStatsImpl) {
                    initialInvalidationRequests = nearCacheStatsImpl->getInvalidationRequests();
                }

                // set will cause an invalidation event sent from the server to the client
                map.set("key1", "value1", 1000);

                // if near cache is enabled
                if (nearCacheStatsImpl) {
                    ASSERT_EQ_EVENTUALLY(initialInvalidationRequests + 1,  nearCacheStatsImpl->getInvalidationRequests());

                    // populate near cache
                    ClientMapTest<TypeParam>::imap->get("key1").get();

                    // When ttl expires at server, the server does not send near cache invalidation.
                    ASSERT_TRUE_ALL_THE_TIME((map.get("key1").get() && nearCacheStatsImpl->getInvalidationRequests() == initialInvalidationRequests + 1), 2);
                } else {
                    // trigger eviction
                    ASSERT_NULL_EVENTUALLY(map.get("key1").get(), std::string);
                }

                ASSERT_TRUE(evict.await(5));

                ASSERT_TRUE(ClientMapTest<TypeParam>::imap->removeEntryListener(id));
            }

            TYPED_TEST(ClientMapTest, testSetConfigTtl) {
                IMap<std::string, std::string> map = ClientMapTest<TypeParam>::client->template getMap<std::string, std::string>(
                        MapClientConfig::ONE_SECOND_MAP_NAME);
                util::CountDownLatch dummy(10);
                util::CountDownLatch evict(1);
                typename ClientMapTest<TypeParam>::template CountdownListener<std::string, std::string> sampleEntryListener(
                        dummy, dummy, dummy, evict);
                std::string id = map.addEntryListener(sampleEntryListener, false);

                monitor::impl::NearCacheStatsImpl *nearCacheStatsImpl = (monitor::impl::NearCacheStatsImpl *) map.getLocalMapStats().getNearCacheStats();

                int64_t initialInvalidationRequests = 0;
                if (nearCacheStatsImpl) {
                    initialInvalidationRequests = nearCacheStatsImpl->getInvalidationRequests();
                }

                // put will cause an invalidation event sent from the server to the client
                map.set("key1", "value1");

                // if near cache is enabled
                if (nearCacheStatsImpl) {
                    ASSERT_EQ_EVENTUALLY(initialInvalidationRequests + 1,  nearCacheStatsImpl->getInvalidationRequests());

                    // populate near cache
                    ClientMapTest<TypeParam>::imap->get("key1").get();

                    // When ttl expires at server, the server does not send near cache invalidation.
                    ASSERT_TRUE_ALL_THE_TIME((map.get("key1").get() && nearCacheStatsImpl->getInvalidationRequests() == initialInvalidationRequests + 1), 2);
                } else {
                    // trigger eviction
                    ASSERT_NULL_EVENTUALLY(map.get("key1").get(), std::string);
                }

                ASSERT_TRUE(evict.await(5));

                ASSERT_TRUE(map.removeEntryListener(id));
            }

            TYPED_TEST(ClientMapTest, testLock) {
                ClientMapTest<TypeParam>::imap->put("key1", "value1");
                ASSERT_EQ("value1", *(ClientMapTest<TypeParam>::imap->get("key1")));
                ClientMapTest<TypeParam>::imap->lock("key1");
                util::CountDownLatch latch(1);
                util::StartedThread t1(ClientMapTest<TypeParam>::testLockThread, &latch, ClientMapTest<TypeParam>::imap);
                ASSERT_TRUE(latch.await(5));
                ASSERT_EQ("value1", *(ClientMapTest<TypeParam>::imap->get("key1")));
                ClientMapTest<TypeParam>::imap->forceUnlock("key1");
            }

            TYPED_TEST(ClientMapTest, testLockTtl) {
                ClientMapTest<TypeParam>::imap->put("key1", "value1");
                ASSERT_EQ("value1", *(ClientMapTest<TypeParam>::imap->get("key1")));
                ClientMapTest<TypeParam>::imap->lock("key1", 2 * 1000);
                util::CountDownLatch latch(1);
                util::StartedThread t1(ClientMapTest<TypeParam>::testLockTTLThread, &latch, ClientMapTest<TypeParam>::imap);
                ASSERT_TRUE(latch.await(10));
                ASSERT_FALSE(ClientMapTest<TypeParam>::imap->isLocked("key1"));
                ASSERT_EQ("value2", *(ClientMapTest<TypeParam>::imap->get("key1")));
                ClientMapTest<TypeParam>::imap->forceUnlock("key1");
            }

            TYPED_TEST(ClientMapTest, testLockTtl2) {
                ClientMapTest<TypeParam>::imap->lock("key1", 3 * 1000);
                util::CountDownLatch latch(2);
                util::StartedThread t1(ClientMapTest<TypeParam>::testLockTTL2Thread, &latch, ClientMapTest<TypeParam>::imap);
                ASSERT_TRUE(latch.await(10));
                ClientMapTest<TypeParam>::imap->forceUnlock("key1");
            }

            TYPED_TEST(ClientMapTest, testTryLock) {
                ASSERT_TRUE(ClientMapTest<TypeParam>::imap->tryLock("key1", 2 * 1000));
                util::CountDownLatch latch(1);
                util::StartedThread t1(ClientMapTest<TypeParam>::testMapTryLockThread1, &latch,
                                ClientMapTest<TypeParam>::imap);

                ASSERT_TRUE(latch.await(100));

                ASSERT_TRUE(ClientMapTest<TypeParam>::imap->isLocked("key1"));

                util::CountDownLatch latch2(1);
                util::StartedThread t2(ClientMapTest<TypeParam>::testMapTryLockThread2, &latch2,
                                ClientMapTest<TypeParam>::imap);

                util::sleep(1);
                ClientMapTest<TypeParam>::imap->unlock("key1");
                ASSERT_TRUE(latch2.await(100));
                ASSERT_TRUE(ClientMapTest<TypeParam>::imap->isLocked("key1"));
                ClientMapTest<TypeParam>::imap->forceUnlock("key1");
            }

            TYPED_TEST(ClientMapTest, testForceUnlock) {
                ClientMapTest<TypeParam>::imap->lock("key1");
                util::CountDownLatch latch(1);
                util::StartedThread t2(ClientMapTest<TypeParam>::testMapForceUnlockThread, &latch,
                                ClientMapTest<TypeParam>::imap);
                ASSERT_TRUE(latch.await(100));
                t2.join();
                ASSERT_FALSE(ClientMapTest<TypeParam>::imap->isLocked("key1"));

            }

            TYPED_TEST(ClientMapTest, testValues) {
                ClientMapTest<TypeParam>::fillMap();
                std::vector<std::string> tempVector;
                query::SqlPredicate predicate("this == value1");
                tempVector = ClientMapTest<TypeParam>::imap->values(predicate);
                ASSERT_EQ(1U, tempVector.size());

                std::vector<std::string>::iterator it = tempVector.begin();
                ASSERT_EQ("value1", *it);
            }

            TYPED_TEST(ClientMapTest, testValuesWithPredicate) {
                const int numItems = 20;
                for (int i = 0; i < numItems; ++i) {
                    ClientMapTest<TypeParam>::intMap->put(i, 2 * i);
                }

                std::vector<int> values = ClientMapTest<TypeParam>::intMap->values();
                ASSERT_EQ(numItems, (int) values.size());
                std::sort(values.begin(), values.end());
                for (int i = 0; i < numItems; ++i) {
                    ASSERT_EQ(2 * i, values[i]);
                }

                // EqualPredicate
                // key == 5
                values = ClientMapTest<TypeParam>::intMap->values(
                        query::EqualPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5));
                ASSERT_EQ(1, (int) values.size());
                ASSERT_EQ(2 * 5, values[0]);

                // value == 8
                values = ClientMapTest<TypeParam>::intMap->values(
                        query::EqualPredicate<int>(query::QueryConstants::getValueAttributeName(), 8));
                ASSERT_EQ(1, (int) values.size());
                ASSERT_EQ(8, values[0]);

                // key == numItems
                values = ClientMapTest<TypeParam>::intMap->values(
                        query::EqualPredicate<int>(query::QueryConstants::getKeyAttributeName(), numItems));
                ASSERT_EQ(0, (int) values.size());

                // NotEqual Predicate
                // key != 5
                values = ClientMapTest<TypeParam>::intMap->values(
                        query::NotEqualPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5));
                ASSERT_EQ(numItems - 1, (int) values.size());
                std::sort(values.begin(), values.end());
                for (int i = 0; i < numItems - 1; ++i) {
                    if (i >= 5) {
                        ASSERT_EQ(2 * (i + 1), values[i]);
                    } else {
                        ASSERT_EQ(2 * i, values[i]);
                    }
                }

                // this(value) != 8
                values = ClientMapTest<TypeParam>::intMap->values(
                        query::NotEqualPredicate<int>(query::QueryConstants::getValueAttributeName(), 8));
                ASSERT_EQ(numItems - 1, (int) values.size());
                std::sort(values.begin(), values.end());
                for (int i = 0; i < numItems - 1; ++i) {
                    if (i >= 4) {
                        ASSERT_EQ(2 * (i + 1), values[i]);
                    } else {
                        ASSERT_EQ(2 * i, values[i]);
                    }
                }

                // TruePredicate
                values = ClientMapTest<TypeParam>::intMap->values(query::TruePredicate());
                ASSERT_EQ(numItems, (int) values.size());
                std::sort(values.begin(), values.end());
                for (int i = 0; i < numItems; ++i) {
                    ASSERT_EQ(2 * i, values[i]);
                }

                // FalsePredicate
                values = ClientMapTest<TypeParam>::intMap->values(query::FalsePredicate());
                ASSERT_EQ(0, (int) values.size());

                // BetweenPredicate
                // 5 <= key <= 10
                values = ClientMapTest<TypeParam>::intMap->values(
                        query::BetweenPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5, 10));
                std::sort(values.begin(), values.end());
                ASSERT_EQ(6, (int) values.size());
                for (int i = 0; i < 6; ++i) {
                    ASSERT_EQ(2 * (i + 5), values[i]);
                }

                // 20 <= key <=30
                values = ClientMapTest<TypeParam>::intMap->values(
                        query::BetweenPredicate<int>(query::QueryConstants::getKeyAttributeName(), 20, 30));
                ASSERT_EQ(0, (int) values.size());

                // GreaterLessPredicate
                // value <= 10
                values = ClientMapTest<TypeParam>::intMap->values(
                        query::GreaterLessPredicate<int>(query::QueryConstants::getValueAttributeName(), 10, true,
                                                         true));
                ASSERT_EQ(6, (int) values.size());
                std::sort(values.begin(), values.end());
                for (int i = 0; i < 6; ++i) {
                    ASSERT_EQ(2 * i, values[i]);
                }

                // key < 7
                values = ClientMapTest<TypeParam>::intMap->values(
                        query::GreaterLessPredicate<int>(query::QueryConstants::getKeyAttributeName(), 7, false, true));
                ASSERT_EQ(7, (int) values.size());
                std::sort(values.begin(), values.end());
                for (int i = 0; i < 7; ++i) {
                    ASSERT_EQ(2 * i, values[i]);
                }

                // value >= 15
                values = ClientMapTest<TypeParam>::intMap->values(
                        query::GreaterLessPredicate<int>(query::QueryConstants::getValueAttributeName(), 15, true,
                                                         false));
                ASSERT_EQ(12, (int) values.size());
                std::sort(values.begin(), values.end());
                for (int i = 0; i < 12; ++i) {
                    ASSERT_EQ(2 * (i + 8), values[i]);
                }

                // key > 5
                values = ClientMapTest<TypeParam>::intMap->values(
                        query::GreaterLessPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5, false,
                                                         false));
                ASSERT_EQ(14, (int) values.size());
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
                values = ClientMapTest<TypeParam>::intMap->values(
                        query::InPredicate<int>(query::QueryConstants::getKeyAttributeName(), inVals));
                ASSERT_EQ(3, (int) values.size());
                std::sort(values.begin(), values.end());
                ASSERT_EQ(2 * 4, values[0]);
                ASSERT_EQ(2 * 10, values[1]);
                ASSERT_EQ(2 * 19, values[2]);

                // value in {4, 10, 19}
                values = ClientMapTest<TypeParam>::intMap->values(
                        query::InPredicate<int>(query::QueryConstants::getValueAttributeName(), inVals));
                ASSERT_EQ(2, (int) values.size());
                std::sort(values.begin(), values.end());
                ASSERT_EQ(4, values[0]);
                ASSERT_EQ(10, values[1]);

                // InstanceOfPredicate
                // value instanceof Integer
                values = ClientMapTest<TypeParam>::intMap->values(query::InstanceOfPredicate("java.lang.Integer"));
                ASSERT_EQ(20, (int) values.size());
                std::sort(values.begin(), values.end());
                for (int i = 0; i < numItems; ++i) {
                    ASSERT_EQ(2 * i, values[i]);
                }

                values = ClientMapTest<TypeParam>::intMap->values(query::InstanceOfPredicate("java.lang.String"));
                ASSERT_EQ(0, (int) values.size());

                // NotPredicate
                // !(5 <= key <= 10)
                std::auto_ptr<query::Predicate> bp = std::auto_ptr<query::Predicate>(new query::BetweenPredicate<int>(
                        query::QueryConstants::getKeyAttributeName(), 5, 10));
                query::NotPredicate notPredicate(bp);
                values = ClientMapTest<TypeParam>::intMap->values(notPredicate);
                ASSERT_EQ(14, (int) values.size());
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
                values = ClientMapTest<TypeParam>::intMap->values(query::AndPredicate().add(bp).add(inPred));
                ASSERT_EQ(1, (int) values.size());
                std::sort(values.begin(), values.end());
                ASSERT_EQ(10, values[0]);

                // OrPredicate
                // 5 <= key <= 10 OR Values in {4, 10, 19} = values {4, 10, 12, 14, 16, 18, 20}
                bp = std::auto_ptr<query::Predicate>(
                        new query::BetweenPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5, 10));
                inPred = std::auto_ptr<query::Predicate>(
                        new query::InPredicate<int>(query::QueryConstants::getValueAttributeName(), inVals));
                values = ClientMapTest<TypeParam>::intMap->values(query::OrPredicate().add(bp).add(inPred));
                ASSERT_EQ(7, (int) values.size());
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
                    ClientMapTest<TypeParam>::imap->put(key, value);
                }
                ClientMapTest<TypeParam>::imap->put("key_111_test", "myvalue_111_test");
                ClientMapTest<TypeParam>::imap->put("key_22_test", "myvalue_22_test");

                // LikePredicate
                // value LIKE "value1" : {"value1"}
                std::vector<std::string> strValues = ClientMapTest<TypeParam>::imap->values(
                        query::LikePredicate(query::QueryConstants::getValueAttributeName(), "value1"));
                ASSERT_EQ(1, (int) strValues.size());
                ASSERT_EQ("value1", strValues[0]);

                // ILikePredicate
                // value ILIKE "%VALue%1%" : {"myvalue_111_test", "value1", "value10", "value11"}
                strValues = ClientMapTest<TypeParam>::imap->values(
                        query::ILikePredicate(query::QueryConstants::getValueAttributeName(), "%VALue%1%"));
                ASSERT_EQ(4, (int) strValues.size());
                std::sort(strValues.begin(), strValues.end());
                ASSERT_EQ("myvalue_111_test", strValues[0]);
                ASSERT_EQ("value1", strValues[1]);
                ASSERT_EQ("value10", strValues[2]);
                ASSERT_EQ("value11", strValues[3]);

                // value ILIKE "%VAL%2%" : {"myvalue_22_test", "value2"}
                strValues = ClientMapTest<TypeParam>::imap->values(
                        query::ILikePredicate(query::QueryConstants::getValueAttributeName(), "%VAL%2%"));
                ASSERT_EQ(2, (int) strValues.size());
                std::sort(strValues.begin(), strValues.end());
                ASSERT_EQ("myvalue_22_test", strValues[0]);
                ASSERT_EQ("value2", strValues[1]);

                // SqlPredicate
                // __key BETWEEN 4 and 7 : {4, 5, 6, 7} -> {8, 10, 12, 14}
                char sql[100];
                util::hz_snprintf(sql, 50, "%s BETWEEN 4 and 7", query::QueryConstants::getKeyAttributeName());
                values = ClientMapTest<TypeParam>::intMap->values(query::SqlPredicate(sql));
                ASSERT_EQ(4, (int) values.size());
                std::sort(values.begin(), values.end());
                for (int i = 0; i < 4; ++i) {
                    ASSERT_EQ(2 * (i + 4), values[i]);
                }

                // RegexPredicate
                // value matches the regex ".*value.*2.*" : {myvalue_22_test, value2}
                strValues = ClientMapTest<TypeParam>::imap->values(
                        query::RegexPredicate(query::QueryConstants::getValueAttributeName(), ".*value.*2.*"));
                ASSERT_EQ(2, (int) strValues.size());
                std::sort(strValues.begin(), strValues.end());
                ASSERT_EQ("myvalue_22_test", strValues[0]);
                ASSERT_EQ("value2", strValues[1]);
            }

            TYPED_TEST(ClientMapTest, testValuesWithPagingPredicate) {
                int predSize = 5;
                const int totalEntries = 25;

                for (int i = 0; i < totalEntries; ++i) {
                    ClientMapTest<TypeParam>::intMap->put(i, i);
                }

                query::PagingPredicate<int, int> predicate((size_t) predSize);

                std::vector<int> values = ClientMapTest<TypeParam>::intMap->values(predicate);
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    ASSERT_EQ(i, values[i]);
                }

                values = ClientMapTest<TypeParam>::intMap->values(predicate);
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    ASSERT_EQ(i, values[i]);
                }

                predicate.nextPage();
                values = ClientMapTest<TypeParam>::intMap->values(predicate);
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

                ASSERT_EQ(1, (int) predicate.getPage());

                predicate.setPage(4);

                values = ClientMapTest<TypeParam>::intMap->values(predicate);
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
                ASSERT_EQ(3, (int) anchorEntry->first);
                ASSERT_EQ(19, *anchorEntry->second.first);
                ASSERT_EQ(19, *anchorEntry->second.second);

                predicate.nextPage();
                values = ClientMapTest<TypeParam>::intMap->values(predicate);
                ASSERT_EQ(0, (int) values.size());

                predicate.setPage(0);
                values = ClientMapTest<TypeParam>::intMap->values(predicate);
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    ASSERT_EQ(i, values[i]);
                }

                predicate.previousPage();
                ASSERT_EQ(0, (int) predicate.getPage());

                predicate.setPage(5);
                values = ClientMapTest<TypeParam>::intMap->values(predicate);
                ASSERT_EQ(0, (int) values.size());

                predicate.setPage(3);
                values = ClientMapTest<TypeParam>::intMap->values(predicate);
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    ASSERT_EQ(3 * predSize + i, values[i]);
                }

                predicate.previousPage();
                values = ClientMapTest<TypeParam>::intMap->values(predicate);
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    ASSERT_EQ(2 * predSize + i, values[i]);
                }

                // test PagingPredicate with inner predicate (value < 10)
                std::auto_ptr<query::Predicate> lessThanTenPredicate(std::auto_ptr<query::Predicate>(
                        new query::GreaterLessPredicate<int>(query::QueryConstants::getValueAttributeName(), 9, false,
                                                             true)));
                query::PagingPredicate<int, int> predicate2(lessThanTenPredicate, 5);
                values = ClientMapTest<TypeParam>::intMap->values(predicate2);
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    ASSERT_EQ(i, values[i]);
                }

                predicate2.nextPage();
                // match values 5,6, 7, 8
                values = ClientMapTest<TypeParam>::intMap->values(predicate2);
                ASSERT_EQ(predSize - 1, (int) values.size());
                for (int i = 0; i < predSize - 1; ++i) {
                    ASSERT_EQ(predSize + i, values[i]);
                }

                predicate2.nextPage();
                values = ClientMapTest<TypeParam>::intMap->values(predicate2);
                ASSERT_EQ(0, (int) values.size());

                // test paging predicate with comparator
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);
                Employee empl3("deniz", 25);
                Employee empl4("ali", 33);
                Employee empl5("veli", 44);
                Employee empl6("aylin", 5);

                ClientMapTest<TypeParam>::employees->put(3, empl1);
                ClientMapTest<TypeParam>::employees->put(4, empl2);
                ClientMapTest<TypeParam>::employees->put(5, empl3);
                ClientMapTest<TypeParam>::employees->put(6, empl4);
                ClientMapTest<TypeParam>::employees->put(7, empl5);
                ClientMapTest<TypeParam>::employees->put(8, empl6);

                predSize = 2;
                query::PagingPredicate<int, Employee> predicate3(
                        std::auto_ptr<query::EntryComparator<int, Employee> >(new EmployeeEntryComparator()),
                        (size_t) predSize);
                std::vector<Employee> result = ClientMapTest<TypeParam>::employees->values(predicate3);
                ASSERT_EQ(2, (int) result.size());
                ASSERT_EQ(empl6, result[0]);
                ASSERT_EQ(empl2, result[1]);

                predicate3.nextPage();
                result = ClientMapTest<TypeParam>::employees->values(predicate3);
                ASSERT_EQ(2, (int) result.size());
                ASSERT_EQ(empl3, result[0]);
                ASSERT_EQ(empl4, result[1]);
            }

            TYPED_TEST(ClientMapTest, testKeySetWithPredicate) {
                const int numItems = 20;
                for (int i = 0; i < numItems; ++i) {
                    ClientMapTest<TypeParam>::intMap->put(i, 2 * i);
                }

                std::vector<int> keys = ClientMapTest<TypeParam>::intMap->keySet();
                ASSERT_EQ(numItems, (int) keys.size());
                std::sort(keys.begin(), keys.end());
                for (int i = 0; i < numItems; ++i) {
                    ASSERT_EQ(i, keys[i]);
                }

                // EqualPredicate
                // key == 5
                keys = ClientMapTest<TypeParam>::intMap->keySet(
                        query::EqualPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5));
                ASSERT_EQ(1, (int) keys.size());
                ASSERT_EQ(5, keys[0]);

                // value == 8
                keys = ClientMapTest<TypeParam>::intMap->keySet(
                        query::EqualPredicate<int>(query::QueryConstants::getValueAttributeName(), 8));
                ASSERT_EQ(1, (int) keys.size());
                ASSERT_EQ(4, keys[0]);

                // key == numItems
                keys = ClientMapTest<TypeParam>::intMap->keySet(
                        query::EqualPredicate<int>(query::QueryConstants::getKeyAttributeName(), numItems));
                ASSERT_EQ(0, (int) keys.size());

                // NotEqual Predicate
                // key != 5
                keys = ClientMapTest<TypeParam>::intMap->keySet(
                        query::NotEqualPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5));
                ASSERT_EQ(numItems - 1, (int) keys.size());
                std::sort(keys.begin(), keys.end());
                for (int i = 0; i < numItems - 1; ++i) {
                    if (i >= 5) {
                        ASSERT_EQ(i + 1, keys[i]);
                    } else {
                        ASSERT_EQ(i, keys[i]);
                    }
                }

                // value != 8
                keys = ClientMapTest<TypeParam>::intMap->keySet(
                        query::NotEqualPredicate<int>(query::QueryConstants::getValueAttributeName(), 8));
                ASSERT_EQ(numItems - 1, (int) keys.size());
                std::sort(keys.begin(), keys.end());
                for (int i = 0; i < numItems - 1; ++i) {
                    if (i >= 4) {
                        ASSERT_EQ(i + 1, keys[i]);
                    } else {
                        ASSERT_EQ(i, keys[i]);
                    }
                }

                // TruePredicate
                keys = ClientMapTest<TypeParam>::intMap->keySet(query::TruePredicate());
                ASSERT_EQ(numItems, (int) keys.size());
                std::sort(keys.begin(), keys.end());
                for (int i = 0; i < numItems; ++i) {
                    ASSERT_EQ(i, keys[i]);
                }

                // FalsePredicate
                keys = ClientMapTest<TypeParam>::intMap->keySet(query::FalsePredicate());
                ASSERT_EQ(0, (int) keys.size());

                // BetweenPredicate
                // 5 <= key <= 10
                keys = ClientMapTest<TypeParam>::intMap->keySet(
                        query::BetweenPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5, 10));
                std::sort(keys.begin(), keys.end());
                ASSERT_EQ(6, (int) keys.size());
                for (int i = 0; i < 6; ++i) {
                    ASSERT_EQ((i + 5), keys[i]);
                }

                // 20 <= key <=30
                keys = ClientMapTest<TypeParam>::intMap->keySet(
                        query::BetweenPredicate<int>(query::QueryConstants::getKeyAttributeName(), 20, 30));
                ASSERT_EQ(0, (int) keys.size());

                // GreaterLessPredicate
                // value <= 10
                keys = ClientMapTest<TypeParam>::intMap->keySet(
                        query::GreaterLessPredicate<int>(query::QueryConstants::getValueAttributeName(), 10, true,
                                                         true));
                ASSERT_EQ(6, (int) keys.size());
                std::sort(keys.begin(), keys.end());
                for (int i = 0; i < 6; ++i) {
                    ASSERT_EQ(i, keys[i]);
                }

                // key < 7
                keys = ClientMapTest<TypeParam>::intMap->keySet(
                        query::GreaterLessPredicate<int>(query::QueryConstants::getKeyAttributeName(), 7, false, true));
                ASSERT_EQ(7, (int) keys.size());
                std::sort(keys.begin(), keys.end());
                for (int i = 0; i < 7; ++i) {
                    ASSERT_EQ(i, keys[i]);
                }

                // value >= 15
                keys = ClientMapTest<TypeParam>::intMap->keySet(
                        query::GreaterLessPredicate<int>(query::QueryConstants::getValueAttributeName(), 15, true,
                                                         false));
                ASSERT_EQ(12, (int) keys.size());
                std::sort(keys.begin(), keys.end());
                for (int i = 0; i < 12; ++i) {
                    ASSERT_EQ(i + 8, keys[i]);
                }

                // key > 5
                keys = ClientMapTest<TypeParam>::intMap->keySet(
                        query::GreaterLessPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5, false,
                                                         false));
                ASSERT_EQ(14, (int) keys.size());
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
                keys = ClientMapTest<TypeParam>::intMap->keySet(
                        query::InPredicate<int>(query::QueryConstants::getKeyAttributeName(), inVals));
                ASSERT_EQ(3, (int) keys.size());
                std::sort(keys.begin(), keys.end());
                ASSERT_EQ(4, keys[0]);
                ASSERT_EQ(10, keys[1]);
                ASSERT_EQ(19, keys[2]);

                // value in {4, 10, 19}
                keys = ClientMapTest<TypeParam>::intMap->keySet(
                        query::InPredicate<int>(query::QueryConstants::getValueAttributeName(), inVals));
                ASSERT_EQ(2, (int) keys.size());
                std::sort(keys.begin(), keys.end());
                ASSERT_EQ(2, keys[0]);
                ASSERT_EQ(5, keys[1]);

                // InstanceOfPredicate
                // value instanceof Integer
                keys = ClientMapTest<TypeParam>::intMap->keySet(query::InstanceOfPredicate("java.lang.Integer"));
                ASSERT_EQ(20, (int) keys.size());
                std::sort(keys.begin(), keys.end());
                for (int i = 0; i < numItems; ++i) {
                    ASSERT_EQ(i, keys[i]);
                }

                keys = ClientMapTest<TypeParam>::intMap->keySet(query::InstanceOfPredicate("java.lang.String"));
                ASSERT_EQ(0, (int) keys.size());

                // NotPredicate
                // !(5 <= key <= 10)
                std::auto_ptr<query::Predicate> bp = std::auto_ptr<query::Predicate>(new query::BetweenPredicate<int>(
                        query::QueryConstants::getKeyAttributeName(), 5, 10));
                query::NotPredicate notPredicate(bp);
                keys = ClientMapTest<TypeParam>::intMap->keySet(notPredicate);
                ASSERT_EQ(14, (int) keys.size());
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
                keys = ClientMapTest<TypeParam>::intMap->keySet(query::AndPredicate().add(bp).add(inPred));
                ASSERT_EQ(1, (int) keys.size());
                std::sort(keys.begin(), keys.end());
                ASSERT_EQ(5, keys[0]);

                // OrPredicate
                // 5 <= key <= 10 OR Values in {4, 10, 19} = keys {2, 5, 6, 7, 8, 9, 10}
                bp = std::auto_ptr<query::Predicate>(
                        new query::BetweenPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5, 10));
                inPred = std::auto_ptr<query::Predicate>(
                        new query::InPredicate<int>(query::QueryConstants::getValueAttributeName(), inVals));
                keys = ClientMapTest<TypeParam>::intMap->keySet(query::OrPredicate().add(bp).add(inPred));
                ASSERT_EQ(7, (int) keys.size());
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
                    ClientMapTest<TypeParam>::imap->put(key, value);
                }
                ClientMapTest<TypeParam>::imap->put("key_111_test", "myvalue_111_test");
                ClientMapTest<TypeParam>::imap->put("key_22_test", "myvalue_22_test");

                // LikePredicate
                // value LIKE "value1" : {"value1"}
                std::vector<std::string> strKeys = ClientMapTest<TypeParam>::imap->keySet(
                        query::LikePredicate(query::QueryConstants::getValueAttributeName(), "value1"));
                ASSERT_EQ(1, (int) strKeys.size());
                ASSERT_EQ("key1", strKeys[0]);

                // ILikePredicate
                // value ILIKE "%VALue%1%" : {"key_111_test", "key1", "key10", "key11"}
                strKeys = ClientMapTest<TypeParam>::imap->keySet(
                        query::ILikePredicate(query::QueryConstants::getValueAttributeName(), "%VALue%1%"));
                ASSERT_EQ(4, (int) strKeys.size());
                std::sort(strKeys.begin(), strKeys.end());
                ASSERT_EQ("key1", strKeys[0]);
                ASSERT_EQ("key10", strKeys[1]);
                ASSERT_EQ("key11", strKeys[2]);
                ASSERT_EQ("key_111_test", strKeys[3]);

                // key ILIKE "%VAL%2%" : {"key_22_test", "key2"}
                strKeys = ClientMapTest<TypeParam>::imap->keySet(
                        query::ILikePredicate(query::QueryConstants::getValueAttributeName(), "%VAL%2%"));
                ASSERT_EQ(2, (int) strKeys.size());
                std::sort(strKeys.begin(), strKeys.end());
                ASSERT_EQ("key2", strKeys[0]);
                ASSERT_EQ("key_22_test", strKeys[1]);

                // SqlPredicate
                // __key BETWEEN 4 and 7 : {4, 5, 6, 7} -> {8, 10, 12, 14}
                char sql[100];
                util::hz_snprintf(sql, 50, "%s BETWEEN 4 and 7", query::QueryConstants::getKeyAttributeName());
                keys = ClientMapTest<TypeParam>::intMap->keySet(query::SqlPredicate(sql));
                ASSERT_EQ(4, (int) keys.size());
                std::sort(keys.begin(), keys.end());
                for (int i = 0; i < 4; ++i) {
                    ASSERT_EQ(i + 4, keys[i]);
                }

                // RegexPredicate
                // value matches the regex ".*value.*2.*" : {key_22_test, value2}
                strKeys = ClientMapTest<TypeParam>::imap->keySet(
                        query::RegexPredicate(query::QueryConstants::getValueAttributeName(), ".*value.*2.*"));
                ASSERT_EQ(2, (int) strKeys.size());
                std::sort(strKeys.begin(), strKeys.end());
                ASSERT_EQ("key2", strKeys[0]);
                ASSERT_EQ("key_22_test", strKeys[1]);
            }

            TYPED_TEST(ClientMapTest, testKeySetWithPagingPredicate) {
                int predSize = 5;
                const int totalEntries = 25;

                for (int i = 0; i < totalEntries; ++i) {
                    ClientMapTest<TypeParam>::intMap->put(i, i);
                }

                query::PagingPredicate<int, int> predicate((size_t) predSize);

                std::vector<int> values = ClientMapTest<TypeParam>::intMap->keySet(predicate);
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    ASSERT_EQ(i, values[i]);
                }

                values = ClientMapTest<TypeParam>::intMap->keySet(predicate);
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    ASSERT_EQ(i, values[i]);
                }

                predicate.nextPage();
                values = ClientMapTest<TypeParam>::intMap->keySet(predicate);
                ASSERT_EQ(predSize, (int) values.size());

                for (int i = 0; i < predSize; ++i) {
                    ASSERT_EQ(predSize + i, values[i]);
                }

                const std::pair<int *, int *> *anchor = predicate.getAnchor();
                ASSERT_NE((const std::pair<int *, int *> *) NULL, anchor);
                ASSERT_NE((int *) NULL, anchor->first);
                ASSERT_EQ(9, *anchor->first);

                ASSERT_EQ(1, (int) predicate.getPage());

                predicate.setPage(4);

                values = ClientMapTest<TypeParam>::intMap->keySet(predicate);
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
                ASSERT_EQ(3, (int) anchorEntry->first);
                ASSERT_EQ(19, *anchorEntry->second.first);

                predicate.nextPage();
                values = ClientMapTest<TypeParam>::intMap->keySet(predicate);
                ASSERT_EQ(0, (int) values.size());

                predicate.setPage(0);
                values = ClientMapTest<TypeParam>::intMap->keySet(predicate);
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    ASSERT_EQ(i, values[i]);
                }

                predicate.previousPage();
                ASSERT_EQ(0, (int) predicate.getPage());

                predicate.setPage(5);
                values = ClientMapTest<TypeParam>::intMap->keySet(predicate);
                ASSERT_EQ(0, (int) values.size());

                predicate.setPage(3);
                values = ClientMapTest<TypeParam>::intMap->keySet(predicate);
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    ASSERT_EQ(3 * predSize + i, values[i]);
                }

                predicate.previousPage();
                values = ClientMapTest<TypeParam>::intMap->keySet(predicate);
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    ASSERT_EQ(2 * predSize + i, values[i]);
                }

                // test PagingPredicate with inner predicate (value < 10)
                std::auto_ptr<query::Predicate> lessThanTenPredicate(std::auto_ptr<query::Predicate>(
                        new query::GreaterLessPredicate<int>(query::QueryConstants::getValueAttributeName(), 9, false,
                                                             true)));
                query::PagingPredicate<int, int> predicate2(lessThanTenPredicate, 5);
                values = ClientMapTest<TypeParam>::intMap->keySet(predicate2);
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    ASSERT_EQ(i, values[i]);
                }

                predicate2.nextPage();
                // match values 5,6, 7, 8
                values = ClientMapTest<TypeParam>::intMap->keySet(predicate2);
                ASSERT_EQ(predSize - 1, (int) values.size());
                for (int i = 0; i < predSize - 1; ++i) {
                    ASSERT_EQ(predSize + i, values[i]);
                }

                predicate2.nextPage();
                values = ClientMapTest<TypeParam>::intMap->keySet(predicate2);
                ASSERT_EQ(0, (int) values.size());

                // test paging predicate with comparator
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);
                Employee empl3("deniz", 25);
                Employee empl4("ali", 33);
                Employee empl5("veli", 44);
                Employee empl6("aylin", 5);

                ClientMapTest<TypeParam>::employees->put(3, empl1);
                ClientMapTest<TypeParam>::employees->put(4, empl2);
                ClientMapTest<TypeParam>::employees->put(5, empl3);
                ClientMapTest<TypeParam>::employees->put(6, empl4);
                ClientMapTest<TypeParam>::employees->put(7, empl5);
                ClientMapTest<TypeParam>::employees->put(8, empl6);

                predSize = 2;
                query::PagingPredicate<int, Employee> predicate3(
                        std::auto_ptr<query::EntryComparator<int, Employee> >(new EmployeeEntryKeyComparator()),
                        (size_t) predSize);
                std::vector<int> result = ClientMapTest<TypeParam>::employees->keySet(predicate3);
                // since keyset result only returns keys from the server, no ordering based on the value but ordered based on the keys
                ASSERT_EQ(2, (int) result.size());
                ASSERT_EQ(3, result[0]);
                ASSERT_EQ(4, result[1]);

                predicate3.nextPage();
                result = ClientMapTest<TypeParam>::employees->keySet(predicate3);
                ASSERT_EQ(2, (int) result.size());
                ASSERT_EQ(5, result[0]);
                ASSERT_EQ(6, result[1]);
            }

            TYPED_TEST(ClientMapTest, testEntrySetWithPredicate) {
                const int numItems = 20;
                std::vector<std::pair<int, int> > expected(numItems);
                for (int i = 0; i < numItems; ++i) {
                    ClientMapTest<TypeParam>::intMap->put(i, 2 * i);
                    expected[i] = std::pair<int, int>(i, 2 * i);
                }

                std::vector<std::pair<int, int> > entries = ClientMapTest<TypeParam>::intMap->entrySet();
                ASSERT_EQ(numItems, (int) entries.size());
                std::sort(entries.begin(), entries.end());
                for (int i = 0; i < numItems; ++i) {
                    ASSERT_EQ(expected[i], entries[i]);
                }

                // EqualPredicate
                // key == 5
                entries = ClientMapTest<TypeParam>::intMap->entrySet(
                        query::EqualPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5));
                ASSERT_EQ(1, (int) entries.size());
                ASSERT_EQ(expected[5], entries[0]);

                // value == 8
                entries = ClientMapTest<TypeParam>::intMap->entrySet(
                        query::EqualPredicate<int>(query::QueryConstants::getValueAttributeName(), 8));
                ASSERT_EQ(1, (int) entries.size());
                ASSERT_EQ(expected[4], entries[0]);

                // key == numItems
                entries = ClientMapTest<TypeParam>::intMap->entrySet(
                        query::EqualPredicate<int>(query::QueryConstants::getKeyAttributeName(), numItems));
                ASSERT_EQ(0, (int) entries.size());

                // NotEqual Predicate
                // key != 5
                entries = ClientMapTest<TypeParam>::intMap->entrySet(
                        query::NotEqualPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5));
                ASSERT_EQ(numItems - 1, (int) entries.size());
                std::sort(entries.begin(), entries.end());
                for (int i = 0; i < numItems - 1; ++i) {
                    if (i >= 5) {
                        ASSERT_EQ(expected[i + 1], entries[i]);
                    } else {
                        ASSERT_EQ(expected[i], entries[i]);
                    }
                }

                // value != 8
                entries = ClientMapTest<TypeParam>::intMap->entrySet(
                        query::NotEqualPredicate<int>(query::QueryConstants::getValueAttributeName(), 8));
                ASSERT_EQ(numItems - 1, (int) entries.size());
                std::sort(entries.begin(), entries.end());
                for (int i = 0; i < numItems - 1; ++i) {
                    if (i >= 4) {
                        ASSERT_EQ(expected[i + 1], entries[i]);
                    } else {
                        ASSERT_EQ(expected[i], entries[i]);
                    }
                }

                // TruePredicate
                entries = ClientMapTest<TypeParam>::intMap->entrySet(query::TruePredicate());
                ASSERT_EQ(numItems, (int) entries.size());
                std::sort(entries.begin(), entries.end());
                for (int i = 0; i < numItems; ++i) {
                    ASSERT_EQ(expected[i], entries[i]);
                }

                // FalsePredicate
                entries = ClientMapTest<TypeParam>::intMap->entrySet(query::FalsePredicate());
                ASSERT_EQ(0, (int) entries.size());

                // BetweenPredicate
                // 5 <= key <= 10
                entries = ClientMapTest<TypeParam>::intMap->entrySet(
                        query::BetweenPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5, 10));
                std::sort(entries.begin(), entries.end());
                ASSERT_EQ(6, (int) entries.size());
                for (int i = 0; i < 6; ++i) {
                    ASSERT_EQ(expected[i + 5], entries[i]);
                }

                // 20 <= key <=30
                entries = ClientMapTest<TypeParam>::intMap->entrySet(
                        query::BetweenPredicate<int>(query::QueryConstants::getKeyAttributeName(), 20, 30));
                ASSERT_EQ(0, (int) entries.size());

                // GreaterLessPredicate
                // value <= 10
                entries = ClientMapTest<TypeParam>::intMap->entrySet(
                        query::GreaterLessPredicate<int>(query::QueryConstants::getValueAttributeName(), 10, true,
                                                         true));
                ASSERT_EQ(6, (int) entries.size());
                std::sort(entries.begin(), entries.end());
                for (int i = 0; i < 6; ++i) {
                    ASSERT_EQ(expected[i], entries[i]);
                }

                // key < 7
                entries = ClientMapTest<TypeParam>::intMap->entrySet(
                        query::GreaterLessPredicate<int>(query::QueryConstants::getKeyAttributeName(), 7, false, true));
                ASSERT_EQ(7, (int) entries.size());
                std::sort(entries.begin(), entries.end());
                for (int i = 0; i < 7; ++i) {
                    ASSERT_EQ(expected[i], entries[i]);
                }

                // value >= 15
                entries = ClientMapTest<TypeParam>::intMap->entrySet(
                        query::GreaterLessPredicate<int>(query::QueryConstants::getValueAttributeName(), 15, true,
                                                         false));
                ASSERT_EQ(12, (int) entries.size());
                std::sort(entries.begin(), entries.end());
                for (int i = 0; i < 12; ++i) {
                    ASSERT_EQ(expected[i + 8], entries[i]);
                }

                // key > 5
                entries = ClientMapTest<TypeParam>::intMap->entrySet(
                        query::GreaterLessPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5, false,
                                                         false));
                ASSERT_EQ(14, (int) entries.size());
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
                entries = ClientMapTest<TypeParam>::intMap->entrySet(
                        query::InPredicate<int>(query::QueryConstants::getKeyAttributeName(), inVals));
                ASSERT_EQ(3, (int) entries.size());
                std::sort(entries.begin(), entries.end());
                ASSERT_EQ(expected[4], entries[0]);
                ASSERT_EQ(expected[10], entries[1]);
                ASSERT_EQ(expected[19], entries[2]);

                // value in {4, 10, 19}
                entries = ClientMapTest<TypeParam>::intMap->entrySet(
                        query::InPredicate<int>(query::QueryConstants::getValueAttributeName(), inVals));
                ASSERT_EQ(2, (int) entries.size());
                std::sort(entries.begin(), entries.end());
                ASSERT_EQ(expected[2], entries[0]);
                ASSERT_EQ(expected[5], entries[1]);

                // InstanceOfPredicate
                // value instanceof Integer
                entries = ClientMapTest<TypeParam>::intMap->entrySet(query::InstanceOfPredicate("java.lang.Integer"));
                ASSERT_EQ(20, (int) entries.size());
                std::sort(entries.begin(), entries.end());
                for (int i = 0; i < numItems; ++i) {
                    ASSERT_EQ(expected[i], entries[i]);
                }

                entries = ClientMapTest<TypeParam>::intMap->entrySet(query::InstanceOfPredicate("java.lang.String"));
                ASSERT_EQ(0, (int) entries.size());

                // NotPredicate
                // !(5 <= key <= 10)
                std::auto_ptr<query::Predicate> bp = std::auto_ptr<query::Predicate>(new query::BetweenPredicate<int>(
                        query::QueryConstants::getKeyAttributeName(), 5, 10));
                query::NotPredicate notPredicate(bp);
                entries = ClientMapTest<TypeParam>::intMap->entrySet(notPredicate);
                ASSERT_EQ(14, (int) entries.size());
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
                entries = ClientMapTest<TypeParam>::intMap->entrySet(query::AndPredicate().add(bp).add(inPred));
                ASSERT_EQ(1, (int) entries.size());
                std::sort(entries.begin(), entries.end());
                ASSERT_EQ(expected[5], entries[0]);

                // OrPredicate
                // 5 <= key <= 10 OR Values in {4, 10, 19} = entries {2, 5, 6, 7, 8, 9, 10}
                bp = std::auto_ptr<query::Predicate>(
                        new query::BetweenPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5, 10));
                inPred = std::auto_ptr<query::Predicate>(
                        new query::InPredicate<int>(query::QueryConstants::getValueAttributeName(), inVals));
                entries = ClientMapTest<TypeParam>::intMap->entrySet(query::OrPredicate().add(bp).add(inPred));
                ASSERT_EQ(7, (int) entries.size());
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
                    ClientMapTest<TypeParam>::imap->put(key, value);
                    expectedStrEntries[i] = std::pair<std::string, std::string>(key, value);
                }
                ClientMapTest<TypeParam>::imap->put("key_111_test", "myvalue_111_test");
                expectedStrEntries[12] = std::pair<std::string, std::string>("key_111_test", "myvalue_111_test");
                ClientMapTest<TypeParam>::imap->put("key_22_test", "myvalue_22_test");
                expectedStrEntries[13] = std::pair<std::string, std::string>("key_22_test", "myvalue_22_test");

                // LikePredicate
                // value LIKE "value1" : {"value1"}
                std::vector<std::pair<std::string, std::string> > strEntries = ClientMapTest<TypeParam>::imap->entrySet(
                        query::LikePredicate(query::QueryConstants::getValueAttributeName(), "value1"));
                ASSERT_EQ(1, (int) strEntries.size());
                ASSERT_EQ(expectedStrEntries[1], strEntries[0]);

                // ILikePredicate
                // value ILIKE "%VALue%1%" : {"key_111_test", "key1", "key10", "key11"}
                strEntries = ClientMapTest<TypeParam>::imap->entrySet(
                        query::ILikePredicate(query::QueryConstants::getValueAttributeName(), "%VALue%1%"));
                ASSERT_EQ(4, (int) strEntries.size());
                std::sort(strEntries.begin(), strEntries.end());
                ASSERT_EQ(expectedStrEntries[1], strEntries[0]);
                ASSERT_EQ(expectedStrEntries[10], strEntries[1]);
                ASSERT_EQ(expectedStrEntries[11], strEntries[2]);
                ASSERT_EQ(expectedStrEntries[12], strEntries[3]);

                // key ILIKE "%VAL%2%" : {"key_22_test", "key2"}
                strEntries = ClientMapTest<TypeParam>::imap->entrySet(
                        query::ILikePredicate(query::QueryConstants::getValueAttributeName(), "%VAL%2%"));
                ASSERT_EQ(2, (int) strEntries.size());
                std::sort(strEntries.begin(), strEntries.end());
                ASSERT_EQ(expectedStrEntries[2], strEntries[0]);
                ASSERT_EQ(expectedStrEntries[13], strEntries[1]);

                // SqlPredicate
                // __key BETWEEN 4 and 7 : {4, 5, 6, 7} -> {8, 10, 12, 14}
                char sql[100];
                util::hz_snprintf(sql, 50, "%s BETWEEN 4 and 7", query::QueryConstants::getKeyAttributeName());
                entries = ClientMapTest<TypeParam>::intMap->entrySet(query::SqlPredicate(sql));
                ASSERT_EQ(4, (int) entries.size());
                std::sort(entries.begin(), entries.end());
                for (int i = 0; i < 4; ++i) {
                    ASSERT_EQ(expected[i + 4], entries[i]);
                }

                // RegexPredicate
                // value matches the regex ".*value.*2.*" : {key_22_test, value2}
                strEntries = ClientMapTest<TypeParam>::imap->entrySet(
                        query::RegexPredicate(query::QueryConstants::getValueAttributeName(), ".*value.*2.*"));
                ASSERT_EQ(2, (int) strEntries.size());
                std::sort(strEntries.begin(), strEntries.end());
                ASSERT_EQ(expectedStrEntries[2], strEntries[0]);
                ASSERT_EQ(expectedStrEntries[13], strEntries[1]);
            }

            TYPED_TEST(ClientMapTest, testEntrySetWithPagingPredicate) {
                int predSize = 5;
                const int totalEntries = 25;

                for (int i = 0; i < totalEntries; ++i) {
                    ClientMapTest<TypeParam>::intMap->put(i, i);
                }

                query::PagingPredicate<int, int> predicate((size_t) predSize);

                std::vector<std::pair<int, int> > values = ClientMapTest<TypeParam>::intMap->entrySet(predicate);
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    std::pair<int, int> value(i, i);
                    ASSERT_EQ(value, values[i]);
                }

                values = ClientMapTest<TypeParam>::intMap->entrySet(predicate);
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    std::pair<int, int> value(i, i);
                    ASSERT_EQ(value, values[i]);
                }

                predicate.nextPage();
                values = ClientMapTest<TypeParam>::intMap->entrySet(predicate);
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

                ASSERT_EQ(1, (int) predicate.getPage());

                predicate.setPage(4);

                values = ClientMapTest<TypeParam>::intMap->entrySet(predicate);
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
                ASSERT_EQ(3, (int) anchorEntry->first);
                ASSERT_EQ(19, *anchorEntry->second.first);
                ASSERT_EQ(19, *anchorEntry->second.second);

                predicate.nextPage();
                values = ClientMapTest<TypeParam>::intMap->entrySet(predicate);
                ASSERT_EQ(0, (int) values.size());

                predicate.setPage(0);
                values = ClientMapTest<TypeParam>::intMap->entrySet(predicate);
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    std::pair<int, int> value(i, i);
                    ASSERT_EQ(value, values[i]);
                }

                predicate.previousPage();
                ASSERT_EQ(0, (int) predicate.getPage());

                predicate.setPage(5);
                values = ClientMapTest<TypeParam>::intMap->entrySet(predicate);
                ASSERT_EQ(0, (int) values.size());

                predicate.setPage(3);
                values = ClientMapTest<TypeParam>::intMap->entrySet(predicate);
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    std::pair<int, int> value(3 * predSize + i, 3 * predSize + i);
                    ASSERT_EQ(value, values[i]);
                }

                predicate.previousPage();
                values = ClientMapTest<TypeParam>::intMap->entrySet(predicate);
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
                values = ClientMapTest<TypeParam>::intMap->entrySet(predicate2);
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    std::pair<int, int> value(i, i);
                    ASSERT_EQ(value, values[i]);
                }

                predicate2.nextPage();
                // match values 5,6, 7, 8
                values = ClientMapTest<TypeParam>::intMap->entrySet(predicate2);
                ASSERT_EQ(predSize - 1, (int) values.size());
                for (int i = 0; i < predSize - 1; ++i) {
                    std::pair<int, int> value(predSize + i, predSize + i);
                    ASSERT_EQ(value, values[i]);
                }

                predicate2.nextPage();
                values = ClientMapTest<TypeParam>::intMap->entrySet(predicate2);
                ASSERT_EQ(0, (int) values.size());

                // test paging predicate with comparator
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);
                Employee empl3("deniz", 25);
                Employee empl4("ali", 33);
                Employee empl5("veli", 44);
                Employee empl6("aylin", 5);

                ClientMapTest<TypeParam>::employees->put(3, empl1);
                ClientMapTest<TypeParam>::employees->put(4, empl2);
                ClientMapTest<TypeParam>::employees->put(5, empl3);
                ClientMapTest<TypeParam>::employees->put(6, empl4);
                ClientMapTest<TypeParam>::employees->put(7, empl5);
                ClientMapTest<TypeParam>::employees->put(8, empl6);

                predSize = 2;
                query::PagingPredicate<int, Employee> predicate3(
                        std::auto_ptr<query::EntryComparator<int, Employee> >(new EmployeeEntryComparator()),
                        (size_t) predSize);
                std::vector<std::pair<int, Employee> > result = ClientMapTest<TypeParam>::employees->entrySet(
                        predicate3);
                ASSERT_EQ(2, (int) result.size());
                std::pair<int, Employee> value(8, empl6);
                ASSERT_EQ(value, result[0]);
                value = std::pair<int, Employee>(4, empl2);
                ASSERT_EQ(value, result[1]);

                predicate3.nextPage();
                result = ClientMapTest<TypeParam>::employees->entrySet(predicate3);
                ASSERT_EQ(2, (int) result.size());
                value = std::pair<int, Employee>(5, empl3);
                ASSERT_EQ(value, result[0]);
                value = std::pair<int, Employee>(6, empl4);
                ASSERT_EQ(value, result[1]);
            }

            TYPED_TEST(ClientMapTest, testReplace) {
                boost::shared_ptr<std::string> temp = ClientMapTest<TypeParam>::imap->replace("key1", "value");
                ASSERT_EQ(temp.get(), (std::string *) NULL);

                std::string tempKey = "key1";
                std::string tempValue = "value1";
                ClientMapTest<TypeParam>::imap->put(tempKey, tempValue);

                ASSERT_EQ("value1", *(ClientMapTest<TypeParam>::imap->replace("key1", "value2")));
                ASSERT_EQ("value2", *(ClientMapTest<TypeParam>::imap->get("key1")));

                ASSERT_FALSE(ClientMapTest<TypeParam>::imap->replace("key1", "value1", "value3"));
                ASSERT_EQ("value2", *(ClientMapTest<TypeParam>::imap->get("key1")));

                ASSERT_TRUE(ClientMapTest<TypeParam>::imap->replace("key1", "value2", "value3"));
                ASSERT_EQ("value3", *(ClientMapTest<TypeParam>::imap->get("key1")));
            }

            TYPED_TEST(ClientMapTest, testListenerWithPortableKey) {
                IMap<Employee, int> tradeMap = ClientMapTest<TypeParam>::client->template getMap<Employee, int>(
                        "tradeMap");
                util::CountDownLatch countDownLatch(1);
                util::AtomicInt atomicInteger(0);
                typename ClientMapTest<TypeParam>::SampleEntryListenerForPortableKey listener(countDownLatch,
                                                                                              atomicInteger);
                Employee key("a", 1);
                std::string id = tradeMap.addEntryListener(listener, key, true);
                Employee key2("a", 2);
                tradeMap.put(key2, 1);
                tradeMap.put(key, 3);
                ASSERT_TRUE(countDownLatch.await(5));
                ASSERT_EQ(1, (int) atomicInteger);

                ASSERT_TRUE(tradeMap.removeEntryListener(id));
            }

            TYPED_TEST(ClientMapTest, testListener) {
                util::CountDownLatch latch1Add(5);
                util::CountDownLatch latch1Remove(2);
                util::CountDownLatch dummy(10);
                util::CountDownLatch latch2Add(1);
                util::CountDownLatch latch2Remove(1);

                typename ClientMapTest<TypeParam>::template CountdownListener<std::string, std::string> listener1(
                        latch1Add, latch1Remove, dummy, dummy);
                typename ClientMapTest<TypeParam>::template CountdownListener<std::string, std::string> listener2(
                        latch2Add, latch2Remove, dummy, dummy);

                std::string listener1ID = ClientMapTest<TypeParam>::imap->addEntryListener(listener1, false);
                std::string listener2ID = ClientMapTest<TypeParam>::imap->addEntryListener(listener2, "key3", true);

                util::sleep(2);

                ClientMapTest<TypeParam>::imap->put("key1", "value1");
                ClientMapTest<TypeParam>::imap->put("key2", "value2");
                ClientMapTest<TypeParam>::imap->put("key3", "value3");
                ClientMapTest<TypeParam>::imap->put("key4", "value4");
                ClientMapTest<TypeParam>::imap->put("key5", "value5");

                ClientMapTest<TypeParam>::imap->remove("key1");
                ClientMapTest<TypeParam>::imap->remove("key3");

                ASSERT_TRUE(latch1Add.await(10));
                ASSERT_TRUE(latch1Remove.await(10));
                ASSERT_TRUE(latch2Add.await(5));
                ASSERT_TRUE(latch2Remove.await(5));

                ASSERT_TRUE(ClientMapTest<TypeParam>::imap->removeEntryListener(listener1ID));
                ASSERT_TRUE(ClientMapTest<TypeParam>::imap->removeEntryListener(listener2ID));

            }

            TYPED_TEST(ClientMapTest, testListenerWithTruePredicate) {
                util::CountDownLatch latchAdd(3);
                util::CountDownLatch latchRemove(1);
                util::CountDownLatch latchEvict(1);
                util::CountDownLatch latchUpdate(1);

                typename ClientMapTest<TypeParam>::template CountdownListener<int, int> listener(latchAdd, latchRemove,
                                                                                                 latchUpdate,
                                                                                                 latchEvict);

                std::string listenerId = ClientMapTest<TypeParam>::intMap->addEntryListener(listener,
                                                                                            query::TruePredicate(),
                                                                                            false);

                ClientMapTest<TypeParam>::intMap->put(1, 1);
                ClientMapTest<TypeParam>::intMap->put(2, 2);
                ClientMapTest<TypeParam>::intMap->put(3, 3, 1000); // evict after 1 second
                ClientMapTest<TypeParam>::intMap->remove(2);

                util::sleep(2);

                ASSERT_EQ(NULL, ClientMapTest<TypeParam>::intMap->get(3).get()); // trigger eviction

                // update an entry
                ClientMapTest<TypeParam>::intMap->set(1, 5);
                boost::shared_ptr<int> value = ClientMapTest<TypeParam>::intMap->get(1);
                ASSERT_NE((int *) NULL, value.get());
                ASSERT_EQ(5, *value);

                util::CountDownLatchWaiter latches;
                latches.add(latchAdd).add(latchRemove).add(latchUpdate).add(latchEvict);
                ASSERT_TRUE(latches.awaitMillis(2000));

                ASSERT_TRUE(ClientMapTest<TypeParam>::intMap->removeEntryListener(listenerId));
            }

            TYPED_TEST(ClientMapTest, testListenerWithFalsePredicate) {
                util::CountDownLatch latchAdd(3);
                util::CountDownLatch latchRemove(1);
                util::CountDownLatch latchEvict(1);
                util::CountDownLatch latchUpdate(1);

                typename ClientMapTest<TypeParam>::template CountdownListener<int, int> listener(latchAdd, latchRemove,
                                                                                                 latchUpdate,
                                                                                                 latchEvict);

                std::string listenerId = ClientMapTest<TypeParam>::intMap->addEntryListener(listener,
                                                                                            query::FalsePredicate(),
                                                                                            false);

                ClientMapTest<TypeParam>::intMap->put(1, 1);
                ClientMapTest<TypeParam>::intMap->put(2, 2);
                ClientMapTest<TypeParam>::intMap->put(3, 3, 1000); // evict after 1 second
                ClientMapTest<TypeParam>::intMap->remove(2);

                util::sleep(2);

                ASSERT_EQ(NULL, ClientMapTest<TypeParam>::intMap->get(3).get()); // trigger eviction

                // update an entry
                ClientMapTest<TypeParam>::intMap->set(1, 5);
                boost::shared_ptr<int> value = ClientMapTest<TypeParam>::intMap->get(1);
                ASSERT_NE((int *) NULL, value.get());
                ASSERT_EQ(5, *value);

                util::CountDownLatchWaiter latches;
                latches.add(latchAdd).add(latchRemove).add(latchUpdate).add(latchEvict);
                ASSERT_FALSE(latches.awaitMillis(2000));

                ASSERT_TRUE(ClientMapTest<TypeParam>::intMap->removeEntryListener(listenerId));
            }

            TYPED_TEST(ClientMapTest, testListenerWithEqualPredicate) {
                util::CountDownLatch latchAdd(1);
                util::CountDownLatch latchRemove(1);
                util::CountDownLatch latchEvict(1);
                util::CountDownLatch latchUpdate(1);

                typename ClientMapTest<TypeParam>::template CountdownListener<int, int> listener(latchAdd, latchRemove,
                                                                                                 latchUpdate,
                                                                                                 latchEvict);

                std::string listenerId = ClientMapTest<TypeParam>::intMap->addEntryListener(listener,
                                                                                            query::EqualPredicate<int>(
                                                                                                    query::QueryConstants::getKeyAttributeName(),
                                                                                                    3), true);

                ClientMapTest<TypeParam>::intMap->put(1, 1);
                ClientMapTest<TypeParam>::intMap->put(2, 2);
                ClientMapTest<TypeParam>::intMap->put(3, 3, 1000); // evict after 1 second
                ClientMapTest<TypeParam>::intMap->remove(2);

                util::sleep(2);

                ASSERT_EQ(NULL, ClientMapTest<TypeParam>::intMap->get(3).get()); // trigger eviction

                // update an entry
                ClientMapTest<TypeParam>::intMap->set(1, 5);
                boost::shared_ptr<int> value = ClientMapTest<TypeParam>::intMap->get(1);
                ASSERT_NE((int *) NULL, value.get());
                ASSERT_EQ(5, *value);

                util::CountDownLatchWaiter latches;
                latches.add(latchAdd).add(latchEvict);
                ASSERT_TRUE(latches.awaitMillis(2000));

                latches.reset();
                latches.add(latchUpdate).add(latchRemove);
                ASSERT_FALSE(latches.awaitMillis(2000));

                ASSERT_TRUE(ClientMapTest<TypeParam>::intMap->removeEntryListener(listenerId));
            }

            TYPED_TEST(ClientMapTest, testListenerWithNotEqualPredicate) {
                util::CountDownLatch latchAdd(2);
                util::CountDownLatch latchRemove(1);
                util::CountDownLatch latchEvict(1);
                util::CountDownLatch latchUpdate(1);

                typename ClientMapTest<TypeParam>::template CountdownListener<int, int> listener(latchAdd, latchRemove,
                                                                                                 latchUpdate,
                                                                                                 latchEvict);

                std::string listenerId = ClientMapTest<TypeParam>::intMap->addEntryListener(listener,
                                                                                            query::NotEqualPredicate<int>(
                                                                                                    query::QueryConstants::getKeyAttributeName(),
                                                                                                    3), true);

                ClientMapTest<TypeParam>::intMap->put(1, 1);
                ClientMapTest<TypeParam>::intMap->put(2, 2);
                ClientMapTest<TypeParam>::intMap->put(3, 3, 1000); // evict after 1 second
                ClientMapTest<TypeParam>::intMap->remove(2);

                util::sleep(2);

                ASSERT_EQ(NULL, ClientMapTest<TypeParam>::intMap->get(3).get()); // trigger eviction

                // update an entry
                ClientMapTest<TypeParam>::intMap->set(1, 5);
                boost::shared_ptr<int> value = ClientMapTest<TypeParam>::intMap->get(1);
                ASSERT_NE((int *) NULL, value.get());
                ASSERT_EQ(5, *value);

                util::CountDownLatchWaiter latches;
                latches.add(latchAdd).add(latchRemove).add(latchUpdate);
                ASSERT_TRUE(latches.awaitMillis(2000));

                latches.reset();
                latches.add(latchEvict);
                ASSERT_FALSE(latches.awaitMillis(2000));

                ASSERT_TRUE(ClientMapTest<TypeParam>::intMap->removeEntryListener(listenerId));
            }

            TYPED_TEST(ClientMapTest, testListenerWithGreaterLessPredicate) {
                util::CountDownLatch latchAdd(2);
                util::CountDownLatch latchRemove(1);
                util::CountDownLatch latchEvict(1);
                util::CountDownLatch latchUpdate(1);

                typename ClientMapTest<TypeParam>::template CountdownListener<int, int> listener(latchAdd, latchRemove,
                                                                                                 latchUpdate,
                                                                                                 latchEvict);

                // key <= 2
                std::string listenerId = ClientMapTest<TypeParam>::intMap->addEntryListener(listener,
                                                                                            query::GreaterLessPredicate<int>(
                                                                                                    query::QueryConstants::getKeyAttributeName(),
                                                                                                    2, true, true),
                                                                                            false);

                ClientMapTest<TypeParam>::intMap->put(1, 1);
                ClientMapTest<TypeParam>::intMap->put(2, 2);
                ClientMapTest<TypeParam>::intMap->put(3, 3, 1000); // evict after 1 second
                ClientMapTest<TypeParam>::intMap->remove(2);

                util::sleep(2);

                ASSERT_EQ(NULL, ClientMapTest<TypeParam>::intMap->get(3).get()); // trigger eviction

                // update an entry
                ClientMapTest<TypeParam>::intMap->set(1, 5);
                boost::shared_ptr<int> value = ClientMapTest<TypeParam>::intMap->get(1);
                ASSERT_NE((int *) NULL, value.get());
                ASSERT_EQ(5, *value);

                util::CountDownLatchWaiter latches;
                latches.add(latchAdd).add(latchRemove).add(latchUpdate);
                ASSERT_TRUE(latches.awaitMillis(2000));

                ASSERT_FALSE(latchEvict.awaitMillis(2000));

                ASSERT_TRUE(ClientMapTest<TypeParam>::intMap->removeEntryListener(listenerId));
            }

            TYPED_TEST(ClientMapTest, testListenerWithBetweenPredicate) {
                util::CountDownLatch latchAdd(2);
                util::CountDownLatch latchRemove(1);
                util::CountDownLatch latchEvict(1);
                util::CountDownLatch latchUpdate(1);

                typename ClientMapTest<TypeParam>::template CountdownListener<int, int> listener(latchAdd, latchRemove,
                                                                                                 latchUpdate,
                                                                                                 latchEvict);

                // 1 <=key <= 2
                std::string listenerId = ClientMapTest<TypeParam>::intMap->addEntryListener(listener,
                                                                                            query::BetweenPredicate<int>(
                                                                                                    query::QueryConstants::getKeyAttributeName(),
                                                                                                    1, 2), true);

                ClientMapTest<TypeParam>::intMap->put(1, 1);
                ClientMapTest<TypeParam>::intMap->put(2, 2);
                ClientMapTest<TypeParam>::intMap->put(3, 3, 1000); // evict after 1 second
                ClientMapTest<TypeParam>::intMap->remove(2);

                util::sleep(2);

                ASSERT_EQ(NULL, ClientMapTest<TypeParam>::intMap->get(3).get()); // trigger eviction

                // update an entry
                ClientMapTest<TypeParam>::intMap->set(1, 5);
                boost::shared_ptr<int> value = ClientMapTest<TypeParam>::intMap->get(1);
                ASSERT_NE((int *) NULL, value.get());
                ASSERT_EQ(5, *value);

                util::CountDownLatchWaiter latches;
                latches.add(latchAdd).add(latchRemove).add(latchUpdate);
                ASSERT_TRUE(latches.awaitMillis(2000));

                ASSERT_FALSE(latchEvict.awaitMillis(2000));

                ASSERT_TRUE(ClientMapTest<TypeParam>::intMap->removeEntryListener(listenerId));
            }

            TYPED_TEST(ClientMapTest, testListenerWithSqlPredicate) {
                util::CountDownLatch latchAdd(1);
                util::CountDownLatch latchRemove(1);
                util::CountDownLatch latchEvict(1);
                util::CountDownLatch latchUpdate(1);

                typename ClientMapTest<TypeParam>::template CountdownListener<int, int> listener(latchAdd, latchRemove,
                                                                                                 latchUpdate,
                                                                                                 latchEvict);

                // 1 <=key <= 2
                std::string listenerId = ClientMapTest<TypeParam>::intMap->addEntryListener(listener,
                                                                                            query::SqlPredicate(
                                                                                                    "__key < 2"), true);

                ClientMapTest<TypeParam>::intMap->put(1, 1);
                ClientMapTest<TypeParam>::intMap->put(2, 2);
                ClientMapTest<TypeParam>::intMap->put(3, 3, 1000); // evict after 1 second
                ClientMapTest<TypeParam>::intMap->remove(2);

                util::sleep(2);

                ASSERT_EQ(NULL, ClientMapTest<TypeParam>::intMap->get(3).get()); // trigger eviction

                // update an entry
                ClientMapTest<TypeParam>::intMap->set(1, 5);
                boost::shared_ptr<int> value = ClientMapTest<TypeParam>::intMap->get(1);
                ASSERT_NE((int *) NULL, value.get());
                ASSERT_EQ(5, *value);

                util::CountDownLatchWaiter latches;
                latches.add(latchAdd).add(latchUpdate);
                ASSERT_TRUE(latches.awaitMillis(2000));

                latches.reset();
                latches.add(latchRemove).add(latchEvict);
                ASSERT_FALSE(latches.awaitMillis(2000));

                ASSERT_TRUE(ClientMapTest<TypeParam>::intMap->removeEntryListener(listenerId));
            }

            TYPED_TEST(ClientMapTest, testListenerWithRegExPredicate) {
                util::CountDownLatch latchAdd(2);
                util::CountDownLatch latchRemove(1);
                util::CountDownLatch latchEvict(1);
                util::CountDownLatch latchUpdate(1);

                typename ClientMapTest<TypeParam>::template CountdownListener<std::string, std::string> listener(
                        latchAdd, latchRemove, latchUpdate, latchEvict);

                // key matches any word containing ".*met.*"
                std::string listenerId = ClientMapTest<TypeParam>::imap->addEntryListener(listener,
                                                                                          query::RegexPredicate(
                                                                                                  query::QueryConstants::getKeyAttributeName(),
                                                                                                  ".*met.*"), true);

                ClientMapTest<TypeParam>::imap->put("ilkay", "yasar");
                ClientMapTest<TypeParam>::imap->put("mehmet", "demir");
                ClientMapTest<TypeParam>::imap->put("metin", "ozen", 1000); // evict after 1 second
                ClientMapTest<TypeParam>::imap->put("hasan", "can");
                ClientMapTest<TypeParam>::imap->remove("mehmet");

                util::sleep(2);

                ASSERT_EQ((std::string *) NULL, ClientMapTest<TypeParam>::imap->get("metin").get()); // trigger eviction

                // update an entry
                ClientMapTest<TypeParam>::imap->set("hasan", "suphi");
                boost::shared_ptr<std::string> value = ClientMapTest<TypeParam>::imap->get("hasan");
                ASSERT_NE((std::string *) NULL, value.get());
                ASSERT_EQ("suphi", *value);

                util::CountDownLatchWaiter latches;
                latches.add(latchAdd).add(latchRemove).add(latchEvict);
                ASSERT_TRUE(latches.awaitMillis(2000));

                ASSERT_FALSE(latchUpdate.awaitMillis(2000));

                ASSERT_TRUE(ClientMapTest<TypeParam>::imap->removeEntryListener(listenerId));
            }

            TYPED_TEST(ClientMapTest, testListenerWithInstanceOfPredicate) {
                util::CountDownLatch latchAdd(3);
                util::CountDownLatch latchRemove(1);
                util::CountDownLatch latchEvict(1);
                util::CountDownLatch latchUpdate(1);

                typename ClientMapTest<TypeParam>::template CountdownListener<int, int> listener(latchAdd, latchRemove,
                                                                                                 latchUpdate,
                                                                                                 latchEvict);

                // 1 <=key <= 2
                std::string listenerId = ClientMapTest<TypeParam>::intMap->addEntryListener(listener,
                                                                                            query::InstanceOfPredicate(
                                                                                                    "java.lang.Integer"),
                                                                                            false);

                ClientMapTest<TypeParam>::intMap->put(1, 1);
                ClientMapTest<TypeParam>::intMap->put(2, 2);
                ClientMapTest<TypeParam>::intMap->put(3, 3, 1000); // evict after 1 second
                ClientMapTest<TypeParam>::intMap->remove(2);

                util::sleep(2);

                ASSERT_EQ(NULL, ClientMapTest<TypeParam>::intMap->get(3).get()); // trigger eviction

                // update an entry
                ClientMapTest<TypeParam>::intMap->set(1, 5);
                boost::shared_ptr<int> value = ClientMapTest<TypeParam>::intMap->get(1);
                ASSERT_NE((int *) NULL, value.get());
                ASSERT_EQ(5, *value);

                util::CountDownLatchWaiter latches;
                latches.add(latchAdd).add(latchRemove).add(latchUpdate).add(latchEvict);
                ASSERT_TRUE(latches.awaitMillis(2000));

                ASSERT_TRUE(ClientMapTest<TypeParam>::intMap->removeEntryListener(listenerId));
            }

            TYPED_TEST(ClientMapTest, testListenerWithNotPredicate) {
                util::CountDownLatch latchAdd(2);
                util::CountDownLatch latchRemove(1);
                util::CountDownLatch latchEvict(1);
                util::CountDownLatch latchUpdate(1);

                typename ClientMapTest<TypeParam>::template CountdownListener<int, int> listener(latchAdd, latchRemove,
                                                                                                 latchUpdate,
                                                                                                 latchEvict);

                // key >= 3
                std::auto_ptr<query::Predicate> greaterLessPred = std::auto_ptr<query::Predicate>(
                        new query::GreaterLessPredicate<int>(query::QueryConstants::getKeyAttributeName(), 3, true,
                                                             false));
                query::NotPredicate notPredicate(greaterLessPred);
                std::string listenerId = ClientMapTest<TypeParam>::intMap->addEntryListener(listener, notPredicate,
                                                                                            false);

                ClientMapTest<TypeParam>::intMap->put(1, 1);
                ClientMapTest<TypeParam>::intMap->put(2, 2);
                ClientMapTest<TypeParam>::intMap->put(3, 3, 1000); // evict after 1 second
                ClientMapTest<TypeParam>::intMap->remove(2);

                util::sleep(2);

                ASSERT_EQ(NULL, ClientMapTest<TypeParam>::intMap->get(3).get()); // trigger eviction

                // update an entry
                ClientMapTest<TypeParam>::intMap->set(1, 5);
                boost::shared_ptr<int> value = ClientMapTest<TypeParam>::intMap->get(1);
                ASSERT_NE((int *) NULL, value.get());
                ASSERT_EQ(5, *value);

                util::CountDownLatchWaiter latches;
                latches.add(latchAdd).add(latchRemove).add(latchUpdate);
                ASSERT_TRUE(latches.awaitMillis(2000));

                latches.reset();
                latches.add(latchEvict);
                ASSERT_FALSE(latches.awaitMillis(1000));

                ASSERT_TRUE(ClientMapTest<TypeParam>::intMap->removeEntryListener(listenerId));
            }

            TYPED_TEST(ClientMapTest, testListenerWithAndPredicate) {
                util::CountDownLatch latchAdd(1);
                util::CountDownLatch latchRemove(1);
                util::CountDownLatch latchEvict(1);
                util::CountDownLatch latchUpdate(1);

                typename ClientMapTest<TypeParam>::template CountdownListener<int, int> listener(latchAdd, latchRemove,
                                                                                                 latchUpdate,
                                                                                                 latchEvict);

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
                std::string listenerId = ClientMapTest<TypeParam>::intMap->addEntryListener(listener, predicate, false);

                ClientMapTest<TypeParam>::intMap->put(1, 1);
                ClientMapTest<TypeParam>::intMap->put(2, 2);
                ClientMapTest<TypeParam>::intMap->put(3, 3, 1000); // evict after 1 second
                ClientMapTest<TypeParam>::intMap->remove(2);

                util::sleep(2);

                ASSERT_EQ(NULL, ClientMapTest<TypeParam>::intMap->get(3).get()); // trigger eviction

                // update an entry
                ClientMapTest<TypeParam>::intMap->set(1, 5);
                boost::shared_ptr<int> value = ClientMapTest<TypeParam>::intMap->get(1);
                ASSERT_NE((int *) NULL, value.get());
                ASSERT_EQ(5, *value);

                util::CountDownLatchWaiter latches;
                latches.add(latchAdd).add(latchUpdate);
                ASSERT_TRUE(latches.awaitMillis(2000));

                latches.reset();
                latches.add(latchEvict).add(latchRemove);
                ASSERT_FALSE(latches.awaitMillis(1000));

                ASSERT_TRUE(ClientMapTest<TypeParam>::intMap->removeEntryListener(listenerId));
            }

            TYPED_TEST(ClientMapTest, testListenerWithOrPredicate) {
                util::CountDownLatch latchAdd(2);
                util::CountDownLatch latchRemove(1);
                util::CountDownLatch latchEvict(1);
                util::CountDownLatch latchUpdate(1);

                typename ClientMapTest<TypeParam>::template CountdownListener<int, int> listener(latchAdd, latchRemove,
                                                                                                 latchUpdate,
                                                                                                 latchEvict);

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
                std::string listenerId = ClientMapTest<TypeParam>::intMap->addEntryListener(listener, predicate, true);

                ClientMapTest<TypeParam>::intMap->put(1, 1);
                ClientMapTest<TypeParam>::intMap->put(2, 2);
                ClientMapTest<TypeParam>::intMap->put(3, 3, 1000); // evict after 1 second
                ClientMapTest<TypeParam>::intMap->remove(2);

                util::sleep(2);

                ASSERT_EQ(NULL, ClientMapTest<TypeParam>::intMap->get(3).get()); // trigger eviction

                // update an entry
                ClientMapTest<TypeParam>::intMap->set(1, 5);
                boost::shared_ptr<int> value = ClientMapTest<TypeParam>::intMap->get(1);
                ASSERT_NE((int *) NULL, value.get());
                ASSERT_EQ(5, *value);

                util::CountDownLatchWaiter latches;
                latches.add(latchAdd).add(latchEvict).add(latchRemove);
                ASSERT_TRUE(latches.awaitMillis(2000));

                ASSERT_FALSE(latchUpdate.awaitMillis(2000));

                ASSERT_TRUE(ClientMapTest<TypeParam>::intMap->removeEntryListener(listenerId));
            }

            TYPED_TEST(ClientMapTest, testClearEvent) {
                util::CountDownLatch latch(1);
                typename ClientMapTest<TypeParam>::ClearListener clearListener(latch);
                std::string listenerId = ClientMapTest<TypeParam>::imap->addEntryListener(clearListener, false);
                ClientMapTest<TypeParam>::imap->put("key1", "value1");
                ClientMapTest<TypeParam>::imap->clear();
                ASSERT_TRUE(latch.await(120));
                ClientMapTest<TypeParam>::imap->removeEntryListener(listenerId);
            }

            TYPED_TEST(ClientMapTest, testEvictAllEvent) {
                util::CountDownLatch latch(1);
                typename ClientMapTest<TypeParam>::EvictListener evictListener(latch);
                std::string listenerId = ClientMapTest<TypeParam>::imap->addEntryListener(evictListener, false);
                ClientMapTest<TypeParam>::imap->put("key1", "value1");
                ClientMapTest<TypeParam>::imap->evictAll();
                ASSERT_TRUE(latch.await(120));
                ClientMapTest<TypeParam>::imap->removeEntryListener(listenerId);
            }

            TYPED_TEST(ClientMapTest, testMapWithPortable) {
                boost::shared_ptr<Employee> n1 = ClientMapTest<TypeParam>::employees->get(1);
                ASSERT_EQ(n1.get(), (Employee *) NULL);
                Employee employee("sancar", 24);
                boost::shared_ptr<Employee> ptr = ClientMapTest<TypeParam>::employees->put(1, employee);
                ASSERT_EQ(ptr.get(), (Employee *) NULL);
                ASSERT_FALSE(ClientMapTest<TypeParam>::employees->isEmpty());
                EntryView<int, Employee> view = ClientMapTest<TypeParam>::employees->getEntryView(1);
                ASSERT_EQ(view.value, employee);
                ASSERT_EQ(view.key, 1);

                ClientMapTest<TypeParam>::employees->addIndex("a", true);
                ClientMapTest<TypeParam>::employees->addIndex("n", false);
            }

            TYPED_TEST(ClientMapTest, testMapStoreRelatedRequests) {
                ClientMapTest<TypeParam>::imap->putTransient("ali", "veli", 1100);
                ClientMapTest<TypeParam>::imap->flush();
                ASSERT_EQ(1, ClientMapTest<TypeParam>::imap->size());
                ASSERT_FALSE(ClientMapTest<TypeParam>::imap->evict("deli"));
                ASSERT_TRUE(ClientMapTest<TypeParam>::imap->evict("ali"));
                ASSERT_EQ(ClientMapTest<TypeParam>::imap->get("ali").get(), (std::string *) NULL);
            }

            TYPED_TEST(ClientMapTest, testExecuteOnKey) {
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);

                ClientMapTest<TypeParam>::employees->put(3, empl1);
                ClientMapTest<TypeParam>::employees->put(4, empl2);

                typename ClientMapTest<TypeParam>::EntryMultiplier processor(4);

                boost::shared_ptr<int> result = ClientMapTest<TypeParam>::employees->template executeOnKey<int, typename ClientMapTest<TypeParam>::EntryMultiplier>(
                        4, processor);

                ASSERT_NE((int *) NULL, result.get());
                ASSERT_EQ(4 * processor.getMultiplier(), *result);
            }

            TYPED_TEST(ClientMapTest, testSubmitToKey) {
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);

                ClientMapTest<TypeParam>::employees->put(3, empl1);
                ClientMapTest<TypeParam>::employees->put(4, empl2);

                typename ClientMapTest<TypeParam>::EntryMultiplier processor(4);

                Future<int> future =
                        ClientMapTest<TypeParam>::employees->template submitToKey<int, typename ClientMapTest<TypeParam>::EntryMultiplier>(
                        4, processor);

                future_status status = future.wait_for(2 * 1000);
                ASSERT_EQ(future_status::ready, status);
                std::auto_ptr<int> result = future.get();
                ASSERT_NE((int *) NULL, result.get());
                ASSERT_EQ(4 * processor.getMultiplier(), *result);
            }

            TYPED_TEST(ClientMapTest, testExecuteOnNonExistentKey) {
                typename ClientMapTest<TypeParam>::EntryMultiplier processor(4);

                boost::shared_ptr<int> result = ClientMapTest<TypeParam>::employees->template executeOnKey<int, typename ClientMapTest<TypeParam>::EntryMultiplier>(
                        17, processor);

                ASSERT_NE((int *) NULL, result.get());
                ASSERT_EQ(-1, *result);
            }

            TYPED_TEST(ClientMapTest, testExecuteOnKeys) {
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);
                Employee empl3("deniz", 25);

                ClientMapTest<TypeParam>::employees->put(3, empl1);
                ClientMapTest<TypeParam>::employees->put(4, empl2);
                ClientMapTest<TypeParam>::employees->put(5, empl3);

                typename ClientMapTest<TypeParam>::EntryMultiplier processor(4);

                std::set<int> keys;
                keys.insert(3);
                keys.insert(5);
                // put non existent key
                keys.insert(999);

                std::map<int, boost::shared_ptr<int> > result = ClientMapTest<TypeParam>::employees->template executeOnKeys<int, typename ClientMapTest<TypeParam>::EntryMultiplier>(
                        keys, processor);

                ASSERT_EQ(3, (int) result.size());
                ASSERT_NE(result.end(), result.find(3));
                ASSERT_NE(result.end(), result.find(5));
                ASSERT_NE(result.end(), result.find(999));
                ASSERT_EQ(3 * processor.getMultiplier(), *result[3]);
                ASSERT_EQ(5 * processor.getMultiplier(), *result[5]);
                ASSERT_EQ(-1, *result[999]);
            }

            TYPED_TEST(ClientMapTest, testExecuteOnEntries) {
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);
                Employee empl3("deniz", 25);

                ClientMapTest<TypeParam>::employees->put(3, empl1);
                ClientMapTest<TypeParam>::employees->put(4, empl2);
                ClientMapTest<TypeParam>::employees->put(5, empl3);

                typename ClientMapTest<TypeParam>::EntryMultiplier processor(4);

                std::map<int, boost::shared_ptr<int> > result = ClientMapTest<TypeParam>::employees->template executeOnEntries<int, typename ClientMapTest<TypeParam>::EntryMultiplier>(
                        processor);

                ASSERT_EQ(3, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(3)));
                ASSERT_TRUE((result.end() != result.find(4)));
                ASSERT_TRUE((result.end() != result.find(5)));
                ASSERT_EQ(3 * processor.getMultiplier(), *result[3]);
                ASSERT_EQ(4 * processor.getMultiplier(), *result[4]);
                ASSERT_EQ(5 * processor.getMultiplier(), *result[5]);
            }

            TYPED_TEST(ClientMapTest, testExecuteOnEntriesWithTruePredicate) {
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);
                Employee empl3("deniz", 25);

                ClientMapTest<TypeParam>::employees->put(3, empl1);
                ClientMapTest<TypeParam>::employees->put(4, empl2);
                ClientMapTest<TypeParam>::employees->put(5, empl3);

                typename ClientMapTest<TypeParam>::EntryMultiplier processor(4);

                std::map<int, boost::shared_ptr<int> > result = ClientMapTest<TypeParam>::employees->template executeOnEntries<int, typename ClientMapTest<TypeParam>::EntryMultiplier>(
                        processor, query::TruePredicate());

                ASSERT_EQ(3, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(3)));
                ASSERT_TRUE((result.end() != result.find(4)));
                ASSERT_TRUE((result.end() != result.find(5)));
                ASSERT_EQ(3 * processor.getMultiplier(), *result[3]);
                ASSERT_EQ(4 * processor.getMultiplier(), *result[4]);
                ASSERT_EQ(5 * processor.getMultiplier(), *result[5]);
            }

            TYPED_TEST(ClientMapTest, testExecuteOnEntriesWithFalsePredicate) {
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);
                Employee empl3("deniz", 25);

                ClientMapTest<TypeParam>::employees->put(3, empl1);
                ClientMapTest<TypeParam>::employees->put(4, empl2);
                ClientMapTest<TypeParam>::employees->put(5, empl3);

                typename ClientMapTest<TypeParam>::EntryMultiplier processor(4);

                std::map<int, boost::shared_ptr<int> > result = ClientMapTest<TypeParam>::employees->template executeOnEntries<int, typename ClientMapTest<TypeParam>::EntryMultiplier>(
                        processor, query::FalsePredicate());

                ASSERT_EQ(0, (int) result.size());
            }

            TYPED_TEST(ClientMapTest, testExecuteOnEntriesWithAndPredicate) {
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);
                Employee empl3("deniz", 25);

                ClientMapTest<TypeParam>::employees->put(3, empl1);
                ClientMapTest<TypeParam>::employees->put(4, empl2);
                ClientMapTest<TypeParam>::employees->put(5, empl3);

                query::AndPredicate andPredicate;
                /* 25 <= age <= 35 AND age = 35 */
                andPredicate.add(
                        std::auto_ptr<query::Predicate>(new query::BetweenPredicate<int>("a", 25, 35))).add(
                        std::auto_ptr<query::Predicate>(
                                new query::NotPredicate(
                                        std::auto_ptr<query::Predicate>(new query::EqualPredicate<int>("a", 35)))));

                typename ClientMapTest<TypeParam>::EntryMultiplier processor(4);

                std::map<int, boost::shared_ptr<int> > result = ClientMapTest<TypeParam>::employees->template executeOnEntries<int, typename ClientMapTest<TypeParam>::EntryMultiplier>(
                        processor, andPredicate);

                ASSERT_EQ(1, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(5)));
                ASSERT_EQ(5 * processor.getMultiplier(), *result[5]);
            }

            TYPED_TEST(ClientMapTest, testExecuteOnEntriesWithOrPredicate) {
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);
                Employee empl3("deniz", 25);

                ClientMapTest<TypeParam>::employees->put(3, empl1);
                ClientMapTest<TypeParam>::employees->put(4, empl2);
                ClientMapTest<TypeParam>::employees->put(5, empl3);

                query::OrPredicate orPredicate;
                /* age == 21 OR age > 25 */
                orPredicate.add(
                        std::auto_ptr<query::Predicate>(new query::EqualPredicate<int>("a", 21))).add(
                        std::auto_ptr<query::Predicate>(new query::GreaterLessPredicate<int>("a", 25, false, false)));

                typename ClientMapTest<TypeParam>::EntryMultiplier processor(4);

                std::map<int, boost::shared_ptr<int> > result = ClientMapTest<TypeParam>::employees->template executeOnEntries<int, typename ClientMapTest<TypeParam>::EntryMultiplier>(
                        processor, orPredicate);

                ASSERT_EQ(2, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(3)));
                ASSERT_TRUE((result.end() != result.find(4)));
                ASSERT_EQ(3 * processor.getMultiplier(), *result[3]);
                ASSERT_EQ(4 * processor.getMultiplier(), *result[4]);
            }

            TYPED_TEST(ClientMapTest, testExecuteOnEntriesWithBetweenPredicate) {
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);
                Employee empl3("deniz", 25);

                ClientMapTest<TypeParam>::employees->put(3, empl1);
                ClientMapTest<TypeParam>::employees->put(4, empl2);
                ClientMapTest<TypeParam>::employees->put(5, empl3);

                typename ClientMapTest<TypeParam>::EntryMultiplier processor(4);

                std::map<int, boost::shared_ptr<int> > result = ClientMapTest<TypeParam>::employees->template executeOnEntries<int, typename ClientMapTest<TypeParam>::EntryMultiplier>(
                        processor, query::BetweenPredicate<int>("a", 25, 35));

                ASSERT_EQ(2, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(3)));
                ASSERT_TRUE((result.end() != result.find(5)));
                ASSERT_EQ(3 * processor.getMultiplier(), *result[3]);
                ASSERT_EQ(5 * processor.getMultiplier(), *result[5]);
            }

            TYPED_TEST(ClientMapTest, testExecuteOnEntriesWithEqualPredicate) {
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);
                Employee empl3("deniz", 25);

                ClientMapTest<TypeParam>::employees->put(3, empl1);
                ClientMapTest<TypeParam>::employees->put(4, empl2);
                ClientMapTest<TypeParam>::employees->put(5, empl3);

                typename ClientMapTest<TypeParam>::EntryMultiplier processor(4);

                std::map<int, boost::shared_ptr<int> > result = ClientMapTest<TypeParam>::employees->template executeOnEntries<int, typename ClientMapTest<TypeParam>::EntryMultiplier>(
                        processor, query::EqualPredicate<int>("a", 25));

                ASSERT_EQ(1, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(5)));

                result = ClientMapTest<TypeParam>::employees->template executeOnEntries<int, typename ClientMapTest<TypeParam>::EntryMultiplier>(
                        processor, query::EqualPredicate<int>("a", 10));

                ASSERT_EQ(0, (int) result.size());
            }

            TYPED_TEST(ClientMapTest, testExecuteOnEntriesWithNotEqualPredicate) {
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);
                Employee empl3("deniz", 25);

                ClientMapTest<TypeParam>::employees->put(3, empl1);
                ClientMapTest<TypeParam>::employees->put(4, empl2);
                ClientMapTest<TypeParam>::employees->put(5, empl3);

                typename ClientMapTest<TypeParam>::EntryMultiplier processor(4);

                std::map<int, boost::shared_ptr<int> > result = ClientMapTest<TypeParam>::employees->template executeOnEntries<int, typename ClientMapTest<TypeParam>::EntryMultiplier>(
                        processor, query::NotEqualPredicate<int>("a", 25));

                ASSERT_EQ(2, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(3)));
                ASSERT_TRUE((result.end() != result.find(4)));
            }

            TYPED_TEST(ClientMapTest, testExecuteOnEntriesWithGreaterLessPredicate) {
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);
                Employee empl3("deniz", 25);

                ClientMapTest<TypeParam>::employees->put(3, empl1);
                ClientMapTest<TypeParam>::employees->put(4, empl2);
                ClientMapTest<TypeParam>::employees->put(5, empl3);

                typename ClientMapTest<TypeParam>::EntryMultiplier processor(4);

                std::map<int, boost::shared_ptr<int> > result = ClientMapTest<TypeParam>::employees->template executeOnEntries<int, typename ClientMapTest<TypeParam>::EntryMultiplier>(
                        processor, query::GreaterLessPredicate<int>("a", 25, false, true)); // <25 matching

                ASSERT_EQ(1, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(4)));

                result = ClientMapTest<TypeParam>::employees->template executeOnEntries<int, typename ClientMapTest<TypeParam>::EntryMultiplier>(
                        processor, query::GreaterLessPredicate<int>("a", 25, true, true)); // <=25 matching

                ASSERT_EQ(2, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(4)));
                ASSERT_TRUE((result.end() != result.find(5)));

                result = ClientMapTest<TypeParam>::employees->template executeOnEntries<int, typename ClientMapTest<TypeParam>::EntryMultiplier>(
                        processor, query::GreaterLessPredicate<int>("a", 25, false, false)); // >25 matching

                ASSERT_EQ(1, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(3)));

                result = ClientMapTest<TypeParam>::employees->template executeOnEntries<int, typename ClientMapTest<TypeParam>::EntryMultiplier>(
                        processor, query::GreaterLessPredicate<int>("a", 25, true, false)); // >=25 matching

                ASSERT_EQ(2, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(3)));
                ASSERT_TRUE((result.end() != result.find(5)));
            }

            TYPED_TEST(ClientMapTest, testExecuteOnEntriesWithLikePredicate) {
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);
                Employee empl3("deniz", 25);

                ClientMapTest<TypeParam>::employees->put(3, empl1);
                ClientMapTest<TypeParam>::employees->put(4, empl2);
                ClientMapTest<TypeParam>::employees->put(5, empl3);

                typename ClientMapTest<TypeParam>::EntryMultiplier processor(4);

                std::map<int, boost::shared_ptr<int> > result = ClientMapTest<TypeParam>::employees->template executeOnEntries<int, typename ClientMapTest<TypeParam>::EntryMultiplier>(
                        processor, query::LikePredicate("n", "deniz"));

                ASSERT_EQ(1, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(5)));
            }

            TYPED_TEST(ClientMapTest, testExecuteOnEntriesWithILikePredicate) {
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);
                Employee empl3("deniz", 25);

                ClientMapTest<TypeParam>::employees->put(3, empl1);
                ClientMapTest<TypeParam>::employees->put(4, empl2);
                ClientMapTest<TypeParam>::employees->put(5, empl3);

                typename ClientMapTest<TypeParam>::EntryMultiplier processor(4);

                std::map<int, boost::shared_ptr<int> > result = ClientMapTest<TypeParam>::employees->template executeOnEntries<int, typename ClientMapTest<TypeParam>::EntryMultiplier>(
                        processor, query::ILikePredicate("n", "deniz"));

                ASSERT_EQ(1, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(5)));
            }

            TYPED_TEST(ClientMapTest, testExecuteOnEntriesWithInPredicate) {
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);
                Employee empl3("deniz", 25);

                ClientMapTest<TypeParam>::employees->put(3, empl1);
                ClientMapTest<TypeParam>::employees->put(4, empl2);
                ClientMapTest<TypeParam>::employees->put(5, empl3);

                typename ClientMapTest<TypeParam>::EntryMultiplier processor(4);

                std::vector<std::string> values;
                values.push_back("ahmet");
                query::InPredicate<std::string> predicate("n", values);
                predicate.add("mehmet");
                std::map<int, boost::shared_ptr<int> > result = ClientMapTest<TypeParam>::employees->template executeOnEntries<int, typename ClientMapTest<TypeParam>::EntryMultiplier>(
                        processor, predicate);

                ASSERT_EQ(2, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(3)));
                ASSERT_TRUE((result.end() != result.find(4)));
            }

            TYPED_TEST(ClientMapTest, testExecuteOnEntriesWithInstanceOfPredicate) {
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);
                Employee empl3("deniz", 25);

                ClientMapTest<TypeParam>::employees->put(3, empl1);
                ClientMapTest<TypeParam>::employees->put(4, empl2);
                ClientMapTest<TypeParam>::employees->put(5, empl3);

                typename ClientMapTest<TypeParam>::EntryMultiplier processor(4);
                std::map<int, boost::shared_ptr<int> > result = ClientMapTest<TypeParam>::employees->template executeOnEntries<int, typename ClientMapTest<TypeParam>::EntryMultiplier>(
                        processor, query::InstanceOfPredicate("com.hazelcast.client.test.Employee"));

                ASSERT_EQ(3, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(3)));
                ASSERT_TRUE((result.end() != result.find(4)));
                ASSERT_TRUE((result.end() != result.find(5)));
            }

            TYPED_TEST(ClientMapTest, testExecuteOnEntriesWithNotPredicate) {
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);
                Employee empl3("deniz", 25);

                ClientMapTest<TypeParam>::employees->put(3, empl1);
                ClientMapTest<TypeParam>::employees->put(4, empl2);
                ClientMapTest<TypeParam>::employees->put(5, empl3);

                typename ClientMapTest<TypeParam>::EntryMultiplier processor(4);
                std::auto_ptr<query::Predicate> eqPredicate(new query::EqualPredicate<int>("a", 25));
                query::NotPredicate notPredicate(eqPredicate);
                std::map<int, boost::shared_ptr<int> > result = ClientMapTest<TypeParam>::employees->template executeOnEntries<int, typename ClientMapTest<TypeParam>::EntryMultiplier>(
                        processor, notPredicate);

                ASSERT_EQ(2, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(3)));
                ASSERT_TRUE((result.end() != result.find(4)));

                query::NotPredicate notFalsePredicate(std::auto_ptr<query::Predicate>(new query::FalsePredicate()));
                result = ClientMapTest<TypeParam>::employees->template executeOnEntries<int, typename ClientMapTest<TypeParam>::EntryMultiplier>(
                        processor, notFalsePredicate);

                ASSERT_EQ(3, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(3)));
                ASSERT_TRUE((result.end() != result.find(4)));
                ASSERT_TRUE((result.end() != result.find(5)));

                query::NotPredicate notBetweenPredicate(
                        std::auto_ptr<query::Predicate>(new query::BetweenPredicate<int>("a", 25, 35)));
                result = ClientMapTest<TypeParam>::employees->template executeOnEntries<int, typename ClientMapTest<TypeParam>::EntryMultiplier>(
                        processor, notBetweenPredicate);

                ASSERT_EQ(1, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(4)));
            }

            TYPED_TEST(ClientMapTest, testExecuteOnEntriesWithRegexPredicate) {
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);
                Employee empl3("deniz", 25);

                ClientMapTest<TypeParam>::employees->put(3, empl1);
                ClientMapTest<TypeParam>::employees->put(4, empl2);
                ClientMapTest<TypeParam>::employees->put(5, empl3);

                typename ClientMapTest<TypeParam>::EntryMultiplier processor(4);

                std::map<int, boost::shared_ptr<int> > result = ClientMapTest<TypeParam>::employees->template executeOnEntries<int, typename ClientMapTest<TypeParam>::EntryMultiplier>(
                        processor, query::RegexPredicate("n", ".*met"));

                ASSERT_EQ(2, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(3)));
                ASSERT_TRUE((result.end() != result.find(4)));
            }

            TYPED_TEST(ClientMapTest, testAddInterceptor) {
                std::string prefix("My Prefix");
                typename ClientMapTest<TypeParam>::MapGetInterceptor interceptor(prefix);
                std::string interceptorId = ClientMapTest<TypeParam>::imap->
                        template addInterceptor<typename ClientMapTest<TypeParam>::MapGetInterceptor>(interceptor);

                boost::shared_ptr<std::string> val = ClientMapTest<TypeParam>::imap->get("nonexistent");
                ASSERT_NE((std::string *) NULL, val.get());
                ASSERT_EQ(prefix, *val);

                val = ClientMapTest<TypeParam>::imap->put("key1", "value1");
                ASSERT_EQ((std::string *) NULL, val.get());

                val = ClientMapTest<TypeParam>::imap->get("key1");
                ASSERT_NE((std::string *) NULL, val.get());
                ASSERT_EQ(prefix + "value1", *val);

                ClientMapTest<TypeParam>::imap->removeInterceptor(interceptorId);
            }
        }
    }
}
