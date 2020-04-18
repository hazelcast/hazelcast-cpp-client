/*
 * Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
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
#include "HazelcastServerFactory.h"
#include "HazelcastServer.h"
#include "ClientTestSupport.h"
#include <regex>
#include <vector>
#include "ringbuffer/StartsWithStringFilter.h"
#include "serialization/Employee.h"
#include "ClientTestSupportBase.h"
#include <hazelcast/client/ClientConfig.h>
#include <hazelcast/client/exception/IllegalStateException.h>
#include <hazelcast/client/HazelcastClient.h>
#include <hazelcast/client/serialization/pimpl/SerializationService.h>
#include <hazelcast/util/UuidUtil.h>
#include <hazelcast/client/impl/Partition.h>
#include <gtest/gtest.h>
#include <thread>
#include <hazelcast/client/spi/ClientContext.h>
#include <hazelcast/client/connection/ClientConnectionManagerImpl.h>
#include <hazelcast/client/protocol/Principal.h>
#include <hazelcast/client/connection/Connection.h>
#include <ClientTestSupport.h>
#include <memory>
#include <hazelcast/client/proxy/ClientPNCounterProxy.h>
#include <hazelcast/client/serialization/pimpl/DataInput.h>
#include <hazelcast/util/AddressUtil.h>
#include <hazelcast/util/RuntimeAvailableProcessors.h>
#include <hazelcast/client/serialization/pimpl/DataOutput.h>
#include <hazelcast/util/AddressHelper.h>
#include <hazelcast/client/exception/IOException.h>
#include <hazelcast/client/protocol/ClientExceptionFactory.h>
#include <hazelcast/util/IOUtil.h>
#include <hazelcast/util/CountDownLatch.h>
#include <ClientTestSupportBase.h>
#include <hazelcast/util/Executor.h>
#include <hazelcast/util/Util.h>
#include <hazelcast/util/impl/SimpleExecutorService.h>
#include <TestHelperFunctions.h>
#include <ostream>
#include <hazelcast/util/ILogger.h>
#include <ctime>
#include <errno.h>
#include <hazelcast/client/LifecycleListener.h>
#include "serialization/TestRawDataPortable.h"
#include "serialization/TestSerializationConstants.h"
#include "serialization/TestMainPortable.h"
#include "serialization/TestNamedPortable.h"
#include "serialization/TestInvalidReadPortable.h"
#include "serialization/TestInvalidWritePortable.h"
#include "serialization/TestInnerPortable.h"
#include "serialization/TestNamedPortableV2.h"
#include "serialization/TestNamedPortableV3.h"
#include <hazelcast/client/SerializationConfig.h>
#include <hazelcast/client/HazelcastJsonValue.h>
#include <stdint.h>
#include "customSerialization/TestCustomSerializerX.h"
#include "customSerialization/TestCustomXSerializable.h"
#include "customSerialization/TestCustomPersonSerializer.h"
#include "serialization/ChildTemplatedPortable2.h"
#include "serialization/ParentTemplatedPortable.h"
#include "serialization/ChildTemplatedPortable1.h"
#include "serialization/ObjectCarryingPortable.h"
#include "serialization/TestDataSerializable.h"
#include <hazelcast/client/internal/nearcache/impl/NearCacheRecordStore.h>
#include <hazelcast/client/internal/nearcache/impl/store/NearCacheDataRecordStore.h>
#include <hazelcast/client/internal/nearcache/impl/store/NearCacheObjectRecordStore.h>
#include <hazelcast/client/query/FalsePredicate.h>
#include <set>
#include <hazelcast/client/query/EqualPredicate.h>
#include <hazelcast/client/query/QueryConstants.h>
#include <HazelcastServer.h>
#include "TestHelperFunctions.h"
#include <cmath>
#include <hazelcast/client/spi/impl/sequence/CallIdSequenceWithoutBackpressure.h>
#include <hazelcast/util/Thread.h>
#include <hazelcast/client/spi/impl/sequence/CallIdSequenceWithBackpressure.h>
#include <hazelcast/client/spi/impl/sequence/FailFastCallIdSequence.h>
#include <iostream>
#include <string>
#include "executor/tasks/SelectAllMembers.h"
#include "executor/tasks/IdentifiedFactory.h"
#include <hazelcast/client/serialization/ObjectDataOutput.h>
#include <hazelcast/client/serialization/ObjectDataInput.h>
#include "executor/tasks/CancellationAwareTask.h"
#include "executor/tasks/NullCallable.h"
#include "executor/tasks/SerializedCounterCallable.h"
#include "executor/tasks/MapPutPartitionAwareCallable.h"
#include "executor/tasks/SelectNoMembers.h"
#include "executor/tasks/GetMemberUuidTask.h"
#include "executor/tasks/FailingCallable.h"
#include "executor/tasks/AppendCallable.h"
#include "executor/tasks/TaskWithUnserializableResponse.h"
#include <executor/tasks/CancellationAwareTask.h>
#include <executor/tasks/FailingCallable.h>
#include <executor/tasks/SelectNoMembers.h>
#include <executor/tasks/SerializedCounterCallable.h>
#include <executor/tasks/TaskWithUnserializableResponse.h>
#include <executor/tasks/GetMemberUuidTask.h>
#include <executor/tasks/AppendCallable.h>
#include <executor/tasks/SelectAllMembers.h>
#include <executor/tasks/MapPutPartitionAwareCallable.h>
#include <executor/tasks/NullCallable.h>
#include <stdlib.h>
#include <fstream>
#include <boost/asio.hpp>
#include <cassert>

#ifdef HZ_BUILD_WITH_SSL
#include <openssl/crypto.h>
#endif

#include "hazelcast/client/config/ClientAwsConfig.h"
#include "hazelcast/client/aws/impl/DescribeInstances.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/client/connection/ClientConnectionManagerImpl.h"
#include "hazelcast/client/serialization/ObjectDataOutput.h"
#include "hazelcast/client/serialization/ObjectDataInput.h"
#include "hazelcast/client/exception/ProtocolExceptions.h"
#include "hazelcast/client/internal/socket/SSLSocket.h"
#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/util/CountDownLatch.h"
#include "hazelcast/client/MembershipListener.h"
#include "hazelcast/client/InitialMembershipEvent.h"
#include "hazelcast/client/InitialMembershipListener.h"
#include "hazelcast/client/MemberAttributeEvent.h"
#include "hazelcast/client/EntryAdapter.h"
#include "hazelcast/client/LifecycleListener.h"
#include "hazelcast/client/SocketInterceptor.h"
#include "hazelcast/client/Socket.h"
#include "hazelcast/client/Cluster.h"
#include "hazelcast/util/Sync.h"
#include "hazelcast/client/query/SqlPredicate.h"
#include "hazelcast/util/Util.h"
#include "hazelcast/util/Runnable.h"
#include "hazelcast/util/Thread.h"
#include "hazelcast/util/ILogger.h"
#include "hazelcast/client/IMap.h"
#include "hazelcast/util/Bits.h"
#include "hazelcast/util/SyncHttpsClient.h"
#include "hazelcast/client/exception/IOException.h"
#include "hazelcast/util/AtomicInt.h"
#include "hazelcast/util/BlockingConcurrentQueue.h"
#include "hazelcast/util/UTFUtil.h"
#include "hazelcast/util/ConcurrentQueue.h"
#include "hazelcast/util/impl/SimpleExecutorService.h"
#include "hazelcast/util/Future.h"
#include "hazelcast/util/concurrent/locks/LockSupport.h"
#include "hazelcast/client/ExecutionCallback.h"
#include "hazelcast/client/Pipelining.h"
#include "hazelcast/client/exception/IllegalArgumentException.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/serialization/PortableReader.h"
#include "hazelcast/client/serialization/pimpl/SerializationService.h"
#include "hazelcast/client/SerializationConfig.h"
#include "hazelcast/util/MurmurHash3.h"
#include "hazelcast/client/ILock.h"
#include "hazelcast/client/ITopic.h"
#include "hazelcast/client/protocol/ClientMessage.h"
#include "hazelcast/client/protocol/ClientProtocolErrorCodes.h"
#include "hazelcast/client/adaptor/RawPointerSet.h"
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
#include "hazelcast/client/adaptor/RawPointerMap.h"
#include "hazelcast/client/serialization/IdentifiedDataSerializable.h"
#include "hazelcast/client/adaptor/RawPointerList.h"
#include "hazelcast/client/adaptor/RawPointerTransactionalQueue.h"
#include "hazelcast/client/ItemListener.h"
#include "hazelcast/client/adaptor/RawPointerQueue.h"
#include "hazelcast/client/adaptor/RawPointerTransactionalMap.h"
#include "hazelcast/client/MultiMap.h"
#include "hazelcast/client/adaptor/RawPointerMultiMap.h"
#include "hazelcast/client/adaptor/RawPointerTransactionalMultiMap.h"
#include "hazelcast/util/LittleEndianBufferWrapper.h"
#include "hazelcast/client/exception/IllegalStateException.h"
#include "hazelcast/client/EntryEvent.h"
#include "hazelcast/client/HazelcastJsonValue.h"
#include "hazelcast/client/ISemaphore.h"
#include "hazelcast/client/IAtomicLong.h"
#include "hazelcast/client/mixedtype/MultiMap.h"
#include "hazelcast/client/mixedtype/IList.h"
#include "hazelcast/client/IList.h"
#include "hazelcast/client/IQueue.h"
#include "hazelcast/client/mixedtype/IQueue.h"
#include "hazelcast/client/ClientProperties.h"
#include "hazelcast/client/config/ClientAwsConfig.h"
#include "hazelcast/client/aws/utility/CloudUtility.h"
#include "hazelcast/client/ISet.h"
#include "hazelcast/client/mixedtype/ISet.h"
#include "hazelcast/client/ICountDownLatch.h"
#include "hazelcast/client/ReliableTopic.h"
#include "hazelcast/client/IdGenerator.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(disable: 4996) //for unsafe getenv
#endif

namespace hazelcast {
    namespace client {
        namespace test {
            class ClientReplicatedMapTest : public ClientTestSupport {
            public:
            protected:
                static const int OPERATION_COUNT;

                typedef std::vector<std::pair<int, int> > TEST_VALUES_TYPE;

                class SamplePortable : public serialization::Portable {
                public:
                    SamplePortable(int a) : a(a) {}

                    SamplePortable() {}

                    virtual int getFactoryId() const {
                        return 5;
                    }

                    virtual int getClassId() const {
                        return 6;
                    }

                    virtual void writePortable(serialization::PortableWriter &writer) const {
                        writer.writeInt("a", a);
                    }

                    virtual void readPortable(serialization::PortableReader &reader) {
                        a = reader.readInt("a");
                    }

                    int32_t a;
                };

                class SamplePortableFactory : public serialization::PortableFactory {
                public:
                    virtual unique_ptr<serialization::Portable> create(int32_t classId) const {
                        return unique_ptr<serialization::Portable>(new SamplePortable());
                    }
                };

                bool findValueForKey(int key, TEST_VALUES_TYPE &testValues, int &value) {
                    for (const TEST_VALUES_TYPE::value_type &entry : testValues) {
                        if (key == entry.first) {
                            value = entry.second;
                            return true;
                        }
                    }
                    return false;
                }

                template<typename T>
                bool contains(std::shared_ptr<DataArray<T> > &values, const T &value) {
                    for (size_t i = 0; i < values->size(); ++i) {
                        if (*values->get(i) == value) {
                            return true;
                        }
                    }
                    return false;
                }

                TEST_VALUES_TYPE buildTestValues() {
                    TEST_VALUES_TYPE testValues;
                    for (int i = 0; i < 100; ++i) {
                        testValues.push_back(std::make_pair(i, i * i));
                    }
                    return testValues;
                }

                virtual void TearDown() {
                }

                static void SetUpTestCase() {
                    factory = new HazelcastServerFactory(g_srvFactory->getServerAddress(), "hazelcast/test/resources/replicated-map-binary-in-memory-config-hazelcast.xml");
                    instance1 = new HazelcastServer(*factory);
                    client = new HazelcastClient(getConfig());
                    client2 = new HazelcastClient(getConfig());
                }

                static void TearDownTestCase() {
                    delete client;
                    delete client2;
                    delete instance1;
                    delete factory;

                    client = NULL;
                    client2 = NULL;
                    instance1 = NULL;
                    factory = NULL;
                }

                static ClientConfig getClientConfigWithNearCacheInvalidationEnabled() {
                    std::shared_ptr<config::NearCacheConfig<int, int> > nearCacheConfig(
                            new config::NearCacheConfig<int, int>());
                    nearCacheConfig->setInvalidateOnChange(true).setInMemoryFormat(config::BINARY);
                    return ClientConfig().addNearCacheConfig(nearCacheConfig);
                }

                static HazelcastServer *instance1;
                static HazelcastClient *client;
                static HazelcastClient *client2;
                static HazelcastServerFactory *factory;
            };

            HazelcastServer *ClientReplicatedMapTest::instance1 = NULL;
            HazelcastClient *ClientReplicatedMapTest::client = NULL;
            HazelcastClient *ClientReplicatedMapTest::client2 = NULL;
            HazelcastServerFactory *ClientReplicatedMapTest::factory = NULL;
            const int ClientReplicatedMapTest::OPERATION_COUNT = 100;

            TEST_F(ClientReplicatedMapTest, testEmptyMapIsEmpty) {
                std::shared_ptr<ReplicatedMap<int, int> > map = client->getReplicatedMap<int, int>(getTestName());
                ASSERT_TRUE(map->isEmpty()) << "map should be empty";
            }

            TEST_F(ClientReplicatedMapTest, testNonEmptyMapIsNotEmpty) {
                std::shared_ptr<ReplicatedMap<int, int> > map = client->getReplicatedMap<int, int>(getTestName());
                map->put(1, 1);
                ASSERT_FALSE(map->isEmpty()) << "map should not be empty";
            }

            TEST_F(ClientReplicatedMapTest, testPutAll) {
                std::shared_ptr<ReplicatedMap<std::string, std::string> > map1 =
                        client->getReplicatedMap<std::string, std::string>(getTestName());

                std::shared_ptr<ReplicatedMap<std::string, std::string> > map2 =
                        client2->getReplicatedMap<std::string, std::string>(getTestName());

                std::map<std::string, std::string> mapTest;
                for (int i = 0; i < OPERATION_COUNT; i++) {
                    std::ostringstream out;
                    out << "foo-" << i;
                    mapTest[out.str()] = "bar";
                }
                map1->putAll(mapTest);
                ASSERT_EQ((int32_t) mapTest.size(), map1->size());
                std::shared_ptr<LazyEntryArray<std::string, std::string> > entries = map1->entrySet();
                for (size_t j = 0; j < entries->size(); ++j) {
                    const string *key = entries->getKey(j);
                    ASSERT_NOTNULL(key, std::string);
                    ASSERT_EQ(0U, entries->getKey(j)->find("foo-"));
                    const string *value = entries->getValue(j);
                    ASSERT_NOTNULL(value, std::string);
                    ASSERT_EQ("bar", *value);
                }

                entries = map2->entrySet();
                for (size_t j = 0; j < entries->size(); ++j) {
                    const string *key = entries->getKey(j);
                    ASSERT_NOTNULL(key, std::string);
                    ASSERT_EQ(0U, entries->getKey(j)->find("foo-"));
                    const string *value = entries->getValue(j);
                    ASSERT_NOTNULL(value, std::string);
                    ASSERT_EQ("bar", *value);
                }

// TODO add server side data check using remote controller scripting
            }

            TEST_F(ClientReplicatedMapTest, testGet) {
                std::shared_ptr<ReplicatedMap<std::string, std::string> > map1 = client->getReplicatedMap<std::string, std::string>(
                        getTestName());
                std::shared_ptr<ReplicatedMap<std::string, std::string> > map2 = client2->getReplicatedMap<std::string, std::string>(
                        getTestName());

                for (int i = 0; i < OPERATION_COUNT; i++) {
                    std::ostringstream out;
                    out << "foo-" << i;
                    map1->put(out.str(), "bar");
                }

                for (int i = 0; i < OPERATION_COUNT; i++) {
                    std::ostringstream out;
                    out << "foo-" << i;
                    ASSERT_NOTNULL(map1->get(out.str()).get(), std::string);
                    ASSERT_NOTNULL(map2->get(out.str()).get(), std::string);
                    ASSERT_EQ("bar", *map1->get(out.str()));
                    ASSERT_EQ("bar", *map2->get(out.str()));
                }
            }

            TEST_F(ClientReplicatedMapTest, testPutNullReturnValueDeserialization) {
                std::shared_ptr<ReplicatedMap<int, int> > map = client->getReplicatedMap<int, int>(getTestName());
                ASSERT_NULL("Put should return null", map->put(1, 2).get(), int);
            }

            TEST_F(ClientReplicatedMapTest, testPutReturnValueDeserialization) {
                std::shared_ptr<ReplicatedMap<int, int> > map = client->getReplicatedMap<int, int>(getTestName());
                map->put(1, 2);

                std::shared_ptr<int> value = map->put(1, 3);
                ASSERT_NOTNULL(value.get(), int);
                ASSERT_EQ(2, *value);
            }

            TEST_F(ClientReplicatedMapTest, testAdd) {
                std::shared_ptr<ReplicatedMap<std::string, std::string> > map1 =
                        client->getReplicatedMap<std::string, std::string>(getTestName());

                std::shared_ptr<ReplicatedMap<std::string, std::string> > map2 =
                        client2->getReplicatedMap<std::string, std::string>(getTestName());

                for (int i = 0; i < OPERATION_COUNT; i++) {
                    std::ostringstream out;
                    out << "foo-" << i;
                    map1->put(out.str(), "bar");
                }

                ASSERT_EQ(OPERATION_COUNT, map2->size());

                std::shared_ptr<LazyEntryArray<std::string, std::string> > entries = map2->entrySet();
                for (size_t j = 0; j < entries->size(); ++j) {
                    const string *key = entries->getKey(j);
                    ASSERT_NOTNULL(key, std::string);
                    ASSERT_EQ(0U, entries->getKey(j)->find("foo-"));
                    const string *value = entries->getValue(j);
                    ASSERT_NOTNULL(value, std::string);
                    ASSERT_EQ("bar", *value);
                }

                entries = map1->entrySet();
                for (size_t j = 0; j < entries->size(); ++j) {
                    const string *key = entries->getKey(j);
                    ASSERT_NOTNULL(key, std::string);
                    ASSERT_EQ(0U, entries->getKey(j)->find("foo-"));
                    const string *value = entries->getValue(j);
                    ASSERT_NOTNULL(value, std::string);
                    ASSERT_EQ("bar", *value);
                }
            }

            TEST_F(ClientReplicatedMapTest, testClear) {
                std::shared_ptr<ReplicatedMap<std::string, std::string> > map1 =
                        client->getReplicatedMap<std::string, std::string>(getTestName());

                std::shared_ptr<ReplicatedMap<std::string, std::string> > map2 =
                        client2->getReplicatedMap<std::string, std::string>(getTestName());

                for (int i = 0; i < OPERATION_COUNT; i++) {
                    std::ostringstream out;
                    out << "foo-" << i;
                    map1->put(out.str(), "bar");
                }

                ASSERT_EQ(OPERATION_COUNT, map2->size());

                std::shared_ptr<LazyEntryArray<std::string, std::string> > entries = map2->entrySet();
                for (size_t j = 0; j < entries->size(); ++j) {
                    const string *key = entries->getKey(j);
                    ASSERT_NOTNULL(key, std::string);
                    ASSERT_EQ(0U, entries->getKey(j)->find("foo-"));
                    const string *value = entries->getValue(j);
                    ASSERT_NOTNULL(value, std::string);
                    ASSERT_EQ("bar", *value);
                }

                entries = map1->entrySet();
                for (size_t j = 0; j < entries->size(); ++j) {
                    const string *key = entries->getKey(j);
                    ASSERT_NOTNULL(key, std::string);
                    ASSERT_EQ(0U, entries->getKey(j)->find("foo-"));
                    const string *value = entries->getValue(j);
                    ASSERT_NOTNULL(value, std::string);
                    ASSERT_EQ("bar", *value);
                }

                map1->clear();
                ASSERT_EQ(0, map1->size());
                ASSERT_EQ(0, map2->size());
            }

            TEST_F(ClientReplicatedMapTest, testUpdate) {
                std::shared_ptr<ReplicatedMap<std::string, std::string> > map1 =
                        client->getReplicatedMap<std::string, std::string>(getTestName());

                std::shared_ptr<ReplicatedMap<std::string, std::string> > map2 =
                        client2->getReplicatedMap<std::string, std::string>(getTestName());

                for (int i = 0; i < OPERATION_COUNT; i++) {
                    std::ostringstream out;
                    out << "foo-" << i;
                    map1->put(out.str(), "bar");
                }

                ASSERT_EQ(OPERATION_COUNT, map2->size());

                std::shared_ptr<LazyEntryArray<std::string, std::string> > entries = map2->entrySet();
                for (size_t j = 0; j < entries->size(); ++j) {
                    const string *key = entries->getKey(j);
                    ASSERT_NOTNULL(key, std::string);
                    ASSERT_EQ(0U, entries->getKey(j)->find("foo-"));
                    const string *value = entries->getValue(j);
                    ASSERT_NOTNULL(value, std::string);
                    ASSERT_EQ("bar", *value);
                }

                entries = map1->entrySet();
                for (size_t j = 0; j < entries->size(); ++j) {
                    const string *key = entries->getKey(j);
                    ASSERT_NOTNULL(key, std::string);
                    ASSERT_EQ(0U, entries->getKey(j)->find("foo-"));
                    const string *value = entries->getValue(j);
                    ASSERT_NOTNULL(value, std::string);
                    ASSERT_EQ("bar", *value);
                }

                for (int i = 0; i < OPERATION_COUNT; i++) {
                    std::ostringstream out;
                    out << "foo-" << i;
                    map2->put(out.str(), "bar2");
                }

                entries = map2->entrySet();
                for (size_t j = 0; j < entries->size(); ++j) {
                    const string *value = entries->getValue(j);
                    ASSERT_NOTNULL(value, std::string);
                    ASSERT_EQ("bar2", *value);
                }

                entries = map1->entrySet();
                for (size_t j = 0; j < entries->size(); ++j) {
                    const string *value = entries->getValue(j);
                    ASSERT_NOTNULL(value, std::string);
                    ASSERT_EQ("bar2", *value);
                }

            }

            TEST_F(ClientReplicatedMapTest, testRemove) {
                std::shared_ptr<ReplicatedMap<std::string, std::string> > map1 =
                        client->getReplicatedMap<std::string, std::string>(getTestName());

                std::shared_ptr<ReplicatedMap<std::string, std::string> > map2 =
                        client2->getReplicatedMap<std::string, std::string>(getTestName());

                for (int i = 0; i < OPERATION_COUNT; i++) {
                    std::ostringstream out;
                    out << "foo-" << i;
                    map1->put(out.str(), "bar");
                }

                ASSERT_EQ(OPERATION_COUNT, map2->size());

                std::shared_ptr<LazyEntryArray<std::string, std::string> > entries = map2->entrySet();
                for (size_t j = 0; j < entries->size(); ++j) {
                    const string *key = entries->getKey(j);
                    ASSERT_NOTNULL(key, std::string);
                    ASSERT_EQ(0U, entries->getKey(j)->find("foo-"));
                    const string *value = entries->getValue(j);
                    ASSERT_NOTNULL(value, std::string);
                    ASSERT_EQ("bar", *value);
                }

                entries = map1->entrySet();
                for (size_t j = 0; j < entries->size(); ++j) {
                    const string *key = entries->getKey(j);
                    ASSERT_NOTNULL(key, std::string);
                    ASSERT_EQ(0U, entries->getKey(j)->find("foo-"));
                    const string *value = entries->getValue(j);
                    ASSERT_NOTNULL(value, std::string);
                    ASSERT_EQ("bar", *value);
                }

                for (int i = 0; i < OPERATION_COUNT; i++) {
                    std::ostringstream out;
                    out << "foo-" << i;
                    std::shared_ptr<std::string> value = map2->remove(out.str());
                    ASSERT_NOTNULL(value.get(), std::string);
                    ASSERT_EQ("bar", *value);
                }

                for (int i = 0; i < OPERATION_COUNT; i++) {
                    std::ostringstream out;
                    out << "foo-" << i;
                    ASSERT_NULL("Removed value should not exist for map1", map1->get(out.str()).get(), std::string);
                    ASSERT_NULL("Removed value should not exist for map2", map2->get(out.str()).get(), std::string);
                }
            }

            TEST_F(ClientReplicatedMapTest, testSize) {
                std::shared_ptr<ReplicatedMap<int, int> > map1 = client->getReplicatedMap<int, int>(getTestName());
                std::shared_ptr<ReplicatedMap<int, int> > map2 = client2->getReplicatedMap<int, int>(getTestName());

                TEST_VALUES_TYPE testValues = buildTestValues();
                size_t half = testValues.size() / 2;
                for (size_t i = 0; i < testValues.size(); i++) {
                    std::shared_ptr<ReplicatedMap<int, int> > map = i < half ? map1 : map2;
                    std::pair<int, int> &entry = testValues[i];
                    map->put(entry.first, entry.second);
                }

                ASSERT_EQ((int32_t) testValues.size(), map1->size());
                ASSERT_EQ((int32_t) testValues.size(), map2->size());
            }

            TEST_F(ClientReplicatedMapTest, testContainsKey) {
                std::shared_ptr<ReplicatedMap<std::string, std::string> > map1 =
                        client->getReplicatedMap<std::string, std::string>(getTestName());

                std::shared_ptr<ReplicatedMap<std::string, std::string> > map2 =
                        client2->getReplicatedMap<std::string, std::string>(getTestName());

                for (int i = 0; i < OPERATION_COUNT; i++) {
                    std::ostringstream out;
                    out << "foo-" << i;
                    map1->put(out.str(), "bar");
                }

                for (int i = 0; i < OPERATION_COUNT; i++) {
                    std::ostringstream out;
                    out << "foo-" << i;
                    ASSERT_TRUE(map2->containsKey(out.str()));
                }

                for (int i = 0; i < OPERATION_COUNT; i++) {
                    std::ostringstream out;
                    out << "foo-" << i;
                    ASSERT_TRUE(map1->containsKey(out.str()));
                }
            }

            TEST_F(ClientReplicatedMapTest, testContainsValue) {
                std::shared_ptr<ReplicatedMap<int, int> > map1 = client->getReplicatedMap<int, int>(getTestName());
                std::shared_ptr<ReplicatedMap<int, int> > map2 = client2->getReplicatedMap<int, int>(getTestName());

                TEST_VALUES_TYPE testValues = buildTestValues();
                size_t half = testValues.size() / 2;
                for (size_t i = 0; i < testValues.size(); i++) {
                    std::shared_ptr<ReplicatedMap<int, int> > map = i < half ? map1 : map2;
                    std::pair<int, int> &entry = testValues[i];
                    map->put(entry.first, entry.second);
                }

                for (TEST_VALUES_TYPE::value_type &entry : testValues) {
                    ASSERT_TRUE(map2->containsValue(entry.second));
                }

                for (TEST_VALUES_TYPE::value_type &entry : testValues) {
                    ASSERT_TRUE(map1->containsValue(entry.second));
                }
            }

            TEST_F(ClientReplicatedMapTest, testValues) {
                std::shared_ptr<ReplicatedMap<int, int> > map1 = client->getReplicatedMap<int, int>(getTestName());
                std::shared_ptr<ReplicatedMap<int, int> > map2 = client2->getReplicatedMap<int, int>(getTestName());

                TEST_VALUES_TYPE testValues = buildTestValues();
                size_t half = testValues.size() / 2;
                for (size_t i = 0; i < testValues.size(); i++) {
                    std::shared_ptr<ReplicatedMap<int, int> > map = i < half ? map1 : map2;
                    std::pair<int, int> &entry = testValues[i];
                    map->put(entry.first, entry.second);
                }

                std::shared_ptr<DataArray<int> > values1 = map1->values();
                std::shared_ptr<DataArray<int> > values2 = map2->values();

                for (TEST_VALUES_TYPE::value_type &entry : testValues) {
                    ASSERT_TRUE(contains(values1, entry.second));
                    ASSERT_TRUE(contains(values2, entry.second));
                }
            }

            TEST_F(ClientReplicatedMapTest, testKeySet) {
                std::shared_ptr<ReplicatedMap<int, int> > map1 = client->getReplicatedMap<int, int>(getTestName());
                std::shared_ptr<ReplicatedMap<int, int> > map2 = client2->getReplicatedMap<int, int>(getTestName());

                TEST_VALUES_TYPE testValues = buildTestValues();
                size_t half = testValues.size() / 2;
                for (size_t i = 0; i < testValues.size(); i++) {
                    std::shared_ptr<ReplicatedMap<int, int> > map = i < half ? map1 : map2;
                    std::pair<int, int> &entry = testValues[i];
                    map->put(entry.first, entry.second);
                }

                std::shared_ptr<DataArray<int> > keys1 = map1->keySet();
                std::shared_ptr<DataArray<int> > keys2 = map2->keySet();

                for (TEST_VALUES_TYPE::value_type &entry : testValues) {
                    ASSERT_TRUE(contains(keys1, entry.first));
                    ASSERT_TRUE(contains(keys2, entry.first));
                }
            }

            TEST_F(ClientReplicatedMapTest, testEntrySet) {
                std::shared_ptr<ReplicatedMap<int, int> > map1 = client->getReplicatedMap<int, int>(getTestName());
                std::shared_ptr<ReplicatedMap<int, int> > map2 = client2->getReplicatedMap<int, int>(getTestName());

                TEST_VALUES_TYPE testValues = buildTestValues();
                size_t half = testValues.size() / 2;
                for (size_t i = 0; i < testValues.size(); i++) {
                    std::shared_ptr<ReplicatedMap<int, int> > map = i < half ? map1 : map2;
                    std::pair<int, int> &entry = testValues[i];
                    map->put(entry.first, entry.second);
                }

                std::shared_ptr<LazyEntryArray<int, int> > entrySet1 = map1->entrySet();
                std::shared_ptr<LazyEntryArray<int, int> > entrySet2 = map2->entrySet();

                for (size_t j = 0; j < entrySet2->size(); ++j) {
                    int value;
                    ASSERT_TRUE(findValueForKey(*entrySet2->getKey(j), testValues, value));
                    ASSERT_EQ(value, *entrySet2->getValue(j));
                }

                for (size_t j = 0; j < entrySet1->size(); ++j) {
                    int value;
                    ASSERT_TRUE(findValueForKey(*entrySet1->getKey(j), testValues, value));
                    ASSERT_EQ(value, *entrySet1->getValue(j));
                }
            }

            TEST_F(ClientReplicatedMapTest, testRetrieveUnknownValue) {
                std::shared_ptr<ReplicatedMap<std::string, std::string> > map =
                        client->getReplicatedMap<std::string, std::string>(getTestName());

                std::shared_ptr<std::string> value = map->get("foo");
                ASSERT_NULL("No entry with key foo should exist", value.get(), std::string);
            }

            TEST_F(ClientReplicatedMapTest, testNearCacheInvalidation) {
                std::string mapName = randomString();

                ClientConfig clientConfig = getClientConfigWithNearCacheInvalidationEnabled();
                HazelcastClient client1(clientConfig);
                HazelcastClient client2(clientConfig);

                std::shared_ptr<ReplicatedMap<int, int> > replicatedMap1 = client1.getReplicatedMap<int, int>(
                        mapName);

                replicatedMap1->put(1, 1);
// puts key 1 to Near Cache
                replicatedMap1->get(1);

                std::shared_ptr<ReplicatedMap<int, int> > replicatedMap2 = client2.getReplicatedMap<int, int>(
                        mapName);
// this should invalidate Near Cache of replicatedMap1
                replicatedMap2->clear();

                ASSERT_NULL_EVENTUALLY(replicatedMap1->get(1).get(), int);
            }

            TEST_F(ClientReplicatedMapTest, testClientPortableWithoutRegisteringToNode) {
                ClientConfig clientConfig;
                SerializationConfig serializationConfig;
                serializationConfig.addPortableFactory(5, std::shared_ptr<serialization::PortableFactory>(
                        new SamplePortableFactory()));
                clientConfig.setSerializationConfig(serializationConfig);

                HazelcastClient client(clientConfig);
                std::shared_ptr<ReplicatedMap<int, SamplePortable> > sampleMap = client.getReplicatedMap<int, SamplePortable>(
                        getTestName());
                sampleMap->put(1, SamplePortable(666));
                std::shared_ptr<SamplePortable> samplePortable = sampleMap->get(1);
                ASSERT_NOTNULL(samplePortable.get(), SamplePortable);
                ASSERT_EQ(666, samplePortable->a);
            }
        }
    }
}




namespace hazelcast {
    namespace client {
        namespace test {
            class ClientReplicatedMapListenerTest : public ClientTestSupport {
            protected:
                class EventCountingListener : public EntryListener<int, int> {
                public:
                    EventCountingListener() : keys(UINT_MAX) {}

                    virtual void entryAdded(const EntryEvent<int, int> &event) {
                        keys.push(event.getKey());
                        ++addCount;
                    }

                    virtual void entryRemoved(const EntryEvent<int, int> &event) {
                        keys.push(event.getKey());
                        ++removeCount;
                    }

                    virtual void entryUpdated(const EntryEvent<int, int> &event) {
                        keys.push(event.getKey());
                        ++updateCount;
                    }

                    virtual void entryEvicted(const EntryEvent<int, int> &event) {
                        keys.push(event.getKey());
                        ++evictCount;
                    }

                    virtual void entryExpired(const EntryEvent<int, int> &event) {
                    }

                    virtual void entryMerged(const EntryEvent<int, int> &event) {
                    }

                    virtual void entryLoaded(const EntryEvent<int, int> &event) {
                    }

                    virtual void mapEvicted(const MapEvent &event) {
                        ++mapEvictCount;
                    }

                    virtual void mapCleared(const MapEvent &event) {
                        ++mapClearCount;
                    }

                    hazelcast::util::BlockingConcurrentQueue<int> keys;
                    hazelcast::util::AtomicInt addCount;
                    hazelcast::util::AtomicInt removeCount;
                    hazelcast::util::AtomicInt updateCount;
                    hazelcast::util::AtomicInt evictCount;
                    hazelcast::util::AtomicInt mapClearCount;
                    hazelcast::util::AtomicInt mapEvictCount;
                };

                virtual void TearDown() {
                }

                static void SetUpTestCase() {
                    instance1 = new HazelcastServer(*g_srvFactory);
                    instance2 = new HazelcastServer(*g_srvFactory);
                    client = new HazelcastClient(getConfig());
                    client2 = new HazelcastClient(getConfig());
                }

                static void TearDownTestCase() {
                    delete client;
                    delete client2;
                    delete instance1;
                    delete instance2;

                    client = NULL;
                    client2 = NULL;
                    instance1 = NULL;
                    instance2 = NULL;
                }

                static HazelcastServer *instance1;
                static HazelcastServer *instance2;
                static HazelcastClient *client;
                static HazelcastClient *client2;
            };

            HazelcastServer *ClientReplicatedMapListenerTest::instance1 = NULL;
            HazelcastServer *ClientReplicatedMapListenerTest::instance2 = NULL;
            HazelcastClient *ClientReplicatedMapListenerTest::client = NULL;
            HazelcastClient *ClientReplicatedMapListenerTest::client2 = NULL;

            TEST_F(ClientReplicatedMapListenerTest, testEntryAdded) {
                std::shared_ptr<ReplicatedMap<int, int> > replicatedMap = client->getReplicatedMap<int, int>(
                        getTestName());
                std::shared_ptr<EventCountingListener> listener(new EventCountingListener());
                replicatedMap->addEntryListener(listener);
                replicatedMap->put(1, 1);
                ASSERT_EQ_EVENTUALLY(1, listener->addCount.load());
            }

            TEST_F(ClientReplicatedMapListenerTest, testEntryUpdated) {
                std::shared_ptr<ReplicatedMap<int, int> > replicatedMap = client->getReplicatedMap<int, int>(
                        getTestName());
                std::shared_ptr<EventCountingListener> listener(new EventCountingListener());
                replicatedMap->addEntryListener(listener);
                replicatedMap->put(1, 1);
                replicatedMap->put(1, 2);
                ASSERT_EQ_EVENTUALLY(1, listener->updateCount.load());
            }

            TEST_F(ClientReplicatedMapListenerTest, testEntryRemoved) {
                std::shared_ptr<ReplicatedMap<int, int> > replicatedMap = client->getReplicatedMap<int, int>(
                        getTestName());
                std::shared_ptr<EventCountingListener> listener(new EventCountingListener());
                replicatedMap->addEntryListener(listener);
                replicatedMap->put(1, 1);
                replicatedMap->remove(1);
                ASSERT_EQ_EVENTUALLY(1, listener->removeCount.load());
            }

            TEST_F(ClientReplicatedMapListenerTest, testMapClear) {
                std::shared_ptr<ReplicatedMap<int, int> > replicatedMap = client->getReplicatedMap<int, int>(
                        getTestName());
                std::shared_ptr<EventCountingListener> listener(new EventCountingListener());
                replicatedMap->addEntryListener(listener);
                replicatedMap->put(1, 1);
                replicatedMap->clear();
                ASSERT_EQ_EVENTUALLY(1, listener->mapClearCount.load());
            }

            TEST_F(ClientReplicatedMapListenerTest, testListenToKeyForEntryAdded) {
                std::shared_ptr<ReplicatedMap<int, int> > replicatedMap = client->getReplicatedMap<int, int>(
                        getTestName());
                std::shared_ptr<EventCountingListener> listener(new EventCountingListener());
                replicatedMap->addEntryListener(listener, 1);
                replicatedMap->put(1, 1);
                replicatedMap->put(2, 2);
                ASSERT_TRUE_EVENTUALLY(
                        listener->keys.size() == 1U && listener->keys.pop() == 1 && listener->addCount.load() == 1);
            }

            TEST_F(ClientReplicatedMapListenerTest, testListenWithPredicate) {
                std::shared_ptr<ReplicatedMap<int, int> > replicatedMap = client->getReplicatedMap<int, int>(
                        getTestName());
                std::shared_ptr<EventCountingListener> listener(new EventCountingListener());
                replicatedMap->addEntryListener(listener, query::FalsePredicate());
                replicatedMap->put(2, 2);
// Check for 3 seconds
                ASSERT_TRUE_ALL_THE_TIME((listener->addCount.load() == 0), 3);
            }

        }
    }
}




namespace hazelcast {
    namespace client {
        namespace test {
            class BasicClientReplicatedMapNearCacheTest
                    : public ClientTestSupport, public ::testing::WithParamInterface<config::InMemoryFormat> {
            public:
                static void SetUpTestSuite() {
                    instance = new HazelcastServer(*g_srvFactory);
                    instance2 = new HazelcastServer(*g_srvFactory);
                }

                static void TearDownTestSuite() {
                    delete instance2;
                    delete instance;
                    instance2 = NULL;
                    instance = NULL;
                }

                virtual void SetUp() {
                    nearCacheConfig = NearCacheTestUtils::createNearCacheConfig<int, std::string>(GetParam(),
                                                                                                  getTestName());
                }

                virtual void TearDown() {
                    if (nearCachedMap.get()) {
                        nearCachedMap->destroy();
                    }
                    if (noNearCacheMap.get()) {
                        noNearCacheMap->destroy();
                    }
                    if (NULL != client.get()) {
                        client->shutdown();
                    }
                    if (NULL != nearCachedClient.get()) {
                        nearCachedClient->shutdown();
                    }
                }

            protected:
                /**
                 * Provides utility methods for unified Near Cache tests.
                 */
                class NearCacheTestUtils : public ClientTestSupport {
                public:
                    /**
                     * Creates a {@link NearCacheConfig} with a given {@link InMemoryFormat}.
                     *
                     * @param inMemoryFormat the {@link InMemoryFormat} to set
                     * @return the {@link NearCacheConfig}
                     */
                    template<typename K, typename V>
                    static std::shared_ptr<config::NearCacheConfig<K, V> > createNearCacheConfig(
                            config::InMemoryFormat inMemoryFormat, const std::string &mapName) {
                        std::shared_ptr<config::NearCacheConfig<K, V> > nearCacheConfig(
                                new config::NearCacheConfig<K, V>());

                        nearCacheConfig->setName(mapName).setInMemoryFormat(inMemoryFormat).setInvalidateOnChange(true);

                        return nearCacheConfig;
                    }

                    /**
                     * Configures the {@link EvictionConfig} of the given {@link NearCacheConfig}.
                     *
                     * @param nearCacheConfig the {@link NearCacheConfig} to configure
                     * @param evictionPolicy  the {@link EvictionPolicy} to set
                     * @param maxSizePolicy   the {@link MaxSizePolicy} to set
                     * @param maxSize         the max size to set
                     */
                    template<typename K, typename V>
                    static void setEvictionConfig(config::NearCacheConfig<K, V> &nearCacheConfig,
                                                  config::EvictionPolicy evictionPolicy,
                                                  typename config::EvictionConfig<K, V>::MaxSizePolicy maxSizePolicy,
                                                  int maxSize) {
                        nearCacheConfig.getEvictionConfig()
                                ->setEvictionPolicy(evictionPolicy)
                                .setMaximumSizePolicy(maxSizePolicy)
                                .setSize(maxSize);
                    }

                    /**
                     * Asserts the {@link NearCacheStats} for expected values.
                     *
                     * @param stats                   stats of the near cache
                     * @param expectedOwnedEntryCount the expected owned entry count
                     * @param expectedHits            the expected Near Cache hits
                     * @param expectedMisses          the expected Near Cache misses
                     */
                    static void assertNearCacheStats(monitor::NearCacheStats &stats,
                                                     int64_t expectedOwnedEntryCount, int64_t expectedHits,
                                                     int64_t expectedMisses) {
                        assertNearCacheStats(stats, expectedOwnedEntryCount, expectedHits, expectedMisses, 0, 0);
                    }

                    /**
                     * Asserts the {@link NearCacheStats} for expected values.
                     *
                     * @param stats                   stats of the near cache
                     * @param expectedOwnedEntryCount the expected owned entry count
                     * @param expectedHits            the expected Near Cache hits
                     * @param expectedMisses          the expected Near Cache misses
                     * @param expectedEvictions       the expected Near Cache evictions
                     * @param expectedExpirations     the expected Near Cache expirations
                     */
                    static void assertNearCacheStats(monitor::NearCacheStats &stats,
                                                     int64_t expectedOwnedEntryCount, int64_t expectedHits,
                                                     int64_t expectedMisses,
                                                     int64_t expectedEvictions, int64_t expectedExpirations) {
                        assertEqualsFormat("Near Cache entry count should be %ld, but was %ld ",
                                           expectedOwnedEntryCount, stats.getOwnedEntryCount(), stats);
                        assertEqualsFormat("Near Cache hits should be %ld, but were %ld ",
                                           expectedHits, stats.getHits(), stats);
                        assertEqualsFormat("Near Cache misses should be %ld, but were %ld ",
                                           expectedMisses, stats.getMisses(), stats);
                        assertEqualsFormat("Near Cache evictions should be %ld, but were %ld ",
                                           expectedEvictions, stats.getEvictions(), stats);
                        assertEqualsFormat("Near Cache expirations should be %ld, but were %ld ",
                                           expectedExpirations, stats.getExpirations(), stats);
                    }

                    static void assertEqualsFormat(const char *messageFormat, int64_t expected, int64_t actual,
                                                   monitor::NearCacheStats &stats) {
                        char buf[300];
                        hazelcast::util::hz_snprintf(buf, 300, messageFormat, expected, actual);
                        ASSERT_EQ(expected, actual) << buf << "(" << stats.toString() << ")";
                    }

                private:
                    NearCacheTestUtils();

                    NearCacheTestUtils(const NearCacheTestUtils &);
                };

                /**
                 * The default count to be inserted into the Near Caches.
                 */
                static const int DEFAULT_RECORD_COUNT;

                void createContext() {
                    createNoNearCacheContext();
                    createNearCacheContext();
                }

                void createNoNearCacheContext() {
                    client = std::unique_ptr<HazelcastClient>(new HazelcastClient(getConfig()));
                    noNearCacheMap = client->getReplicatedMap<int, std::string>(getTestName());
                }

                void createNearCacheContext() {
                    ClientConfig nearCachedClientConfig = getConfig();
                    nearCachedClientConfig.addNearCacheConfig(nearCacheConfig);
                    nearCachedClient = std::unique_ptr<HazelcastClient>(new HazelcastClient(nearCachedClientConfig));
                    nearCachedMap = nearCachedClient->getReplicatedMap<int, std::string>(getTestName());
                    spi::ClientContext clientContext(*nearCachedClient);
                    nearCacheManager = &clientContext.getNearCacheManager();
                    nearCache = nearCacheManager->
                            getNearCache<int, std::string, serialization::pimpl::Data>(getTestName());
                    this->stats = (nearCache.get() == NULL) ? NULL : &nearCache->getNearCacheStats();
                }

                void testContainsKey(bool useNearCachedMapForRemoval) {
                    createNoNearCacheContext();

                    // populate map
                    noNearCacheMap->put(1, "value1");
                    noNearCacheMap->put(2, "value2");
                    noNearCacheMap->put(3, "value3");

                    createNearCacheContext();

                    // populate Near Cache
                    nearCachedMap->get(1);
                    nearCachedMap->get(2);
                    nearCachedMap->get(3);

                    ASSERT_TRUE(nearCachedMap->containsKey(1));
                    ASSERT_TRUE(nearCachedMap->containsKey(2));
                    ASSERT_TRUE(nearCachedMap->containsKey(3));
                    ASSERT_FALSE(nearCachedMap->containsKey(5));

                    // remove a key which is in the Near Cache
                    std::shared_ptr<ReplicatedMap<int, std::string> > &adapter = useNearCachedMapForRemoval
                                                                                 ? nearCachedMap
                                                                                 : noNearCacheMap;
                    adapter->remove(1);

                    WAIT_TRUE_EVENTUALLY(checkContainKeys());
                    ASSERT_FALSE(nearCachedMap->containsKey(1));
                    ASSERT_TRUE(nearCachedMap->containsKey(2));
                    ASSERT_TRUE(nearCachedMap->containsKey(3));
                    ASSERT_FALSE(nearCachedMap->containsKey(5));
                }

                bool checkContainKeys() {
                    return !nearCachedMap->containsKey(1) && nearCachedMap->containsKey(2) &&
                           nearCachedMap->containsKey(3) && !nearCachedMap->containsKey(5);
                }

                void
                assertNearCacheInvalidationRequests(monitor::NearCacheStats &stats, int64_t invalidationRequests) {
                    if (nearCacheConfig->isInvalidateOnChange() && invalidationRequests > 0) {
                        monitor::impl::NearCacheStatsImpl &nearCacheStatsImpl = (monitor::impl::NearCacheStatsImpl &) stats;
                        ASSERT_EQ_EVENTUALLY(invalidationRequests, nearCacheStatsImpl.getInvalidationRequests());
                        nearCacheStatsImpl.resetInvalidationEvents();
                    }
                }

                void populateMap() {
                    char buf[30];
                    for (int i = 0; i < DEFAULT_RECORD_COUNT; i++) {
                        hazelcast::util::hz_snprintf(buf, 30, "value-%d", i);
                        noNearCacheMap->put(i, buf);
                    }

                    assertNearCacheInvalidationRequests(*stats, DEFAULT_RECORD_COUNT);
                }

                void populateNearCache() {
                    char buf[30];
                    for (int i = 0; i < DEFAULT_RECORD_COUNT; i++) {
                        std::shared_ptr<string> value = nearCachedMap->get(i);
                        ASSERT_NOTNULL(value.get(), std::string);
                        hazelcast::util::hz_snprintf(buf, 30, "value-%d", i);
                        ASSERT_EQ(buf, *value);
                    }
                }

                std::shared_ptr<serialization::pimpl::Data> getNearCacheKey(int key) {
                    spi::ClientContext clientContext(*client);
                    return clientContext.getSerializationService().toSharedData<int>(&key);
                }

                int64_t getExpectedMissesWithLocalUpdatePolicy() {
                    if (nearCacheConfig->getLocalUpdatePolicy() == config::NearCacheConfig<int, std::string>::CACHE) {
                        // we expect the first and second get() to be hits, since the value should be already be cached
                        return stats->getMisses();
                    }
                    // we expect the first get() to be a miss, due to the replaced / invalidated value
                    return stats->getMisses() + 1;
                }

                int64_t getExpectedHitsWithLocalUpdatePolicy() {
                    if (nearCacheConfig->getLocalUpdatePolicy() == config::NearCacheConfig<int, std::string>::CACHE) {
                        // we expect the first and second get() to be hits, since the value should be already be cached
                        return stats->getHits() + 2;
                    }
                    // we expect the second get() to be a hit, since it should be served from the Near Cache
                    return stats->getHits() + 1;
                }

                bool checkMissesAndHits(int64_t &expectedMisses, int64_t &expectedHits,
                                        std::shared_ptr<std::string> &value) {
                    expectedMisses = getExpectedMissesWithLocalUpdatePolicy();
                    expectedHits = getExpectedHitsWithLocalUpdatePolicy();

                    value = nearCachedMap->get(1);
                    if (NULL == value.get() || *value != "newValue") {
                        return false;
                    }
                    value = nearCachedMap->get(1);
                    if (NULL == value.get() || *value != "newValue") {
                        return false;
                    }

                    return expectedHits == stats->getHits() && expectedMisses == stats->getMisses();
                }

                void whenPutAllIsUsed_thenNearCacheShouldBeInvalidated(bool useNearCacheAdapter) {
                    createNoNearCacheContext();

                    createNearCacheContext();

                    populateMap();

                    populateNearCache();

                    std::map<int, std::string> invalidationMap;
                    char buf[30];
                    for (int i = 0; i < DEFAULT_RECORD_COUNT; i++) {
                        hazelcast::util::hz_snprintf(buf, 30, "value-%d", i);
                        invalidationMap[i] = buf;
                    }

                    // this should invalidate the Near Cache
                    std::shared_ptr<ReplicatedMap<int, std::string> > &adapter = useNearCacheAdapter ? nearCachedMap
                                                                                                     : noNearCacheMap;
                    adapter->putAll(invalidationMap);

                    WAIT_EQ_EVENTUALLY(0, nearCache->size());
                    ASSERT_EQ(0, nearCache->size()) << "Invalidation is not working on putAll()";
                }

                std::shared_ptr<config::NearCacheConfig<int, std::string> > nearCacheConfig;
                std::unique_ptr<HazelcastClient> client;
                std::unique_ptr<HazelcastClient> nearCachedClient;
                std::shared_ptr<ReplicatedMap<int, std::string> > noNearCacheMap;
                std::shared_ptr<ReplicatedMap<int, std::string> > nearCachedMap;
                hazelcast::client::internal::nearcache::NearCacheManager *nearCacheManager;
                std::shared_ptr<hazelcast::client::internal::nearcache::NearCache<serialization::pimpl::Data, std::string> > nearCache;
                monitor::NearCacheStats *stats;
                static HazelcastServer *instance;
                static HazelcastServer *instance2;
            };

            const int BasicClientReplicatedMapNearCacheTest::DEFAULT_RECORD_COUNT = 1000;
            HazelcastServer *BasicClientReplicatedMapNearCacheTest::instance = NULL;
            HazelcastServer *BasicClientReplicatedMapNearCacheTest::instance2 = NULL;

            /**
             * Checks that the Near Cache keys are correctly checked when {@link DataStructureAdapter#containsKey(Object)} is used.
             *
             * This variant uses the {@link NearCacheTestContext#nearCacheAdapter}, so there is no Near Cache invalidation necessary.
             */
            TEST_P(BasicClientReplicatedMapNearCacheTest, testContainsKey_withUpdateOnNearCacheAdapter) {
                testContainsKey(true);
            }

            /**
             * Checks that the memory costs are calculated correctly.
             *
             * This variant uses the noNearCacheMap(client with no near cache), so we need to configure Near Cache
             * invalidation.
             */
            TEST_P(BasicClientReplicatedMapNearCacheTest, testContainsKey_withUpdateOnDataAdapter) {
                nearCacheConfig->setInvalidateOnChange(true);
                testContainsKey(false);
            }

            /**
             * Checks that the Near Cache never returns its internal {@link NearCache#NULL_OBJECT} to the public API.
             */
            TEST_P(BasicClientReplicatedMapNearCacheTest,
                   whenEmptyMap_thenPopulatedNearCacheShouldReturnNull_neverNULLOBJECT) {
                createContext();

                for (int i = 0; i < DEFAULT_RECORD_COUNT; i++) {
                    // populate Near Cache
                    ASSERT_NULL("Expected null from original data structure for key " << i,
                                nearCachedMap->get(i).get(), std::string);
                    // fetch value from Near Cache
                    ASSERT_NULL("Expected null from Near cached data structure for key " << i,
                                nearCachedMap->get(i).get(), std::string);

                    // fetch internal value directly from Near Cache
                    std::shared_ptr<serialization::pimpl::Data> key = getNearCacheKey(i);
                    std::shared_ptr<std::string> value = nearCache->get(key);
                    if (value.get() != NULL) {
                        // the internal value should either be `null` or `NULL_OBJECT`
                        std::shared_ptr<std::string> nullObj = std::static_pointer_cast<std::string>(
                                hazelcast::client::internal::nearcache::NearCache<int, std::string>::NULL_OBJECT);
                        ASSERT_EQ(nullObj, nearCache->get(key)) << "Expected NULL_OBJECT in Near Cache for key " << i;
                    }
                }
            }

