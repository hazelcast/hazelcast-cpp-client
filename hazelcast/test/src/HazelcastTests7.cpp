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
            class ClientMultiMapTest : public ClientTestSupport {
            protected:
                virtual void TearDown() {
                    // clear mm
                    mm->clear();
                }

                static void SetUpTestCase() {
                    instance = new HazelcastServer(*g_srvFactory);
                    client = new HazelcastClient(getConfig());
                    mm = new MultiMap<std::string, std::string>(client->getMultiMap<std::string, std::string>("MyMultiMap"));
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
                static MultiMap<std::string, std::string> *mm;
            };

            HazelcastServer *ClientMultiMapTest::instance = NULL;
            HazelcastClient *ClientMultiMapTest::client = NULL;
            MultiMap<std::string, std::string> *ClientMultiMapTest::mm = NULL;

            TEST_F(ClientMultiMapTest, testPutGetRemove) {
                ASSERT_TRUE(mm->put("key1", "value1"));
                ASSERT_TRUE(mm->put("key1", "value2"));
                ASSERT_TRUE(mm->put("key1", "value3"));

                ASSERT_TRUE(mm->put("key2", "value4"));
                ASSERT_TRUE(mm->put("key2", "value5"));

                ASSERT_EQ(3, mm->valueCount("key1"));
                ASSERT_EQ(2, mm->valueCount("key2"));
                ASSERT_EQ(5, mm->size());

                std::vector<std::string> coll = mm->get("key1");
                ASSERT_EQ(3, (int) coll.size());

                coll = mm->remove("key2");
                ASSERT_EQ(2, (int) coll.size());
                ASSERT_EQ(0, mm->valueCount("key2"));
                ASSERT_EQ(0, (int) mm->get("key2").size());

                ASSERT_FALSE(mm->remove("key1", "value4"));
                ASSERT_EQ(3, mm->size());

                ASSERT_TRUE(mm->remove("key1", "value2"));
                ASSERT_EQ(2, mm->size());

                ASSERT_TRUE(mm->remove("key1", "value1"));
                ASSERT_EQ(1, mm->size());
                ASSERT_EQ("value3", mm->get("key1")[0]);
            }


            TEST_F(ClientMultiMapTest, testKeySetEntrySetAndValues) {
                ASSERT_TRUE(mm->put("key1", "value1"));
                ASSERT_TRUE(mm->put("key1", "value2"));
                ASSERT_TRUE(mm->put("key1", "value3"));

                ASSERT_TRUE(mm->put("key2", "value4"));
                ASSERT_TRUE(mm->put("key2", "value5"));


                ASSERT_EQ(2, (int) mm->keySet().size());
                ASSERT_EQ(5, (int) mm->values().size());
                ASSERT_EQ(5, (int) mm->entrySet().size());
            }


            TEST_F(ClientMultiMapTest, testContains) {
                ASSERT_TRUE(mm->put("key1", "value1"));
                ASSERT_TRUE(mm->put("key1", "value2"));
                ASSERT_TRUE(mm->put("key1", "value3"));

                ASSERT_TRUE(mm->put("key2", "value4"));
                ASSERT_TRUE(mm->put("key2", "value5"));

                ASSERT_FALSE(mm->containsKey("key3"));
                ASSERT_TRUE(mm->containsKey("key1"));

                ASSERT_FALSE(mm->containsValue("value6"));
                ASSERT_TRUE(mm->containsValue("value4"));

                ASSERT_FALSE(mm->containsEntry("key1", "value4"));
                ASSERT_FALSE(mm->containsEntry("key2", "value3"));
                ASSERT_TRUE(mm->containsEntry("key1", "value1"));
                ASSERT_TRUE(mm->containsEntry("key2", "value5"));
            }

            class MyMultiMapListener : public EntryAdapter<std::string, std::string> {
            public:
                MyMultiMapListener(hazelcast::util::CountDownLatch &addedLatch,
                                   hazelcast::util::CountDownLatch &removedLatch)
                        : addedLatch(addedLatch), removedLatch(removedLatch) {
                }

                void entryAdded(const EntryEvent<std::string, std::string> &event) {
                    addedLatch.countDown();
                }

                void entryRemoved(const EntryEvent<std::string, std::string> &event) {
                    removedLatch.countDown();
                }

            private:
                hazelcast::util::CountDownLatch &addedLatch;
                hazelcast::util::CountDownLatch &removedLatch;
            };

            TEST_F(ClientMultiMapTest, testListener) {
                hazelcast::util::CountDownLatch latch1Add(8);
                hazelcast::util::CountDownLatch latch1Remove(4);

                hazelcast::util::CountDownLatch latch2Add(3);
                hazelcast::util::CountDownLatch latch2Remove(3);

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

                ASSERT_TRUE(latch1Add.await(20));
                ASSERT_TRUE(latch1Remove.await(20));

                ASSERT_TRUE(latch2Add.await(20));
                ASSERT_TRUE(latch2Remove.await(20));

                ASSERT_TRUE(mm->removeEntryListener(id1));
                ASSERT_TRUE(mm->removeEntryListener(id2));

            }

            void lockThread(hazelcast::util::ThreadArgs &args) {
                MultiMap<std::string, std::string> *mm = (MultiMap<std::string, std::string> *) args.arg0;
                hazelcast::util::CountDownLatch *latch = (hazelcast::util::CountDownLatch *) args.arg1;
                if (!mm->tryLock("key1")) {
                    latch->countDown();
                }
            }

            TEST_F(ClientMultiMapTest, testLock) {
                mm->lock("key1");
                hazelcast::util::CountDownLatch latch(1);
                hazelcast::util::StartedThread t(lockThread, mm, &latch);
                ASSERT_TRUE(latch.await(5));
                mm->forceUnlock("key1");
            }

            void lockTtlThread(hazelcast::util::ThreadArgs &args) {
                MultiMap<std::string, std::string> *mm = (MultiMap<std::string, std::string> *) args.arg0;
                hazelcast::util::CountDownLatch *latch = (hazelcast::util::CountDownLatch *) args.arg1;

                if (!mm->tryLock("key1")) {
                    latch->countDown();
                }

                if (mm->tryLock("key1", 5 * 1000)) {
                    latch->countDown();
                }
            }

            TEST_F(ClientMultiMapTest, testLockTtl) {
                mm->lock("key1", 3 * 1000);
                hazelcast::util::CountDownLatch latch(2);
                hazelcast::util::StartedThread t(lockTtlThread, mm, &latch);
                ASSERT_TRUE(latch.await(10));
                mm->forceUnlock("key1");
            }


            void tryLockThread(hazelcast::util::ThreadArgs &args) {
                MultiMap<std::string, std::string> *mm = (MultiMap<std::string, std::string> *) args.arg0;
                hazelcast::util::CountDownLatch *latch = (hazelcast::util::CountDownLatch *) args.arg1;
                try {
                    if (!mm->tryLock("key1", 2)) {
                        latch->countDown();
                    }
                } catch (...) {
                    std::cerr << "Unexpected exception at ClientMultiMapTest tryLockThread" << std::endl;
                }
            }

            void tryLockThread2(hazelcast::util::ThreadArgs &args) {
                MultiMap<std::string, std::string> *mm = (MultiMap<std::string, std::string> *) args.arg0;
                hazelcast::util::CountDownLatch *latch = (hazelcast::util::CountDownLatch *) args.arg1;
                try {
                    if (mm->tryLock("key1", 20 * 1000)) {
                        latch->countDown();
                    }
                } catch (...) {
                    std::cerr << "Unexpected exception at ClientMultiMapTest lockThread2" << std::endl;
                }
            }

            TEST_F(ClientMultiMapTest, testTryLock) {
                ASSERT_TRUE(mm->tryLock("key1", 2 * 1000));
                hazelcast::util::CountDownLatch latch(1);
                hazelcast::util::StartedThread t(tryLockThread, mm, &latch);
                ASSERT_TRUE(latch.await(100));
                ASSERT_TRUE(mm->isLocked("key1"));

                hazelcast::util::CountDownLatch latch2(1);
                hazelcast::util::StartedThread t2(tryLockThread2, mm, &latch2);

                hazelcast::util::sleep(1);
                mm->unlock("key1");
                ASSERT_TRUE(latch2.await(100));
                ASSERT_TRUE(mm->isLocked("key1"));
                mm->forceUnlock("key1");
            }

            void forceUnlockThread(hazelcast::util::ThreadArgs &args) {
                MultiMap<std::string, std::string> *mm = (MultiMap<std::string, std::string> *) args.arg0;
                hazelcast::util::CountDownLatch *latch = (hazelcast::util::CountDownLatch *) args.arg1;
                mm->forceUnlock("key1");
                latch->countDown();
            }

            TEST_F(ClientMultiMapTest, testForceUnlock) {
                mm->lock("key1");
                hazelcast::util::CountDownLatch latch(1);
                hazelcast::util::StartedThread t(forceUnlockThread, mm, &latch);
                ASSERT_TRUE(latch.await(100));
                ASSERT_FALSE(mm->isLocked("key1"));
            }
        }
    }
}




using namespace hazelcast::client::mixedtype;

namespace hazelcast {
    namespace client {
        namespace test {
            class MixedListTest : public ClientTestSupport {
            protected:
                class MyListItemListener : public MixedItemListener {
                public:
                    MyListItemListener(hazelcast::util::CountDownLatch& latch)
                            : latch(latch) {

                    }

                    virtual void itemAdded(const ItemEvent<TypedData> &item) {
                        latch.countDown();
                    }

                    virtual void itemRemoved(const ItemEvent<TypedData> &item) {
                    }
                private:
                    hazelcast::util::CountDownLatch& latch;
                };

                virtual void TearDown() {
                    // clear list
                    list->clear();
                }

                static void SetUpTestCase() {
#ifdef HZ_BUILD_WITH_SSL
                    sslFactory = new HazelcastServerFactory(g_srvFactory->getServerAddress(), getSslFilePath());
                    instance = new HazelcastServer(*sslFactory);
#else
                    instance = new HazelcastServer(*g_srvFactory);
#endif

                    ClientConfig clientConfig = getConfig();

#ifdef HZ_BUILD_WITH_SSL
                    config::ClientNetworkConfig networkConfig;
                    config::SSLConfig sslConfig;
                    sslConfig.setEnabled(true).addVerifyFile(getCAFilePath()).setCipherList("HIGH");
                    networkConfig.setSSLConfig(sslConfig);
                    clientConfig.setNetworkConfig(networkConfig);
#endif // HZ_BUILD_WITH_SSL

                    client = new HazelcastClient(clientConfig);
                    list = new mixedtype::IList(client->toMixedType().getList("MyMixedList"));
                }

                static void TearDownTestCase() {
                    delete list;
                    delete client;
                    delete instance;
                    delete sslFactory;

                    list = NULL;
                    client = NULL;
                    instance = NULL;
                }

                static HazelcastServer *instance;
                static HazelcastClient *client;
                static mixedtype::IList *list;
                static HazelcastServerFactory *sslFactory;
            };

            HazelcastServer *MixedListTest::instance = NULL;
            HazelcastClient *MixedListTest::client = NULL;
            mixedtype::IList *MixedListTest::list = NULL;
            HazelcastServerFactory *MixedListTest::sslFactory = NULL;

            TEST_F(MixedListTest, testAddAll) {
                std::vector<std::string> l;
                l.push_back("item1");
                l.push_back("item2");
                ASSERT_TRUE(list->addAll<std::string>(l));

                ASSERT_TRUE(list->addAll<std::string>(1, l));
                ASSERT_EQ(4, list->size());

                ASSERT_EQ("item1", *(list->get(0).get<std::string>()));
                ASSERT_EQ("item1", *(list->get(1).get<std::string>()));
                ASSERT_EQ("item2", *(list->get(2).get<std::string>()));
                ASSERT_EQ("item2", *(list->get(3).get<std::string>()));
            }

            TEST_F(MixedListTest, testAddSetRemove) {
                ASSERT_TRUE(list->add<std::string>("item1"));
                ASSERT_TRUE(list->add<std::string>("item2"));
                list->add<std::string>(0, "item3");
                ASSERT_EQ(3, list->size());
                std::unique_ptr<std::string> temp = list->IList::set<std::string>(2, "item4").get<std::string>();
                ASSERT_EQ("item2", *temp);

                ASSERT_EQ(3, list->size());
                ASSERT_EQ("item3", *(list->get(0).get<std::string>()));
                ASSERT_EQ("item1", *(list->get(1).get<std::string>()));
                ASSERT_EQ("item4", *(list->get(2).get<std::string>()));

                ASSERT_FALSE(list->remove<std::string>("item2"));
                ASSERT_TRUE(list->remove<std::string>("item3"));

                temp = list->remove(1).get<std::string>();
                ASSERT_EQ("item4", *temp);

                ASSERT_EQ(1, list->size());
                ASSERT_EQ("item1", *(list->get(0).get<std::string>()));
            }

