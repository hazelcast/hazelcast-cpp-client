/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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

using namespace hazelcast::client::mixedtype;

namespace hazelcast {
    namespace client {
        namespace test {
            namespace mixedmap {
                class MapClientConfig : public ClientConfig {
                public:
                    MapClientConfig() {
                    }

                    virtual ~MapClientConfig() {
                    }
                };

                class NearCachedDataMapClientConfig : public MapClientConfig {
                public:
                    NearCachedDataMapClientConfig() {
                        std::shared_ptr<mixedtype::config::MixedNearCacheConfig> nearCacheConfig(
                                new mixedtype::config::MixedNearCacheConfig("MixedMapTestMap"));
                        addMixedNearCacheConfig(nearCacheConfig);

                        addNearCacheConfig<std::string, std::string>(std::shared_ptr<config::NearCacheConfig<std::string, std::string> >(new config::NearCacheConfig<std::string, std::string>("OneSecondTtlMap")));
                    }

                };

                class MixedMapAPITest : public ClientTestSupport, public ::testing::WithParamInterface<MapClientConfig *> {
                public:
                    MixedMapAPITest() : clientConfig(GetParam()) {
#ifdef HZ_BUILD_WITH_SSL
                        config::SSLConfig sslConfig;
                        sslConfig.setEnabled(true).setProtocol(config::tlsv12).addVerifyFile(getCAFilePath());
                        clientConfig->getNetworkConfig().setSSLConfig(sslConfig);
#endif // HZ_BUILD_WITH_SSL

                        client.reset(new HazelcastClient(*clientConfig));

                        imap = new mixedtype::IMap(client->toMixedType().getMap("MixedMapTestMap"));
                    }

                    static void SetUpTestSuite() {
#ifdef HZ_BUILD_WITH_SSL
                        sslFactory = new HazelcastServerFactory(g_srvFactory->getServerAddress(), getSslFilePath());
                        instance = new HazelcastServer(*sslFactory);
                        instance2 = new HazelcastServer(*sslFactory);
#else
                        instance = new HazelcastServer(*g_srvFactory);
                        instance2 = new HazelcastServer(*g_srvFactory);
#endif
                    }

                    static void TearDownTestSuite() {
                        delete instance2;
                        delete instance;
                        delete sslFactory;

                        instance2 = NULL;
                        instance = NULL;
                    }

                protected:
                    virtual void TearDown() {
                        imap->destroy();
                        client->toMixedType().getMap("OneSecondTtlMap").destroy();
                    }

                    class MapGetInterceptor : public serialization::IdentifiedDataSerializable {
                    public:
                        MapGetInterceptor(const std::string &prefix) : prefix(
                                std::unique_ptr<std::string>(new std::string(prefix))) {}

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
                        std::unique_ptr<std::string> prefix;
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

                    void fillMap() {
                        for (int i = 0; i < 10; i++) {
                            std::string key = "key";
                            key += hazelcast::util::IOUtil::to_string(i);
                            std::string value = "value";
                            value += hazelcast::util::IOUtil::to_string(i);
                            imap->put<std::string, std::string>(key, value);
                        }
                    }

                    static HazelcastServer *instance;
                    static HazelcastServer *instance2;
                    static HazelcastServerFactory *sslFactory;
                    MapClientConfig *clientConfig;
                    std::unique_ptr<HazelcastClient> client;
                    mixedtype::IMap *imap;
                };

                INSTANTIATE_TEST_SUITE_P(MixedMapAPITestInstance,
                                         MixedMapAPITest,
                                         ::testing::Values(new MapClientConfig(), new NearCachedDataMapClientConfig()));

                HazelcastServer *MixedMapAPITest::instance = NULL;
                HazelcastServer *MixedMapAPITest::instance2 = NULL;
                HazelcastServerFactory *MixedMapAPITest::sslFactory = NULL;

                void tryPutThread(hazelcast::util::ThreadArgs &args) {
                    hazelcast::util::CountDownLatch *latch = (hazelcast::util::CountDownLatch *) args.arg0;
                    mixedtype::IMap *imap = (mixedtype::IMap *) args.arg1;
                    bool result = imap->tryPut<std::string, std::string>("key1", "value3", 1 * 1000);
                    if (!result) {
                        latch->countDown();
                    }
                }

                void tryRemoveThread(hazelcast::util::ThreadArgs &args) {
                    hazelcast::util::CountDownLatch *latch = (hazelcast::util::CountDownLatch *) args.arg0;
                    mixedtype::IMap *imap = (mixedtype::IMap *) args.arg1;
                    bool result = imap->tryRemove<std::string>("key2", 1 * 1000);
                    if (!result) {
                        latch->countDown();
                    }
                }

                void testLockThread(hazelcast::util::ThreadArgs &args) {
                    hazelcast::util::CountDownLatch *latch = (hazelcast::util::CountDownLatch *) args.arg0;
                    mixedtype::IMap *imap = (mixedtype::IMap *) args.arg1;
                    imap->tryPut<std::string, std::string>("key1", "value2", 1);
                    latch->countDown();
                }

                void testLockTTLThread(hazelcast::util::ThreadArgs &args) {
                    hazelcast::util::CountDownLatch *latch = (hazelcast::util::CountDownLatch *) args.arg0;
                    mixedtype::IMap *imap = (mixedtype::IMap *) args.arg1;
                    imap->tryPut<std::string, std::string>("key1", "value2", 5 * 1000);
                    latch->countDown();
                }

                void testLockTTL2Thread(hazelcast::util::ThreadArgs &args) {
                    hazelcast::util::CountDownLatch *latch = (hazelcast::util::CountDownLatch *) args.arg0;
                    mixedtype::IMap *imap = (mixedtype::IMap *) args.arg1;
                    if (!imap->tryLock<std::string>("key1")) {
                        latch->countDown();
                    }
                    if (imap->tryLock<std::string>("key1", 5 * 1000)) {
                        latch->countDown();
                    }
                }

                void testMapTryLockThread1(hazelcast::util::ThreadArgs &args) {
                    hazelcast::util::CountDownLatch *latch = (hazelcast::util::CountDownLatch *) args.arg0;
                    mixedtype::IMap *imap = (mixedtype::IMap *) args.arg1;
                    if (!imap->tryLock<std::string>("key1", 2)) {
                        latch->countDown();
                    }
                }

                void testMapTryLockThread2(hazelcast::util::ThreadArgs &args) {
                    hazelcast::util::CountDownLatch *latch = (hazelcast::util::CountDownLatch *) args.arg0;
                    mixedtype::IMap *imap = (mixedtype::IMap *) args.arg1;
                    if (imap->tryLock<std::string>("key1", 20 * 1000)) {
                        latch->countDown();
                    }
                }

                void testMapForceUnlockThread(hazelcast::util::ThreadArgs &args) {
                    hazelcast::util::CountDownLatch *latch = (hazelcast::util::CountDownLatch *) args.arg0;
                    mixedtype::IMap *imap = (mixedtype::IMap *) args.arg1;
                    imap->forceUnlock<std::string>("key1");
                    latch->countDown();
                }

                class CountdownListener : public MixedEntryListener {
                public:
                    CountdownListener(hazelcast::util::CountDownLatch &addLatch, hazelcast::util::CountDownLatch &removeLatch,
                                      hazelcast::util::CountDownLatch &updateLatch, hazelcast::util::CountDownLatch &evictLatch)
                            : addLatch(addLatch), removeLatch(removeLatch), updateLatch(updateLatch),
                              evictLatch(evictLatch) {
                    }

                    virtual void entryAdded(const MixedEntryEvent &event) {
                        addLatch.countDown();
                    }

                    virtual void entryRemoved(const MixedEntryEvent &event) {
                        removeLatch.countDown();
                    }

                    virtual void entryUpdated(const MixedEntryEvent &event) {
                        updateLatch.countDown();
                    }

                    virtual void entryEvicted(const MixedEntryEvent &event) {
                        evictLatch.countDown();
                    }

                    virtual void entryExpired(const MixedEntryEvent &event) {
                    }

                    virtual void entryMerged(const MixedEntryEvent &event) {

                    }

                    virtual void mapEvicted(const MapEvent &event) {
                    }

                    virtual void mapCleared(const MapEvent &event) {
                    }

                private:
                    hazelcast::util::CountDownLatch &addLatch;
                    hazelcast::util::CountDownLatch &removeLatch;
                    hazelcast::util::CountDownLatch &updateLatch;
                    hazelcast::util::CountDownLatch &evictLatch;
                };

                class MyListener : public MixedEntryListener {
                public:
                    MyListener(hazelcast::util::CountDownLatch &latch, hazelcast::util::CountDownLatch &nullLatch)
                            : latch(latch), nullLatch(nullLatch) {
                    }

                    virtual void entryAdded(const MixedEntryEvent &event) {
                        latch.countDown();
                    }

                    virtual void entryRemoved(const MixedEntryEvent &event) {
                    }

                    virtual void entryUpdated(const MixedEntryEvent &event) {
                    }

                    virtual void entryEvicted(const MixedEntryEvent &event) {
                        std::unique_ptr<std::string> oldValue = event.getOldValue()->get<std::string>();
                        if (oldValue->compare("")) {
                            nullLatch.countDown();
                        }
                        latch.countDown();
                    }

                    virtual void entryExpired(const MixedEntryEvent &event) {

                    }

                    virtual void entryMerged(const MixedEntryEvent &event) {

                    }

                    virtual void mapEvicted(const MapEvent &event) {
                    }

                    virtual void mapCleared(const MapEvent &event) {

                    }

                private:
                    hazelcast::util::CountDownLatch &latch;
                    hazelcast::util::CountDownLatch &nullLatch;
                };

                class ClearListener : public MixedEntryListener {
                public:
                    ClearListener(hazelcast::util::CountDownLatch &latch) : latch(latch) {
                    }

                    virtual void entryAdded(const MixedEntryEvent &event) {
                    }

                    virtual void entryRemoved(const MixedEntryEvent &event) {
                    }

                    virtual void entryUpdated(const MixedEntryEvent &event) {
                    }

                    virtual void entryEvicted(const MixedEntryEvent &event) {
                    }

                    virtual void entryExpired(const MixedEntryEvent &event) {
                    }

                    virtual void entryMerged(const MixedEntryEvent &event) {
                    }

                    virtual void mapEvicted(const MapEvent &event) {
                    }

                    void mapCleared(const MapEvent &event) {
                        latch.countDown();
                    }

                private:
                    hazelcast::util::CountDownLatch &latch;
                };

                class EvictListener : public MixedEntryListener {
                public:
                    EvictListener(hazelcast::util::CountDownLatch &latch) : latch(latch) {
                    }

                    virtual void entryAdded(const MixedEntryEvent &event) {
                    }

                    virtual void entryRemoved(const MixedEntryEvent &event) {
                    }

                    virtual void entryUpdated(const MixedEntryEvent &event) {
                    }

                    virtual void entryEvicted(const MixedEntryEvent &event) {
                    }

                    virtual void entryExpired(const MixedEntryEvent &event) {
                    }

                    virtual void entryMerged(const MixedEntryEvent &event) {
                    }

                    virtual void mapCleared(const MapEvent &event) {
                    }

                    virtual void mapEvicted(const MapEvent &event) {
                        latch.countDown();
                    }

                private:
                    hazelcast::util::CountDownLatch &latch;
                };

                class SampleEntryListenerForPortableKey : public MixedEntryListener {
                public:
                    SampleEntryListenerForPortableKey(hazelcast::util::CountDownLatch &latch, hazelcast::util::AtomicInt &atomicInteger)
                            : latch(latch), atomicInteger(atomicInteger) {

                    }

                    virtual void entryAdded(const MixedEntryEvent &event) {
                        ++atomicInteger;
                        latch.countDown();
                    }

                    virtual void entryRemoved(const MixedEntryEvent &event) {
                    }

                    virtual void entryUpdated(const MixedEntryEvent &event) {
                    }

                    virtual void entryEvicted(const MixedEntryEvent &event) {
                    }

                    virtual void entryExpired(const MixedEntryEvent &event) {
                    }

                    virtual void entryMerged(const MixedEntryEvent &event) {
                    }

                    virtual void mapEvicted(const MapEvent &event) {
                    }

                    virtual void mapCleared(const MapEvent &event) {
                    }

                private:
                    hazelcast::util::CountDownLatch &latch;
                    hazelcast::util::AtomicInt &atomicInteger;
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

                TEST_P(MixedMapAPITest, testIssue537) {
                    hazelcast::util::CountDownLatch latch(2);
                    hazelcast::util::CountDownLatch nullLatch(1);
                    MyListener myListener(latch, nullLatch);
                    std::string id = imap->addEntryListener(myListener, true);

                    imap->put<std::string, std::string>("key1", "value1", 2 * 1000);

                    ASSERT_TRUE(latch.await(10));
                    ASSERT_TRUE(nullLatch.await(1));

                    ASSERT_TRUE(imap->removeEntryListener(id));

                    imap->put<std::string, std::string>("key2", "value2");
                    ASSERT_EQ(1, imap->size());
                }

                TEST_P(MixedMapAPITest, testContains) {
                    fillMap();

                    ASSERT_FALSE(imap->containsKey<std::string>("key10"));
                    ASSERT_TRUE(imap->containsKey<std::string>("key1"));

                    ASSERT_FALSE(imap->containsValue<std::string>("value10"));
                    ASSERT_TRUE(imap->containsValue<std::string>("value1"));

                }

                TEST_P(MixedMapAPITest, testGet) {
                    fillMap();
                    for (int i = 0; i < 10; i++) {
                        std::string key = "key";
                        key += hazelcast::util::IOUtil::to_string(i);
                        std::unique_ptr<std::string> temp = imap->get<std::string>(key).get<std::string>();

                        std::string value = "value";
                        value += hazelcast::util::IOUtil::to_string(i);
                        ASSERT_EQ(*temp, value);
                    }
                }

                TEST_P(MixedMapAPITest, testRemoveAndDelete) {
                    fillMap();
                    std::unique_ptr<std::string> temp = imap->remove<std::string>("key10").get<std::string>();
                    ASSERT_EQ(temp.get(), (std::string *) NULL);
                    imap->deleteEntry<std::string>("key9");
                    ASSERT_EQ(imap->size(), 9);
                    for (int i = 0; i < 9; i++) {
                        std::string key = "key";
                        key += hazelcast::util::IOUtil::to_string(i);
                        std::unique_ptr<std::string> temp2 = imap->remove<std::string>(key).get<std::string>();
                        std::string value = "value";
                        value += hazelcast::util::IOUtil::to_string(i);
                        ASSERT_EQ(*temp2, value);
                    }
                    ASSERT_EQ(imap->size(), 0);
                }

                TEST_P(MixedMapAPITest, testRemoveIfSame) {
                    fillMap();

                    ASSERT_FALSE((imap->remove<std::string, std::string>("key2", "value")));
                    ASSERT_EQ(10, imap->size());

                    ASSERT_TRUE((imap->remove<std::string, std::string>("key2", "value2")));
                    ASSERT_EQ(9, imap->size());

                }

                TEST_P(MixedMapAPITest, testRemoveAll) {
                    fillMap();

                    imap->removeAll(
                            query::EqualPredicate<std::string>(query::QueryConstants::getKeyAttributeName(), "key5"));

                    std::unique_ptr<std::string> value = imap->get<std::string>("key5").get<std::string>();

                    ASSERT_NULL("key5 should not exist", value.get(), std::string);

                    query::LikePredicate likeAllValues(query::QueryConstants::getValueAttributeName(), "value%");

                    imap->removeAll(likeAllValues);

                    ASSERT_TRUE(imap->isEmpty());
                }


/**
                 * This failure with this test is reported at https://github.com/hazelcast/hazelcast-cpp-client/issues/379
                 * Do not forget to enable this test once the issue is resolved!!!
                 */
                TEST_P(MixedMapAPITest, DISABLED_testGetAllPutAll) {
                    std::map<std::string, std::string> mapTemp;

                    for (int i = 0; i < 100; i++) {
                        mapTemp[hazelcast::util::IOUtil::to_string(i)] = hazelcast::util::IOUtil::to_string(i);
                    }
                    imap->putAll<std::string, std::string>(mapTemp);
                    ASSERT_EQ(imap->size(), 100);

                    for (int i = 0; i < 100; i++) {
                        std::string expected = hazelcast::util::IOUtil::to_string(i);
                        std::unique_ptr<std::string> actual = imap->get<std::string>(
                                hazelcast::util::IOUtil::to_string(i)).get<std::string>();
                        ASSERT_EQ(expected, *actual);
                    }

                    std::set<std::string> tempSet;
                    tempSet.insert(hazelcast::util::IOUtil::to_string(1));
                    tempSet.insert(hazelcast::util::IOUtil::to_string(3));

                    std::vector<std::pair<TypedData, TypedData> > m2 = imap->getAll<std::string>(tempSet);

                    ASSERT_EQ(2U, m2.size());
                    std::unique_ptr<std::string> key1 = m2[0].first.get<std::string>();
                    ASSERT_NE((std::string *) NULL, key1.get());
                    std::unique_ptr<std::string> value1 = m2[0].second.get<std::string>();
                    ASSERT_NE((std::string *) NULL, value1.get());
                    ASSERT_EQ(*key1, *value1);
                    ASSERT_TRUE(*key1 == "1" || *key1 == "3");

                    const std::pair<TypedData, TypedData> &entry = m2[1];
                    ASSERT_NE((std::string *) NULL, entry.first.get<std::string>().get());
                    ASSERT_NE((std::string *) NULL, entry.second.get<std::string>().get());
                    ASSERT_EQ(*entry.first.get<std::string>(), *entry.second.get<std::string>());
                    ASSERT_TRUE(*entry.first.get<std::string>() == "1" || *entry.first.get<std::string>() == "3");
                    ASSERT_NE(*key1, *entry.first.get<std::string>());
                }