/**
             * Checks that the Near Cache updates value for keys which are already in the Near Cache,
             * even if the Near Cache is full and the eviction is disabled (via {@link com.hazelcast.config.EvictionPolicy#NONE}.
             *
             * This variant uses the {@link NearCacheTestContext#nearCacheAdapter}, so there is no Near Cache invalidation necessary.
             */
            TEST_P(BasicClientReplicatedMapNearCacheTest,
                   whenCacheIsFull_thenPutOnSameKeyShouldUpdateValue_withUpdateOnNearCacheAdapter) {
                int size = DEFAULT_RECORD_COUNT / 2;
                NearCacheTestUtils::setEvictionConfig<int, std::string>(*nearCacheConfig, config::NONE,
                                                                        config::EvictionConfig<int, std::string>::ENTRY_COUNT,
                                                                        size);

                nearCacheConfig->setInvalidateOnChange(false);

                createNoNearCacheContext();

                createNearCacheContext();

                populateMap();

                populateNearCache();

                ASSERT_EQ(size, nearCache->size());
                std::shared_ptr<std::string> value = nearCachedMap->get(1);
                ASSERT_NOTNULL(value.get(), std::string);
                ASSERT_EQ("value-1", *value);

                nearCachedMap->put(1, "newValue");

                int64_t expectedMisses = getExpectedMissesWithLocalUpdatePolicy();
                int64_t expectedHits = getExpectedHitsWithLocalUpdatePolicy();

                value = nearCachedMap->get(1);
                ASSERT_NOTNULL(value.get(), std::string);
                ASSERT_EQ("newValue", *value);
                value = nearCachedMap->get(1);
                ASSERT_NOTNULL(value.get(), std::string);
                ASSERT_EQ("newValue", *value);

                NearCacheTestUtils::assertNearCacheStats(*stats, size, expectedHits, expectedMisses);
            }