            TEST_F(MixedListTest, testIndexOf) {
                ASSERT_TRUE(list->add<std::string>("item1"));
                ASSERT_TRUE(list->add<std::string>("item2"));
                ASSERT_TRUE(list->add<std::string>("item1"));
                ASSERT_TRUE(list->add<std::string>("item4"));

                ASSERT_EQ(-1, list->indexOf<std::string>("item5"));
                ASSERT_EQ(0, list->indexOf<std::string>("item1"));

                ASSERT_EQ(-1, list->lastIndexOf<std::string>("item6"));
                ASSERT_EQ(2, list->lastIndexOf<std::string>("item1"));
            }

            TEST_F(MixedListTest, testToArray) {
                ASSERT_TRUE(list->add<std::string>("item1"));
                ASSERT_TRUE(list->add<std::string>("item2"));
                ASSERT_TRUE(list->add<std::string>("item1"));
                ASSERT_TRUE(list->add<std::string>("item4"));

                std::vector<TypedData> ar = list->toArray();

                ASSERT_EQ("item1", *ar[0].get<std::string>());
                ASSERT_EQ("item2", *ar[1].get<std::string>());
                ASSERT_EQ("item1", *ar[2].get<std::string>());
                ASSERT_EQ("item4", *ar[3].get<std::string>());

                std::vector<TypedData> arr2 = list->subList(1, 3);

                ASSERT_EQ(2, (int) arr2.size());
                ASSERT_EQ("item2", *arr2[0].get<std::string>());
                ASSERT_EQ("item1", *arr2[1].get<std::string>());
            }

            TEST_F(MixedListTest, testContains) {
                ASSERT_TRUE(list->add<std::string>("item1"));
                ASSERT_TRUE(list->add<std::string>("item2"));
                ASSERT_TRUE(list->add<std::string>("item1"));
                ASSERT_TRUE(list->add<std::string>("item4"));

                ASSERT_FALSE(list->contains<std::string>("item3"));
                ASSERT_TRUE(list->contains<std::string>("item2"));

                std::vector<std::string> l;
                l.push_back("item4");
                l.push_back("item3");

                ASSERT_FALSE(list->containsAll<std::string>(l));
                ASSERT_TRUE(list->add<std::string>("item3"));
                ASSERT_TRUE(list->containsAll<std::string>(l));
            }

            TEST_F(MixedListTest, testRemoveRetainAll) {
                ASSERT_TRUE(list->add<std::string>("item1"));
                ASSERT_TRUE(list->add<std::string>("item2"));
                ASSERT_TRUE(list->add<std::string>("item1"));
                ASSERT_TRUE(list->add<std::string>("item4"));

                std::vector<std::string> l;
                l.push_back("item4");
                l.push_back("item3");

                ASSERT_TRUE(list->removeAll<std::string>(l));
                ASSERT_EQ(3, (int) list->size());
                ASSERT_FALSE(list->removeAll<std::string>(l));
                ASSERT_EQ(3, (int) list->size());

                l.clear();
                l.push_back("item1");
                l.push_back("item2");
                ASSERT_FALSE(list->retainAll<std::string>(l));
                ASSERT_EQ(3, (int) list->size());

                l.clear();
                ASSERT_TRUE(list->retainAll<std::string>(l));
                ASSERT_EQ(0, (int) list->size());

            }

            TEST_F(MixedListTest, testListener) {
                hazelcast::util::CountDownLatch latch(5);

                MyListItemListener listener(latch);
                std::string registrationId = list->addItemListener(listener, true);

                for (int i = 0; i < 5; i++) {
                    list->add(std::string("item") + hazelcast::util::IOUtil::to_string(i));
                }

                ASSERT_TRUE(latch.await(20));

                ASSERT_TRUE(list->removeItemListener(registrationId));
            }
        }
    }
}




namespace hazelcast {
    namespace client {
        namespace test {
            class ClientListTest : public ClientTestSupport {
            protected:
                class MyListItemListener : public ItemListener<std::string> {
                public:
                    MyListItemListener(hazelcast::util::CountDownLatch& latch)
                            : latch(latch) {

                    }

                    void itemAdded(const ItemEvent<std::string>& itemEvent) {
                        int type = itemEvent.getEventType();
                        assertEquals((int) ItemEventType::ADDED, type);
                        assertEquals("MyList", itemEvent.getName());
                        std::string host = itemEvent.getMember().getAddress().getHost();
                        assertTrue(host == "localhost" || host == "127.0.0.1");
                        assertEquals(5701, itemEvent.getMember().getAddress().getPort());
                        assertEquals("item-1", itemEvent.getItem());
                        latch.countDown();
                    }

                    void itemRemoved(const ItemEvent<std::string>& item) {
                    }

                private:
                    hazelcast::util::CountDownLatch& latch;
                };

                virtual void TearDown() {
                    // clear list
                    list->clear();
                }

                static void SetUpTestCase() {
#ifdef HZ_BUILD_WITH_SSL
                    sslFactory = new HazelcastServerFactory(g_srvFactory->getServerAddress(), getSslFilePath());
                    instance = new HazelcastServer(*sslFactory);
#else
                    instance = new HazelcastServer(*g_srvFactory);
#endif

                    ClientConfig clientConfig = getConfig();

#ifdef HZ_BUILD_WITH_SSL
                    config::ClientNetworkConfig networkConfig;
                    config::SSLConfig sslConfig;
                    sslConfig.setEnabled(true).addVerifyFile(getCAFilePath()).setCipherList("HIGH");
                    networkConfig.setSSLConfig(sslConfig);
                    clientConfig.setNetworkConfig(networkConfig);
#endif // HZ_BUILD_WITH_SSL

                    client = new HazelcastClient(clientConfig);
                    list = new IList<std::string>(client->getList<std::string>("MyList"));
                }

                static void TearDownTestCase() {
                    delete list;
                    delete client;
                    delete instance;
                    delete sslFactory;

                    list = NULL;
                    client = NULL;
                    instance = NULL;
                }

                static HazelcastServer *instance;
                static HazelcastClient *client;
                static IList<std::string> *list;
                static HazelcastServerFactory *sslFactory;
            };

            HazelcastServer *ClientListTest::instance = NULL;
            HazelcastClient *ClientListTest::client = NULL;
            IList<std::string> *ClientListTest::list = NULL;
            HazelcastServerFactory *ClientListTest::sslFactory = NULL;

            TEST_F(ClientListTest, testAddAll) {
                std::vector<std::string> l;
                l.push_back("item1");
                l.push_back("item2");
                ASSERT_TRUE(list->addAll(l));

                ASSERT_TRUE(list->addAll(1, l));
                ASSERT_EQ(4, list->size());

                ASSERT_EQ("item1", *(list->get(0)));
                ASSERT_EQ("item1", *(list->get(1)));
                ASSERT_EQ("item2", *(list->get(2)));
                ASSERT_EQ("item2", *(list->get(3)));
            }

            TEST_F(ClientListTest, testAddSetRemove) {
                ASSERT_TRUE(list->add("item1"));
                ASSERT_TRUE(list->add("item2"));
                list->add(0, "item3");
                ASSERT_EQ(3, list->size());
                std::shared_ptr<std::string> temp = list->set(2, "item4");
                ASSERT_EQ("item2", *temp);

                ASSERT_EQ(3, list->size());
                ASSERT_EQ("item3", *(list->get(0)));
                ASSERT_EQ("item1", *(list->get(1)));
                ASSERT_EQ("item4", *(list->get(2)));

                ASSERT_FALSE(list->remove("item2"));
                ASSERT_TRUE(list->remove("item3"));

                temp = list->remove(1);
                ASSERT_EQ("item4", *temp);

                ASSERT_EQ(1, list->size());
                ASSERT_EQ("item1", *(list->get(0)));
            }

            TEST_F(ClientListTest, testIndexOf) {
                ASSERT_TRUE(list->add("item1"));
                ASSERT_TRUE(list->add("item2"));
                ASSERT_TRUE(list->add("item1"));
                ASSERT_TRUE(list->add("item4"));

                ASSERT_EQ(-1, list->indexOf("item5"));
                ASSERT_EQ(0, list->indexOf("item1"));

                ASSERT_EQ(-1, list->lastIndexOf("item6"));
                ASSERT_EQ(2, list->lastIndexOf("item1"));
            }

            TEST_F(ClientListTest, testToArray) {
                ASSERT_TRUE(list->add("item1"));
                ASSERT_TRUE(list->add("item2"));
                ASSERT_TRUE(list->add("item1"));
                ASSERT_TRUE(list->add("item4"));

                std::vector<std::string> ar = list->toArray();

                ASSERT_EQ("item1", ar[0]);
                ASSERT_EQ("item2", ar[1]);
                ASSERT_EQ("item1", ar[2]);
                ASSERT_EQ("item4", ar[3]);

                std::vector<std::string> arr2 = list->subList(1, 3);

                ASSERT_EQ(2, (int) arr2.size());
                ASSERT_EQ("item2", arr2[0]);
                ASSERT_EQ("item1", arr2[1]);
            }

            TEST_F(ClientListTest, testContains) {
                ASSERT_TRUE(list->add("item1"));
                ASSERT_TRUE(list->add("item2"));
                ASSERT_TRUE(list->add("item1"));
                ASSERT_TRUE(list->add("item4"));

                ASSERT_FALSE(list->contains("item3"));
                ASSERT_TRUE(list->contains("item2"));

                std::vector<std::string> l;
                l.push_back("item4");
                l.push_back("item3");

                ASSERT_FALSE(list->containsAll(l));
                ASSERT_TRUE(list->add("item3"));
                ASSERT_TRUE(list->containsAll(l));
            }

            TEST_F(ClientListTest, testRemoveRetainAll) {
                ASSERT_TRUE(list->add("item1"));
                ASSERT_TRUE(list->add("item2"));
                ASSERT_TRUE(list->add("item1"));
                ASSERT_TRUE(list->add("item4"));

                std::vector<std::string> l;
                l.push_back("item4");
                l.push_back("item3");

                ASSERT_TRUE(list->removeAll(l));
                ASSERT_EQ(3, (int) list->size());
                ASSERT_FALSE(list->removeAll(l));
                ASSERT_EQ(3, (int) list->size());

                l.clear();
                l.push_back("item1");
                l.push_back("item2");
                ASSERT_FALSE(list->retainAll(l));
                ASSERT_EQ(3, (int) list->size());

                l.clear();
                ASSERT_TRUE(list->retainAll(l));
                ASSERT_EQ(0, (int) list->size());

            }

            TEST_F(ClientListTest, testListener) {
                hazelcast::util::CountDownLatch latch(1);

                MyListItemListener listener(latch);
                std::string registrationId = list->addItemListener(listener, true);

                list->add("item-1");

                ASSERT_TRUE(latch.await(20));

                ASSERT_TRUE(list->removeItemListener(registrationId));
            }

            TEST_F(ClientListTest, testIsEmpty) {
                ASSERT_TRUE(list->isEmpty());
                ASSERT_TRUE(list->add("item1"));
                ASSERT_FALSE(list->isEmpty());
            }

        }
    }
}





namespace hazelcast {
    namespace client {
        namespace test {
            class ClientQueueTest : public ClientTestSupport {
            protected:
                virtual void TearDown() {
                    q->clear();
                }

                static void SetUpTestCase() {
                    instance = new HazelcastServer(*g_srvFactory);
                    client = new HazelcastClient(getConfig());
                    q = new IQueue<std::string>(client->getQueue<std::string>("MyQueue"));
                }

                static void TearDownTestCase() {
                    delete q;
                    delete client;
                    delete instance;

                    q = NULL;
                    client = NULL;
                    instance = NULL;
                }

                static HazelcastServer *instance;
                static HazelcastClient *client;
                static IQueue<std::string> *q;
            };

            HazelcastServer *ClientQueueTest::instance = NULL;
            HazelcastClient *ClientQueueTest::client = NULL;
            IQueue<std::string> *ClientQueueTest::q = NULL;

            class QueueTestItemListener : public ItemListener<std::string> {
            public:
                QueueTestItemListener(hazelcast::util::CountDownLatch &latch)
                        : latch(latch) {

                }

                void itemAdded(const ItemEvent<std::string> &itemEvent) {
                    latch.countDown();
                }

                void itemRemoved(const ItemEvent<std::string> &item) {
                }

            private:
                hazelcast::util::CountDownLatch &latch;
            };

            TEST_F(ClientQueueTest, testListener) {
                ASSERT_EQ(0, q->size());

                hazelcast::util::CountDownLatch latch(5);

                QueueTestItemListener listener(latch);
                std::string id = q->addItemListener(listener, true);

                hazelcast::util::sleep(1);

                for (int i = 0; i < 5; i++) {
                    ASSERT_TRUE(q->offer(std::string("event_item") + hazelcast::util::IOUtil::to_string(i)));
                }

                ASSERT_TRUE(latch.await(5));
                ASSERT_TRUE(q->removeItemListener(id));

                // added for test coverage
                ASSERT_NO_THROW(q->destroy());
            }