                TEST_P(MixedMapAPITest, testTryPutRemove) {
                    ASSERT_TRUE((imap->tryPut<std::string, std::string>("key1", "value1", 1 * 1000)));
                    ASSERT_TRUE((imap->tryPut<std::string, std::string>("key2", "value2", 1 * 1000)));
                    imap->lock<std::string>("key1");
                    imap->lock<std::string>("key2");

                    hazelcast::util::CountDownLatch latch(2);

                    hazelcast::util::StartedThread t1(tryPutThread, &latch, imap);
                    hazelcast::util::StartedThread t2(tryRemoveThread, &latch, imap);

                    ASSERT_TRUE(latch.await(20));
                    ASSERT_EQ("value1", *(imap->get<std::string>("key1").get<std::string>()));
                    ASSERT_EQ("value2", *(imap->get<std::string>("key2").get<std::string>()));
                    imap->forceUnlock<std::string>("key1");
                    imap->forceUnlock<std::string>("key2");
                }

                TEST_P(MixedMapAPITest, testGetEntryViewForNonExistentData) {
                    std::unique_ptr<EntryView<TypedData, TypedData> > view = imap->getEntryView<std::string>(
                            "non-existent");

                    ASSERT_EQ((EntryView<TypedData, TypedData> *) NULL, view.get());

// put an entry that will expire in 1 milliseconds
                    imap->put<std::string, std::string>("short_entry", "short living value", 1);

                    ASSERT_EQ_EVENTUALLY((EntryView<TypedData, TypedData> *) NULL,
                                         (imap->getEntryView<std::string>("short_entry").get()));
                }

                TEST_P(MixedMapAPITest, testPutTtl) {
                    mixedtype::IMap &map = *imap;

                    hazelcast::util::CountDownLatch dummy(10);
                    hazelcast::util::CountDownLatch evict(1);
                    CountdownListener sampleEntryListener(dummy, dummy, dummy, evict);
                    std::string id = map.addEntryListener(sampleEntryListener, false);

                    monitor::impl::NearCacheStatsImpl *nearCacheStatsImpl = (monitor::impl::NearCacheStatsImpl *) map.getLocalMapStats().getNearCacheStats();

                    int64_t initialInvalidationRequests = 0;
                    if (nearCacheStatsImpl) {
                        initialInvalidationRequests = nearCacheStatsImpl->getInvalidationRequests();
                    }

// put will cause an invalidation event sent from the server to the client
                    map.put<std::string, std::string>("key1", "value1", 1000);

// if near cache is enabled
                    if (nearCacheStatsImpl) {
                        ASSERT_EQ_EVENTUALLY(initialInvalidationRequests + 1,
                                             nearCacheStatsImpl->getInvalidationRequests());

// populate near cache
                        map.get<std::string>("key1");

// When ttl expires at server, the server does not send near cache invalidation.
                        ASSERT_TRUE_ALL_THE_TIME((map.get<std::string>("key1").get<std::string>().get() &&
                                                  nearCacheStatsImpl->getInvalidationRequests() ==
                                                  initialInvalidationRequests + 1), 2);
                    } else {
// trigger eviction
                        ASSERT_NULL_EVENTUALLY(map.get<std::string>("key1").get<std::string>().get(), std::string);
                    }

                    ASSERT_TRUE(evict.await(5));

                    ASSERT_TRUE(imap->removeEntryListener(id));
                }

                TEST_P(MixedMapAPITest, testPutConfigTtl) {
                    mixedtype::IMap map = client->toMixedType().getMap("OneSecondTtlMap");
                    hazelcast::util::CountDownLatch dummy(10);
                    hazelcast::util::CountDownLatch evict(1);
                    CountdownListener sampleEntryListener(dummy, dummy, dummy, evict);
                    std::string id = map.addEntryListener(sampleEntryListener, false);

                    monitor::impl::NearCacheStatsImpl *nearCacheStatsImpl = (monitor::impl::NearCacheStatsImpl *) map.getLocalMapStats().getNearCacheStats();

                    int64_t initialInvalidationRequests = 0;
                    if (nearCacheStatsImpl) {
                        initialInvalidationRequests = nearCacheStatsImpl->getInvalidationRequests();
                    }

// put will cause an invalidation event sent from the server to the client
                    map.put<std::string, std::string>("key1", "value1");

// if near cache is enabled
                    if (nearCacheStatsImpl) {
                        ASSERT_EQ_EVENTUALLY(initialInvalidationRequests + 1,
                                             nearCacheStatsImpl->getInvalidationRequests());

// populate near cache
                        map.get<std::string>("key1");

// When ttl expires at server, the server does not send near cache invalidation.
                        ASSERT_TRUE_ALL_THE_TIME((map.get<std::string>("key1").get<std::string>().get() &&
                                                  nearCacheStatsImpl->getInvalidationRequests() ==
                                                  initialInvalidationRequests + 1), 2);
                    } else {
// trigger eviction
                        ASSERT_NULL_EVENTUALLY(map.get<std::string>("key1").get<std::string>().get(), std::string);
                    }

                    ASSERT_TRUE(evict.await(5));

                    ASSERT_TRUE(map.removeEntryListener(id));
                }

                TEST_P(MixedMapAPITest, testPutIfAbsent) {
                    std::unique_ptr<std::string> o = imap->putIfAbsent<std::string, std::string>("key1",
                                                                                                 "value1").get<std::string>();
                    ASSERT_EQ(o.get(), (std::string *) NULL);
                    ASSERT_EQ("value1",
                              *(imap->putIfAbsent<std::string, std::string>("key1", "value3").get<std::string>()));
                }

                TEST_P(MixedMapAPITest, testPutIfAbsentTtl) {
                    ASSERT_EQ((std::string *) NULL, (imap->putIfAbsent<std::string, std::string>("key1", "value1",
                                                                                                 1000).get<std::string>().get()));
                    ASSERT_EQ("value1", *(imap->putIfAbsent<std::string, std::string>("key1", "value3",
                                                                                      1000).get<std::string>()));

                    ASSERT_NULL_EVENTUALLY((imap->putIfAbsent<std::string, std::string>("key1", "value3",
                                                                                        1000).get<std::string>().get()),
                                           std::string);
                    ASSERT_EQ("value3", *(imap->putIfAbsent<std::string, std::string>("key1", "value4",
                                                                                      1000).get<std::string>()));
                }

                TEST_P(MixedMapAPITest, testSet) {
                    imap->IMap::set<std::string, std::string>("key1", "value1");
                    ASSERT_EQ("value1", *(imap->get<std::string>("key1").get<std::string>()));

                    imap->IMap::set<std::string, std::string>("key1", "value2");
                    ASSERT_EQ("value2", *(imap->get<std::string>("key1").get<std::string>()));
                }

                TEST_P(MixedMapAPITest, testSetTtl) {
                    mixedtype::IMap &map = *imap;

                    hazelcast::util::CountDownLatch dummy(10);
                    hazelcast::util::CountDownLatch evict(1);
                    CountdownListener sampleEntryListener(dummy, dummy, dummy, evict);
                    std::string id = map.addEntryListener(sampleEntryListener, false);

                    monitor::impl::NearCacheStatsImpl *nearCacheStatsImpl = (monitor::impl::NearCacheStatsImpl *) map.getLocalMapStats().getNearCacheStats();

                    int64_t initialInvalidationRequests = 0;
                    if (nearCacheStatsImpl) {
                        initialInvalidationRequests = nearCacheStatsImpl->getInvalidationRequests();
                    }

// set will cause an invalidation event sent from the server to the client
                    map.IMap::set<std::string, std::string>("key1", "value1", 1000);

// if near cache is enabled
                    if (nearCacheStatsImpl) {
                        ASSERT_EQ_EVENTUALLY(initialInvalidationRequests + 1,
                                             nearCacheStatsImpl->getInvalidationRequests());

// populate near cache
                        map.get<std::string>("key1");

// When ttl expires at server, the server does not send near cache invalidation.
                        ASSERT_TRUE_ALL_THE_TIME((map.get<std::string>("key1").get<std::string>().get() &&
                                                  nearCacheStatsImpl->getInvalidationRequests() ==
                                                  initialInvalidationRequests + 1), 2);
                    } else {
// trigger eviction
                        ASSERT_NULL_EVENTUALLY(map.get<std::string>("key1").get<std::string>().get(), std::string);
                    }

                    ASSERT_TRUE(evict.await(5));

                    ASSERT_TRUE(map.removeEntryListener(id));
                }

                TEST_P(MixedMapAPITest, testSetConfigTtl) {
                    mixedtype::IMap map = client->toMixedType().getMap("OneSecondTtlMap");
                    hazelcast::util::CountDownLatch dummy(10);
                    hazelcast::util::CountDownLatch evict(1);
                    CountdownListener sampleEntryListener(dummy, dummy, dummy, evict);
                    std::string id = map.addEntryListener(sampleEntryListener, false);

                    monitor::impl::NearCacheStatsImpl *nearCacheStatsImpl = (monitor::impl::NearCacheStatsImpl *) map.getLocalMapStats().getNearCacheStats();

                    int64_t initialInvalidationRequests = 0;
                    if (nearCacheStatsImpl) {
                        initialInvalidationRequests = nearCacheStatsImpl->getInvalidationRequests();
                    }

// set will cause an invalidation event sent from the server to the client
                    map.IMap::set<std::string, std::string>("key1", "value1");

// if near cache is enabled
                    if (nearCacheStatsImpl) {
                        ASSERT_EQ_EVENTUALLY(initialInvalidationRequests + 1,
                                             nearCacheStatsImpl->getInvalidationRequests());

// populate near cache
                        map.get<std::string>("key1");

// When ttl expires at server, the server does not send near cache invalidation.
                        ASSERT_TRUE_ALL_THE_TIME((map.get<std::string>("key1").get<std::string>().get() &&
                                                  nearCacheStatsImpl->getInvalidationRequests() ==
                                                  initialInvalidationRequests + 1), 2);
                    } else {
// trigger eviction
                        ASSERT_NULL_EVENTUALLY(map.get<std::string>("key1").get<std::string>().get(), std::string);
                    }

                    ASSERT_TRUE(evict.await(5));

                    ASSERT_TRUE(map.removeEntryListener(id));
                }

                TEST_P(MixedMapAPITest, testLock) {
                    imap->put<std::string, std::string>("key1", "value1");
                    ASSERT_EQ("value1", *(imap->get<std::string>("key1").get<std::string>()));
                    imap->lock<std::string>("key1");
                    hazelcast::util::CountDownLatch latch(1);
                    hazelcast::util::StartedThread t1(testLockThread, &latch, imap);
                    ASSERT_TRUE(latch.await(5));
                    ASSERT_EQ("value1", *(imap->get<std::string>("key1").get<std::string>()));
                    imap->forceUnlock<std::string>("key1");

                }

                TEST_P(MixedMapAPITest, testLockTtl) {
                    imap->put<std::string, std::string>("key1", "value1");
                    ASSERT_EQ("value1", *(imap->get<std::string>("key1").get<std::string>()));
                    imap->lock<std::string>("key1", 2 * 1000);
                    hazelcast::util::CountDownLatch latch(1);
                    hazelcast::util::StartedThread t1(testLockTTLThread, &latch, imap);
                    ASSERT_TRUE(latch.await(10));
                    ASSERT_FALSE(imap->isLocked<std::string>("key1"));
                    ASSERT_EQ("value2", *(imap->get<std::string>("key1").get<std::string>()));
                    imap->forceUnlock<std::string>("key1");

                }

                TEST_P(MixedMapAPITest, testLockTtl2) {
                    imap->lock<std::string>("key1", 3 * 1000);
                    hazelcast::util::CountDownLatch latch(2);
                    hazelcast::util::StartedThread t1(testLockTTL2Thread, &latch, imap);
                    ASSERT_TRUE(latch.await(10));
                    imap->forceUnlock<std::string>("key1");

                }

                TEST_P(MixedMapAPITest, testTryLock) {

                    ASSERT_TRUE(imap->tryLock<std::string>("key1", 2 * 1000));
                    hazelcast::util::CountDownLatch latch(1);
                    hazelcast::util::StartedThread t1(testMapTryLockThread1, &latch, imap);

                    ASSERT_TRUE(latch.await(100));

                    ASSERT_TRUE(imap->isLocked<std::string>("key1"));

                    hazelcast::util::CountDownLatch latch2(1);
                    hazelcast::util::StartedThread t2(testMapTryLockThread2, &latch2, imap);

                    hazelcast::util::sleep(1);
                    imap->unlock<std::string>("key1");
                    ASSERT_TRUE(latch2.await(100));
                    ASSERT_TRUE(imap->isLocked<std::string>("key1"));
                    imap->forceUnlock<std::string>("key1");

                }

                TEST_P(MixedMapAPITest, testForceUnlock) {
                    imap->lock<std::string>("key1");
                    hazelcast::util::CountDownLatch latch(1);
                    hazelcast::util::StartedThread t2(testMapForceUnlockThread, &latch, imap);
                    ASSERT_TRUE(latch.await(100));
                    t2.join();
                    ASSERT_FALSE(imap->isLocked<std::string>("key1"));

                }