/**
             * Checks that the Near Cache updates value for keys which are already in the Near Cache,
             * even if the Near Cache is full an the eviction is disabled (via {@link com.hazelcast.config.EvictionPolicy#NONE}.
             *
             * This variant uses the {@link NearCacheTestContext#dataAdapter}, so we need to configure Near Cache invalidation.
             */
            TEST_P(BasicClientReplicatedMapNearCacheTest,
                   whenCacheIsFull_thenPutOnSameKeyShouldUpdateValue_withUpdateOnDataAdapter) {
                int size = DEFAULT_RECORD_COUNT / 2;
                NearCacheTestUtils::setEvictionConfig<int, std::string>(*nearCacheConfig, config::NONE,
                                                                        config::EvictionConfig<int, std::string>::ENTRY_COUNT,
                                                                        size);
                nearCacheConfig->setInvalidateOnChange(true);

                createNoNearCacheContext();

                createNearCacheContext();

                populateMap();

                populateNearCache();

                ASSERT_EQ(size, nearCache->size());
                std::shared_ptr<std::string> value = nearCachedMap->get(1);
                ASSERT_NOTNULL(value.get(), std::string);
                ASSERT_EQ("value-1", *value);

                noNearCacheMap->put(1, "newValue");

// we have to use assertTrueEventually since the invalidation is done asynchronously
                int64_t expectedMisses = 0;
                int64_t expectedHits = 0;
                WAIT_TRUE_EVENTUALLY(checkMissesAndHits(expectedMisses, expectedHits, value));
                SCOPED_TRACE("whenCacheIsFull_thenPutOnSameKeyShouldUpdateValue_withUpdateOnDataAdapter");
                NearCacheTestUtils::assertNearCacheStats(*stats, size, expectedHits, expectedMisses);
            }