            void testOfferPollThread2(hazelcast::util::ThreadArgs &args) {
                IQueue<std::string> *q = (IQueue<std::string> *) args.arg0;
                hazelcast::util::sleep(2);
                q->offer("item1");
            }

            TEST_F(ClientQueueTest, testOfferPoll) {
                for (int i = 0; i < 10; i++) {
                    bool result = q->offer("item");
                    ASSERT_TRUE(result);
                }
                ASSERT_EQ(10, q->size());
                q->poll();
                bool result = q->offer("item", 5);
                ASSERT_TRUE(result);

                for (int i = 0; i < 10; i++) {
                    ASSERT_NE(q->poll().get(), (std::string *) NULL);
                }
                ASSERT_EQ(0, q->size());

                hazelcast::util::StartedThread t2(testOfferPollThread2, q);

                std::shared_ptr<std::string> item = q->poll(30 * 1000);
                ASSERT_NE(item.get(), (std::string *) NULL);
                ASSERT_EQ("item1", *item);
                t2.join();
            }

            TEST_F(ClientQueueTest, testPeek) {
                ASSERT_TRUE(q->offer("peek 1"));
                ASSERT_TRUE(q->offer("peek 2"));
                ASSERT_TRUE(q->offer("peek 3"));

                std::shared_ptr<std::string> item = q->peek();
                ASSERT_NE((std::string *) NULL, item.get());
                ASSERT_EQ("peek 1", *item);
            }

            TEST_F(ClientQueueTest, testTake) {
                q->put("peek 1");
                ASSERT_TRUE(q->offer("peek 2"));
                ASSERT_TRUE(q->offer("peek 3"));

                std::shared_ptr<std::string> item = q->take();
                ASSERT_NE((std::string *) NULL, item.get());
                ASSERT_EQ("peek 1", *item);

                item = q->take();
                ASSERT_NE((std::string *) NULL, item.get());
                ASSERT_EQ("peek 2", *item);

                item = q->take();
                ASSERT_NE((std::string *) NULL, item.get());
                ASSERT_EQ("peek 3", *item);

                ASSERT_TRUE(q->isEmpty());

// start a thread to insert an item
                hazelcast::util::StartedThread t2(testOfferPollThread2, q);

                item = q->take();  //  should block till it gets an item
                ASSERT_NE((std::string *) NULL, item.get());
                ASSERT_EQ("item1", *item);

                t2.join();
            }

            TEST_F(ClientQueueTest, testRemainingCapacity) {
                int capacity = q->remainingCapacity();
                ASSERT_TRUE(capacity > 10000);
                q->offer("item");
                ASSERT_EQ(capacity - 1, q->remainingCapacity());
            }


            TEST_F(ClientQueueTest, testRemove) {
                ASSERT_TRUE(q->offer("item1"));
                ASSERT_TRUE(q->offer("item2"));
                ASSERT_TRUE(q->offer("item3"));

                ASSERT_FALSE(q->remove("item4"));
                ASSERT_EQ(3, q->size());

                ASSERT_TRUE(q->remove("item2"));

                ASSERT_EQ(2, q->size());

                ASSERT_EQ("item1", *(q->poll()));
                ASSERT_EQ("item3", *(q->poll()));
            }


            TEST_F(ClientQueueTest, testContains) {
                ASSERT_TRUE(q->offer("item1"));
                ASSERT_TRUE(q->offer("item2"));
                ASSERT_TRUE(q->offer("item3"));
                ASSERT_TRUE(q->offer("item4"));
                ASSERT_TRUE(q->offer("item5"));


                ASSERT_TRUE(q->contains("item3"));
                ASSERT_FALSE(q->contains("item"));

                std::vector<std::string> list;
                list.push_back("item4");
                list.push_back("item2");

                ASSERT_TRUE(q->containsAll(list));

                list.push_back("item");
                ASSERT_FALSE(q->containsAll(list));
            }

            TEST_F(ClientQueueTest, testDrain) {
                ASSERT_TRUE(q->offer("item1"));
                ASSERT_TRUE(q->offer("item2"));
                ASSERT_TRUE(q->offer("item3"));
                ASSERT_TRUE(q->offer("item4"));
                ASSERT_TRUE(q->offer("item5"));

                std::vector<std::string> list;
                size_t result = q->drainTo(list, 2);
                ASSERT_EQ(2U, result);
                ASSERT_EQ("item1", list[0]);
                ASSERT_EQ("item2", list[1]);

                std::vector<std::string> list2;
                result = q->drainTo(list2);
                ASSERT_EQ(3U, result);
                ASSERT_EQ("item3", list2[0]);
                ASSERT_EQ("item4", list2[1]);
                ASSERT_EQ("item5", list2[2]);

                ASSERT_TRUE(q->offer("item1"));
                ASSERT_TRUE(q->offer("item2"));
                ASSERT_TRUE(q->offer("item3"));
                list2.clear();
                result = q->drainTo(list2, 5);
                ASSERT_EQ(3U, result);
                ASSERT_EQ("item1", list2[0]);
                ASSERT_EQ("item2", list2[1]);
                ASSERT_EQ("item3", list2[2]);
            }

            TEST_F(ClientQueueTest, testToArray) {
                ASSERT_TRUE(q->offer("item1"));
                ASSERT_TRUE(q->offer("item2"));
                ASSERT_TRUE(q->offer("item3"));
                ASSERT_TRUE(q->offer("item4"));
                ASSERT_TRUE(q->offer("item5"));

                std::vector<std::string> array = q->toArray();
                size_t size = array.size();
                for (size_t i = 0; i < size; i++) {
                    ASSERT_EQ(std::string("item") + hazelcast::util::IOUtil::to_string(i + 1), array[i]);
                }
            }

            TEST_F(ClientQueueTest, testAddAll) {
                std::vector<std::string> coll;
                coll.push_back("item1");
                coll.push_back("item2");
                coll.push_back("item3");
                coll.push_back("item4");

                ASSERT_TRUE(q->addAll(coll));
                int size = q->size();
                ASSERT_EQ(size, (int) coll.size());
            }

            TEST_F(ClientQueueTest, testRemoveRetain) {
                ASSERT_TRUE(q->offer("item1"));
                ASSERT_TRUE(q->offer("item2"));
                ASSERT_TRUE(q->offer("item3"));
                ASSERT_TRUE(q->offer("item4"));
                ASSERT_TRUE(q->offer("item5"));

                std::vector<std::string> list;
                list.push_back("item8");
                list.push_back("item9");
                ASSERT_FALSE(q->removeAll(list));
                ASSERT_EQ(5, q->size());

                list.push_back("item3");
                list.push_back("item4");
                list.push_back("item1");
                ASSERT_TRUE(q->removeAll(list));
                ASSERT_EQ(2, q->size());

                list.clear();
                list.push_back("item2");
                list.push_back("item5");
                ASSERT_FALSE(q->retainAll(list));
                ASSERT_EQ(2, q->size());

                list.clear();
                ASSERT_TRUE(q->retainAll(list));
                ASSERT_EQ(0, q->size());
            }

            TEST_F(ClientQueueTest, testClear) {
                ASSERT_TRUE(q->offer("item1"));
                ASSERT_TRUE(q->offer("item2"));
                ASSERT_TRUE(q->offer("item3"));
                ASSERT_TRUE(q->offer("item4"));
                ASSERT_TRUE(q->offer("item5"));

                q->clear();

                ASSERT_EQ(0, q->size());
                ASSERT_EQ(q->poll().get(), (std::string *) NULL);
            }

            TEST_F(ClientQueueTest, testIsEmpty) {
                ASSERT_TRUE(q->isEmpty());
                ASSERT_TRUE(q->offer("item1"));
                ASSERT_FALSE(q->isEmpty());
            }

            TEST_F(ClientQueueTest, testPut) {
                q->put("item1");
                ASSERT_EQ(1, q->size());
            }
        }
    }
}





using namespace hazelcast::client::mixedtype;

namespace hazelcast {
    namespace client {
        namespace test {
            class MixedQueueTest : public ClientTestSupport {
            protected:
                virtual void TearDown() {
                    q->clear();
                }

                static void SetUpTestCase() {
                    instance = new HazelcastServer(*g_srvFactory);
                    client = new HazelcastClient;
                    q = new mixedtype::IQueue(client->toMixedType().getQueue("MyQueue"));
                }

                static void TearDownTestCase() {
                    delete q;
                    delete client;
                    delete instance;

                    q = NULL;
                    client = NULL;
                    instance = NULL;
                }

                static void testOfferPollThread2(hazelcast::util::ThreadArgs &args) {
                    mixedtype::IQueue *queue = (mixedtype::IQueue *) args.arg0;
                    hazelcast::util::sleep(2);
                    queue->offer<std::string>("item1");
                }

                static HazelcastServer *instance;
                static HazelcastClient *client;
                static mixedtype::IQueue *q;
            };

            HazelcastServer *MixedQueueTest::instance = NULL;
            HazelcastClient *MixedQueueTest::client = NULL;
            mixedtype::IQueue *MixedQueueTest::q = NULL;

            class MixedQueueTestItemListener : public MixedItemListener {
            public:
                MixedQueueTestItemListener(hazelcast::util::CountDownLatch &latch)
                        : latch(latch) {
                }

                virtual void itemAdded(const ItemEvent<TypedData> &item) {
                    latch.countDown();
                }

                virtual void itemRemoved(const ItemEvent<TypedData> &item) {
                }

            private:
                hazelcast::util::CountDownLatch &latch;
            };

            TEST_F(MixedQueueTest, testListener) {
                ASSERT_EQ(0, q->size());

                hazelcast::util::CountDownLatch latch(5);

                MixedQueueTestItemListener listener(latch);
                std::string id = q->addItemListener(listener, true);

                hazelcast::util::sleep(1);

                for (int i = 0; i < 5; i++) {
                    ASSERT_TRUE(
                            q->offer<std::string>(std::string("event_item") + hazelcast::util::IOUtil::to_string(i)));
                }

                ASSERT_TRUE(latch.await(5));
                ASSERT_TRUE(q->removeItemListener(id));

                // added for test coverage
                ASSERT_NO_THROW(q->destroy());
            }

            TEST_F(MixedQueueTest, testOfferPoll) {
                for (int i = 0; i < 10; i++) {
                    bool result = q->offer<std::string>("item");
                    ASSERT_TRUE(result);
                }
                ASSERT_EQ(10, q->size());
                q->poll();
                bool result = q->offer<std::string>("item", 5);
                ASSERT_TRUE(result);

                for (int i = 0; i < 10; i++) {
                    ASSERT_NE(q->poll().get<std::string>().get(), (std::string *) NULL);
                }
                ASSERT_EQ(0, q->size());

                hazelcast::util::StartedThread t2(testOfferPollThread2, q);

                std::unique_ptr<std::string> item = q->poll(30 * 1000).get<std::string>();
                ASSERT_NE(item.get(), (std::string *) NULL);
                ASSERT_EQ("item1", *item);
                t2.join();
            }

            TEST_F(MixedQueueTest, testPeek) {
                ASSERT_TRUE(q->offer<std::string>("peek 1"));
                ASSERT_TRUE(q->offer<std::string>("peek 2"));
                ASSERT_TRUE(q->offer<std::string>("peek 3"));

                std::unique_ptr<std::string> item = q->peek().get<std::string>();
                ASSERT_NE((std::string *) NULL, item.get());
                ASSERT_EQ("peek 1", *item);
            }

            TEST_F(MixedQueueTest, testTake) {
                ASSERT_TRUE(q->offer<std::string>("peek 1"));
                ASSERT_TRUE(q->offer<std::string>("peek 2"));
                ASSERT_TRUE(q->offer<std::string>("peek 3"));

                std::unique_ptr<std::string> item = q->take().get<std::string>();
                ASSERT_NE((std::string *) NULL, item.get());
                ASSERT_EQ("peek 1", *item);

                item = q->take().get<std::string>();
                ASSERT_NE((std::string *) NULL, item.get());
                ASSERT_EQ("peek 2", *item);

                item = q->take().get<std::string>();
                ASSERT_NE((std::string *) NULL, item.get());
                ASSERT_EQ("peek 3", *item);

                ASSERT_TRUE(q->isEmpty());

// start a thread to insert an item
                hazelcast::util::StartedThread t2(testOfferPollThread2, q);

                item = q->take().get<std::string>();  //  should block till it gets an item
                ASSERT_NE((std::string *) NULL, item.get());
                ASSERT_EQ("item1", *item);

                t2.join();
            }

            TEST_F(MixedQueueTest, testRemainingCapacity) {
                int capacity = q->remainingCapacity();
                ASSERT_TRUE(capacity > 10000);
                q->offer<std::string>("item");
                ASSERT_EQ(capacity - 1, q->remainingCapacity());
            }