                TEST_P(MixedMapAPITest, testValues) {

                    fillMap();
                    query::SqlPredicate predicate("this == value1");
                    std::vector<TypedData> tempVector = imap->values(predicate);
                    ASSERT_EQ(1U, tempVector.size());

                    ASSERT_EQ("value1", *tempVector[0].get<std::string>());
                }

/*
                TEST_P(MixedMapAPITest, testValuesWithPredicate) {
                    const int numItems = 20;
                    for (int i = 0; i < numItems; ++i) {
                        imap->put<int, int>(i, 2 * i);
                    }

                    std::vector<TypedData> values = imap->values();
                    ASSERT_EQ(numItems, (int) values.size());
                    std::vector<int> actualValues;
                    for (int i = 0; i < (int) values.size(); ++i) {
                        int *value = values[i].get<int>().get();
                        ASSERT_NE((const int *) NULL, value);
                        actualValues.push_back(*value);
                    }
                    std::sort(actualValues.begin(), actualValues.end());
                    for (int i = 0; i < numItems; ++i) {
                        ASSERT_EQ(2 * i, actualValues[i]);
                    }

                    // EqualPredicate
                    // key == 5
                    values = imap->values(
                            query::EqualPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5));
                    ASSERT_EQ(1, (int) values.size());
                    ASSERT_NE((const int *) NULL, values[0].get<int>().get());
                    ASSERT_EQ(2 * 5, *values[0].get<int>());

                    // value == 8
                    values = imap->values(
                            query::EqualPredicate<int>(query::QueryConstants::getValueAttributeName(), 8));
                    ASSERT_EQ(1, (int) values.size());
                    ASSERT_NE((const int *) NULL, values[0].get<int>().get());
                    ASSERT_EQ(8, *values[0].get<int>());

                    // key == numItems
                    values = imap->values(
                            query::EqualPredicate<int>(query::QueryConstants::getKeyAttributeName(), numItems));
                    ASSERT_EQ(0, (int) values.size());

                    // NotEqual Predicate
                    // key != 5
                    values = imap->values(
                            query::NotEqualPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5));
                    ASSERT_EQ(numItems - 1, (int) values.size());
                    actualValues.clear();
                    for (int i = 0; i < (int) values.size(); ++i) {
                        std::unique_ptr<int> value = values[i].get<int>();
                        ASSERT_NE((const int *) NULL, value.get());
                        actualValues.push_back(*value);
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
                    values = imap->values(
                            query::NotEqualPredicate<int>(query::QueryConstants::getValueAttributeName(), 8));
                    ASSERT_EQ(numItems - 1, (int) values.size());
                    actualValues.clear();
                    for (int i = 0; i < (int) values.size(); ++i) {
                        std::unique_ptr<int> value = values[i].get<int>();
                        ASSERT_NE((const int *) NULL, value.get());
                        actualValues.push_back(*value);
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
                    values = imap->values(query::TruePredicate());
                    ASSERT_EQ(numItems, (int) values.size());
                    actualValues.clear();
                    for (int i = 0; i < (int) values.size(); ++i) {
                        std::unique_ptr<int> value = values[i].get<int>();
                        ASSERT_NE((const int *) NULL, value.get());
                        actualValues.push_back(*value);
                    }
                    std::sort(actualValues.begin(), actualValues.end());
                    for (int i = 0; i < numItems; ++i) {
                        ASSERT_EQ(2 * i, actualValues[i]);
                    }

                    // FalsePredicate
                    values = imap->values(query::FalsePredicate());
                    ASSERT_EQ(0, (int) values.size());

                    // BetweenPredicate
                    // 5 <= key <= 10
                    values = imap->values(
                            query::BetweenPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5, 10));
                    ASSERT_EQ(6, (int) values.size());
                    actualValues.clear();
                    for (int i = 0; i < (int) values.size(); ++i) {
                        std::unique_ptr<int> value = values[i].get<int>();
                        ASSERT_NE((const int *) NULL, value.get());
                        actualValues.push_back(*value);
                    }
                    std::sort(actualValues.begin(), actualValues.end());
                    for (int i = 0; i < 6; ++i) {
                        ASSERT_EQ(2 * (i + 5), actualValues[i]);
                    }

                    // 20 <= key <=30
                    values = imap->values(
                            query::BetweenPredicate<int>(query::QueryConstants::getKeyAttributeName(), 20, 30));
                    ASSERT_EQ(0, (int) values.size());

                    // GreaterLessPredicate
                    // value <= 10
                    values = imap->values(
                            query::GreaterLessPredicate<int>(query::QueryConstants::getValueAttributeName(), 10, true,
                                                             true));
                    ASSERT_EQ(6, (int) values.size());
                    actualValues.clear();
                    for (int i = 0; i < (int) values.size(); ++i) {
                        std::unique_ptr<int> value = values[i].get<int>();
                        ASSERT_NE((const int *) NULL, value.get());
                        actualValues.push_back(*value);
                    }
                    std::sort(actualValues.begin(), actualValues.end());
                    for (int i = 0; i < 6; ++i) {
                        ASSERT_EQ(2 * i, actualValues[i]);
                    }

                    // key < 7
                    values = imap->values(
                            query::GreaterLessPredicate<int>(query::QueryConstants::getKeyAttributeName(), 7, false,
                                                             true));
                    ASSERT_EQ(7, (int) values.size());
                    actualValues.clear();
                    for (int i = 0; i < (int) values.size(); ++i) {
                        std::unique_ptr<int> value = values[i].get<int>();
                        ASSERT_NE((const int *) NULL, value.get());
                        actualValues.push_back(*value);
                    }
                    std::sort(actualValues.begin(), actualValues.end());
                    for (int i = 0; i < 7; ++i) {
                        ASSERT_EQ(2 * i, actualValues[i]);
                    }

                    // value >= 15
                    values = imap->values(
                            query::GreaterLessPredicate<int>(query::QueryConstants::getValueAttributeName(), 15, true,
                                                             false));
                    ASSERT_EQ(12, (int) values.size());
                    actualValues.clear();
                    for (int i = 0; i < (int) values.size(); ++i) {
                        std::unique_ptr<int> value = values[i].get<int>();
                        ASSERT_NE((const int *) NULL, value.get());
                        actualValues.push_back(*value);
                    }
                    std::sort(actualValues.begin(), actualValues.end());
                    for (int i = 0; i < 12; ++i) {
                        ASSERT_EQ(2 * (i + 8), actualValues[i]);
                    }

                    // key > 5
                    values = imap->values(
                            query::GreaterLessPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5, false,
                                                             false));
                    ASSERT_EQ(14, (int) values.size());
                    actualValues.clear();
                    for (int i = 0; i < (int) values.size(); ++i) {
                        std::unique_ptr<int> value = values[i].get<int>();
                        ASSERT_NE((const int *) NULL, value.get());
                        actualValues.push_back(*value);
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
                    values = imap->values(
                            query::InPredicate<int>(query::QueryConstants::getKeyAttributeName(), inVals));
                    ASSERT_EQ(3, (int) values.size());
                    actualValues.clear();
                    for (int i = 0; i < (int) values.size(); ++i) {
                        std::unique_ptr<int> value = values[i].get<int>();
                        ASSERT_NE((const int *) NULL, value.get());
                        actualValues.push_back(*value);
                    }
                    std::sort(actualValues.begin(), actualValues.end());
                    ASSERT_EQ(2 * 4, actualValues[0]);
                    ASSERT_EQ(2 * 10, actualValues[1]);
                    ASSERT_EQ(2 * 19, actualValues[2]);

                    // value in {4, 10, 19}
                    values = imap->values(
                            query::InPredicate<int>(query::QueryConstants::getValueAttributeName(), inVals));
                    ASSERT_EQ(2, (int) values.size());
                    actualValues.clear();
                    for (int i = 0; i < (int) values.size(); ++i) {
                        std::unique_ptr<int> value = values[i].get<int>();
                        ASSERT_NE((const int *) NULL, value.get());
                        actualValues.push_back(*value);
                    }
                    std::sort(actualValues.begin(), actualValues.end());
                    ASSERT_EQ(4, actualValues[0]);
                    ASSERT_EQ(10, actualValues[1]);

                    // InstanceOfPredicate
                    // value instanceof Integer
                    values = imap->values(query::InstanceOfPredicate("java.lang.Integer"));
                    ASSERT_EQ(20, (int) values.size());
                    actualValues.clear();
                    for (int i = 0; i < (int) values.size(); ++i) {
                        std::unique_ptr<int> value = values[i].get<int>();
                        ASSERT_NE((const int *) NULL, value.get());
                        actualValues.push_back(*value);
                    }
                    std::sort(actualValues.begin(), actualValues.end());
                    for (int i = 0; i < numItems; ++i) {
                        ASSERT_EQ(2 * i, actualValues[i]);
                    }

                    values = imap->values(query::InstanceOfPredicate("java.lang.String"));
                    ASSERT_EQ(0, (int) values.size());

                    // NotPredicate
                    // !(5 <= key <= 10)
                    std::unique_ptr<query::Predicate> bp = std::unique_ptr<query::Predicate>(
                            new query::BetweenPredicate<int>(
                                    query::QueryConstants::getKeyAttributeName(), 5, 10));
                    query::NotPredicate notPredicate(bp);
                    values = imap->values(notPredicate);
                    ASSERT_EQ(14, (int) values.size());
                    actualValues.clear();
                    for (int i = 0; i < (int) values.size(); ++i) {
                        std::unique_ptr<int> value = values[i].get<int>();
                        ASSERT_NE((const int *) NULL, value.get());
                        actualValues.push_back(*value);
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
                    bp = std::unique_ptr<query::Predicate>(
                            new query::BetweenPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5, 10));
                    std::unique_ptr<query::Predicate> inPred = std::unique_ptr<query::Predicate>(
                            new query::InPredicate<int>(query::QueryConstants::getValueAttributeName(), inVals));
                    values = imap->values(query::AndPredicate().add(bp).add(inPred));
                    ASSERT_EQ(1, (int) values.size());
                    ASSERT_EQ(10, *values[0].get<int>());

                    // OrPredicate
                    // 5 <= key <= 10 OR Values in {4, 10, 19} = values {4, 10, 12, 14, 16, 18, 20}
                    bp = std::unique_ptr<query::Predicate>(
                            new query::BetweenPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5, 10));
                    inPred = std::unique_ptr<query::Predicate>(
                            new query::InPredicate<int>(query::QueryConstants::getValueAttributeName(), inVals));
                    values = imap->values(query::OrPredicate().add(bp).add(inPred));
                    ASSERT_EQ(7, (int) values.size());
                    actualValues.clear();
                    for (int i = 0; i < (int) values.size(); ++i) {
                        std::unique_ptr<int> value = values[i].get<int>();
                        ASSERT_NE((const int *) NULL, value.get());
                        actualValues.push_back(*value);
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
                        key += hazelcast::util::IOUtil::to_string(i);
                        std::string value = "value";
                        value += hazelcast::util::IOUtil::to_string(i);
                        imap->put<std::string, std::string>(key, value);
                    }
                    imap->put<std::string, std::string>("key_111_test", "myvalue_111_test");
                    imap->put<std::string, std::string>("key_22_test", "myvalue_22_test");

                    // LikePredicate
                    // value LIKE "value1" : {"value1"}
                    values = imap->keySet(
                            query::LikePredicate(query::QueryConstants::getValueAttributeName(), "value1"));
                    ASSERT_EQ(1, (int) values.size());
                    std::unique_ptr<std::string> stringValue = values[0].get<std::string>();
                    ASSERT_NE((const std::string *) NULL, stringValue.get());
                    ASSERT_EQ("key1", *stringValue);

                    // ILikePredicate
                    // value ILIKE "%VALue%1%" : {"myvalue_111_test", "value1", "value10", "value11"}
                    values = imap->keySet(
                            query::ILikePredicate(query::QueryConstants::getValueAttributeName(), "%VALue%1%"));
                    ASSERT_EQ(4, (int) values.size());
                    std::vector<std::string> actualStrs;
                    for (int i = 0; i < (int) values.size(); ++i) {
                        std::unique_ptr<std::string> value = values[i].get<std::string>();
                        ASSERT_NE((const std::string *) NULL, value.get());
                        actualStrs.push_back(*value);
                    }
                    std::sort(actualStrs.begin(), actualStrs.end());
                    ASSERT_EQ("key1", actualStrs[0]);
                    ASSERT_EQ("key10", actualStrs[1]);
                    ASSERT_EQ("key11", actualStrs[2]);
                    ASSERT_EQ("key_111_test", actualStrs[3]);

                    // value ILIKE "%VAL%2%" : {"myvalue_22_test", "value2"}
                    values = imap->keySet(
                            query::ILikePredicate(query::QueryConstants::getValueAttributeName(), "%VAL%2%"));
                    ASSERT_EQ(2, (int) values.size());
                    actualStrs.clear();
                    for (int i = 0; i < (int) values.size(); ++i) {
                        std::unique_ptr<std::string> value = values[i].get<std::string>();
                        ASSERT_NE((const std::string *) NULL, value.get());
                        actualStrs.push_back(*value);
                    }
                    std::sort(actualStrs.begin(), actualStrs.end());
                    ASSERT_EQ("key2", actualStrs[0]);
                    ASSERT_EQ("key_22_test", actualStrs[1]);

                    // SqlPredicate
                    // __key BETWEEN 4 and 7 : {4, 5, 6, 7} -> {8, 10, 12, 14}
                    char sql[100];
                    hazelcast::util::hz_snprintf(sql, 50, "%s BETWEEN 4 and 7", query::QueryConstants::getKeyAttributeName());
                    values = imap->values(query::SqlPredicate(sql));
                    ASSERT_EQ(4, (int) values.size());
                    actualValues.clear();
                    for (int i = 0; i < (int) values.size(); ++i) {
                        std::unique_ptr<int> value = values[i].get<int>();
                        ASSERT_NE((const int *) NULL, value.get());
                        actualValues.push_back(*value);
                    }
                    std::sort(actualValues.begin(), actualValues.end());
                    for (int i = 0; i < 4; ++i) {
                        ASSERT_EQ(2 * (i + 4), actualValues[i]);
                    }

                    // RegexPredicate
                    // value matches the regex ".*value.*2.*" : {myvalue_22_test, value2}
                    values = imap->keySet(
                            query::RegexPredicate(query::QueryConstants::getValueAttributeName(), ".*value.*2.*"));
                    ASSERT_EQ(2, (int) values.size());
                    actualStrs.clear();
                    for (int i = 0; i < (int) values.size(); ++i) {
                        std::unique_ptr<std::string> value = values[i].get<std::string>();
                        ASSERT_NE((const std::string *) NULL, value.get());
                        actualStrs.push_back(*value);
                    }
                    std::sort(actualStrs.begin(), actualStrs.end());
                    ASSERT_EQ("key2", actualStrs[0]);
                    ASSERT_EQ("key_22_test", actualStrs[1]);
                }
*/

/*
                TEST_P(MixedMapAPITest, testValuesWithPagingPredicate) {
                    int predSize = 5;
                    const int totalEntries = 25;

                    for (int i = 0; i < totalEntries; ++i) {
                        imap->put<int, int>(i, i);
                    }

                    query::PagingPredicate<int, int> predicate((size_t) predSize);

                    std::vector<int> values = imap->values(predicate);
                    ASSERT_EQ(predSize, (int) values.size());
                    for (int i = 0; i < predSize; ++i) {
                        ASSERT_EQ(i, values[i]);
                    }

                    values = imap->values(predicate);
                    ASSERT_EQ(predSize, (int) values.size());
                    for (int i = 0; i < predSize; ++i) {
                        ASSERT_EQ(i, values[i]);
                    }

                    predicate.nextPage();
                    values = imap->values(predicate);
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

                    values = imap->values(predicate);
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
                    values = imap->values(predicate);
                    ASSERT_EQ(0, (int) values.size());

                    predicate.setPage(0);
                    values = imap->values(predicate);
                    ASSERT_EQ(predSize, (int) values.size());
                    for (int i = 0; i < predSize; ++i) {
                        ASSERT_EQ(i, values[i]);
                    }

                    predicate.previousPage();
                    ASSERT_EQ(0, (int) predicate.getPage());

                    predicate.setPage(5);
                    values = imap->values(predicate);
                    ASSERT_EQ(0, (int) values.size());

                    predicate.setPage(3);
                    values = imap->values(predicate);
                    ASSERT_EQ(predSize, (int) values.size());
                    for (int i = 0; i < predSize; ++i) {
                        ASSERT_EQ(3 * predSize + i, values[i]);
                    }

                    predicate.previousPage();
                    values = imap->values(predicate);
                    ASSERT_EQ(predSize, (int) values.size());
                    for (int i = 0; i < predSize; ++i) {
                        ASSERT_EQ(2 * predSize + i, values[i]);
                    }

                    // test PagingPredicate with inner predicate (value < 10)
                    std::unique_ptr<query::Predicate> lessThanTenPredicate(std::unique_ptr<query::Predicate>(
                            new query::GreaterLessPredicate<int>(query::QueryConstants::getValueAttributeName(), 9,
                                                                 false,
                                                                 true)));
                    query::PagingPredicate<int, int> predicate2(lessThanTenPredicate, 5);
                    values = imap->values(predicate2);
                    ASSERT_EQ(predSize, (int) values.size());
                    for (int i = 0; i < predSize; ++i) {
                        ASSERT_EQ(i, values[i]);
                    }

                    predicate2.nextPage();
                    // match values 5,6, 7, 8
                    values = imap->values(predicate2);
                    ASSERT_EQ(predSize - 1, (int) values.size());
                    for (int i = 0; i < predSize - 1; ++i) {
                        ASSERT_EQ(predSize + i, values[i]);
                    }

                    predicate2.nextPage();
                    values = imap->values(predicate2);
                    ASSERT_EQ(0, (int) values.size());

                    // test paging predicate with comparator
                    Employee empl1("ahmet", 35);
                    Employee empl2("mehmet", 21);
                    Employee empl3("deniz", 25);
                    Employee empl4("ali", 33);
                    Employee empl5("veli", 44);
                    Employee empl6("aylin", 5);

                    imap->put<int, Employee>(3, empl1);
                    imap->put<int, Employee>(4, empl2);
                    imap->put<int, Employee>(5, empl3);
                    imap->put<int, Employee>(6, empl4);
                    imap->put<int, Employee>(7, empl5);
                    imap->put<int, Employee>(8, empl6);

                    predSize = 2;
                    query::PagingPredicate<int, Employee> predicate3(
                            std::unique_ptr<query::EntryComparator<int, Employee> >(new EmployeeEntryComparator()),
                            (size_t) predSize);
                    std::vector<Employee> employees = imap->values<int, Employee>(predicate3);
                    ASSERT_EQ(2, (int) employees.size());
                    ASSERT_EQ(empl6, employees[0]);
                    ASSERT_EQ(empl2, employees[1]);

                    predicate3.nextPage();
                    employees = imap->values<int, Employee>(predicate3);
                    ASSERT_EQ(2, (int) employees.size());
                    ASSERT_EQ(empl3, employees[0]);
                    ASSERT_EQ(empl4,  employees[1]);
                }
*/

/*
                TEST_P(MixedMapAPITest, testKeySetWithPredicate) {
                    const int numItems = 20;
                    for (int i = 0; i < numItems; ++i) {
                        imap->put<int, int>(i, 2 * i);
                    }

                    std::unique_ptr<DataArray<int> > values = imap->keySet();
                    ASSERT_EQ(numItems, (int) values.size());
                    std::vector<int> actualValues;
                    for (int i = 0; i < (int) values.size(); ++i) {
                        ASSERT_NE((const int *) NULL, values->get(i));
                        actualValues.push_back(*values->get(i));
                    }
                    std::sort(actualValues.begin(), actualValues.end());
                    for (int i = 0; i < numItems; ++i) {
                        ASSERT_EQ(i, actualValues[i]);
                    }

                    // EqualPredicate
                    // key == 5
                    values = imap->keySet(
                            query::EqualPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5));
                    ASSERT_EQ(1, (int) values.size());
                    ASSERT_NE((const int *) NULL, (*values)[0]);
                    ASSERT_EQ(5, *((*values)[0]));

                    // value == 8
                    values = imap->keySet(
                            query::EqualPredicate<int>(query::QueryConstants::getValueAttributeName(), 8));
                    ASSERT_EQ(1, (int) values.size());
                    ASSERT_NE((const int *) NULL, (*values)[0]);
                    ASSERT_EQ(4, *((*values)[0]));

                    // key == numItems
                    values = imap->keySet(
                            query::EqualPredicate<int>(query::QueryConstants::getKeyAttributeName(), numItems));
                    ASSERT_EQ(0, (int) values.size());

                    // NotEqual Predicate
                    // key != 5
                    values = imap->keySet(
                            query::NotEqualPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5));
                    ASSERT_EQ(numItems - 1, (int) values.size());
                    actualValues.clear();
                    for (int i = 0; i < (int) values.size(); ++i) {
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
                    values = imap->keySet(
                            query::NotEqualPredicate<int>(query::QueryConstants::getValueAttributeName(), 8));
                    ASSERT_EQ(numItems - 1, (int) values.size());
                    actualValues.clear();
                    for (int i = 0; i < (int) values.size(); ++i) {
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
                    values = imap->keySet(query::TruePredicate());
                    ASSERT_EQ(numItems, (int) values.size());
                    actualValues.clear();
                    for (int i = 0; i < (int) values.size(); ++i) {
                        ASSERT_NE((const int *) NULL, values->get(i));
                        actualValues.push_back(*values->get(i));
                    }
                    std::sort(actualValues.begin(), actualValues.end());
                    for (int i = 0; i < numItems; ++i) {
                        ASSERT_EQ(i, actualValues[i]);
                    }

                    // FalsePredicate
                    values = imap->keySet(query::FalsePredicate());
                    ASSERT_EQ(0, (int) values.size());

                    // BetweenPredicate
                    // 5 <= key <= 10
                    values = imap->keySet(
                            query::BetweenPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5, 10));
                    ASSERT_EQ(6, (int) values.size());
                    actualValues.clear();
                    for (int i = 0; i < (int) values.size(); ++i) {
                        ASSERT_NE((const int *) NULL, values->get(i));
                        actualValues.push_back(*values->get(i));
                    }
                    std::sort(actualValues.begin(), actualValues.end());
                    for (int i = 0; i < 6; ++i) {
                        ASSERT_EQ((i + 5), actualValues[i]);
                    }

                    // 20 <= key <=30
                    values = imap->keySet(
                            query::BetweenPredicate<int>(query::QueryConstants::getKeyAttributeName(), 20, 30));
                    ASSERT_EQ(0, (int) values.size());

                    // GreaterLessPredicate
                    // value <= 10
                    values = imap->keySet(
                            query::GreaterLessPredicate<int>(query::QueryConstants::getValueAttributeName(), 10, true,
                                                             true));
                    ASSERT_EQ(6, (int) values.size());
                    actualValues.clear();
                    for (int i = 0; i < (int) values.size(); ++i) {
                        ASSERT_NE((const int *) NULL, values->get(i));
                        actualValues.push_back(*values->get(i));
                    }
                    std::sort(actualValues.begin(), actualValues.end());
                    for (int i = 0; i < 6; ++i) {
                        ASSERT_EQ(i, actualValues[i]);
                    }

                    // key < 7
                    values = imap->keySet(
                            query::GreaterLessPredicate<int>(query::QueryConstants::getKeyAttributeName(), 7, false,
                                                             true));
                    ASSERT_EQ(7, (int) values.size());
                    actualValues.clear();
                    for (int i = 0; i < (int) values.size(); ++i) {
                        ASSERT_NE((const int *) NULL, values->get(i));
                        actualValues.push_back(*values->get(i));
                    }
                    std::sort(actualValues.begin(), actualValues.end());
                    for (int i = 0; i < 7; ++i) {
                        ASSERT_EQ(i, actualValues[i]);
                    }

                    // value >= 15
                    values = imap->keySet(
                            query::GreaterLessPredicate<int>(query::QueryConstants::getValueAttributeName(), 15, true,
                                                             false));
                    ASSERT_EQ(12, (int) values.size());
                    actualValues.clear();
                    for (int i = 0; i < (int) values.size(); ++i) {
                        ASSERT_NE((const int *) NULL, values->get(i));
                        actualValues.push_back(*values->get(i));
                    }
                    std::sort(actualValues.begin(), actualValues.end());
                    for (int i = 0; i < 12; ++i) {
                        ASSERT_EQ((i + 8), actualValues[i]);
                    }

                    // key > 5
                    values = imap->keySet(
                            query::GreaterLessPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5, false,
                                                             false));
                    ASSERT_EQ(14, (int) values.size());
                    actualValues.clear();
                    for (int i = 0; i < (int) values.size(); ++i) {
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
                    values = imap->keySet(
                            query::InPredicate<int>(query::QueryConstants::getKeyAttributeName(), inVals));
                    ASSERT_EQ(3, (int) values.size());
                    actualValues.clear();
                    for (int i = 0; i < (int) values.size(); ++i) {
                        ASSERT_NE((const int *) NULL, values->get(i));
                        actualValues.push_back(*values->get(i));
                    }
                    std::sort(actualValues.begin(), actualValues.end());
                    ASSERT_EQ(4, actualValues[0]);
                    ASSERT_EQ(10, actualValues[1]);
                    ASSERT_EQ(19, actualValues[2]);

                    // value in {4, 10, 19}
                    values = imap->keySet(
                            query::InPredicate<int>(query::QueryConstants::getValueAttributeName(), inVals));
                    ASSERT_EQ(2, (int) values.size());
                    actualValues.clear();
                    for (int i = 0; i < (int) values.size(); ++i) {
                        ASSERT_NE((const int *) NULL, values->get(i));
                        actualValues.push_back(*values->get(i));
                    }
                    std::sort(actualValues.begin(), actualValues.end());
                    ASSERT_EQ(2, actualValues[0]);
                    ASSERT_EQ(5, actualValues[1]);

                    // InstanceOfPredicate
                    // value instanceof Integer
                    values = imap->keySet(query::InstanceOfPredicate("java.lang.Integer"));
                    ASSERT_EQ(20, (int) values.size());
                    actualValues.clear();
                    for (int i = 0; i < (int) values.size(); ++i) {
                        ASSERT_NE((const int *) NULL, values->get(i));
                        actualValues.push_back(*values->get(i));
                    }
                    std::sort(actualValues.begin(), actualValues.end());
                    for (int i = 0; i < numItems; ++i) {
                        ASSERT_EQ(i, actualValues[i]);
                    }

                    values = imap->keySet(query::InstanceOfPredicate("java.lang.String"));
                    ASSERT_EQ(0, (int) values.size());

                    // NotPredicate
                    // !(5 <= key <= 10)
                    std::unique_ptr<query::Predicate> bp = std::unique_ptr<query::Predicate>(
                            new query::BetweenPredicate<int>(
                                    query::QueryConstants::getKeyAttributeName(), 5, 10));
                    query::NotPredicate notPredicate(bp);
                    values = imap->keySet(notPredicate);
                    ASSERT_EQ(14, (int) values.size());
                    actualValues.clear();
                    for (int i = 0; i < (int) values.size(); ++i) {
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
                    bp = std::unique_ptr<query::Predicate>(
                            new query::BetweenPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5, 10));
                    std::unique_ptr<query::Predicate> inPred = std::unique_ptr<query::Predicate>(
                            new query::InPredicate<int>(query::QueryConstants::getValueAttributeName(), inVals));
                    values = imap->keySet(query::AndPredicate().add(bp).add(inPred));
                    ASSERT_EQ(1, (int) values.size());
                    ASSERT_EQ(5, *(values->release(0)));

                    // OrPredicate
                    // 5 <= key <= 10 OR Values in {4, 10, 19} = values {4, 10, 12, 14, 16, 18, 20}
                    bp = std::unique_ptr<query::Predicate>(
                            new query::BetweenPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5, 10));
                    inPred = std::unique_ptr<query::Predicate>(
                            new query::InPredicate<int>(query::QueryConstants::getValueAttributeName(), inVals));
                    values = imap->keySet(query::OrPredicate().add(bp).add(inPred));
                    ASSERT_EQ(7, (int) values.size());
                    actualValues.clear();
                    for (int i = 0; i < (int) values.size(); ++i) {
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
                        key += hazelcast::util::IOUtil::to_string(i);
                        std::string value = "value";
                        value += hazelcast::util::IOUtil::to_string(i);
                        imap->put<std::string, std::string>(key, value);
                    }
                    imap->put<std::string, std::string>("key_111_test", "myvalue_111_test");
                    imap->put<std::string, std::string>("key_22_test", "myvalue_22_test");

                    // LikePredicate
                    // value LIKE "value1" : {"value1"}
                    std::unique_ptr<DataArray<std::string> > strValues = imap->keySet(
                            query::LikePredicate(query::QueryConstants::getValueAttributeName(), "value1"));
                    ASSERT_EQ(1, (int) strvalues.size());
                    ASSERT_NE((const std::string *) NULL, strValues->get(0));
                    ASSERT_EQ("key1", *strValues->get(0));

                    // ILikePredicate
                    // value ILIKE "%VALue%1%" : {"key_111_test", "key1", "key10", "key11"}
                    strValues = imap->keySet(
                            query::ILikePredicate(query::QueryConstants::getValueAttributeName(), "%VALue%1%"));
                    ASSERT_EQ(4, (int) strvalues.size());
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
                    hazelcast::util::hz_snprintf(sql, 50, "%s BETWEEN 4 and 7", query::QueryConstants::getKeyAttributeName());
                    values = imap->keySet(query::SqlPredicate(sql));
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

                TEST_P(MixedMapAPITest, testKeySetWithPagingPredicate) {
                    int predSize = 5;
                    const int totalEntries = 25;

                    for (int i = 0; i < totalEntries; ++i) {
                        imap->put<int, int>(i, i);
                    }

                    query::PagingPredicate<int, int> predicate((size_t) predSize);

                    std::unique_ptr<DataArray<int> > values = imap->keySet(predicate);
                    ASSERT_EQ(predSize, (int) values->size());
                    for (int i = 0; i < predSize; ++i) {
                        ASSERT_NE((const int *) NULL, values->get(i));
                        ASSERT_EQ(i, *values->get(i));
                    }

                    values = imap->keySet(predicate);
                    ASSERT_EQ(predSize, (int) values->size());
                    for (int i = 0; i < predSize; ++i) {
                        ASSERT_NE((const int *) NULL, values->get(i));
                        ASSERT_EQ(i, *values->get(i));
                    }

                    predicate.nextPage();
                    values = imap->keySet(predicate);
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

                    values = imap->keySet(predicate);
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
                    values = imap->keySet(predicate);
                    ASSERT_EQ(0, (int) values->size());

                    predicate.setPage(0);
                    values = imap->keySet(predicate);
                    ASSERT_EQ(predSize, (int) values->size());
                    for (int i = 0; i < predSize; ++i) {
                        ASSERT_NE((const int *) NULL, values->get(i));
                        ASSERT_EQ(i, *values->get(i));
                    }

                    predicate.previousPage();
                    ASSERT_EQ(0, (int) predicate.getPage());

                    predicate.setPage(5);
                    values = imap->keySet(predicate);
                    ASSERT_EQ(0, (int) values->size());

                    predicate.setPage(3);
                    values = imap->keySet(predicate);
                    ASSERT_EQ(predSize, (int) values->size());
                    for (int i = 0; i < predSize; ++i) {
                        ASSERT_NE((const int *) NULL, values->get(i));
                        ASSERT_EQ(3 * predSize + i, *values->get(i));
                    }

                    predicate.previousPage();
                    values = imap->keySet(predicate);
                    ASSERT_EQ(predSize, (int) values->size());
                    for (int i = 0; i < predSize; ++i) {
                        const int *value = values->get(i);
                        ASSERT_NE((const int *) NULL, value);
                        ASSERT_EQ(2 * predSize + i, *value);
                    }

                    // test PagingPredicate with inner predicate (value < 10)
                    std::unique_ptr<query::Predicate> lessThanTenPredicate(std::unique_ptr<query::Predicate>(
                            new query::GreaterLessPredicate<int>(query::QueryConstants::getValueAttributeName(), 9,
                                                                 false,
                                                                 true)));
                    query::PagingPredicate<int, int> predicate2(lessThanTenPredicate, 5);
                    values = imap->keySet(predicate2);
                    ASSERT_EQ(predSize, (int) values->size());
                    for (int i = 0; i < predSize; ++i) {
                        ASSERT_NE((const int *) NULL, values->get(i));
                        ASSERT_EQ(i, *values->get(i));
                    }

                    predicate2.nextPage();
                    // match values 5,6, 7, 8
                    values = imap->keySet(predicate2);
                    ASSERT_EQ(predSize - 1, (int) values->size());
                    for (int i = 0; i < predSize - 1; ++i) {
                        ASSERT_NE((const int *) NULL, values->get(i));
                        ASSERT_EQ(predSize + i, *values->get(i));
                    }

                    predicate2.nextPage();
                    values = imap->keySet(predicate2);
                    ASSERT_EQ(0, (int) values->size());

                    // test paging predicate with comparator
                    Employee empl1("ahmet", 35);
                    Employee empl2("mehmet", 21);
                    Employee empl3("deniz", 25);
                    Employee empl4("ali", 33);
                    Employee empl5("veli", 44);
                    Employee empl6("aylin", 5);

                    imap->put<int, Employee>(3, empl1);
                    imap->put<int, Employee>(4, empl2);
                    imap->put<int, Employee>(5, empl3);
                    imap->put<int, Employee>(6, empl4);
                    imap->put<int, Employee>(7, empl5);
                    imap->put<int, Employee>(8, empl6);

                    predSize = 2;
                    query::PagingPredicate<int, Employee> predicate3(
                            std::unique_ptr<query::EntryComparator<int, Employee> >(new EmployeeEntryKeyComparator()),
                            (size_t) predSize);
                    std::unique_ptr<DataArray<int> > result = imap->keySet(predicate3);
                    // since keyset result only returns keys from the server, no ordering based on the value but ordered based on the keys
                    ASSERT_EQ(2, (int) result->size());
                    ASSERT_NE((const int *) NULL, (*result)[0]);
                    ASSERT_NE((const int *) NULL, (*result)[1]);
                    ASSERT_EQ(3, *((*result)[0]));
                    ASSERT_EQ(4, *result->get(1));

                    predicate3.nextPage();
                    result = imap->keySet(predicate3);
                    ASSERT_EQ(2, (int) result->size());
                    ASSERT_NE((const int *) NULL, (*result)[0]);
                    ASSERT_NE((const int *) NULL, (*result)[1]);
                    ASSERT_EQ(5, *((*result)[0]));
                    ASSERT_EQ(6, *result->get(1));
                }

                TEST_P(MixedMapAPITest, testEntrySetWithPredicate) {
                    const int numItems = 20;
                    std::vector<std::pair<int, int> > expected(numItems);
                    for (int i = 0; i < numItems; ++i) {
                        imap->put<int, int>(i, 2 * i);
                        expected[i] = std::pair<int, int>(i, 2 * i);
                    }

                    std::unique_ptr<EntryArray<int, int> > entries = imap->entrySet();
                    ASSERT_EQ(numItems, (int) entries->size());
                    entries->sort(query::ENTRY);
                    for (int i = 0; i < numItems; ++i) {
                        std::pair<int, int> entry(*entries->getKey(i), *entries->getValue(i));
                        ASSERT_EQ(expected[i], entry);
                    }

                    // EqualPredicate
                    // key == 5
                    entries = imap->entrySet(
                            query::EqualPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5));
                    ASSERT_EQ(1, (int) entries->size());
                    std::pair<int, int> entry1(*entries->getKey(0), *entries->getValue(0));
                    ASSERT_EQ(expected[5], entry1);

                    // value == 8
                    entries = imap->entrySet(
                            query::EqualPredicate<int>(query::QueryConstants::getValueAttributeName(), 8));
                    ASSERT_EQ(1, (int) entries->size());
                    std::pair<int, int> entry2(*entries->getKey(0), *entries->getValue(0));
                    ASSERT_EQ(expected[4], entry2);

                    // key == numItems
                    entries = imap->entrySet(
                            query::EqualPredicate<int>(query::QueryConstants::getKeyAttributeName(), numItems));
                    ASSERT_EQ(0, (int) entries->size());

                    // NotEqual Predicate
                    // key != 5
                    entries = imap->entrySet(
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
                    entries = imap->entrySet(
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
                    entries = imap->entrySet(query::TruePredicate());
                    ASSERT_EQ(numItems, (int) entries->size());
                    entries->sort(query::ENTRY);
                    for (int i = 0; i < numItems; ++i) {
                        std::pair<int, int> entry(*entries->getKey(i), *entries->getValue(i));
                        ASSERT_EQ(expected[i], entry);
                    }

                    // FalsePredicate
                    entries = imap->entrySet(query::FalsePredicate());
                    ASSERT_EQ(0, (int) entries->size());

                    // BetweenPredicate
                    // 5 <= key <= 10
                    entries = imap->entrySet(
                            query::BetweenPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5, 10));
                    entries->sort(query::ENTRY);
                    ASSERT_EQ(6, (int) entries->size());
                    for (int i = 0; i < 6; ++i) {
                        std::pair<int, int> entry(*entries->getKey(i), *entries->getValue(i));
                        ASSERT_EQ(expected[i + 5], entry);
                    }

                    // 20 <= key <=30
                    entries = imap->entrySet(
                            query::BetweenPredicate<int>(query::QueryConstants::getKeyAttributeName(), 20, 30));
                    ASSERT_EQ(0, (int) entries->size());

                    // GreaterLessPredicate
                    // value <= 10
                    entries = imap->entrySet(
                            query::GreaterLessPredicate<int>(query::QueryConstants::getValueAttributeName(), 10, true,
                                                             true));
                    ASSERT_EQ(6, (int) entries->size());
                    entries->sort(query::ENTRY);
                    for (int i = 0; i < 6; ++i) {
                        std::pair<int, int> entry(*entries->getKey(i), *entries->getValue(i));
                        ASSERT_EQ(expected[i], entry);
                    }

                    // key < 7
                    entries = imap->entrySet(
                            query::GreaterLessPredicate<int>(query::QueryConstants::getKeyAttributeName(), 7, false,
                                                             true));
                    ASSERT_EQ(7, (int) entries->size());
                    entries->sort(query::ENTRY);
                    for (int i = 0; i < 7; ++i) {
                        std::pair<int, int> entry(*entries->getKey(i), *entries->getValue(i));
                        ASSERT_EQ(expected[i], entry);
                    }

                    // value >= 15
                    entries = imap->entrySet(
                            query::GreaterLessPredicate<int>(query::QueryConstants::getValueAttributeName(), 15, true,
                                                             false));
                    ASSERT_EQ(12, (int) entries->size());
                    entries->sort(query::ENTRY);
                    for (int i = 0; i < 12; ++i) {
                        std::pair<int, int> entry(*entries->getKey(i), *entries->getValue(i));
                        ASSERT_EQ(expected[i + 8], entry);
                    }

                    // key > 5
                    entries = imap->entrySet(
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
                    entries = imap->entrySet(
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
                    entries = imap->entrySet(
                            query::InPredicate<int>(query::QueryConstants::getValueAttributeName(), inVals));
                    ASSERT_EQ(2, (int) entries->size());
                    entries->sort(query::ENTRY);
                    std::pair<int, int> entry(*entries->getKey(0), *entries->getValue(0));
                    ASSERT_EQ(expected[2], entry);
                    entry = std::pair<int, int>(*entries->getKey(1), *entries->getValue(1));
                    ASSERT_EQ(expected[5], entry);

                    // InstanceOfPredicate
                    // value instanceof Integer
                    entries = imap->entrySet(query::InstanceOfPredicate("java.lang.Integer"));
                    ASSERT_EQ(20, (int) entries->size());
                    entries->sort(query::ENTRY);
                    for (int i = 0; i < numItems; ++i) {
                        std::pair<int, int> item(*entries->getKey(i), *entries->getValue(i));
                        ASSERT_EQ(expected[i], item);
                    }

                    entries = imap->entrySet(query::InstanceOfPredicate("java.lang.String"));
                    ASSERT_EQ(0, (int) entries->size());

                    // NotPredicate
                    // !(5 <= key <= 10)
                    std::unique_ptr<query::Predicate> bp = std::unique_ptr<query::Predicate>(
                            new query::BetweenPredicate<int>(
                                    query::QueryConstants::getKeyAttributeName(), 5, 10));
                    query::NotPredicate notPredicate(bp);
                    entries = imap->entrySet(notPredicate);
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
                    bp = std::unique_ptr<query::Predicate>(
                            new query::BetweenPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5, 10));
                    std::unique_ptr<query::Predicate> inPred = std::unique_ptr<query::Predicate>(
                            new query::InPredicate<int>(query::QueryConstants::getValueAttributeName(), inVals));
                    entries = imap->entrySet(query::AndPredicate().add(bp).add(inPred));
                    ASSERT_EQ(1, (int) entries->size());
                    entries->sort(query::ENTRY);
                    entry = std::pair<int, int>(*entries->getKey(0), *entries->getValue(0));
                    ASSERT_EQ(expected[5], entry);

                    // OrPredicate
                    // 5 <= key <= 10 OR Values in {4, 10, 19} = entries keys {2, 5, 6, 7, 8, 9, 10}
                    bp = std::unique_ptr<query::Predicate>(
                            new query::BetweenPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5, 10));
                    inPred = std::unique_ptr<query::Predicate>(
                            new query::InPredicate<int>(query::QueryConstants::getValueAttributeName(), inVals));
                    entries = imap->entrySet(query::OrPredicate().add(bp).add(inPred));
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
                        key += hazelcast::util::IOUtil::to_string(i);
                        std::string value = "value";
                        value += hazelcast::util::IOUtil::to_string(i);
                        imap->put<std::string, std::string>(key, value);
                        expectedStrEntries[i] = std::pair<std::string, std::string>(key, value);
                    }
                    imap->put<std::string, std::string>("key_111_test", "myvalue_111_test");
                    expectedStrEntries[12] = std::pair<std::string, std::string>("key_111_test", "myvalue_111_test");
                    imap->put<std::string, std::string>("key_22_test", "myvalue_22_test");
                    expectedStrEntries[13] = std::pair<std::string, std::string>("key_22_test", "myvalue_22_test");

                    // LikePredicate
                    // value LIKE "value1" : {"value1"}
                    std::unique_ptr<EntryArray<std::string, std::string> > strEntries = imap->entrySet(
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
                    hazelcast::util::hz_snprintf(sql, 50, "%s BETWEEN 4 and 7", query::QueryConstants::getKeyAttributeName());
                    entries = imap->entrySet(query::SqlPredicate(sql));
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

                TEST_P(MixedMapAPITest, testEntrySetWithPagingPredicate) {
                    int predSize = 5;
                    const int totalEntries = 25;

                    for (int i = 0; i < totalEntries; ++i) {
                        imap->put<int, int>(i, i);
                    }

                    query::PagingPredicate<int, int> predicate((size_t) predSize);

                    std::unique_ptr<EntryArray<int, int> > values = imap->entrySet(predicate);
                    ASSERT_EQ(predSize, (int) values->size());
                    for (int i = 0; i < predSize; ++i) {
                        std::pair<int, int> expected(i, i);
                        std::pair<int, int> actual(*values->getKey(i), *values->getValue(i));
                        ASSERT_EQ(expected, actual);
                    }

                    values = imap->entrySet(predicate);
                    ASSERT_EQ(predSize, (int) values->size());
                    for (int i = 0; i < predSize; ++i) {
                        std::pair<int, int> expected(i, i);
                        std::pair<int, int> actual(*values->getKey(i), *values->getValue(i));
                        ASSERT_EQ(expected, actual);
                    }

                    predicate.nextPage();
                    values = imap->entrySet(predicate);
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

                    values = imap->entrySet(predicate);
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
                    values = imap->entrySet(predicate);
                    ASSERT_EQ(0, (int) values->size());

                    predicate.setPage(0);
                    values = imap->entrySet(predicate);
                    ASSERT_EQ(predSize, (int) values->size());
                    for (int i = 0; i < predSize; ++i) {
                        std::pair<int, int> expected(i, i);
                        std::pair<int, int> actual(*values->getKey(i), *values->getValue(i));
                        ASSERT_EQ(expected, actual);
                    }

                    predicate.previousPage();
                    ASSERT_EQ(0, (int) predicate.getPage());

                    predicate.setPage(5);
                    values = imap->entrySet(predicate);
                    ASSERT_EQ(0, (int) values->size());

                    predicate.setPage(3);
                    values = imap->entrySet(predicate);
                    ASSERT_EQ(predSize, (int) values->size());
                    for (int i = 0; i < predSize; ++i) {
                        std::pair<int, int> expected(3 * predSize + i, 3 * predSize + i);
                        std::pair<int, int> actual(*values->getKey(i), *values->getValue(i));
                        ASSERT_EQ(expected, actual);
                    }

                    predicate.previousPage();
                    values = imap->entrySet(predicate);
                    ASSERT_EQ(predSize, (int) values->size());
                    for (int i = 0; i < predSize; ++i) {
                        std::pair<int, int> expected(2 * predSize + i, 2 * predSize + i);
                        std::pair<int, int> actual(*values->getKey(i), *values->getValue(i));
                        ASSERT_EQ(expected, actual);
                    }

                    // test PagingPredicate with inner predicate (value < 10)
                    std::unique_ptr<query::Predicate> lessThanTenPredicate(std::unique_ptr<query::Predicate>(
                            new query::GreaterLessPredicate<int>(query::QueryConstants::getValueAttributeName(), 9,
                                                                 false,
                                                                 true)));
                    query::PagingPredicate<int, int> predicate2(lessThanTenPredicate, 5);
                    values = imap->entrySet(predicate2);
                    ASSERT_EQ(predSize, (int) values->size());
                    for (int i = 0; i < predSize; ++i) {
                        std::pair<int, int> expected(i, i);
                        std::pair<int, int> actual(*values->getKey(i), *values->getValue(i));
                        ASSERT_EQ(expected, actual);
                    }

                    predicate2.nextPage();
                    // match values 5,6, 7, 8
                    values = imap->entrySet(predicate2);
                    ASSERT_EQ(predSize - 1, (int) values->size());
                    for (int i = 0; i < predSize - 1; ++i) {
                        std::pair<int, int> expected(predSize + i, predSize + i);
                        std::pair<int, int> actual(*values->getKey(i), *values->getValue(i));
                        ASSERT_EQ(expected, actual);
                    }

                    predicate2.nextPage();
                    values = imap->entrySet(predicate2);
                    ASSERT_EQ(0, (int) values->size());

                    // test paging predicate with comparator
                    Employee empl1("ahmet", 35);
                    Employee empl2("mehmet", 21);
                    Employee empl3("deniz", 25);
                    Employee empl4("ali", 33);
                    Employee empl5("veli", 44);
                    Employee empl6("aylin", 5);

                    imap->put<int, Employee>(3, empl1);
                    imap->put<int, Employee>(4, empl2);
                    imap->put<int, Employee>(5, empl3);
                    imap->put<int, Employee>(6, empl4);
                    imap->put<int, Employee>(7, empl5);
                    imap->put<int, Employee>(8, empl6);

                    predSize = 2;
                    query::PagingPredicate<int, Employee> predicate3(
                            std::unique_ptr<query::EntryComparator<int, Employee> >(new EmployeeEntryComparator()),
                            (size_t) predSize);
                    std::unique_ptr<EntryArray<int, Employee> > result = imap->entrySet(predicate3);
                    ASSERT_EQ(2, (int) result->size());
                    std::pair<int, Employee> expected(8, empl6);
                    std::pair<int, Employee> actual(*result->getKey(0), *result->getValue(0));
                    ASSERT_EQ(expected, actual);
                    expected = std::pair<int, Employee>(4, empl2);
                    actual = std::pair<int, Employee>(*result->getKey(1), *result->getValue(1));
                    ASSERT_EQ(expected, actual);
                }

*/
                TEST_P(MixedMapAPITest, testReplace) {
                    std::unique_ptr<std::string> temp = imap->replace<std::string, std::string>("key1",
                                                                                                "value").get<std::string>();
                    ASSERT_EQ((std::string *) NULL, temp.get());

                    std::string tempKey = "key1";
                    std::string tempValue = "value1";
                    imap->put<std::string, std::string>(tempKey, tempValue);

                    ASSERT_EQ("value1",
                              (*(imap->replace<std::string, std::string>("key1", "value2").get<std::string>())));
                    ASSERT_EQ("value2", *(imap->get<std::string>("key1").get<std::string>()));

                    ASSERT_FALSE((imap->replace<std::string, std::string, std::string>("key1", "value1", "value3")));
                    ASSERT_EQ("value2", *(imap->get<std::string>("key1").get<std::string>()));

                    ASSERT_TRUE((imap->replace<std::string, std::string, std::string>("key1", "value2", "value3")));
                    ASSERT_EQ("value3", (*(imap->get<std::string>("key1").get<std::string>())));
                }