/**
             * Checks that the Near Cache values are eventually invalidated when {@link DataStructureAdapter#putAll(Map)} is used.
             *
             * This variant uses the nearCacheMap, so there is no Near Cache invalidation necessary.
             */
            TEST_P(BasicClientReplicatedMapNearCacheTest,
                   whenPutAllIsUsed_thenNearCacheShouldBeInvalidated_withUpdateOnNearCacheAdapter) {
                whenPutAllIsUsed_thenNearCacheShouldBeInvalidated(true);
            }

/**
             * Checks that the Near Cache values are eventually invalidated when {@link DataStructureAdapter#putAll(Map)} is used.
             *
             * This variant uses the noNearCacheMap, so we need to configure Near Cache invalidation.
             */
            TEST_P(BasicClientReplicatedMapNearCacheTest,
                   whenPutAllIsUsed_thenNearCacheShouldBeInvalidated_withUpdateOnDataAdapter) {
                nearCacheConfig->setInvalidateOnChange(true);
                whenPutAllIsUsed_thenNearCacheShouldBeInvalidated(false);
            }

/**
             * Checks that the {@link com.hazelcast.monitor.NearCacheStats} are calculated correctly.
             */
            TEST_P(BasicClientReplicatedMapNearCacheTest, testNearCacheStats) {
                createNoNearCacheContext();

                createNearCacheContext();

// populate map
                populateMap();

                {
                    SCOPED_TRACE("testNearCacheStats when near cache is empty");
                    NearCacheTestUtils::assertNearCacheStats(*stats, 0, 0, 0);
                }

// populate Near Cache. Will cause misses and will increment the owned entry count
                populateNearCache();
                {
                    SCOPED_TRACE("testNearCacheStats when near cache is initially populated");
                    NearCacheTestUtils::assertNearCacheStats(*stats, DEFAULT_RECORD_COUNT, 0, DEFAULT_RECORD_COUNT);
                }

// make some hits
                populateNearCache();
                {
                    SCOPED_TRACE("testNearCacheStats when near cache is hit after being populated.");
                    NearCacheTestUtils::assertNearCacheStats(*stats, DEFAULT_RECORD_COUNT, DEFAULT_RECORD_COUNT,
                                                             DEFAULT_RECORD_COUNT);
                }
            }

            TEST_P(BasicClientReplicatedMapNearCacheTest, testNearCacheEviction) {
                NearCacheTestUtils::setEvictionConfig<int, std::string>(*nearCacheConfig, config::LRU,
                                                                        config::EvictionConfig<int, std::string>::ENTRY_COUNT,
                                                                        DEFAULT_RECORD_COUNT);
                createNoNearCacheContext();

// all Near Cache implementations use the same eviction algorithm, which evicts a single entry
                int64_t expectedEvictions = 1;

                createNearCacheContext();

// populate map with an extra entry
                populateMap();
                char buf[20];
                hazelcast::util::hz_snprintf(buf, 20, "value-%d", DEFAULT_RECORD_COUNT);
                noNearCacheMap->put(DEFAULT_RECORD_COUNT, buf);

// populate Near Caches
                populateNearCache();

// we expect (size + the extra entry - the expectedEvictions) entries in the Near Cache
                int64_t expectedOwnedEntryCount = DEFAULT_RECORD_COUNT + 1 - expectedEvictions;
                int64_t expectedHits = stats->getHits();
                int64_t expectedMisses = stats->getMisses() + 1;

// trigger eviction via fetching the extra entry
                nearCachedMap->get(DEFAULT_RECORD_COUNT);

                int64_t evictions = stats->getEvictions();
                ASSERT_GE(evictions, expectedEvictions)
                                            << "Near Cache eviction count didn't reach the desired value ("
                                            << expectedEvictions << " vs. " << evictions << ") ("
                                            << stats->toString();

                SCOPED_TRACE("testNearCacheEviction");

                NearCacheTestUtils::assertNearCacheStats(*stats, expectedOwnedEntryCount, expectedHits, expectedMisses,
                                                         expectedEvictions, 0);
            }

            INSTANTIATE_TEST_SUITE_P(ReplicatedMapNearCacheTest, BasicClientReplicatedMapNearCacheTest,
                                     ::testing::Values(config::BINARY, config::OBJECT));
        }
    }
}