            TEST_F(MixedQueueTest, testRemove) {
                ASSERT_TRUE(q->offer<std::string>("item1"));
                ASSERT_TRUE(q->offer<std::string>("item2"));
                ASSERT_TRUE(q->offer<std::string>("item3"));

                ASSERT_FALSE(q->remove<std::string>("item4"));
                ASSERT_EQ(3, q->size());

                ASSERT_TRUE(q->remove<std::string>("item2"));

                ASSERT_EQ(2, q->size());

                ASSERT_EQ("item1", *(q->poll().get<std::string>()));
                ASSERT_EQ("item3", *(q->poll().get<std::string>()));
            }


            TEST_F(MixedQueueTest, testContains) {
                ASSERT_TRUE(q->offer<std::string>("item1"));
                ASSERT_TRUE(q->offer<std::string>("item2"));
                ASSERT_TRUE(q->offer<std::string>("item3"));
                ASSERT_TRUE(q->offer<std::string>("item4"));
                ASSERT_TRUE(q->offer<std::string>("item5"));


                ASSERT_TRUE(q->contains<std::string>("item3"));
                ASSERT_FALSE(q->contains<std::string>("item"));

                std::vector<std::string> list;
                list.push_back("item4");
                list.push_back("item2");

                ASSERT_TRUE(q->containsAll<std::string>(list));

                list.push_back("item");
                ASSERT_FALSE(q->containsAll(list));
            }

            TEST_F(MixedQueueTest, testDrain) {
                ASSERT_TRUE(q->offer<std::string>("item1"));
                ASSERT_TRUE(q->offer<std::string>("item2"));
                ASSERT_TRUE(q->offer<std::string>("item3"));
                ASSERT_TRUE(q->offer<std::string>("item4"));
                ASSERT_TRUE(q->offer<std::string>("item5"));

                std::vector<TypedData> list;
                size_t result = q->drainTo(list, 2);
                ASSERT_EQ(2U, result);
                ASSERT_EQ("item1", *list[0].get<std::string>());
                ASSERT_EQ("item2", *list[1].get<std::string>());

                std::vector<TypedData> list2;
                result = q->drainTo(list2);
                ASSERT_EQ(3U, result);
                ASSERT_EQ("item3", *list2[0].get<std::string>());
                ASSERT_EQ("item4", *list2[1].get<std::string>());
                ASSERT_EQ("item5", *list2[2].get<std::string>());

                ASSERT_TRUE(q->offer<std::string>("item1"));
                ASSERT_TRUE(q->offer<std::string>("item2"));
                ASSERT_TRUE(q->offer<std::string>("item3"));
                list2.clear();
                result = q->drainTo(list2, 5);
                ASSERT_EQ(3U, result);
                ASSERT_EQ("item1", *list2[0].get<std::string>());
                ASSERT_EQ("item2", *list2[1].get<std::string>());
                ASSERT_EQ("item3", *list2[2].get<std::string>());
            }

            TEST_F(MixedQueueTest, testToArray) {
                ASSERT_TRUE(q->offer<std::string>("item1"));
                ASSERT_TRUE(q->offer<std::string>("item2"));
                ASSERT_TRUE(q->offer<std::string>("item3"));
                ASSERT_TRUE(q->offer<std::string>("item4"));
                ASSERT_TRUE(q->offer<std::string>("item5"));

                std::vector<TypedData> array = q->toArray();
                size_t size = array.size();
                for (size_t i = 0; i < size; i++) {
                    ASSERT_EQ(std::string("item") + hazelcast::util::IOUtil::to_string(i + 1),
                              *array[i].get<std::string>());
                }
            }

            TEST_F(MixedQueueTest, testAddAll) {
                std::vector<std::string> coll;
                coll.push_back("item1");
                coll.push_back("item2");
                coll.push_back("item3");
                coll.push_back("item4");

                ASSERT_TRUE(q->addAll<std::string>(coll));
                int size = q->size();
                ASSERT_EQ(size, (int) coll.size());
            }

            TEST_F(MixedQueueTest, testRemoveRetain) {
                ASSERT_TRUE(q->offer<std::string>("item1"));
                ASSERT_TRUE(q->offer<std::string>("item2"));
                ASSERT_TRUE(q->offer<std::string>("item3"));
                ASSERT_TRUE(q->offer<std::string>("item4"));
                ASSERT_TRUE(q->offer<std::string>("item5"));

                std::vector<std::string> list;
                list.push_back("item8");
                list.push_back("item9");
                ASSERT_FALSE(q->removeAll(list));
                ASSERT_EQ(5, q->size());

                list.push_back("item3");
                list.push_back("item4");
                list.push_back("item1");
                ASSERT_TRUE(q->removeAll(list));
                ASSERT_EQ(2, q->size());

                list.clear();
                list.push_back("item2");
                list.push_back("item5");
                ASSERT_FALSE(q->retainAll<std::string>(list));
                ASSERT_EQ(2, q->size());

                list.clear();
                ASSERT_TRUE(q->retainAll<std::string>(list));
                ASSERT_EQ(0, q->size());
            }

            TEST_F(MixedQueueTest, testClear) {
                ASSERT_TRUE(q->offer<std::string>("item1"));
                ASSERT_TRUE(q->offer<std::string>("item2"));
                ASSERT_TRUE(q->offer<std::string>("item3"));
                ASSERT_TRUE(q->offer<std::string>("item4"));
                ASSERT_TRUE(q->offer<std::string>("item5"));

                q->clear();

                ASSERT_EQ(0, q->size());
                ASSERT_EQ(q->poll().get<std::string>().get(), (std::string *) NULL);
            }
        }
    }
}




int hazelcast::client::test::executor::tasks::SelectAllMembers::getFactoryId() const {
    return IdentifiedFactory::FACTORY_ID;
}

int hazelcast::client::test::executor::tasks::SelectAllMembers::getClassId() const {
    return IdentifiedFactory::SELECT_ALL_MEMBERS;
}

void hazelcast::client::test::executor::tasks::SelectAllMembers::writeData(
        hazelcast::client::serialization::ObjectDataOutput &writer) const {
}

void hazelcast::client::test::executor::tasks::SelectAllMembers::readData(
        hazelcast::client::serialization::ObjectDataInput &reader) {
}

hazelcast::client::test::executor::tasks::SelectAllMembers::SelectAllMembers() {}

bool hazelcast::client::test::executor::tasks::SelectAllMembers::select(const hazelcast::client::Member &member) const {
    return true;
}

void hazelcast::client::test::executor::tasks::SelectAllMembers::toString(std::ostream &os) const {
    os << "SelectAllMembers";
}




int hazelcast::client::test::executor::tasks::CancellationAwareTask::getFactoryId() const {
    return IdentifiedFactory::FACTORY_ID;
}

int hazelcast::client::test::executor::tasks::CancellationAwareTask::getClassId() const {
    return IdentifiedFactory::CANCELLATION_AWARE_TASK;
}

void hazelcast::client::test::executor::tasks::CancellationAwareTask::writeData(
        hazelcast::client::serialization::ObjectDataOutput &writer) const {
    writer.writeLong(sleepTime);
}

void hazelcast::client::test::executor::tasks::CancellationAwareTask::readData(
        hazelcast::client::serialization::ObjectDataInput &reader) {
    sleepTime = reader.readLong();
}

hazelcast::client::test::executor::tasks::CancellationAwareTask::CancellationAwareTask(int64_t sleepTime) : sleepTime(
        sleepTime) {}

hazelcast::client::test::executor::tasks::CancellationAwareTask::CancellationAwareTask() {}



int hazelcast::client::test::executor::tasks::NullCallable::getFactoryId() const {
    return IdentifiedFactory::FACTORY_ID;
}

int hazelcast::client::test::executor::tasks::NullCallable::getClassId() const {
    return IdentifiedFactory::NULL_CALLABLE;
}

void hazelcast::client::test::executor::tasks::NullCallable::writeData(
        hazelcast::client::serialization::ObjectDataOutput &writer) const {
}

void hazelcast::client::test::executor::tasks::NullCallable::readData(
        hazelcast::client::serialization::ObjectDataInput &reader) {
}

hazelcast::client::test::executor::tasks::NullCallable::NullCallable() {}



int hazelcast::client::test::executor::tasks::SerializedCounterCallable::getFactoryId() const {
    return IdentifiedFactory::FACTORY_ID;
}

int hazelcast::client::test::executor::tasks::SerializedCounterCallable::getClassId() const {
    return IdentifiedFactory::SERIALIZED_COUNTER_CALLABLE;
}

void hazelcast::client::test::executor::tasks::SerializedCounterCallable::writeData(
        hazelcast::client::serialization::ObjectDataOutput &writer) const {
    writer.writeInt(counter + 1);
}

void hazelcast::client::test::executor::tasks::SerializedCounterCallable::readData(
        hazelcast::client::serialization::ObjectDataInput &reader) {
    counter = reader.readInt() + 1;
}

hazelcast::client::test::executor::tasks::SerializedCounterCallable::SerializedCounterCallable() : counter(0) {}



int hazelcast::client::test::executor::tasks::MapPutPartitionAwareCallable::getFactoryId() const {
    return IdentifiedFactory::FACTORY_ID;
}

int hazelcast::client::test::executor::tasks::MapPutPartitionAwareCallable::getClassId() const {
    return IdentifiedFactory::MAP_PUTPARTITIONAWARE_CALLABLE;
}

void hazelcast::client::test::executor::tasks::MapPutPartitionAwareCallable::writeData(
        hazelcast::client::serialization::ObjectDataOutput &writer) const {
    writer.writeUTF(&mapName);
    writer.writeObject<std::string>(&partitionKey);
}

void hazelcast::client::test::executor::tasks::MapPutPartitionAwareCallable::readData(
        hazelcast::client::serialization::ObjectDataInput &reader) {
    // no need to implement at client side for the tests
}

hazelcast::client::test::executor::tasks::MapPutPartitionAwareCallable::MapPutPartitionAwareCallable() {}

hazelcast::client::test::executor::tasks::MapPutPartitionAwareCallable::MapPutPartitionAwareCallable(
        const std::string &mapName, const std::string &partitionKey) : mapName(mapName), partitionKey(partitionKey) {}

const std::string *hazelcast::client::test::executor::tasks::MapPutPartitionAwareCallable::getPartitionKey() const {
    return &partitionKey;
}



int hazelcast::client::test::executor::tasks::SelectNoMembers::getFactoryId() const {
    return IdentifiedFactory::FACTORY_ID;
}

int hazelcast::client::test::executor::tasks::SelectNoMembers::getClassId() const {
    return IdentifiedFactory::SELECT_NO_MEMBERS;
}

void hazelcast::client::test::executor::tasks::SelectNoMembers::writeData(
        hazelcast::client::serialization::ObjectDataOutput &writer) const {
}

void hazelcast::client::test::executor::tasks::SelectNoMembers::readData(
        hazelcast::client::serialization::ObjectDataInput &reader) {
}

hazelcast::client::test::executor::tasks::SelectNoMembers::SelectNoMembers() {}

bool hazelcast::client::test::executor::tasks::SelectNoMembers::select(const hazelcast::client::Member &member) const {
    return false;
}

void hazelcast::client::test::executor::tasks::SelectNoMembers::toString(std::ostream &os) const {
    os << "SelectNoMembers";
}



int hazelcast::client::test::executor::tasks::GetMemberUuidTask::getFactoryId() const {
    return IdentifiedFactory::FACTORY_ID;
}

int hazelcast::client::test::executor::tasks::GetMemberUuidTask::getClassId() const {
    return IdentifiedFactory::GET_MEMBER_UUID_TASK;
}

void hazelcast::client::test::executor::tasks::GetMemberUuidTask::writeData(
        hazelcast::client::serialization::ObjectDataOutput &writer) const {
}

void hazelcast::client::test::executor::tasks::GetMemberUuidTask::readData(
        hazelcast::client::serialization::ObjectDataInput &reader) {
}

hazelcast::client::test::executor::tasks::GetMemberUuidTask::GetMemberUuidTask() {}



int hazelcast::client::test::executor::tasks::FailingCallable::getFactoryId() const {
    return IdentifiedFactory::FACTORY_ID;
}

int hazelcast::client::test::executor::tasks::FailingCallable::getClassId() const {
    return IdentifiedFactory::FAILING_CALLABLE;
}

void hazelcast::client::test::executor::tasks::FailingCallable::writeData(
        hazelcast::client::serialization::ObjectDataOutput &writer) const {
}

void hazelcast::client::test::executor::tasks::FailingCallable::readData(
        hazelcast::client::serialization::ObjectDataInput &reader) {
}

hazelcast::client::test::executor::tasks::FailingCallable::FailingCallable() {}



std::string hazelcast::client::test::executor::tasks::AppendCallable::APPENDAGE = ":CallableResult";

int hazelcast::client::test::executor::tasks::AppendCallable::getFactoryId() const {
    return IdentifiedFactory::FACTORY_ID;
}

int hazelcast::client::test::executor::tasks::AppendCallable::getClassId() const {
    return IdentifiedFactory::APPEND_CALLABLE;
}

void hazelcast::client::test::executor::tasks::AppendCallable::writeData(
        hazelcast::client::serialization::ObjectDataOutput &writer) const {
    writer.writeUTF(&msg);
}