                TEST_P(MixedMapAPITest, testPredicateListenerWithPortableKey) {
                    hazelcast::util::CountDownLatch countDownLatch(1);
                    hazelcast::util::AtomicInt atomicInteger(0);
                    SampleEntryListenerForPortableKey listener(countDownLatch, atomicInteger);
                    Employee key("a", 1);
                    std::string id = imap->addEntryListener(key, listener, true);
                    Employee key2("a", 2);
                    imap->put<Employee, int>(key2, 1);
                    imap->put<Employee, int>(key, 3);
                    ASSERT_TRUE(countDownLatch.await(5));
                    ASSERT_EQ(1, (int) atomicInteger);

                    ASSERT_TRUE(imap->removeEntryListener(id));
                }

                TEST_P(MixedMapAPITest, testListener) {
                    hazelcast::util::CountDownLatch latch1Add(5);
                    hazelcast::util::CountDownLatch latch1Remove(2);
                    hazelcast::util::CountDownLatch dummy(10);
                    hazelcast::util::CountDownLatch latch2Add(1);
                    hazelcast::util::CountDownLatch latch2Remove(1);

                    CountdownListener listener1(latch1Add, latch1Remove, dummy, dummy);
                    CountdownListener listener2(latch2Add, latch2Remove, dummy, dummy);

                    std::string listener1ID = imap->addEntryListener(listener1, false);
                    std::string listener2ID = imap->addEntryListener<std::string>("key3", listener2, true);

                    hazelcast::util::sleep(2);

                    imap->put<std::string, std::string>("key1", "value1");
                    imap->put<std::string, std::string>("key2", "value2");
                    imap->put<std::string, std::string>("key3", "value3");
                    imap->put<std::string, std::string>("key4", "value4");
                    imap->put<std::string, std::string>("key5", "value5");

                    imap->remove<std::string>("key1");
                    imap->remove<std::string>("key3");

                    ASSERT_TRUE(latch1Add.await(10));
                    ASSERT_TRUE(latch1Remove.await(10));
                    ASSERT_TRUE(latch2Add.await(5));
                    ASSERT_TRUE(latch2Remove.await(5));

                    ASSERT_TRUE(imap->removeEntryListener(listener1ID));
                    ASSERT_TRUE(imap->removeEntryListener(listener2ID));

                }