namespace hazelcast {
    namespace client {
        namespace test {
            class ClientReplicatedMapNearCacheTest : public ClientTestSupport {
            protected:
                /**
                 * The default name used for the data structures which have a Near Cache.
                 */
                static const std::string DEFAULT_NEAR_CACHE_NAME;

                static void SetUpTestCase() {
                    instance = new HazelcastServer(*g_srvFactory);
                    instance2 = new HazelcastServer(*g_srvFactory);
                }

                static void TearDownTestCase() {
                    delete instance2;
                    delete instance;
                    instance2 = NULL;
                    instance = NULL;
                }


                virtual void TearDown() {
                    if (map.get()) {
                        map->destroy();
                    }
                }

                std::shared_ptr<config::NearCacheConfig<int, int> > newNoInvalidationNearCacheConfig() {
                    std::shared_ptr<config::NearCacheConfig<int, int> > nearCacheConfig(newNearCacheConfig());
                    nearCacheConfig->setInMemoryFormat(config::OBJECT);
                    nearCacheConfig->setInvalidateOnChange(false);
                    return nearCacheConfig;
                }

                std::shared_ptr<config::NearCacheConfig<int, int> > newNearCacheConfig() {
                    return std::shared_ptr<config::NearCacheConfig<int, int> >(
                            new config::NearCacheConfig<int, int>());
                }