void hazelcast::client::test::executor::tasks::AppendCallable::readData(
        hazelcast::client::serialization::ObjectDataInput &reader) {
    // No need to implement this part for the client
}

hazelcast::client::test::executor::tasks::AppendCallable::AppendCallable() {}

hazelcast::client::test::executor::tasks::AppendCallable::AppendCallable(const std::string &msg) : msg(msg) {}



int hazelcast::client::test::executor::tasks::TaskWithUnserializableResponse::getFactoryId() const {
    return IdentifiedFactory::FACTORY_ID;
}

int hazelcast::client::test::executor::tasks::TaskWithUnserializableResponse::getClassId() const {
    return IdentifiedFactory::TASK_WITH_UNSERIALIZABLE_RESPONSE;
}

void hazelcast::client::test::executor::tasks::TaskWithUnserializableResponse::writeData(
        hazelcast::client::serialization::ObjectDataOutput &writer) const {
}

void hazelcast::client::test::executor::tasks::TaskWithUnserializableResponse::readData(
        hazelcast::client::serialization::ObjectDataInput &reader) {
}

hazelcast::client::test::executor::tasks::TaskWithUnserializableResponse::TaskWithUnserializableResponse() {}





namespace hazelcast {
    namespace client {
        namespace test {
            class ClientExecutorServiceTest : public ClientTestSupport {
            protected:
                static const size_t numberOfMembers;

                virtual void TearDown() {
                }

                static void SetUpTestCase() {
                    factory = new HazelcastServerFactory(g_srvFactory->getServerAddress(),
                                                         "hazelcast/test/resources/hazelcast-test-executor.xml");
                    for (size_t i = 0; i < numberOfMembers; ++i) {
                        instances.push_back(new HazelcastServer(*factory));
                    }
                    client = new HazelcastClient;
                }

                static void TearDownTestCase() {
                    delete client;
                    for (HazelcastServer *server : instances) {
                        server->shutdown();
                        delete server;
                    }

                    client = NULL;
                }

                class FailingExecutionCallback : public ExecutionCallback<std::string> {
                public:
                    FailingExecutionCallback(const std::shared_ptr<hazelcast::util::CountDownLatch> &latch) : latch(latch) {}

                    virtual void onResponse(const std::shared_ptr<std::string> &response) {
                    }

                    virtual void onFailure(const std::shared_ptr<exception::IException> &e) {
                        latch->countDown();
                        exception = e;
                    }

                    std::shared_ptr<exception::IException> getException() {
                        return exception.get();
                    }

                private:
                    const std::shared_ptr<hazelcast::util::CountDownLatch> latch;
                    hazelcast::util::Sync<std::shared_ptr<exception::IException> > exception;
                };

                class SuccessfullExecutionCallback : public ExecutionCallback<std::string> {
                public:
                    SuccessfullExecutionCallback(const std::shared_ptr<hazelcast::util::CountDownLatch> &latch) : latch(latch) {}

                    virtual void onResponse(const std::shared_ptr<std::string> &response) {
                        latch->countDown();
                    }

                    virtual void onFailure(const std::shared_ptr<exception::IException> &e) {
                    }

                private:
                    const std::shared_ptr<hazelcast::util::CountDownLatch> latch;
                };

                class ResultSettingExecutionCallback : public ExecutionCallback<std::string> {
                public:
                    ResultSettingExecutionCallback(const std::shared_ptr<hazelcast::util::CountDownLatch> &latch) : latch(latch) {}

                    virtual void onResponse(const std::shared_ptr<std::string> &response) {
                        result.set(response);
                        latch->countDown();
                    }

                    virtual void onFailure(const std::shared_ptr<exception::IException> &e) {
                    }

                    std::shared_ptr<std::string> getResult() {
                        return result.get();
                    }

                private:
                    const std::shared_ptr<hazelcast::util::CountDownLatch> latch;
                    hazelcast::util::Sync<std::shared_ptr<std::string>> result;
                };

                class MultiExecutionCompletionCallback : public MultiExecutionCallback<std::string> {
                public:
                    MultiExecutionCompletionCallback(const string &msg,
                                                     const std::shared_ptr<hazelcast::util::CountDownLatch> &responseLatch,
                                                     const std::shared_ptr<hazelcast::util::CountDownLatch> &completeLatch)
                            : msg(
                            msg),
                              responseLatch(
                                      responseLatch),
                              completeLatch(
                                      completeLatch) {}

                    virtual void onResponse(const Member &member, const std::shared_ptr<std::string> &response) {
                        if (response.get() && *response == msg + executor::tasks::AppendCallable::APPENDAGE) {
                            responseLatch->countDown();
                        }
                    }

                    virtual void
                    onFailure(const Member &member, const std::shared_ptr<exception::IException> &exception) {
                    }

                    virtual void onComplete(const std::map<Member, std::shared_ptr<std::string> > &values,
                                            const std::map<Member, std::shared_ptr<exception::IException> > &exceptions) {
                        typedef std::map<Member, std::shared_ptr<std::string> > VALUE_MAP;
                        std::string expectedValue(msg + executor::tasks::AppendCallable::APPENDAGE);
                        for (const VALUE_MAP::value_type &entry  : values) {
                            if (entry.second.get() && *entry.second == expectedValue) {
                                completeLatch->countDown();
                            }
                        }
                    }

                private:
                    std::string msg;
                    const std::shared_ptr<hazelcast::util::CountDownLatch> responseLatch;
                    const std::shared_ptr<hazelcast::util::CountDownLatch> completeLatch;
                };

                class MultiExecutionNullCallback : public MultiExecutionCallback<std::string> {
                public:
                    MultiExecutionNullCallback(const std::shared_ptr<hazelcast::util::CountDownLatch> &responseLatch,
                                               const std::shared_ptr<hazelcast::util::CountDownLatch> &completeLatch)
                            : responseLatch(responseLatch), completeLatch(completeLatch) {}

                    virtual void onResponse(const Member &member, const std::shared_ptr<std::string> &response) {
                        if (response.get() == NULL) {
                            responseLatch->countDown();
                        }
                    }

                    virtual void
                    onFailure(const Member &member, const std::shared_ptr<exception::IException> &exception) {
                    }

                    virtual void onComplete(const std::map<Member, std::shared_ptr<std::string> > &values,
                                            const std::map<Member, std::shared_ptr<exception::IException> > &exceptions) {
                        typedef std::map<Member, std::shared_ptr<std::string> > VALUE_MAP;
                        for (const VALUE_MAP::value_type &entry  : values) {
                            if (entry.second.get() == NULL) {
                                completeLatch->countDown();
                            }
                        }
                    }

                private:
                    const std::shared_ptr<hazelcast::util::CountDownLatch> responseLatch;
                    const std::shared_ptr<hazelcast::util::CountDownLatch> completeLatch;
                };

                static std::vector<HazelcastServer *> instances;
                static HazelcastClient *client;
                static HazelcastServerFactory *factory;
            };

            std::vector<HazelcastServer *>ClientExecutorServiceTest::instances;
            HazelcastClient *ClientExecutorServiceTest::client = NULL;
            HazelcastServerFactory *ClientExecutorServiceTest::factory = NULL;
            const size_t ClientExecutorServiceTest::numberOfMembers = 4;

            TEST_F(ClientExecutorServiceTest, testIsTerminated) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                ASSERT_FALSE(service->isTerminated());
            }

            TEST_F(ClientExecutorServiceTest, testIsShutdown) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                ASSERT_FALSE(service->isShutdown());
            }

            TEST_F(ClientExecutorServiceTest, testShutdown) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                service->shutdown();