                TEST_P(MixedMapAPITest, testListenerWithTruePredicate) {
                    hazelcast::util::CountDownLatch latchAdd(3);
                    hazelcast::util::CountDownLatch latchRemove(1);
                    hazelcast::util::CountDownLatch latchEvict(1);
                    hazelcast::util::CountDownLatch latchUpdate(1);

                    CountdownListener listener(latchAdd, latchRemove, latchUpdate, latchEvict);

                    std::string listenerId = imap->addEntryListener(listener, query::TruePredicate(), false);

                    imap->put<int, int>(1, 1);
                    imap->put<int, int>(2, 2);
                    imap->put<int, int>(3, 3, 1000); // evict after 1 second
                    imap->remove(2);

                    hazelcast::util::sleep(2);

                    ASSERT_EQ((int *) NULL, imap->get<int>(3).get<int>().get()); // trigger eviction

// update an entry
                    imap->IMap::set<int, int>(1, 5);
                    std::unique_ptr<int> value = imap->get<int>(1).get<int>();
                    ASSERT_NE((int *) NULL, value.get());
                    ASSERT_EQ(5, *value);

                    hazelcast::util::CountDownLatchWaiter latches;
                    latches.add(latchAdd).add(latchRemove).add(latchUpdate).add(latchEvict);
                    ASSERT_TRUE(latches.awaitMillis(2000));

                    ASSERT_TRUE(imap->removeEntryListener(listenerId));
                }

                TEST_P(MixedMapAPITest, testListenerWithFalsePredicate) {
                    hazelcast::util::CountDownLatch latchAdd(3);
                    hazelcast::util::CountDownLatch latchRemove(1);
                    hazelcast::util::CountDownLatch latchEvict(1);
                    hazelcast::util::CountDownLatch latchUpdate(1);

                    CountdownListener listener(latchAdd, latchRemove, latchUpdate, latchEvict);

                    std::string listenerId = imap->addEntryListener(listener, query::FalsePredicate(), false);

                    imap->put<int, int>(1, 1);
                    imap->put<int, int>(2, 2);
                    imap->put<int, int>(3, 3, 1000); // evict after 1 second
                    imap->remove(2);

                    hazelcast::util::sleep(2);

                    ASSERT_EQ((int *) NULL, imap->get<int>(3).get<int>().get()); // trigger eviction

// update an entry
                    imap->IMap::set<int, int>(1, 5);
                    std::unique_ptr<int> value = imap->get<int>(1).get<int>();
                    ASSERT_NE((int *) NULL, value.get());
                    ASSERT_EQ(5, *value);

                    hazelcast::util::CountDownLatchWaiter latches;
                    latches.add(latchAdd).add(latchRemove).add(latchUpdate).add(latchEvict);
                    ASSERT_FALSE(latches.awaitMillis(2000));

                    ASSERT_TRUE(imap->removeEntryListener(listenerId));
                }

                TEST_P(MixedMapAPITest, testListenerWithEqualPredicate) {
                    hazelcast::util::CountDownLatch latchAdd(1);
                    hazelcast::util::CountDownLatch latchRemove(1);
                    hazelcast::util::CountDownLatch latchEvict(1);
                    hazelcast::util::CountDownLatch latchUpdate(1);

                    CountdownListener listener(latchAdd, latchRemove, latchUpdate, latchEvict);

                    std::string listenerId = imap->addEntryListener(listener, query::EqualPredicate<int>(
                            query::QueryConstants::getKeyAttributeName(), 3), true);

                    imap->put<int, int>(1, 1);
                    imap->put<int, int>(2, 2);
                    imap->put<int, int>(3, 3, 1000); // evict after 1 second
                    imap->remove(2);

                    hazelcast::util::sleep(2);

                    ASSERT_EQ((int *) NULL, imap->get<int>(3).get<int>().get()); // trigger eviction

// update an entry
                    imap->IMap::set<int, int>(1, 5);
                    std::unique_ptr<int> value = imap->get<int>(1).get<int>();
                    ASSERT_NE((int *) NULL, value.get());
                    ASSERT_EQ(5, *value);

                    hazelcast::util::CountDownLatchWaiter latches;
                    latches.add(latchAdd).add(latchEvict);
                    ASSERT_TRUE(latches.awaitMillis(2000));

                    latches.reset();
                    latches.add(latchUpdate).add(latchRemove);
                    ASSERT_FALSE(latches.awaitMillis(2000));

                    ASSERT_TRUE(imap->removeEntryListener(listenerId));
                }