                std::unique_ptr<ClientConfig> newClientConfig() {
                    return std::unique_ptr<ClientConfig>(new ClientConfig());
                }

                std::shared_ptr<ReplicatedMap<int, int> > getNearCachedMapFromClient(
                        std::shared_ptr<config::NearCacheConfig<int, int> > nearCacheConfig) {
                    std::string mapName = DEFAULT_NEAR_CACHE_NAME;

                    nearCacheConfig->setName(mapName);

                    clientConfig = newClientConfig();
                    clientConfig->addNearCacheConfig(nearCacheConfig);

                    client = std::unique_ptr<HazelcastClient>(new HazelcastClient(*clientConfig));
                    map = client->getReplicatedMap<int, int>(mapName);
                    return map;
                }

                monitor::NearCacheStats *getNearCacheStats(ReplicatedMap<int, int> &map) {
                    return  (static_cast<proxy::ClientReplicatedMapProxy<int, int> &>(map)).getNearCacheStats();
                }

                void assertThatOwnedEntryCountEquals(ReplicatedMap<int, int> &clientMap, int64_t expected) {
                    ASSERT_EQ(expected, getNearCacheStats(clientMap)->getOwnedEntryCount());
                }

                std::unique_ptr<ClientConfig> clientConfig;
                std::shared_ptr<config::NearCacheConfig<int, int> > nearCacheConfig;
                std::unique_ptr<HazelcastClient> client;
                std::shared_ptr<ReplicatedMap<int, int> > map;
                static HazelcastServer *instance;
                static HazelcastServer *instance2;
            };