                ASSERT_TRUE_EVENTUALLY(service->isShutdown());
            }

            TEST_F(ClientExecutorServiceTest, testShutdownMultipleTimes) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                service->shutdown();
                service->shutdown();

                ASSERT_TRUE_EVENTUALLY(service->isShutdown());
            }

            TEST_F(ClientExecutorServiceTest, testCancellationAwareTask_whenTimeOut) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                executor::tasks::CancellationAwareTask task(INT64_MAX);

                std::shared_ptr<ICompletableFuture<bool> > future = service->submit<executor::tasks::CancellationAwareTask, bool>(
                        task);

                ASSERT_THROW(future->get(1, hazelcast::util::concurrent::TimeUnit::SECONDS()),
                             exception::TimeoutException);
            }

            TEST_F(ClientExecutorServiceTest, testFutureAfterCancellationAwareTaskTimeOut) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                executor::tasks::CancellationAwareTask task(INT64_MAX);

                std::shared_ptr<ICompletableFuture<bool> > future = service->submit<executor::tasks::CancellationAwareTask, bool>(
                        task);

                try {
                    future->get(1, hazelcast::util::concurrent::TimeUnit::SECONDS());
                } catch (TimeoutException &ignored) {
                }

                ASSERT_FALSE(future->isDone());
                ASSERT_FALSE(future->isCancelled());
            }

            TEST_F(ClientExecutorServiceTest, testGetFutureAfterCancel) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                executor::tasks::CancellationAwareTask task(INT64_MAX);

                std::shared_ptr<ICompletableFuture<bool> > future = service->submit<executor::tasks::CancellationAwareTask, bool>(
                        task);

                try {
                    future->get(1, hazelcast::util::concurrent::TimeUnit::SECONDS());
                } catch (TimeoutException &ignored) {
                }

                ASSERT_TRUE(future->cancel(true));

                ASSERT_THROW(future->get(), exception::CancellationException);
            }

            TEST_F(ClientExecutorServiceTest, testSubmitFailingCallableException) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                executor::tasks::FailingCallable task;

                std::shared_ptr<ICompletableFuture<std::string> > future = service->submit<executor::tasks::FailingCallable, std::string>(
                        task);

                ASSERT_THROW(future->get(), exception::ExecutionException);
            }

            TEST_F(ClientExecutorServiceTest, testSubmitFailingCallableException_withExecutionCallback) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                std::shared_ptr<hazelcast::util::CountDownLatch> latch(new hazelcast::util::CountDownLatch(1));

                executor::tasks::FailingCallable task;
                std::shared_ptr<ExecutionCallback<std::string> > callback(new FailingExecutionCallback(latch));

                service->submit<executor::tasks::FailingCallable, std::string>(task, callback);

                ASSERT_OPEN_EVENTUALLY(*latch);
            }

            TEST_F(ClientExecutorServiceTest, testSubmitFailingCallableReasonExceptionCause) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                const std::shared_ptr<ICompletableFuture<std::string> > &failingFuture = service->submit<executor::tasks::FailingCallable, std::string>(
                        executor::tasks::FailingCallable());

                try {
                    failingFuture->get();
                } catch (exception::ExecutionException &e) {
                    ASSERT_THROW(e.getCause()->raise(), exception::IllegalStateException);
                }
            }

            TEST_F(ClientExecutorServiceTest, testExecute_withNoMemberSelected) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                std::string mapName = randomMapName();

                executor::tasks::SelectNoMembers selector;

                ASSERT_THROW(service->execute<executor::tasks::MapPutPartitionAwareCallable>(
                        executor::tasks::MapPutPartitionAwareCallable(mapName, randomString()),
                        selector),
                             exception::RejectedExecutionException);
            }

            TEST_F(ClientExecutorServiceTest, testCallableSerializedOnce) {
                std::string name = getTestName();

                std::shared_ptr<IExecutorService> service = client->getExecutorService(name);

                executor::tasks::SerializedCounterCallable counterCallable;

                std::shared_ptr<ICompletableFuture<int> > future = service->submitToKeyOwner<executor::tasks::SerializedCounterCallable, int, std::string>(
                        counterCallable, name);
                std::shared_ptr<int> value = future->get();
                ASSERT_NOTNULL(value.get(), int);
                ASSERT_EQ(2, *value);
            }

            TEST_F(ClientExecutorServiceTest, testCallableSerializedOnce_submitToAddress) {
                std::string name = getTestName();

                std::shared_ptr<IExecutorService> service = client->getExecutorService(name);

                executor::tasks::SerializedCounterCallable counterCallable;

                std::vector<Member> members = client->getCluster().getMembers();
                ASSERT_FALSE(members.empty());
                std::shared_ptr<ICompletableFuture<int> > future = service->submitToMember<executor::tasks::SerializedCounterCallable, int>(
                        counterCallable, members[0]);
                std::shared_ptr<int> value = future->get();
                ASSERT_NOTNULL(value.get(), int);
                ASSERT_EQ(2, *value);
            }

            TEST_F(ClientExecutorServiceTest, testUnserializableResponse_exceptionPropagatesToClient) {
                std::string name = getTestName();

                std::shared_ptr<IExecutorService> service = client->getExecutorService(name);

                executor::tasks::TaskWithUnserializableResponse taskWithUnserializableResponse;

                std::shared_ptr<ICompletableFuture<bool> > future = service->submit<executor::tasks::TaskWithUnserializableResponse, bool>(
                        taskWithUnserializableResponse);

                try {
                    future->get();
                } catch (exception::ExecutionException &e) {
                    ASSERT_THROW(e.getCause()->raise(), exception::HazelcastSerializationException);
                }
            }

            TEST_F(ClientExecutorServiceTest, testUnserializableResponse_exceptionPropagatesToClientCallback) {
                std::string name = getTestName();

                std::shared_ptr<IExecutorService> service = client->getExecutorService(name);

                executor::tasks::TaskWithUnserializableResponse taskWithUnserializableResponse;

                std::shared_ptr<hazelcast::util::CountDownLatch> latch(new hazelcast::util::CountDownLatch(1));

                std::shared_ptr<FailingExecutionCallback> callback(new FailingExecutionCallback(latch));

                service->submit<executor::tasks::TaskWithUnserializableResponse, std::string>(
                        taskWithUnserializableResponse, callback);

                ASSERT_OPEN_EVENTUALLY(*latch);

                std::shared_ptr<exception::IException> exception = callback->getException();
                ASSERT_NOTNULL(exception.get(), exception::IException);
                ASSERT_THROW(exception->raise(), exception::HazelcastSerializationException);
            }

            TEST_F(ClientExecutorServiceTest, testSubmitCallableToMember) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                executor::tasks::GetMemberUuidTask task;

                std::vector<Member> members = client->getCluster().getMembers();
                ASSERT_EQ(numberOfMembers, members.size());

                std::shared_ptr<ICompletableFuture<std::string> > future = service->submitToMember<executor::tasks::GetMemberUuidTask, std::string>(
                        task, members[0]);

                std::shared_ptr<std::string> uuid = future->get();
                ASSERT_NOTNULL(uuid.get(), std::string);
                ASSERT_EQ(members[0].getUuid(), *uuid);
            }

            TEST_F(ClientExecutorServiceTest, testSubmitCallableToMembers) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                executor::tasks::GetMemberUuidTask task;

                std::vector<Member> members = client->getCluster().getMembers();
                ASSERT_EQ(numberOfMembers, members.size());

                std::map<Member, std::shared_ptr<ICompletableFuture<std::string> > > futuresMap = service->submitToMembers<executor::tasks::GetMemberUuidTask, std::string>(
                        task, members);

                for (const Member &member : members) {
                    ASSERT_EQ(1U, futuresMap.count(member));
                    std::shared_ptr<std::string> uuid = futuresMap[member]->get();
                    ASSERT_NOTNULL(uuid.get(), std::string);
                    ASSERT_EQ(member.getUuid(), *uuid);
                }
            }

            TEST_F(ClientExecutorServiceTest, testSubmitCallable_withMemberSelector) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                std::string msg = randomString();
                executor::tasks::AppendCallable callable(msg);
                executor::tasks::SelectAllMembers selectAll;

                std::shared_ptr<ICompletableFuture<std::string> > f = service->submit<executor::tasks::AppendCallable, std::string>(
                        callable, selectAll);

                std::shared_ptr<std::string> result = f->get();
                ASSERT_NOTNULL(result.get(), std::string);
                ASSERT_EQ(msg + executor::tasks::AppendCallable::APPENDAGE, *result);
            }

            TEST_F(ClientExecutorServiceTest, testSubmitCallableToMembers_withMemberSelector) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                executor::tasks::GetMemberUuidTask task;
                executor::tasks::SelectAllMembers selectAll;

                typedef std::map<Member, std::shared_ptr<ICompletableFuture<std::string> > > FUTURESMAP;
                FUTURESMAP futuresMap = service->submitToMembers<executor::tasks::GetMemberUuidTask, std::string>(
                        task, selectAll);

                for (const FUTURESMAP::value_type &pair : futuresMap) {
                    const Member &member = pair.first;
                    const std::shared_ptr<ICompletableFuture<std::string> > &future = pair.second;

                    std::shared_ptr<std::string> uuid = future->get();

                    ASSERT_NOTNULL(uuid.get(), std::string);
                    ASSERT_EQ(member.getUuid(), *uuid);
                }
            }

            TEST_F(ClientExecutorServiceTest, submitCallableToAllMembers) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                std::string msg = randomString();
                executor::tasks::AppendCallable callable(msg);

                typedef std::map<Member, std::shared_ptr<ICompletableFuture<std::string> > > FUTURESMAP;
                FUTURESMAP futuresMap = service->submitToAllMembers<executor::tasks::AppendCallable, std::string>(
                        callable);

                for (const FUTURESMAP::value_type &pair : futuresMap) {
                    const std::shared_ptr<ICompletableFuture<std::string> > &future = pair.second;

                    std::shared_ptr<std::string> result = future->get();

                    ASSERT_NOTNULL(result.get(), std::string);
                    ASSERT_EQ(msg + executor::tasks::AppendCallable::APPENDAGE, *result);
                }
            }

            TEST_F(ClientExecutorServiceTest, submitCallableToMember_withExecutionCallback) {
                std::string testName = getTestName();
                std::shared_ptr<IExecutorService> service = client->getExecutorService(testName);

                executor::tasks::MapPutPartitionAwareCallable callable(testName, randomString());

                std::shared_ptr<hazelcast::util::CountDownLatch> latch(new hazelcast::util::CountDownLatch(1));
                std::shared_ptr<SuccessfullExecutionCallback> callback(new SuccessfullExecutionCallback(latch));

                std::vector<Member> members = client->getCluster().getMembers();
                ASSERT_EQ(numberOfMembers, members.size());

                service->submitToMember<executor::tasks::MapPutPartitionAwareCallable, std::string>(callable,
                                                                                                    members[0],
                                                                                                    callback);

                IMap<std::string, std::string> map = client->getMap<std::string, std::string>(testName);

                ASSERT_OPEN_EVENTUALLY(*latch);
                ASSERT_EQ(1, map.size());
            }

            TEST_F(ClientExecutorServiceTest, submitCallableToMember_withMultiExecutionCallback) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                std::shared_ptr<hazelcast::util::CountDownLatch> responseLatch(
                        new hazelcast::util::CountDownLatch(numberOfMembers));
                std::shared_ptr<hazelcast::util::CountDownLatch> completeLatch(
                        new hazelcast::util::CountDownLatch(numberOfMembers));

                std::string msg = randomString();
                executor::tasks::AppendCallable callable(msg);
                std::vector<Member> members = client->getCluster().getMembers();
                ASSERT_EQ(numberOfMembers, members.size());

                std::shared_ptr<MultiExecutionCallback<std::string> > callback(
                        new MultiExecutionCompletionCallback(msg, responseLatch, completeLatch));

                service->submitToMembers<executor::tasks::AppendCallable, std::string>(callable, members, callback);

                ASSERT_OPEN_EVENTUALLY(*responseLatch);
                ASSERT_OPEN_EVENTUALLY(*completeLatch);
            }

            TEST_F(ClientExecutorServiceTest, submitCallable_withExecutionCallback) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                std::string msg = randomString();
                executor::tasks::AppendCallable callable(msg);
                executor::tasks::SelectAllMembers selector;
                std::shared_ptr<hazelcast::util::CountDownLatch> responseLatch(new hazelcast::util::CountDownLatch(1));
                std::shared_ptr<ResultSettingExecutionCallback> callback(
                        new ResultSettingExecutionCallback(responseLatch));

                service->submit<executor::tasks::AppendCallable, std::string>(callable, selector,
                                                                              static_pointer_cast<ExecutionCallback<std::string>>(
                                                                                      callback));

                ASSERT_OPEN_EVENTUALLY(*responseLatch);
                std::shared_ptr<std::string> message = callback->getResult();
                ASSERT_NOTNULL(message.get(), std::string);
                ASSERT_EQ(msg + executor::tasks::AppendCallable::APPENDAGE, *message);
            }

            TEST_F(ClientExecutorServiceTest, submitCallableToMembers_withExecutionCallback) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                std::shared_ptr<hazelcast::util::CountDownLatch> responseLatch(
                        new hazelcast::util::CountDownLatch(numberOfMembers));
                std::shared_ptr<hazelcast::util::CountDownLatch> completeLatch(
                        new hazelcast::util::CountDownLatch(numberOfMembers));

                std::string msg = randomString();
                executor::tasks::AppendCallable callable(msg);
                executor::tasks::SelectAllMembers selector;

                std::shared_ptr<MultiExecutionCallback<std::string> > callback(
                        new MultiExecutionCompletionCallback(msg, responseLatch, completeLatch));

                service->submitToMembers<executor::tasks::AppendCallable, std::string>(callable, selector, callback);

                ASSERT_OPEN_EVENTUALLY(*responseLatch);
                ASSERT_OPEN_EVENTUALLY(*completeLatch);
            }

            TEST_F(ClientExecutorServiceTest, submitCallableToAllMembers_withMultiExecutionCallback) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                std::shared_ptr<hazelcast::util::CountDownLatch> responseLatch(
                        new hazelcast::util::CountDownLatch(numberOfMembers));
                std::shared_ptr<hazelcast::util::CountDownLatch> completeLatch(
                        new hazelcast::util::CountDownLatch(numberOfMembers));

                std::string msg = randomString();
                executor::tasks::AppendCallable callable(msg);

                std::shared_ptr<MultiExecutionCallback<std::string> > callback(
                        new MultiExecutionCompletionCallback(msg, responseLatch, completeLatch));

                service->submitToAllMembers<executor::tasks::AppendCallable, std::string>(callable, callback);

                ASSERT_OPEN_EVENTUALLY(*completeLatch);
                ASSERT_OPEN_EVENTUALLY(*responseLatch);
            }

            TEST_F(ClientExecutorServiceTest, submitCallableWithNullResultToAllMembers_withMultiExecutionCallback) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                std::shared_ptr<hazelcast::util::CountDownLatch> responseLatch(
                        new hazelcast::util::CountDownLatch(numberOfMembers));
                std::shared_ptr<hazelcast::util::CountDownLatch> completeLatch(
                        new hazelcast::util::CountDownLatch(numberOfMembers));

                executor::tasks::NullCallable callable;

                std::shared_ptr<MultiExecutionCallback<std::string> > callback(
                        new MultiExecutionNullCallback(responseLatch, completeLatch));

                service->submitToAllMembers<executor::tasks::NullCallable, std::string>(callable, callback);

                ASSERT_OPEN_EVENTUALLY(*responseLatch);
                ASSERT_OPEN_EVENTUALLY(*completeLatch);
            }

            TEST_F(ClientExecutorServiceTest, testSubmitCallable) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                std::string msg = randomString();
                executor::tasks::AppendCallable callable(msg);

                std::shared_ptr<ICompletableFuture<std::string> > result = service->submit<executor::tasks::AppendCallable, std::string>(
                        callable);

                std::shared_ptr<std::string> message = result->get();
                ASSERT_NOTNULL(message.get(), std::string);
                ASSERT_EQ(msg + executor::tasks::AppendCallable::APPENDAGE, *message);
            }

            TEST_F(ClientExecutorServiceTest, testSubmitCallable_withExecutionCallback) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                std::string msg = randomString();
                executor::tasks::AppendCallable callable(msg);

                std::shared_ptr<hazelcast::util::CountDownLatch> latch(new hazelcast::util::CountDownLatch(1));
                std::shared_ptr<ResultSettingExecutionCallback> callback(new ResultSettingExecutionCallback(latch));

                service->submit<executor::tasks::AppendCallable, std::string>(callable,
                                                                              static_pointer_cast<ExecutionCallback<std::string>>(
                                                                                      callback));

                ASSERT_OPEN_EVENTUALLY(*latch);
                std::shared_ptr<std::string> value = callback->getResult();
                ASSERT_NOTNULL(value.get(), std::string);
                ASSERT_EQ(msg + executor::tasks::AppendCallable::APPENDAGE, *value);
            }

            TEST_F(ClientExecutorServiceTest, submitCallableToKeyOwner) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                std::string msg = randomString();
                executor::tasks::AppendCallable callable(msg);

                std::shared_ptr<ICompletableFuture<std::string> > f = service->submitToKeyOwner<executor::tasks::AppendCallable, std::string, std::string>(
                        callable, "key");

                std::shared_ptr<std::string> result = f->get();
                ASSERT_NOTNULL(result.get(), std::string);
                ASSERT_EQ(msg + executor::tasks::AppendCallable::APPENDAGE, *result);
            }

            TEST_F(ClientExecutorServiceTest, submitCallableToKeyOwner_withExecutionCallback) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                std::string msg = randomString();
                executor::tasks::AppendCallable callable(msg);

                std::shared_ptr<hazelcast::util::CountDownLatch> latch(new hazelcast::util::CountDownLatch(1));
                std::shared_ptr<ResultSettingExecutionCallback> callback(new ResultSettingExecutionCallback(latch));

                service->submitToKeyOwner<executor::tasks::AppendCallable, std::string, std::string>(callable, "key",
                                                                                                     static_pointer_cast<ExecutionCallback<std::string>>(
                                                                                                             callback));

                ASSERT_OPEN_EVENTUALLY(*latch);
                std::shared_ptr<std::string> value = callback->getResult();
                ASSERT_NOTNULL(value.get(), std::string);
                ASSERT_EQ(msg + executor::tasks::AppendCallable::APPENDAGE, *value);
            }

            TEST_F(ClientExecutorServiceTest, submitCallablePartitionAware) {
                std::string testName = getTestName();
                std::shared_ptr<IExecutorService> service = client->getExecutorService(testName);

                IMap<std::string, std::string> map = client->getMap<std::string, std::string>(testName);

                std::vector<Member> members = client->getCluster().getMembers();
                spi::ClientContext clientContext(*client);
                Member &member = members[0];
                std::string key = generateKeyOwnedBy(clientContext, member);

                executor::tasks::MapPutPartitionAwareCallable callable(testName, key);

                std::shared_ptr<ICompletableFuture<std::string> > f = service->submit<executor::tasks::MapPutPartitionAwareCallable, std::string>(
                        callable);

                std::shared_ptr<std::string> result = f->get();
                ASSERT_NOTNULL(result.get(), std::string);
                ASSERT_EQ(member.getUuid(), *result);
                ASSERT_TRUE(map.containsKey(member.getUuid()));
            }

            TEST_F(ClientExecutorServiceTest, submitCallablePartitionAware_WithExecutionCallback) {
                std::string testName = getTestName();
                std::shared_ptr<IExecutorService> service = client->getExecutorService(testName);

                IMap<std::string, std::string> map = client->getMap<std::string, std::string>(testName);

                std::vector<Member> members = client->getCluster().getMembers();
                spi::ClientContext clientContext(*client);
                Member &member = members[0];
                std::string key = generateKeyOwnedBy(clientContext, member);

                executor::tasks::MapPutPartitionAwareCallable callable(testName, key);

                std::shared_ptr<hazelcast::util::CountDownLatch> latch(new hazelcast::util::CountDownLatch(1));
                std::shared_ptr<ExecutionCallback<std::string>> callback(new ResultSettingExecutionCallback(latch));

                service->submit<executor::tasks::MapPutPartitionAwareCallable, std::string>(callable, callback);

                ASSERT_OPEN_EVENTUALLY(*latch);
                std::shared_ptr<std::string> value = static_pointer_cast<ResultSettingExecutionCallback>(
                        callback)->getResult();
                ASSERT_NOTNULL(value.get(), std::string);
                ASSERT_EQ(member.getUuid(), *value);
                ASSERT_TRUE(map.containsKey(member.getUuid()));
            }

            TEST_F(ClientExecutorServiceTest, testExecute) {
                std::string testName = getTestName();
                std::shared_ptr<IExecutorService> service = client->getExecutorService(testName);

                service->execute<executor::tasks::MapPutPartitionAwareCallable>(
                        executor::tasks::MapPutPartitionAwareCallable(testName, "key"));

                IMap<std::string, std::string> map = client->getMap<std::string, std::string>(testName);

                assertSizeEventually(1, map);
            }

            TEST_F(ClientExecutorServiceTest, testExecute_withMemberSelector) {
                std::string testName = getTestName();
                std::shared_ptr<IExecutorService> service = client->getExecutorService(testName);
                executor::tasks::SelectAllMembers selector;

                service->execute<executor::tasks::MapPutPartitionAwareCallable>(
                        executor::tasks::MapPutPartitionAwareCallable(testName, "key"), selector);
                IMap<std::string, std::string> map = client->getMap<std::string, std::string>(testName);

                assertSizeEventually(1, map);
            }

            TEST_F(ClientExecutorServiceTest, testExecuteOnKeyOwner) {
                std::string testName = getTestName();
                std::shared_ptr<IExecutorService> service = client->getExecutorService(testName);

                IMap<std::string, std::string> map = client->getMap<std::string, std::string>(testName);

                std::vector<Member> members = client->getCluster().getMembers();
                spi::ClientContext clientContext(*client);
                Member &member = members[0];
                std::string targetUuid = member.getUuid();
                std::string key = generateKeyOwnedBy(clientContext, member);

                executor::tasks::MapPutPartitionAwareCallable callable(testName, key);

                service->executeOnKeyOwner<executor::tasks::MapPutPartitionAwareCallable, std::string>(callable, key);

                ASSERT_TRUE_EVENTUALLY(map.containsKey(targetUuid));
            }

            TEST_F(ClientExecutorServiceTest, testExecuteOnMember) {
                std::string testName = getTestName();
                std::shared_ptr<IExecutorService> service = client->getExecutorService(testName);

                IMap<std::string, std::string> map = client->getMap<std::string, std::string>(testName);

                std::vector<Member> members = client->getCluster().getMembers();
                Member &member = members[0];
                std::string targetUuid = member.getUuid();

                executor::tasks::MapPutPartitionAwareCallable callable(testName, "key");

                service->executeOnMember<executor::tasks::MapPutPartitionAwareCallable>(callable, member);

                ASSERT_TRUE_EVENTUALLY(map.containsKey(targetUuid));
            }

            TEST_F(ClientExecutorServiceTest, testExecuteOnMembers) {
                std::string testName = getTestName();
                std::shared_ptr<IExecutorService> service = client->getExecutorService(testName);

                IMap<std::string, std::string> map = client->getMap<std::string, std::string>(testName);

                std::vector<Member> allMembers = client->getCluster().getMembers();
                std::vector<Member> members(allMembers.begin(), allMembers.begin() + 2);

                executor::tasks::MapPutPartitionAwareCallable callable(testName, "key");

                service->executeOnMembers<executor::tasks::MapPutPartitionAwareCallable>(callable, members);

                ASSERT_TRUE_EVENTUALLY(map.containsKey(members[0].getUuid()) && map.containsKey(members[1].getUuid()));
            }

            TEST_F(ClientExecutorServiceTest, testExecuteOnMembers_withEmptyCollection) {
                std::string testName = getTestName();
                std::shared_ptr<IExecutorService> service = client->getExecutorService(testName);

                IMap<std::string, std::string> map = client->getMap<std::string, std::string>(testName);

                executor::tasks::MapPutPartitionAwareCallable callable(testName, "key");

                service->executeOnMembers<executor::tasks::MapPutPartitionAwareCallable>(callable,
                                                                                         std::vector<Member>());

                assertSizeEventually(0, map);
            }

            TEST_F(ClientExecutorServiceTest, testExecuteOnMembers_withSelector) {
                std::string testName = getTestName();
                std::shared_ptr<IExecutorService> service = client->getExecutorService(testName);

                IMap<std::string, std::string> map = client->getMap<std::string, std::string>(testName);

                executor::tasks::MapPutPartitionAwareCallable callable(testName, "key");

                executor::tasks::SelectAllMembers selector;

                service->executeOnMembers<executor::tasks::MapPutPartitionAwareCallable>(callable, selector);

                assertSizeEventually((int) numberOfMembers, map);
            }

            TEST_F(ClientExecutorServiceTest, testExecuteOnAllMembers) {
                std::string testName = getTestName();
                std::shared_ptr<IExecutorService> service = client->getExecutorService(testName);

                IMap<std::string, std::string> map = client->getMap<std::string, std::string>(testName);

                executor::tasks::MapPutPartitionAwareCallable callable(testName, "key");

                service->executeOnAllMembers<executor::tasks::MapPutPartitionAwareCallable>(callable);

                assertSizeEventually((int) numberOfMembers, map);
            }
        }
    }
}