                TEST_P(MixedMapAPITest, testListenerWithNotEqualPredicate) {
                    hazelcast::util::CountDownLatch latchAdd(2);
                    hazelcast::util::CountDownLatch latchRemove(1);
                    hazelcast::util::CountDownLatch latchEvict(1);
                    hazelcast::util::CountDownLatch latchUpdate(1);

                    CountdownListener listener(latchAdd, latchRemove, latchUpdate, latchEvict);

                    std::string listenerId = imap->addEntryListener(listener, query::NotEqualPredicate<int>(
                            query::QueryConstants::getKeyAttributeName(), 3), true);

                    imap->put<int, int>(1, 1);
                    imap->put<int, int>(2, 2);
                    imap->put<int, int>(3, 3, 1000); // evict after 1 second
                    imap->remove(2);

                    hazelcast::util::sleep(2);

                    ASSERT_EQ((int *) NULL, imap->get<int>(3).get<int>().get()); // trigger eviction

// update an entry
                    imap->IMap::set<int, int>(1, 5);
                    std::unique_ptr<int> value = imap->get<int>(1).get<int>();
                    ASSERT_NE((int *) NULL, value.get());
                    ASSERT_EQ(5, *value);

                    hazelcast::util::CountDownLatchWaiter latches;
                    latches.add(latchAdd).add(latchRemove).add(latchUpdate);
                    ASSERT_TRUE(latches.awaitMillis(2000));

                    latches.reset();
                    latches.add(latchEvict);
                    ASSERT_FALSE(latches.awaitMillis(2000));

                    ASSERT_TRUE(imap->removeEntryListener(listenerId));
                }

                TEST_P(MixedMapAPITest, testListenerWithGreaterLessPredicate) {
                    hazelcast::util::CountDownLatch latchAdd(2);
                    hazelcast::util::CountDownLatch latchRemove(1);
                    hazelcast::util::CountDownLatch latchEvict(1);
                    hazelcast::util::CountDownLatch latchUpdate(1);

                    CountdownListener listener(latchAdd, latchRemove, latchUpdate, latchEvict);

// key <= 2
                    std::string listenerId = imap->addEntryListener(listener, query::GreaterLessPredicate<int>(
                            query::QueryConstants::getKeyAttributeName(), 2, true, true), false);

                    imap->put<int, int>(1, 1);
                    imap->put<int, int>(2, 2);
                    imap->put<int, int>(3, 3, 1000); // evict after 1 second
                    imap->remove(2);

                    hazelcast::util::sleep(2);

                    ASSERT_EQ((int *) NULL, imap->get<int>(3).get<int>().get()); // trigger eviction

// update an entry
                    imap->IMap::set<int, int>(1, 5);
                    std::unique_ptr<int> value = imap->get<int>(1).get<int>();
                    ASSERT_NE((int *) NULL, value.get());
                    ASSERT_EQ(5, *value);

                    hazelcast::util::CountDownLatchWaiter latches;
                    latches.add(latchAdd).add(latchRemove).add(latchUpdate);
                    ASSERT_TRUE(latches.awaitMillis(2000));

                    ASSERT_FALSE(latchEvict.awaitMillis(2000));

                    ASSERT_TRUE(imap->removeEntryListener(listenerId));
                }

                TEST_P(MixedMapAPITest, testListenerWithBetweenPredicate) {
                    hazelcast::util::CountDownLatch latchAdd(2);
                    hazelcast::util::CountDownLatch latchRemove(1);
                    hazelcast::util::CountDownLatch latchEvict(1);
                    hazelcast::util::CountDownLatch latchUpdate(1);

                    CountdownListener listener(latchAdd, latchRemove, latchUpdate, latchEvict);

// 1 <=key <= 2
                    std::string listenerId = imap->addEntryListener(listener, query::BetweenPredicate<int>(
                            query::QueryConstants::getKeyAttributeName(), 1, 2), true);

                    imap->put<int, int>(1, 1);
                    imap->put<int, int>(2, 2);
                    imap->put<int, int>(3, 3, 1000); // evict after 1 second
                    imap->remove(2);

                    hazelcast::util::sleep(2);

                    ASSERT_EQ((int *) NULL, imap->get<int>(3).get<int>().get()); // trigger eviction

// update an entry
                    imap->IMap::set<int, int>(1, 5);
                    std::unique_ptr<int> value = imap->get<int>(1).get<int>();
                    ASSERT_NE((int *) NULL, value.get());
                    ASSERT_EQ(5, *value);

                    hazelcast::util::CountDownLatchWaiter latches;
                    latches.add(latchAdd).add(latchRemove).add(latchUpdate);
                    ASSERT_TRUE(latches.awaitMillis(2000));

                    ASSERT_FALSE(latchEvict.awaitMillis(2000));

                    ASSERT_TRUE(imap->removeEntryListener(listenerId));
                }

                TEST_P(MixedMapAPITest, testListenerWithSqlPredicate) {
                    hazelcast::util::CountDownLatch latchAdd(1);
                    hazelcast::util::CountDownLatch latchRemove(1);
                    hazelcast::util::CountDownLatch latchEvict(1);
                    hazelcast::util::CountDownLatch latchUpdate(1);

                    CountdownListener listener(latchAdd, latchRemove, latchUpdate, latchEvict);

// 1 <=key <= 2
                    std::string listenerId = imap->addEntryListener(listener, query::SqlPredicate("__key < 2"), true);

                    imap->put<int, int>(1, 1);
                    imap->put<int, int>(2, 2);
                    imap->put<int, int>(3, 3, 1000); // evict after 1 second
                    imap->remove(2);

                    hazelcast::util::sleep(2);

                    ASSERT_EQ((int *) NULL, imap->get<int>(3).get<int>().get()); // trigger eviction

// update an entry
                    imap->IMap::set<int, int>(1, 5);
                    std::unique_ptr<int> value = imap->get<int>(1).get<int>();
                    ASSERT_NE((int *) NULL, value.get());
                    ASSERT_EQ(5, *value);

                    hazelcast::util::CountDownLatchWaiter latches;
                    latches.add(latchAdd).add(latchUpdate);
                    ASSERT_TRUE(latches.awaitMillis(2000));

                    latches.reset();
                    latches.add(latchRemove).add(latchEvict);
                    ASSERT_FALSE(latches.awaitMillis(2000));

                    ASSERT_TRUE(imap->removeEntryListener(listenerId));
                }

                TEST_P(MixedMapAPITest, testListenerWithRegExPredicate) {
                    hazelcast::util::CountDownLatch latchAdd(2);
                    hazelcast::util::CountDownLatch latchRemove(1);
                    hazelcast::util::CountDownLatch latchEvict(1);
                    hazelcast::util::CountDownLatch latchUpdate(1);

                    CountdownListener listener(latchAdd, latchRemove, latchUpdate,
                                               latchEvict);

// key matches any word containing ".*met.*"
                    std::string listenerId = imap->addEntryListener(listener, query::RegexPredicate(
                            query::QueryConstants::getKeyAttributeName(), ".*met.*"), true);

                    imap->put<std::string, std::string>("ilkay", "yasar");
                    imap->put<std::string, std::string>("mehmet", "demir");
                    imap->put<std::string, std::string>("metin", "ozen", 1000); // evict after 1 second
                    imap->put<std::string, std::string>("hasan", "can");
                    imap->remove<std::string>("mehmet");

                    hazelcast::util::sleep(2);

                    ASSERT_EQ((std::string *) NULL,
                              imap->get<std::string>("metin").get<std::string>().get()); // trigger eviction

// update an entry
                    imap->IMap::set<std::string, std::string>("hasan", "suphi");
                    std::unique_ptr<std::string> value = imap->get<std::string>("hasan").get<std::string>();
                    ASSERT_NE((std::string *) NULL, value.get());
                    ASSERT_EQ("suphi", *value);

                    hazelcast::util::CountDownLatchWaiter latches;
                    latches.add(latchAdd).add(latchRemove).add(latchEvict);
                    ASSERT_TRUE(latches.awaitMillis(2000));

                    ASSERT_FALSE(latchUpdate.awaitMillis(2000));

                    ASSERT_TRUE(imap->removeEntryListener(listenerId));
                }

                TEST_P(MixedMapAPITest, testListenerWithInstanceOfPredicate) {
                    hazelcast::util::CountDownLatch latchAdd(3);
                    hazelcast::util::CountDownLatch latchRemove(1);
                    hazelcast::util::CountDownLatch latchEvict(1);
                    hazelcast::util::CountDownLatch latchUpdate(1);

                    CountdownListener listener(latchAdd, latchRemove, latchUpdate, latchEvict);

// 1 <=key <= 2
                    std::string listenerId = imap->addEntryListener(listener,
                                                                    query::InstanceOfPredicate("java.lang.Integer"),
                                                                    false);

                    imap->put<int, int>(1, 1);
                    imap->put<int, int>(2, 2);
                    imap->put<int, int>(3, 3, 1000); // evict after 1 second
                    imap->remove(2);

                    hazelcast::util::sleep(2);

                    ASSERT_EQ((int *) NULL, imap->get<int>(3).get<int>().get()); // trigger eviction

// update an entry
                    imap->IMap::set<int, int>(1, 5);
                    std::unique_ptr<int> value = imap->get<int>(1).get<int>();
                    ASSERT_NE((int *) NULL, value.get());
                    ASSERT_EQ(5, *value);

                    hazelcast::util::CountDownLatchWaiter latches;
                    latches.add(latchAdd).add(latchRemove).add(latchUpdate).add(latchEvict);
                    ASSERT_TRUE(latches.awaitMillis(2000));

                    ASSERT_TRUE(imap->removeEntryListener(listenerId));
                }

                TEST_P(MixedMapAPITest, testListenerWithNotPredicate) {
                    hazelcast::util::CountDownLatch latchAdd(2);
                    hazelcast::util::CountDownLatch latchRemove(1);
                    hazelcast::util::CountDownLatch latchEvict(1);
                    hazelcast::util::CountDownLatch latchUpdate(1);

                    CountdownListener listener(latchAdd, latchRemove, latchUpdate, latchEvict);

// key >= 3
                    std::unique_ptr<query::Predicate> greaterLessPred = std::unique_ptr<query::Predicate>(
                            new query::GreaterLessPredicate<int>(query::QueryConstants::getKeyAttributeName(), 3, true,
                                                                 false));
                    query::NotPredicate notPredicate(greaterLessPred);
                    std::string listenerId = imap->addEntryListener(listener, notPredicate, false);

                    imap->put<int, int>(1, 1);
                    imap->put<int, int>(2, 2);
                    imap->put<int, int>(3, 3, 1000); // evict after 1 second
                    imap->remove(2);

                    hazelcast::util::sleep(2);

                    ASSERT_EQ((int *) NULL, imap->get<int>(3).get<int>().get()); // trigger eviction

// update an entry
                    imap->IMap::set<int, int>(1, 5);
                    std::unique_ptr<int> value = imap->get<int>(1).get<int>();
                    ASSERT_NE((int *) NULL, value.get());
                    ASSERT_EQ(5, *value);

                    hazelcast::util::CountDownLatchWaiter latches;
                    latches.add(latchAdd).add(latchRemove).add(latchUpdate);
                    ASSERT_TRUE(latches.awaitMillis(2000));

                    latches.reset();
                    latches.add(latchEvict);
                    ASSERT_FALSE(latches.awaitMillis(1000));

                    ASSERT_TRUE(imap->removeEntryListener(listenerId));
                }

                TEST_P(MixedMapAPITest, testListenerWithAndPredicate) {
                    hazelcast::util::CountDownLatch latchAdd(1);
                    hazelcast::util::CountDownLatch latchRemove(1);
                    hazelcast::util::CountDownLatch latchEvict(1);
                    hazelcast::util::CountDownLatch latchUpdate(1);

                    CountdownListener listener(latchAdd, latchRemove, latchUpdate, latchEvict);

// key < 3
                    std::unique_ptr<query::Predicate> greaterLessPred = std::unique_ptr<query::Predicate>(
                            new query::GreaterLessPredicate<int>(query::QueryConstants::getKeyAttributeName(), 3, false,
                                                                 true));
// value == 1
                    std::unique_ptr<query::Predicate> equalPred = std::unique_ptr<query::Predicate>(
                            new query::EqualPredicate<int>(query::QueryConstants::getKeyAttributeName(), 1));
                    query::AndPredicate predicate;
// key < 3 AND key == 1 --> (1, 1)
                    predicate.add(greaterLessPred).add(equalPred);
                    std::string listenerId = imap->addEntryListener(listener, predicate, false);

                    imap->put<int, int>(1, 1);
                    imap->put<int, int>(2, 2);
                    imap->put<int, int>(3, 3, 1000); // evict after 1 second
                    imap->remove(2);

                    hazelcast::util::sleep(2);

                    ASSERT_EQ((int *) NULL, imap->get<int>(3).get<int>().get()); // trigger eviction

// update an entry
                    imap->IMap::set<int, int>(1, 5);
                    std::unique_ptr<int> value = imap->get<int>(1).get<int>();
                    ASSERT_NE((int *) NULL, value.get());
                    ASSERT_EQ(5, *value);

                    hazelcast::util::CountDownLatchWaiter latches;
                    latches.add(latchAdd).add(latchUpdate);
                    ASSERT_TRUE(latches.awaitMillis(2000));

                    latches.reset();
                    latches.add(latchEvict).add(latchRemove);
                    ASSERT_FALSE(latches.awaitMillis(1000));

                    ASSERT_TRUE(imap->removeEntryListener(listenerId));
                }

                TEST_P(MixedMapAPITest, testListenerWithOrPredicate) {
                    hazelcast::util::CountDownLatch latchAdd(2);
                    hazelcast::util::CountDownLatch latchRemove(1);
                    hazelcast::util::CountDownLatch latchEvict(1);
                    hazelcast::util::CountDownLatch latchUpdate(1);

                    CountdownListener listener(latchAdd, latchRemove, latchUpdate, latchEvict);

// key >= 3
                    std::unique_ptr<query::Predicate> greaterLessPred = std::unique_ptr<query::Predicate>(
                            new query::GreaterLessPredicate<int>(query::QueryConstants::getKeyAttributeName(), 3, true,
                                                                 false));
// value == 1
                    std::unique_ptr<query::Predicate> equalPred = std::unique_ptr<query::Predicate>(
                            new query::EqualPredicate<int>(query::QueryConstants::getValueAttributeName(), 2));
                    query::OrPredicate predicate;
// key >= 3 OR value == 2 --> (1, 1), (2, 2)
                    predicate.add(greaterLessPred).add(equalPred);
                    std::string listenerId = imap->addEntryListener(listener, predicate, true);

                    imap->put<int, int>(1, 1);
                    imap->put<int, int>(2, 2);
                    imap->put<int, int>(3, 3, 1000); // evict after 1 second
                    imap->remove(2);

                    hazelcast::util::sleep(2);

                    ASSERT_EQ((int *) NULL, imap->get<int>(3).get<int>().get()); // trigger eviction

// update an entry
                    imap->IMap::set<int, int>(1, 5);
                    std::unique_ptr<int> value = imap->get<int>(1).get<int>();
                    ASSERT_NE((int *) NULL, value.get());
                    ASSERT_EQ(5, *value);

                    hazelcast::util::CountDownLatchWaiter latches;
                    latches.add(latchAdd).add(latchEvict).add(latchRemove);
                    ASSERT_TRUE(latches.awaitMillis(2000));

                    ASSERT_FALSE(latchUpdate.awaitMillis(2000));

                    ASSERT_TRUE(imap->removeEntryListener(listenerId));
                }

                TEST_P(MixedMapAPITest, testClearEvent) {
                    hazelcast::util::CountDownLatch latch(1);
                    ClearListener clearListener(latch);
                    std::string listenerId = imap->addEntryListener(clearListener, false);
                    imap->put<std::string, std::string>("key1", "value1");
                    imap->clear();
                    ASSERT_TRUE(latch.await(120));
                    imap->removeEntryListener(listenerId);
                }

                TEST_P(MixedMapAPITest, testEvictAllEvent) {
                    hazelcast::util::CountDownLatch latch(1);
                    EvictListener evictListener(latch);
                    std::string listenerId = imap->addEntryListener(evictListener, false);
                    imap->put<std::string, std::string>("key1", "value1");
                    imap->evictAll();
                    ASSERT_TRUE(latch.await(120));
                    imap->removeEntryListener(listenerId);
                }

                TEST_P(MixedMapAPITest, testBasicPredicate) {
                    fillMap();

                    query::SqlPredicate predicate("this = 'value1'");
                    std::vector<TypedData> datas = imap->values(predicate);

                    std::unique_ptr<std::string> actualVal = datas[0].get<std::string>();
                    ASSERT_NE((std::string *) NULL, actualVal.get());
                    ASSERT_EQ("value1", *actualVal);

                    datas = imap->keySet(predicate);

                    actualVal = datas[0].get<std::string>();
                    ASSERT_NE((std::string *) NULL, actualVal.get());
                    ASSERT_EQ("key1", *actualVal);


                    std::vector<std::pair<TypedData, TypedData> > entries = imap->entrySet(predicate);
                    actualVal = entries[0].first.get<std::string>();
                    ASSERT_NE((std::string *) NULL, actualVal.get());
                    ASSERT_EQ("key1", *actualVal);

                    actualVal = entries[0].second.get<std::string>();
                    ASSERT_NE((std::string *) NULL, actualVal.get());
                    ASSERT_EQ("value1", *actualVal);
                }