            const std::string ClientReplicatedMapNearCacheTest::DEFAULT_NEAR_CACHE_NAME = "defaultNearCache";
            HazelcastServer *ClientReplicatedMapNearCacheTest::instance = NULL;
            HazelcastServer *ClientReplicatedMapNearCacheTest::instance2 = NULL;

            TEST_F(ClientReplicatedMapNearCacheTest, testGetAllChecksNearCacheFirst) {
                std::shared_ptr<ReplicatedMap<int, int> > map = getNearCachedMapFromClient(
                        newNoInvalidationNearCacheConfig());

                int size = 1003;
                for (int i = 0; i < size; i++) {
                    map->put(i, i);
                }
                // populate Near Cache
                for (int i = 0; i < size; i++) {
                    map->get(i);
                }
                // getAll() generates the Near Cache hits
                for (int i = 0; i < size; i++) {
                    map->get(i);
                }

                monitor::NearCacheStats *stats = getNearCacheStats(*map);
                ASSERT_EQ(size, stats->getOwnedEntryCount());
                ASSERT_EQ(size, stats->getHits());
            }

            TEST_F(ClientReplicatedMapNearCacheTest, testGetAllPopulatesNearCache) {
                std::shared_ptr<ReplicatedMap<int, int> > map = getNearCachedMapFromClient(
                        newNoInvalidationNearCacheConfig());

                int size = 1214;
                for (int i = 0; i < size; i++) {
                    map->put(i, i);
                }
// populate Near Cache
                for (int i = 0; i < size; i++) {
                    map->get(i);
                }
// getAll() generates the Near Cache hits
                for (int i = 0; i < size; i++) {
                    map->get(i);
                }

                assertThatOwnedEntryCountEquals(*map, size);
            }