#ifdef HZ_BUILD_WITH_SSL





namespace hazelcast {
    namespace client {
        namespace test {
            namespace aws {
                class AwsConfigTest : public ::testing::Test {
                };

                TEST_F (AwsConfigTest, testDefaultValues) {
                    client::config::ClientAwsConfig awsConfig;
                    ASSERT_EQ("", awsConfig.getAccessKey());
                    ASSERT_EQ("us-east-1", awsConfig.getRegion());
                    ASSERT_EQ("ec2.amazonaws.com", awsConfig.getHostHeader());
                    ASSERT_EQ("", awsConfig.getIamRole());
                    ASSERT_EQ("", awsConfig.getSecretKey());
                    ASSERT_EQ("", awsConfig.getSecurityGroupName());
                    ASSERT_EQ("", awsConfig.getTagKey());
                    ASSERT_EQ("", awsConfig.getTagValue());
                    ASSERT_FALSE(awsConfig.isInsideAws());
                    ASSERT_FALSE(awsConfig.isEnabled());
                }

                TEST_F (AwsConfigTest, testSetValues) {
                    client::config::ClientAwsConfig awsConfig;

                    awsConfig.setAccessKey("mykey");
                    awsConfig.setRegion("myregion");
                    awsConfig.setHostHeader("myheader");
                    awsConfig.setIamRole("myrole");
                    awsConfig.setSecretKey("mysecret");
                    awsConfig.setSecurityGroupName("mygroup");
                    awsConfig.setTagKey("mytagkey");
                    awsConfig.setTagValue("mytagvalue");
                    awsConfig.setInsideAws(true);
                    awsConfig.setEnabled(true);

                    ASSERT_EQ("mykey", awsConfig.getAccessKey());
                    ASSERT_EQ("myregion", awsConfig.getRegion());
                    ASSERT_EQ("myheader", awsConfig.getHostHeader());
                    ASSERT_EQ("myrole", awsConfig.getIamRole());
                    ASSERT_EQ("mysecret", awsConfig.getSecretKey());
                    ASSERT_EQ("mygroup", awsConfig.getSecurityGroupName());
                    ASSERT_EQ("mytagkey", awsConfig.getTagKey());
                    ASSERT_EQ("mytagvalue", awsConfig.getTagValue());
                    ASSERT_TRUE(awsConfig.isInsideAws());
                    ASSERT_TRUE(awsConfig.isEnabled()) << awsConfig;
                }

                TEST_F (AwsConfigTest, testSetEmptyValues) {
                    client::config::ClientAwsConfig awsConfig;

                    ASSERT_THROW(awsConfig.setAccessKey(""), exception::IllegalArgumentException);
                    ASSERT_THROW(awsConfig.setRegion(""), exception::IllegalArgumentException);
                    ASSERT_THROW(awsConfig.setHostHeader(""), exception::IllegalArgumentException);
                    ASSERT_THROW(awsConfig.setSecretKey(""), exception::IllegalArgumentException);
                }

                TEST_F (AwsConfigTest, testClientConfigUsage) {
                    ClientConfig clientConfig;
                    client::config::ClientAwsConfig &awsConfig = clientConfig.getNetworkConfig().getAwsConfig();
                    awsConfig.setEnabled(true);

                    ASSERT_TRUE(clientConfig.getNetworkConfig().getAwsConfig().isEnabled());

                    client::config::ClientAwsConfig newConfig;

                    clientConfig.getNetworkConfig().setAwsConfig(newConfig);
                    // default constructor sets enabled to false
                    ASSERT_FALSE(clientConfig.getNetworkConfig().getAwsConfig().isEnabled());
                }

                TEST_F (AwsConfigTest, testInvalidAwsMemberPortConfig) {
                    ClientConfig clientConfig;

                    clientConfig.getProperties()[ClientProperties::PROP_AWS_MEMBER_PORT] = "65536";
                    clientConfig.getNetworkConfig().getAwsConfig().setEnabled(true).
                            setAccessKey(getenv("AWS_ACCESS_KEY_ID")).setSecretKey(getenv("AWS_SECRET_ACCESS_KEY")).
                            setTagKey("aws-test-tag").setTagValue("aws-tag-value-1").setInsideAws(true);

                    clientConfig.getProperties()[ClientProperties::PROP_AWS_MEMBER_PORT] = "-1";

                    ASSERT_THROW(HazelcastClient hazelcastClient(clientConfig),
                                 exception::InvalidConfigurationException);
                }
            }
        }
    }
}


#endif // HZ_BUILD_WITH_SSL


#ifdef HZ_BUILD_WITH_SSL





namespace hazelcast {
    namespace client {
        namespace test {
            namespace aws {
                class AwsClientTest : public ::testing::Test {
                };

                TEST_F (AwsClientTest, testClientAwsMemberNonDefaultPortConfig) {
                    ClientConfig clientConfig;

                    clientConfig.getProperties()[ClientProperties::PROP_AWS_MEMBER_PORT] = "60000";
                    clientConfig.getNetworkConfig().getAwsConfig().setEnabled(true).
                            setAccessKey(getenv("AWS_ACCESS_KEY_ID")).setSecretKey(getenv("AWS_SECRET_ACCESS_KEY")).
                            setTagKey("aws-test-tag").setTagValue("aws-tag-value-1");

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
                    clientConfig.getNetworkConfig().getAwsConfig().setInsideAws(true);
#else
                    clientConfig.getNetworkConfig().getAwsConfig().setInsideAws(false);
#endif

                    HazelcastClient hazelcastClient(clientConfig);

                    IMap<int, int> map = hazelcastClient.getMap<int, int>("myMap");
                    map.put(5, 20);
                    std::shared_ptr<int> val = map.get(5);
                    ASSERT_NE((int *) NULL, val.get());
                    ASSERT_EQ(20, *val);
                }