                TEST_P(MixedMapAPITest, testKeySetAndValuesWithPredicates) {
                    Employee emp1("abc-123-xvz", 34);
                    Employee emp2("abc-123-xvz", 20);

                    imap->put<Employee, Employee>(emp1, emp1);
                    ASSERT_EQ((Employee *) NULL, (imap->put<Employee, Employee>(emp2, emp2).get<Employee>().get()));
                    ASSERT_EQ(2, (int) imap->size());
                    ASSERT_EQ(2, (int) imap->keySet().size());
                    query::SqlPredicate predicate("a = 10");
                    ASSERT_EQ(0, (int) imap->keySet(predicate).size());
                    query::SqlPredicate predicate2("a = 10");
                    ASSERT_EQ(0, (int) imap->values(predicate2).size());
                    query::SqlPredicate predicate3("a >= 10");
                    ASSERT_EQ(2, (int) imap->keySet(predicate3).size());
                    ASSERT_EQ(2, (int) imap->values(predicate3).size());
                    ASSERT_EQ(2, (int) imap->size());
                    ASSERT_EQ(2, (int) imap->values().size());
                }

                TEST_P(MixedMapAPITest, testMapWithPortable) {
                    std::unique_ptr<Employee> n1 = imap->get<int>(1).get<Employee>();
                    ASSERT_EQ(n1.get(), (Employee *) NULL);
                    Employee employee("sancar", 24);
                    std::unique_ptr<Employee> ptr = imap->put<int, Employee>(1, employee).get<Employee>();
                    ASSERT_EQ(ptr.get(), (Employee *) NULL);
                    ASSERT_FALSE(imap->isEmpty());
                    std::unique_ptr<EntryView<TypedData, TypedData> > view = imap->getEntryView(1);
                    ASSERT_NE((EntryView<TypedData, TypedData> *) NULL, view.get());
                    ASSERT_EQ(employee, *(view->value.get<Employee>()));
                    ASSERT_EQ(1, *(view->key.get<int>()));

                    imap->addIndex("a", true);
                    imap->addIndex("n", false);
                }

                TEST_P(MixedMapAPITest, testMapStoreRelatedRequests) {
                    imap->putTransient<std::string, std::string>("ali", "veli", 1100);
                    imap->flush();
                    ASSERT_EQ(1, imap->size());
                    ASSERT_FALSE(imap->evict<std::string>("deli"));
                    ASSERT_TRUE(imap->evict<std::string>("ali"));
                    ASSERT_EQ((std::string *) NULL, imap->get<std::string>("ali").get<std::string>().get());
                }

                TEST_P(MixedMapAPITest, testExecuteOnKey) {
                    Employee empl1("ahmet", 35);
                    Employee empl2("mehmet", 21);

                    imap->put<int, Employee>(3, empl1);
                    imap->put<int, Employee>(4, empl2);

                    EntryMultiplier processor(4);

                    std::unique_ptr<int> result = imap->executeOnKey<int, EntryMultiplier>(4, processor).get<int>();

                    ASSERT_NE((int *) NULL, result.get());
                    ASSERT_EQ(4 * processor.getMultiplier(), *result);
                }

                TEST_P(MixedMapAPITest, testExecuteOnNonExistentKey) {
                    EntryMultiplier processor(4);

                    std::unique_ptr<int> result = imap->executeOnKey<int, EntryMultiplier>(17, processor).get<int>();

                    ASSERT_NE((int *) NULL, result.get());
                    ASSERT_EQ(-1, *result);
                }

                TEST_P(MixedMapAPITest, testSubmitToKey) {
                    Employee empl1("ahmet", 35);
                    Employee empl2("mehmet", 21);

                    imap->put<int, Employee>(3, empl1);
                    imap->put<int, Employee>(4, empl2);

// Waits at the server side before running the operation
                    WaitMultiplierProcessor processor(3000, 4);

                    hazelcast::client::Future<TypedData> initialFuture = imap->submitToKey<int, WaitMultiplierProcessor>(
                            4, processor);

// Should invalidate the initialFuture
                    hazelcast::client::Future<TypedData> future = initialFuture;

                    ASSERT_FALSE(initialFuture.valid());
                    ASSERT_THROW(initialFuture.wait_for(1000), exception::FutureUninitialized);
                    ASSERT_TRUE(future.valid());

                    future_status status = future.wait_for(1 * 1000);
                    ASSERT_EQ(future_status::timeout, status);
                    ASSERT_TRUE(future.valid());

                    status = future.wait_for(3 * 1000);
                    ASSERT_EQ(future_status::ready, status);
                    TypedData result = future.get();
                    ASSERT_EQ(4 * processor.getMultiplier(), *result.get<int>());
                    ASSERT_FALSE(future.valid());
                }

                TEST_P(MixedMapAPITest, testSubmitToKeyMultipleAsyncCalls) {
                    Employee empl1("ahmet", 35);
                    Employee empl2("mehmet", 21);

                    imap->put<int, Employee>(3, empl1);
                    imap->put<int, Employee>(4, empl2);

                    int waitTimeInMillis = 500;

// Waits at the server side before running the operation
                    WaitMultiplierProcessor processor(waitTimeInMillis, 4);

                    std::vector<hazelcast::client::Future<TypedData> > allFutures;

// test putting into a vector of futures
                    hazelcast::client::Future<TypedData> future = imap->submitToKey<int, WaitMultiplierProcessor>(3,
                                                                                                                  processor);
                    allFutures.push_back(future);

// test re-assigning a future and putting into the vector
                    future = imap->submitToKey<int, WaitMultiplierProcessor>(
                            3, processor);
                    allFutures.push_back(future);

// test submitting a non-existent key
                    allFutures.push_back(imap->submitToKey<int, WaitMultiplierProcessor>(
                            99, processor));

                    for (std::vector<hazelcast::client::Future<TypedData> >::const_iterator it = allFutures.begin();
                         it != allFutures.end(); ++it) {
                        future_status status = (*it).wait_for(2 * waitTimeInMillis);
                        ASSERT_EQ(future_status::ready, status);
                    }

                    for (std::vector<hazelcast::client::Future<TypedData> >::iterator it = allFutures.begin();
                         it != allFutures.end(); ++it) {
                        TypedData result = (*it).get();
                        ASSERT_NE((int *) NULL, result.get<int>().get());
                        ASSERT_FALSE((*it).valid());
                    }
                }

                TEST_P(MixedMapAPITest, testExecuteOnKeys) {
                    Employee empl1("ahmet", 35);
                    Employee empl2("mehmet", 21);
                    Employee empl3("deniz", 25);

                    imap->put<int, Employee>(3, empl1);
                    imap->put<int, Employee>(4, empl2);
                    imap->put<int, Employee>(5, empl3);

                    EntryMultiplierWithNullableResult processor(4);

                    std::set<int> keys;
                    keys.insert(3);
                    keys.insert(5);
// put non existent key
                    keys.insert(999);

                    std::map<int, TypedData> result = imap->executeOnKeys<int, EntryMultiplier>(keys, processor);

                    ASSERT_EQ(2, (int) result.size());
                    ASSERT_NE(result.end(), result.find(3));
                    ASSERT_NE(result.end(), result.find(5));
                    ASSERT_EQ(result.end(), result.find(999));
                    ASSERT_EQ(3 * processor.getMultiplier(), *result[3].get<int>());
                    ASSERT_EQ(5 * processor.getMultiplier(), *result[5].get<int>());
                }

                TEST_P(MixedMapAPITest, testExecuteOnEntries) {
                    Employee empl1("ahmet", 35);
                    Employee empl2("mehmet", 21);
                    Employee empl3("deniz", 25);

                    imap->put<int, Employee>(3, empl1);
                    imap->put<int, Employee>(4, empl2);
                    imap->put<int, Employee>(5, empl3);

                    EntryMultiplier processor(4);

                    std::map<TypedData, TypedData> result = imap->executeOnEntries<EntryMultiplier>(processor);

                    ASSERT_EQ(3, (int) result.size());
                    for (std::map<TypedData, TypedData>::const_iterator it = result.begin(); it != result.end(); ++it) {
                        std::unique_ptr<int> key = (*it).first.get<int>();
                        std::unique_ptr<int> value = (*it).second.get<int>();
                        ASSERT_TRUE(*key == 3 || *key == 4 || *key == 5);
                        ASSERT_EQ((*key) * processor.getMultiplier(), (*value));
                    }
                }

                TEST_P(MixedMapAPITest, testExecuteOnEntriesWithTruePredicate) {
                    Employee empl1("ahmet", 35);
                    Employee empl2("mehmet", 21);
                    Employee empl3("deniz", 25);

                    imap->put<int, Employee>(3, empl1);
                    imap->put<int, Employee>(4, empl2);
                    imap->put<int, Employee>(5, empl3);

                    EntryMultiplier processor(4);

                    std::map<TypedData, TypedData> result = imap->executeOnEntries<EntryMultiplier>(processor,
                                                                                                    query::TruePredicate());

                    ASSERT_EQ(3, (int) result.size());
                    for (std::map<TypedData, TypedData>::const_iterator it = result.begin(); it != result.end(); ++it) {
                        std::unique_ptr<int> key = (*it).first.get<int>();
                        std::unique_ptr<int> value = (*it).second.get<int>();
                        ASSERT_TRUE(*key == 3 || *key == 4 || *key == 5);
                        ASSERT_EQ((*key) * processor.getMultiplier(), (*value));
                    }
                }

                TEST_P(MixedMapAPITest, testExecuteOnEntriesWithFalsePredicate) {
                    Employee empl1("ahmet", 35);
                    Employee empl2("mehmet", 21);
                    Employee empl3("deniz", 25);

                    imap->put<int, Employee>(3, empl1);
                    imap->put<int, Employee>(4, empl2);
                    imap->put<int, Employee>(5, empl3);

                    EntryMultiplier processor(4);

                    std::map<TypedData, TypedData> result = imap->executeOnEntries<EntryMultiplier>(processor,
                                                                                                    query::FalsePredicate());

                    ASSERT_EQ(0, (int) result.size());
                }

                TEST_P(MixedMapAPITest, testExecuteOnEntriesWithAndPredicate) {
                    Employee empl1("ahmet", 35);
                    Employee empl2("mehmet", 21);
                    Employee empl3("deniz", 25);

                    imap->put<int, Employee>(3, empl1);
                    imap->put<int, Employee>(4, empl2);
                    imap->put<int, Employee>(5, empl3);

                    query::AndPredicate andPredicate;
/* 25 <= age <= 35 AND age = 35 */
                    andPredicate.add(
                            std::unique_ptr<query::Predicate>(new query::BetweenPredicate<int>("a", 25, 35))).add(
                            std::unique_ptr<query::Predicate>(
                                    new query::NotPredicate(
                                            std::unique_ptr<query::Predicate>(
                                                    new query::EqualPredicate<int>("a", 35)))));

                    EntryMultiplier processor(4);

                    std::map<TypedData, TypedData> result = imap->executeOnEntries<EntryMultiplier>(processor,
                                                                                                    andPredicate);

                    ASSERT_EQ(1, (int) result.size());
                    ASSERT_EQ(5, *result.begin()->first.get<int>());
                    ASSERT_EQ(5 * processor.getMultiplier(), *result.begin()->second.get<int>());
                }

                TEST_P(MixedMapAPITest, testExecuteOnEntriesWithOrPredicate) {
                    Employee empl1("ahmet", 35);
                    Employee empl2("mehmet", 21);
                    Employee empl3("deniz", 25);

                    imap->put<int, Employee>(3, empl1);
                    imap->put<int, Employee>(4, empl2);
                    imap->put<int, Employee>(5, empl3);

                    query::OrPredicate orPredicate;
/* age == 21 OR age > 25 */
                    orPredicate.add(
                            std::unique_ptr<query::Predicate>(new query::EqualPredicate<int>("a", 21))).add(
                            std::unique_ptr<query::Predicate>(
                                    new query::GreaterLessPredicate<int>("a", 25, false, false)));

                    EntryMultiplier processor(4);

                    std::map<TypedData, TypedData> result = imap->executeOnEntries<EntryMultiplier>(processor,
                                                                                                    orPredicate);

                    ASSERT_EQ(2, (int) result.size());
                    std::map<hazelcast::client::TypedData, hazelcast::client::TypedData>::iterator iterator = result.begin();
                    std::unique_ptr<int> firstKey = (*iterator).first.get<int>();
                    if (3 == *firstKey) {
                        ASSERT_EQ(3 * processor.getMultiplier(), *(*iterator).second.get<int>());
                        ++iterator;
                        ASSERT_EQ(4, *(*iterator).first.get<int>());
                        ASSERT_EQ(4 * processor.getMultiplier(), *(*iterator).second.get<int>());
                    } else {
                        ASSERT_EQ(4 * processor.getMultiplier(), *(*iterator).second.get<int>());
                        ++iterator;
                        ASSERT_EQ(3, *(*iterator).first.get<int>());
                        ASSERT_EQ(3 * processor.getMultiplier(), *(*iterator).second.get<int>());
                    }
                }

                TEST_P(MixedMapAPITest, testAddInterceptor) {
                    std::string prefix("My Prefix");
                    MapGetInterceptor interceptor(prefix);
                    imap->addInterceptor<MapGetInterceptor>(interceptor);

                    std::unique_ptr<std::string> val = imap->get<std::string>("nonexistent").get<std::string>();
                    ASSERT_NE((std::string *) NULL, val.get());
                    ASSERT_EQ(prefix, *val);

                    val = imap->put<std::string, std::string>("key1", "value1").get<std::string>();
                    ASSERT_EQ((std::string *) NULL, val.get());

                    val = imap->get<std::string>("key1").get<std::string>();
                    ASSERT_NE((std::string *) NULL, val.get());
                    ASSERT_EQ(prefix + "value1", *val);
                }

                TEST_P(MixedMapAPITest, testReadUTFWrittenByJava) {
                    std::string value = "xyzä123 イロハニホヘト チリヌルヲ ワカヨタレソ ツネナラム";
                    std::string key = "myutfkey";
                    imap->put<std::string, std::string>(key, value);
                    UTFValueValidatorProcessor processor;
                    TypedData result = imap->executeOnKey<std::string, UTFValueValidatorProcessor>(key, processor);
                    std::unique_ptr<bool> val = result.get<bool>();
                    ASSERT_NOTNULL(val.get(), bool);
                    ASSERT_TRUE(*val);
                }
            }
        }
    }
}





namespace hazelcast {
    namespace client {
        namespace test {
            class ClientSemaphoreTest : public ClientTestSupport {
            protected:
                virtual void SetUp() {
                }

                virtual void TearDown() {
                }

                static void SetUpTestCase() {
                    instance = new HazelcastServer(*g_srvFactory);
                    client = new HazelcastClient(getConfig());
                }

                static void TearDownTestCase() {
                    delete client;
                    delete instance;

                    client = NULL;
                    instance = NULL;
                }

                static HazelcastServer *instance;
                static HazelcastClient *client;
            };

            HazelcastServer *ClientSemaphoreTest::instance = NULL;
            HazelcastClient *ClientSemaphoreTest::client = NULL;

            void testAcquireThread(hazelcast::util::ThreadArgs& args) {
                ISemaphore *s = (ISemaphore *) args.arg0;
                hazelcast::util::CountDownLatch *latch = (hazelcast::util::CountDownLatch *) args.arg1;
                s->acquire();
                latch->countDown();
            }

            TEST_F(ClientSemaphoreTest, testSemaphoreInit) {
                ISemaphore semaphore = client->getISemaphore(randomString());
                ASSERT_TRUE(semaphore.init(10));
            }

            TEST_F(ClientSemaphoreTest, testSemaphoreNegInit) {
                ISemaphore semaphore = client->getISemaphore(randomString());
                ASSERT_THROW(semaphore.init(-1), exception::IllegalArgumentException);
            }

            TEST_F(ClientSemaphoreTest, testRelease) {
                ISemaphore semaphore = client->getISemaphore(randomString());
                semaphore.init(0);
                semaphore.release();
                ASSERT_EQ(1, semaphore.availablePermits());
            }

            TEST_F(ClientSemaphoreTest, testdrainPermits) {
                ISemaphore semaphore = client->getISemaphore(randomString());
                semaphore.init(10);
                ASSERT_EQ(10, semaphore.drainPermits());
            }

            TEST_F(ClientSemaphoreTest, testAvailablePermits_AfterDrainPermits) {
                ISemaphore semaphore = client->getISemaphore(randomString());
                semaphore.init(10);
                semaphore.drainPermits();
                ASSERT_EQ(0, semaphore.availablePermits());
            }

            TEST_F(ClientSemaphoreTest, testTryAcquire_whenDrainPermits) {
                ISemaphore semaphore = client->getISemaphore(randomString());
                semaphore.init(10);
                semaphore.drainPermits();
                ASSERT_FALSE(semaphore.tryAcquire());
            }

            TEST_F(ClientSemaphoreTest, testAvailablePermits) {
                ISemaphore semaphore = client->getISemaphore(randomString());
                semaphore.init(10);
                ASSERT_EQ(10, semaphore.availablePermits());
            }