            TEST_F(ClientReplicatedMapNearCacheTest, testRemoveAllNearCache) {
                std::shared_ptr<ReplicatedMap<int, int> > map = getNearCachedMapFromClient(newNearCacheConfig());


                int size = 1214;
                for (int i = 0; i < size; i++) {
                    map->put(i, i);
                }
// populate Near Cache
                for (int i = 0; i < size; i++) {
                    map->get(i);
                }

                for (int i = 0; i < size; i++) {
                    map->remove(i);
                }

                assertThatOwnedEntryCountEquals(*map, 0);
            }
        }
    }
}




namespace hazelcast {
    namespace client {

        namespace test {
            class ClientTopicTest : public ClientTestSupport {
            public:
                ClientTopicTest();

            protected:
                HazelcastServer instance;
                ClientConfig clientConfig;
                HazelcastClient client;
                ITopic<std::string> topic;
            };

            ClientTopicTest::ClientTopicTest() : instance(*g_srvFactory), client(getNewClient()),
                                                 topic(client.getTopic<std::string>("ClientTopicTest")) {
            }

            class MyMessageListener : public topic::MessageListener<std::string> {
            public:
                MyMessageListener(hazelcast::util::CountDownLatch &latch)
                        : latch(latch) {
                }

                void onMessage(std::unique_ptr<topic::Message<std::string> > &&message) {
                    latch.countDown();
                }

            private:
                hazelcast::util::CountDownLatch &latch;
            };

            TEST_F(ClientTopicTest, testTopicListeners) {
                hazelcast::util::CountDownLatch latch(10);
                MyMessageListener listener(latch);
                std::string id = topic.addMessageListener(listener);

                for (int i = 0; i < 10; i++) {
                    topic.publish(std::string("naber") + hazelcast::util::IOUtil::to_string(i));
                }
                ASSERT_TRUE(latch.await(20));
                topic.removeMessageListener(id);
            }
        }
    }
}





namespace hazelcast {
    namespace client {

        class HazelcastClient;

        namespace test {
            class MixedTopicTest : public ClientTestSupport {
            public:
                MixedTopicTest();

            protected:
                HazelcastServer instance;
                ClientConfig clientConfig;
                HazelcastClient client;
                mixedtype::ITopic topic;
            };

            class MixedMessageListener : public hazelcast::client::mixedtype::topic::MessageListener {
            public:
                MixedMessageListener(hazelcast::util::CountDownLatch &latch)
                        : latch(latch) {
                }

                void onMessage(std::unique_ptr<client::topic::Message<TypedData> > &&message) {
                    latch.countDown();
                }

            private:
                hazelcast::util::CountDownLatch &latch;
            };


            MixedTopicTest::MixedTopicTest()
                    : instance(*g_srvFactory), client(getNewClient()),
                      topic(client.toMixedType().getTopic("MixedTopicTest")) {
            }

            TEST_F(MixedTopicTest, testTopicListeners) {
                hazelcast::util::CountDownLatch latch(10);
                MixedMessageListener listener(latch);
                std::string id = topic.addMessageListener(listener);

                for (int i = 0; i < 10; i++) {
                    topic.publish<std::string>(std::string("naber") + hazelcast::util::IOUtil::to_string(i));
                }
                ASSERT_TRUE(latch.await(20));
                topic.removeMessageListener(id);
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