                TEST_F (AwsClientTest, testClientAwsMemberWithSecurityGroupDefaultIamRole) {
                    ClientConfig clientConfig;

                    clientConfig.getProperties()[ClientProperties::PROP_AWS_MEMBER_PORT] = "60000";
                    clientConfig.getNetworkConfig().getAwsConfig().setEnabled(true).
                            setSecurityGroupName("launch-wizard-147");

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
                                                                                                                                            // The access key and secret will be retrieved from default IAM role at windows machine
                    clientConfig.getNetworkConfig().getAwsConfig().setInsideAws(true);
#else
                    clientConfig.getNetworkConfig().getAwsConfig().setAccessKey(getenv("AWS_ACCESS_KEY_ID")).
                            setSecretKey(getenv("AWS_SECRET_ACCESS_KEY"));
#endif

                    HazelcastClient hazelcastClient(clientConfig);

                    IMap<int, int> map = hazelcastClient.getMap<int, int>("myMap");
                    map.put(5, 20);
                    std::shared_ptr<int> val = map.get(5);
                    ASSERT_NE((int *) NULL, val.get());
                    ASSERT_EQ(20, *val);
                }

                // FIPS_mode_set is not available for Mac OS X built-in openssl library
#ifndef __APPLE__
                                                                                                                                        TEST_F (AwsClientTest, testFipsEnabledAwsDiscovery) {
                    ClientConfig clientConfig;

                    clientConfig.getProperties()[ClientProperties::PROP_AWS_MEMBER_PORT] = "60000";
                    clientConfig.getNetworkConfig().getAwsConfig().setEnabled(true).
                            setAccessKey(getenv("AWS_ACCESS_KEY_ID")).setSecretKey(getenv("AWS_SECRET_ACCESS_KEY")).
                            setTagKey("aws-test-tag").setTagValue("aws-tag-value-1");

                    #if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
                    clientConfig.getNetworkConfig().getAwsConfig().setInsideAws(true);
                    #else
                    clientConfig.getNetworkConfig().getAwsConfig().setInsideAws(false);
                    #endif

                    // Turn Fips mode on
                    FIPS_mode_set(1);

                    HazelcastClient hazelcastClient(clientConfig);
                    IMap<int, int> map = hazelcastClient.getMap<int, int>("myMap");
                    map.put(5, 20);
                    std::shared_ptr<int> val = map.get(5);
                    ASSERT_NE((int *) NULL, val.get());
                    ASSERT_EQ(20, *val);
                }
#endif // ifndef __APPLE__

                /**
                 * Following test can only run from inside the AWS network
                 */
#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
                                                                                                                                        TEST_F (AwsClientTest, testRetrieveCredentialsFromIamRoleAndConnect) {
                    ClientConfig clientConfig;

                    clientConfig.getProperties()[ClientProperties::PROP_AWS_MEMBER_PORT] = "60000";
                    clientConfig.getNetworkConfig().getAwsConfig().setEnabled(true).setIamRole("cloudbees-role").setTagKey(
                            "aws-test-tag").setTagValue("aws-tag-value-1").setInsideAws(true);

                    HazelcastClient hazelcastClient(clientConfig);
                }

                TEST_F (AwsClientTest, testRetrieveCredentialsFromInstanceProfileDefaultIamRoleAndConnect) {
                    ClientConfig clientConfig;

                    clientConfig.getProperties()[ClientProperties::PROP_AWS_MEMBER_PORT] = "60000";
                    clientConfig.getNetworkConfig().getAwsConfig().setEnabled(true).setTagKey(
                            "aws-test-tag").setTagValue("aws-tag-value-1").setInsideAws(true);

                    HazelcastClient hazelcastClient(clientConfig);
                }
#endif
            }
        }
    }
}

#endif // HZ_BUILD_WITH_SSL


#ifdef HZ_BUILD_WITH_SSL

namespace hazelcast {
    namespace client {
        namespace test {
            namespace aws {
                class DescribeInstancesTest : public ClientTestSupport {
                };

                TEST_F (DescribeInstancesTest, testDescribeInstancesTagAndValueSet) {
                    client::config::ClientAwsConfig awsConfig;
                    awsConfig.setEnabled(true).setAccessKey(getenv("AWS_ACCESS_KEY_ID")).setSecretKey(
                            getenv("AWS_SECRET_ACCESS_KEY")).setTagKey("aws-test-tag").setTagValue("aws-tag-value-1");
                    client::aws::impl::DescribeInstances desc(awsConfig, awsConfig.getHostHeader(), getLogger());
                    std::map<std::string, std::string> results = desc.execute();
                    ASSERT_EQ(results.size(), 1U);
                    ASSERT_NE(results.end(), results.find(getenv("HZ_TEST_AWS_INSTANCE_PRIVATE_IP")));
                }

                TEST_F (DescribeInstancesTest, testDescribeInstancesTagAndNonExistentValueSet) {
                    client::config::ClientAwsConfig awsConfig;
                    awsConfig.setEnabled(true).setAccessKey(getenv("AWS_ACCESS_KEY_ID")).setSecretKey(
                            getenv("AWS_SECRET_ACCESS_KEY")).setTagKey("aws-test-tag").setTagValue(
                            "non-existent-value");
                    client::aws::impl::DescribeInstances desc(awsConfig, awsConfig.getHostHeader(), getLogger());
                    std::map<std::string, std::string> results = desc.execute();
                    ASSERT_TRUE(results.empty());
                }

                TEST_F (DescribeInstancesTest, testDescribeInstancesOnlyTagIsSet) {
                    client::config::ClientAwsConfig awsConfig;
                    awsConfig.setEnabled(true).setAccessKey(getenv("AWS_ACCESS_KEY_ID")).setSecretKey(
                            getenv("AWS_SECRET_ACCESS_KEY")).setTagKey("aws-test-tag");
                    client::aws::impl::DescribeInstances desc(awsConfig, awsConfig.getHostHeader(), getLogger());
                    std::map<std::string, std::string> results = desc.execute();
                    ASSERT_EQ(results.size(), 1U);
                    ASSERT_NE(results.end(), results.find(getenv("HZ_TEST_AWS_INSTANCE_PRIVATE_IP")));
                }

                TEST_F (DescribeInstancesTest, testDescribeInstancesOnlyTagIsSetToNonExistentTag) {
                    client::config::ClientAwsConfig awsConfig;
                    awsConfig.setEnabled(true).setAccessKey(getenv("AWS_ACCESS_KEY_ID")).setSecretKey(
                            getenv("AWS_SECRET_ACCESS_KEY")).setTagKey("non-existent-tag");
                    client::aws::impl::DescribeInstances desc(awsConfig, awsConfig.getHostHeader(), getLogger());
                    std::map<std::string, std::string> results = desc.execute();
                    ASSERT_TRUE(results.empty());
                }

                TEST_F (DescribeInstancesTest, testDescribeInstancesOnlyValueIsSet) {
                    client::config::ClientAwsConfig awsConfig;
                    awsConfig.setEnabled(true).setAccessKey(getenv("AWS_ACCESS_KEY_ID")).setSecretKey(
                            getenv("AWS_SECRET_ACCESS_KEY")).setTagValue("aws-tag-value-1");
                    client::aws::impl::DescribeInstances desc(awsConfig, awsConfig.getHostHeader(), getLogger());
                    std::map<std::string, std::string> results = desc.execute();
                    ASSERT_EQ(results.size(), 1U);
                    ASSERT_NE(results.end(), results.find(getenv("HZ_TEST_AWS_INSTANCE_PRIVATE_IP")));
                }

                TEST_F (DescribeInstancesTest, testDescribeInstancesOnlyValueIsSetToNonExistentValue) {
                    client::config::ClientAwsConfig awsConfig;
                    awsConfig.setEnabled(true).setAccessKey(getenv("AWS_ACCESS_KEY_ID")).setSecretKey(
                            getenv("AWS_SECRET_ACCESS_KEY")).setTagValue("non-existent-value");
                    client::aws::impl::DescribeInstances desc(awsConfig, awsConfig.getHostHeader(), getLogger());
                    std::map<std::string, std::string> results = desc.execute();
                    ASSERT_TRUE(results.empty());
                }

                TEST_F (DescribeInstancesTest, testDescribeInstancesSecurityGroup) {
                    client::config::ClientAwsConfig awsConfig;
                    awsConfig.setEnabled(true).setAccessKey(getenv("AWS_ACCESS_KEY_ID")).setSecretKey(
                            getenv("AWS_SECRET_ACCESS_KEY")).setSecurityGroupName("launch-wizard-147");
                    client::aws::impl::DescribeInstances desc(awsConfig, awsConfig.getHostHeader(), getLogger());
                    std::map<std::string, std::string> results = desc.execute();
                    ASSERT_EQ(results.size(), 1U);
                    ASSERT_NE(results.end(), results.find(getenv("HZ_TEST_AWS_INSTANCE_PRIVATE_IP")));
                }

                TEST_F (DescribeInstancesTest, testDescribeInstancesNonExistentSecurityGroup) {
                    client::config::ClientAwsConfig awsConfig;
                    awsConfig.setEnabled(true).setAccessKey(getenv("AWS_ACCESS_KEY_ID")).setSecretKey(
                            getenv("AWS_SECRET_ACCESS_KEY")).setSecurityGroupName("non-existent-group");
                    client::aws::impl::DescribeInstances desc(awsConfig, awsConfig.getHostHeader(), getLogger());
                    std::map<std::string, std::string> results = desc.execute();
                    ASSERT_TRUE(results.empty());
                }

            }
        }
    }
}

#endif //HZ_BUILD_WITH_SSL


#ifdef HZ_BUILD_WITH_SSL



namespace awsutil = hazelcast::client::aws::utility;

namespace hazelcast {
    namespace client {
        namespace test {
            namespace aws {
                class CloudUtilityTest : public ClientTestSupport {
                };

                TEST_F (CloudUtilityTest, testUnmarshallResponseXml) {
                    std::filebuf fb;
                    ASSERT_TRUE(fb.open("hazelcast/test/resources/sample_aws_response.xml", std::ios::in));
                    std::istream responseStream(&fb);

                    config::ClientAwsConfig awsConfig;
                    std::map<std::string, std::string> results = hazelcast::client::aws::utility::CloudUtility::unmarshalTheResponse(
                            responseStream, getLogger());
                    ASSERT_EQ(4U, results.size());
                    ASSERT_NE(results.end(), results.find("10.0.16.13"));
                    ASSERT_EQ("", results["10.0.16.13"]);
                    ASSERT_NE(results.end(), results.find("10.0.16.17"));
                    ASSERT_EQ("54.85.192.215", results["10.0.16.17"]);
                    ASSERT_NE(results.end(), results.find("10.0.16.25"));
                    ASSERT_EQ("", results["10.0.16.25"]);
                    ASSERT_NE(results.end(), results.find("172.30.4.118"));
                    ASSERT_EQ("54.85.192.213", results["172.30.4.118"]);
                }
            }
        }
    }
}

#endif //HZ_BUILD_WITH_SSL


namespace hazelcast {
    namespace client {
        namespace test {


            TestCustomXSerializable::TestCustomXSerializable() {

            }

            TestCustomXSerializable::TestCustomXSerializable(int id) :id(id){

            }

            bool TestCustomXSerializable::operator==(const TestCustomXSerializable& rhs) const {
                if (this == &rhs)
                    return true;
                if (id != rhs.id) return false;
                return true;
            }

            bool TestCustomXSerializable::operator!=(const TestCustomXSerializable& rhs) const {
                return !(*this == rhs);
            }

            int getHazelcastTypeId(TestCustomXSerializable const* param) {
                return 666;
            }

            TestCustomPerson::TestCustomPerson() {

            }

            TestCustomPerson::TestCustomPerson(const std::string& name):name(name) {

            }

            bool TestCustomPerson::operator==(const TestCustomPerson& rhs) const {
                if (this == &rhs)
                    return true;
                if (name.compare(rhs.name))
                    return false;
                return true;
            }

            bool TestCustomPerson::operator!=(const TestCustomPerson& rhs) const {
                return !(*this == rhs);
            }

            void TestCustomPerson::setName(const std::string& name) {
                this->name = name;
            }

            std::string TestCustomPerson::getName() const {
                return name;
            }

            int getHazelcastTypeId(TestCustomPerson const* param) {
                return 999;
            }

        }
    }
}




namespace hazelcast {
    namespace client {
        namespace test {

            void TestCustomPersonSerializer::write(serialization::ObjectDataOutput & out, const TestCustomPerson& object) {
                out.writeInt(999);
                const std::string &name = object.getName();
                out.writeUTF(&name);
                out.writeInt(999);
            }

            void TestCustomPersonSerializer::read(serialization::ObjectDataInput & in, TestCustomPerson& object) {
                int i = in.readInt();
                assert(i == 999);
                object.setName(*in.readUTF());
                i = in.readInt();
                assert(i == 999);
            }

            int TestCustomPersonSerializer::getHazelcastTypeId() const {
                return 999;
            }

        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