            TEST_F(ClientSemaphoreTest, testAvailableReducePermits) {
                ISemaphore semaphore = client->getISemaphore(randomString());
                semaphore.init(10);
                semaphore.reducePermits(5);
                ASSERT_EQ(5, semaphore.availablePermits());
            }

            TEST_F(ClientSemaphoreTest, testAvailableReducePermits_WhenZero) {
                ISemaphore semaphore = client->getISemaphore(randomString());
                semaphore.init(0);
                semaphore.reducePermits(1);
                ASSERT_EQ(-1, semaphore.availablePermits());
            }

            TEST_F(ClientSemaphoreTest, testAvailableIncreasePermits) {
                ISemaphore semaphore = client->getISemaphore(randomString());
                semaphore.init(10);
                semaphore.drainPermits();
                semaphore.increasePermits(5);
                ASSERT_EQ(5, semaphore.availablePermits());
            }

            TEST_F(ClientSemaphoreTest, testSimpleAcquire) {
                ISemaphore semaphore = client->getISemaphore("testSimpleAcquire");
                int numberOfPermits = 20;
                ASSERT_TRUE(semaphore.init(numberOfPermits));
                for (int i = 0; i < numberOfPermits; i++) {
                    ASSERT_EQ(numberOfPermits - i, semaphore.availablePermits());
                    semaphore.acquire();
                }

                ASSERT_EQ(semaphore.availablePermits(), 0);
            }

            TEST_F(ClientSemaphoreTest, testAcquire) {
                ISemaphore semaphore = client->getISemaphore("testAcquire");
                semaphore.init(10);
                ASSERT_EQ(10, semaphore.drainPermits());

                hazelcast::util::CountDownLatch latch(1);
                hazelcast::util::StartedThread t(testAcquireThread, &semaphore, &latch);

                hazelcast::util::sleep(1);

                semaphore.release(2);
                ASSERT_TRUE(latch.await(10));
                ASSERT_EQ(1, semaphore.availablePermits());

            }

            void testTryAcquireThread(hazelcast::util::ThreadArgs &args) {
                ISemaphore *s = (ISemaphore *) args.arg0;
                hazelcast::util::CountDownLatch *latch = (hazelcast::util::CountDownLatch *) args.arg1;
                if (s->tryAcquire(2, 5 * 1000)) {
                    latch->countDown();
                }
            }

            TEST_F(ClientSemaphoreTest, testTryAcquire) {
                ISemaphore semaphore = client->getISemaphore("testTryAcquire");
                semaphore.init(10);
                ASSERT_TRUE(semaphore.tryAcquire());
                ASSERT_TRUE(semaphore.tryAcquire(9));
                ASSERT_EQ(0, semaphore.availablePermits());
                ASSERT_FALSE(semaphore.tryAcquire(1 * 1000));
                ASSERT_FALSE(semaphore.tryAcquire(2, 1 * 1000));

                hazelcast::util::CountDownLatch latch(1);

                hazelcast::util::StartedThread t(testTryAcquireThread, &semaphore, &latch);

                semaphore.release(2);
                ASSERT_TRUE(latch.await(10));
                ASSERT_EQ(0, semaphore.availablePermits());
            }
        }
    }
}


//
//  ClientAtomiclLong.h
//  hazelcast
//
//  Created by Sancar on 02.08.2013.
//  Copyright (c) 2013 Sancar. All rights reserved.
//


namespace hazelcast {
    namespace client {
        namespace test {
            class IAtomicLongTest : public ClientTestSupport {
            public:
                IAtomicLongTest();

            protected:
                static void SetUpTestCase() {
                    instance = new HazelcastServer(*g_srvFactory);
                }

                static void TearDownTestCase() {
                    delete instance;
                }

                ClientConfig clientConfig;
                HazelcastClient client;
                std::unique_ptr<IAtomicLong> l;

                static HazelcastServer *instance;
            };

            HazelcastServer *IAtomicLongTest::instance = NULL;

            IAtomicLongTest::IAtomicLongTest() : client(getNewClient()),
                                                 l(new IAtomicLong(client.getIAtomicLong(getTestName()))) {
                l->set(0);
            }

            TEST_F(IAtomicLongTest, testSync) {
                ASSERT_EQ(0, l->getAndAdd(2));
                ASSERT_EQ(2, l->get());
                l->set(5);
                ASSERT_EQ(5, l->get());
                ASSERT_EQ(8, l->addAndGet(3));
                ASSERT_FALSE(l->compareAndSet(7, 4));
                ASSERT_EQ(8, l->get());
                ASSERT_TRUE(l->compareAndSet(8, 4));
                ASSERT_EQ(4, l->get());
                ASSERT_EQ(3, l->decrementAndGet());
                ASSERT_EQ(3, l->getAndIncrement());
                ASSERT_EQ(4, l->getAndSet(9));
                ASSERT_EQ(10, l->incrementAndGet());
            }

            TEST_F(IAtomicLongTest, testAsync) {
                std::shared_ptr<ICompletableFuture<int64_t> > future = l->getAndAddAsync(10);
                ASSERT_EQ(0, *future->get());

                std::shared_ptr<ICompletableFuture<bool> > booleanFuture = l->compareAndSetAsync(10, 42);
                ASSERT_TRUE(booleanFuture->get());

                future = l->getAsync();
                ASSERT_EQ(42, *future->get());

                future = l->incrementAndGetAsync();
                ASSERT_EQ(43, *future->get());

                future = l->addAndGetAsync(-13);
                ASSERT_EQ(30, *future->get());

                future = l->decrementAndGetAsync();
                ASSERT_EQ(29, *future->get());

                future = l->getAndSetAsync(15);
                ASSERT_EQ(29, *future->get());

                future = l->getAsync();
                ASSERT_EQ(15, *future->get());

                future = l->getAndIncrementAsync();
                ASSERT_EQ(15, *future->get());

                future = l->getAsync();
                ASSERT_EQ(16, *future->get());

                std::shared_ptr<ICompletableFuture<void> > voidFuture = l->setAsync(55);
                voidFuture->get();

                future = l->getAsync();
                ASSERT_EQ(55, *future->get());
            }
        }
    }
}






using namespace hazelcast::client::mixedtype;

namespace hazelcast {
    namespace client {
        namespace test {
            class MixedMultiMapTest : public ClientTestSupport {
            protected:
                class MyMultiMapListener : public MixedEntryListener {
                public:
                    MyMultiMapListener(hazelcast::util::CountDownLatch& addedLatch, hazelcast::util::CountDownLatch& removedLatch)
                            : addedLatch(addedLatch), removedLatch(removedLatch) {
                    }

                    virtual void entryUpdated(const MixedEntryEvent &event) {
                    }

                    virtual void entryEvicted(const MixedEntryEvent &event) {
                    }

                    virtual void entryExpired(const MixedEntryEvent &event) {
                    }

                    virtual void entryMerged(const MixedEntryEvent &event) {
                    }

                    virtual void mapEvicted(const MapEvent &event) {
                    }

                    virtual void mapCleared(const MapEvent &event) {
                    }

                    virtual void entryAdded(const MixedEntryEvent &event) {
                        addedLatch.countDown();

                    }

                    virtual void entryRemoved(const MixedEntryEvent &event) {
                        removedLatch.countDown();
                    }

                private:
                    hazelcast::util::CountDownLatch& addedLatch;
                    hazelcast::util::CountDownLatch& removedLatch;
                };

                static void lockTtlThread(hazelcast::util::ThreadArgs& args) {
                    mixedtype::MultiMap *map = (mixedtype::MultiMap *)args.arg0;
                    hazelcast::util::CountDownLatch *latch = (hazelcast::util::CountDownLatch *)args.arg1;

                    if (!map->tryLock<std::string>("key1")) {
                        latch->countDown();
                    }

                    if (map->tryLock<std::string>("key1", 5 * 1000)) {
                        latch->countDown();
                    }
                }

                static void forceUnlockThread(hazelcast::util::ThreadArgs& args) {
                    mixedtype::MultiMap *mm = (mixedtype::MultiMap *)args.arg0;
                    hazelcast::util::CountDownLatch *latch = (hazelcast::util::CountDownLatch *)args.arg1;
                    mm->forceUnlock<std::string>("key1");
                    latch->countDown();
                }

                static void lockThread(hazelcast::util::ThreadArgs& args) {
                    mixedtype::MultiMap *mm = (mixedtype::MultiMap *)args.arg0;
                    hazelcast::util::CountDownLatch *latch = (hazelcast::util::CountDownLatch *)args.arg1;
                    if (!mm->tryLock<std::string>("key1")) {
                        latch->countDown();
                    }
                }

                static void tryLockThread(hazelcast::util::ThreadArgs& args) {
                    mixedtype::MultiMap *mm = (mixedtype::MultiMap *)args.arg0;
                    hazelcast::util::CountDownLatch *latch = (hazelcast::util::CountDownLatch *)args.arg1;
                    try {
                        if (!mm->tryLock<std::string>("key1", 2)) {
                            latch->countDown();
                        }
                    } catch (...) {
                        std::cerr << "Unexpected exception at MixedMultiMapTest tryLockThread" << std::endl;
                    }
                }

                static void tryLockThread2(hazelcast::util::ThreadArgs& args) {
                    mixedtype::MultiMap *mm = (mixedtype::MultiMap *)args.arg0;
                    hazelcast::util::CountDownLatch *latch = (hazelcast::util::CountDownLatch *)args.arg1;
                    try {
                        if (mm->tryLock<std::string>("key1", 20 * 1000)) {
                            latch->countDown();
                        }
                    } catch (...) {
                        std::cerr << "Unexpected exception at MixedMultiMapTest lockThread2" << std::endl;
                    }
                }

                virtual void TearDown() {
                    // clear mm
                    mm->clear();
                }

                static void SetUpTestCase() {
                    instance = new HazelcastServer(*g_srvFactory);
                    client = new HazelcastClient;
                    mm = new mixedtype::MultiMap(client->toMixedType().getMultiMap("MixedMultimapTestMap"));
                }

                static void TearDownTestCase() {
                    delete mm;
                    delete client;
                    delete instance;

                    mm = NULL;
                    client = NULL;
                    instance = NULL;
                }

                static HazelcastServer *instance;
                static HazelcastClient *client;
                static mixedtype::MultiMap *mm;
            };

            HazelcastServer *MixedMultiMapTest::instance = NULL;
            HazelcastClient *MixedMultiMapTest::client = NULL;
            mixedtype::MultiMap *MixedMultiMapTest::mm = NULL;

            TEST_F(MixedMultiMapTest, testPutGetRemove) {
                ASSERT_TRUE((mm->put<std::string, std::string>("key1", "value1")));
                ASSERT_TRUE((mm->put<std::string, std::string>("key1", "value2")));
                ASSERT_TRUE((mm->put<std::string, std::string>("key1", "value3")));

                ASSERT_TRUE((mm->put<std::string, std::string>("key2", "value4")));
                ASSERT_TRUE((mm->put<std::string, std::string>("key2", "value5")));

                ASSERT_EQ(3, mm->valueCount<std::string>("key1"));
                ASSERT_EQ(2, mm->valueCount<std::string>("key2"));
                ASSERT_EQ(5, mm->size());

                std::vector<TypedData> coll = mm->get<std::string>("key1");
                ASSERT_EQ(3, (int) coll.size());

                coll = mm->remove<std::string>("key2");
                ASSERT_EQ(2, (int) coll.size());
                ASSERT_EQ(0, mm->valueCount<std::string>("key2"));
                ASSERT_EQ(0, (int) mm->get<std::string>("key2").size());

                ASSERT_FALSE((mm->remove<std::string, std::string>("key1", "value4")));
                ASSERT_EQ(3, mm->size());

                ASSERT_TRUE((mm->remove<std::string, std::string>("key1", "value2")));
                ASSERT_EQ(2, mm->size());

                ASSERT_TRUE((mm->remove<std::string, std::string>("key1", "value1")));
                ASSERT_EQ(1, mm->size());
                coll = mm->get<std::string>("key1");
                std::unique_ptr<std::string> val = coll[0].get<std::string>();
                ASSERT_NE((std::string *) NULL, val.get());
                ASSERT_EQ("value3", *val);
            }

            TEST_F(MixedMultiMapTest, testKeySetEntrySetAndValues) {
                ASSERT_TRUE((mm->put<std::string, std::string>("key1", "value1")));
                ASSERT_TRUE((mm->put<std::string, std::string>("key1", "value2")));
                ASSERT_TRUE((mm->put<std::string, std::string>("key1", "value3")));

                ASSERT_TRUE((mm->put<std::string, std::string>("key2", "value4")));
                ASSERT_TRUE((mm->put<std::string, std::string>("key2", "value5")));


                ASSERT_EQ(2, (int) mm->keySet().size());
                ASSERT_EQ(5, (int) mm->values().size());
                ASSERT_EQ(5, (int) mm->entrySet().size());
            }

            TEST_F(MixedMultiMapTest, testContains) {
                ASSERT_TRUE((mm->put<std::string, std::string>("key1", "value1")));
                ASSERT_TRUE((mm->put<std::string, std::string>("key1", "value2")));
                ASSERT_TRUE((mm->put<std::string, std::string>("key1", "value3")));

                ASSERT_TRUE((mm->put<std::string, std::string>("key2", "value4")));
                ASSERT_TRUE((mm->put<std::string, std::string>("key2", "value5")));

                ASSERT_FALSE(mm->containsKey<std::string>("key3"));
                ASSERT_TRUE(mm->containsKey<std::string>("key1"));

                ASSERT_FALSE(mm->containsValue<std::string>("value6"));
                ASSERT_TRUE(mm->containsValue<std::string>("value4"));

                ASSERT_FALSE((mm->containsEntry<std::string, std::string>("key1", "value4")));
                ASSERT_FALSE((mm->containsEntry<std::string, std::string>("key2", "value3")));
                ASSERT_TRUE((mm->containsEntry<std::string, std::string>("key1", "value1")));
                ASSERT_TRUE((mm->containsEntry<std::string, std::string>("key2", "value5")));
            }

            TEST_F(MixedMultiMapTest, testListener) {
                hazelcast::util::CountDownLatch latch1Add(8);
                hazelcast::util::CountDownLatch latch1Remove(4);

                hazelcast::util::CountDownLatch latch2Add(3);
                hazelcast::util::CountDownLatch latch2Remove(3);

                MyMultiMapListener mmener1(latch1Add, latch1Remove);
                MyMultiMapListener mmener2(latch2Add, latch2Remove);

                std::string id1 = mm->addEntryListener(mmener1, true);
                std::string id2 = mm->addEntryListener<std::string>(mmener2, "key3", true);

                mm->put<std::string, std::string>("key1", "value1");
                mm->put<std::string, std::string>("key1", "value2");
                mm->put<std::string, std::string>("key1", "value3");
                mm->put<std::string, std::string>("key2", "value4");
                mm->put<std::string, std::string>("key2", "value5");

                mm->remove<std::string, std::string>("key1", "value2");

                mm->put<std::string, std::string>("key3", "value6");
                mm->put<std::string, std::string>("key3", "value7");
                mm->put<std::string, std::string>("key3", "value8");

                mm->remove<std::string>("key3");

                ASSERT_TRUE(latch1Add.await(20));
                ASSERT_TRUE(latch1Remove.await(20));

                ASSERT_TRUE(latch2Add.await(20));
                ASSERT_TRUE(latch2Remove.await(20));

                ASSERT_TRUE(mm->removeEntryListener(id1));
                ASSERT_TRUE(mm->removeEntryListener(id2));

            }

            TEST_F(MixedMultiMapTest, testLock) {
                mm->lock<std::string>("key1");
                hazelcast::util::CountDownLatch latch(1);
                hazelcast::util::StartedThread t(lockThread, mm, &latch);
                ASSERT_TRUE(latch.await(5));
                mm->forceUnlock<std::string>("key1");
                t.join();
            }

            TEST_F(MixedMultiMapTest, testLockTtl) {
                mm->lock<std::string>("key1", 3 * 1000);
                hazelcast::util::CountDownLatch latch(2);
                hazelcast::util::StartedThread t(lockTtlThread, mm, &latch);
                ASSERT_TRUE(latch.await(10));
                mm->forceUnlock<std::string>("key1");
                t.join();
            }

            TEST_F(MixedMultiMapTest, testTryLock) {
                ASSERT_TRUE(mm->tryLock<std::string>("key1", 2 * 1000));
                hazelcast::util::CountDownLatch latch(1);
                hazelcast::util::StartedThread t(tryLockThread, mm, &latch);
                ASSERT_TRUE(latch.await(100));
                ASSERT_TRUE(mm->isLocked<std::string>("key1"));

                hazelcast::util::CountDownLatch latch2(1);
                hazelcast::util::StartedThread t2(tryLockThread2, mm, &latch2);

                hazelcast::util::sleep(1);
                mm->unlock<std::string>("key1");
                ASSERT_TRUE(latch2.await(100));
                ASSERT_TRUE(mm->isLocked<std::string>("key1"));
                mm->forceUnlock<std::string>("key1");
            }

            TEST_F(MixedMultiMapTest, testForceUnlock) {
                mm->lock<std::string>("key1");
                hazelcast::util::CountDownLatch latch(1);
                hazelcast::util::StartedThread t(forceUnlockThread, mm, &latch);
                ASSERT_TRUE(latch.await(100));
                ASSERT_FALSE(mm->isLocked<std::string>("key1"));
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

