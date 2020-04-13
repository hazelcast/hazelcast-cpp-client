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
#include <hazelcast/util/Util.h>
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
#include "hazelcast/util/ILogger.h"
#include "hazelcast/client/IMap.h"
#include "hazelcast/util/Bits.h"
#include "hazelcast/util/SyncHttpsClient.h"
#include "hazelcast/client/exception/IOException.h"
#include "hazelcast/util/AtomicInt.h"
#include "hazelcast/util/BlockingConcurrentQueue.h"
#include "hazelcast/util/UTFUtil.h"
#include "hazelcast/util/ConcurrentQueue.h"
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
            namespace adaptor {
                class RawPointerQueueTest : public ClientTestSupport {
                protected:
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

                    virtual void TearDown() {
                        q->clear();
                    }

                    static void SetUpTestCase() {
                        instance = new HazelcastServer(*g_srvFactory);
                        client = new HazelcastClient(getConfig());
                        legacy = new IQueue<std::string>(client->getQueue<std::string>("MyQueue"));
                        q = new client::adaptor::RawPointerQueue<std::string>(*legacy);
                    }

                    static void TearDownTestCase() {
                        delete q;
                        delete legacy;
                        delete client;
                        delete instance;

                        q = NULL;
                        legacy = NULL;
                        client = NULL;
                        instance = NULL;
                    }

                    static HazelcastServer *instance;
                    static HazelcastClient *client;
                    static IQueue<std::string> *legacy;
                    static client::adaptor::RawPointerQueue<std::string> *q;
                };

                HazelcastServer *RawPointerQueueTest::instance = NULL;
                HazelcastClient *RawPointerQueueTest::client = NULL;
                IQueue<std::string> *RawPointerQueueTest::legacy = NULL;
                client::adaptor::RawPointerQueue<std::string> *RawPointerQueueTest::q = NULL;

                TEST_F(RawPointerQueueTest, testListener) {
                    ASSERT_EQ(0, q->size());

                    hazelcast::util::CountDownLatch latch(5);

                    QueueTestItemListener qener(latch);
                    std::string id = q->addItemListener(qener, true);

                    hazelcast::util::sleep(1);

                    for (int i = 0; i < 5; i++) {
                        ASSERT_TRUE(q->offer(std::string("event_item") + hazelcast::util::IOUtil::to_string(i)));
                    }

                    ASSERT_TRUE(latch.await(5));
                    ASSERT_TRUE(q->removeItemListener(id));
                }

                void testOfferPollThread2(hazelcast::util::ThreadArgs &args) {
                    client::adaptor::RawPointerQueue<std::string> *q = (client::adaptor::RawPointerQueue<std::string> *) args.arg0;
                    hazelcast::util::sleep(2);
                    q->offer("item1");
                }

                TEST_F(RawPointerQueueTest, testOfferPoll) {
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

                    std::unique_ptr<std::string> item = q->poll(30 * 1000);
                    ASSERT_NE(item.get(), (std::string *) NULL);
                    ASSERT_EQ("item1", *item);
                    t2.join();
                }

                TEST_F(RawPointerQueueTest, testRemainingCapacity) {
                    int capacity = q->remainingCapacity();
                    ASSERT_TRUE(capacity > 10000);
                    q->offer("item");
                    ASSERT_EQ(capacity - 1, q->remainingCapacity());
                }

                TEST_F(RawPointerQueueTest, testPeek) {
                    ASSERT_TRUE(q->offer("peek 1"));
                    ASSERT_TRUE(q->offer("peek 2"));
                    ASSERT_TRUE(q->offer("peek 3"));

                    std::unique_ptr<std::string> item = q->peek();
                    ASSERT_NE((std::string *) NULL, item.get());
                    ASSERT_EQ("peek 1", *item);
                }

                TEST_F(RawPointerQueueTest, testTake) {
                    ASSERT_TRUE(q->offer("peek 1"));
                    ASSERT_TRUE(q->offer("peek 2"));
                    ASSERT_TRUE(q->offer("peek 3"));

                    std::unique_ptr<std::string> item = q->take();
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

                TEST_F(RawPointerQueueTest, testRemove) {
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


                TEST_F(RawPointerQueueTest, testContains) {
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

                TEST_F(RawPointerQueueTest, testDrain) {
                    ASSERT_TRUE(q->offer("item1"));
                    ASSERT_TRUE(q->offer("item2"));
                    ASSERT_TRUE(q->offer("item3"));
                    ASSERT_TRUE(q->offer("item4"));
                    ASSERT_TRUE(q->offer("item5"));

                    std::unique_ptr<client::DataArray<std::string> > list = q->drainTo(2);
                    ASSERT_EQ((size_t) 2U, list->size());
                    ASSERT_NE((std::string *) NULL, list->get(0));
                    ASSERT_NE((std::string *) NULL, list->get(1));
                    ASSERT_EQ("item1", *list->get(0));
                    ASSERT_EQ("item2", *list->get(1));

                    list = q->drainTo();
                    ASSERT_EQ((size_t) 3U, list->size());
                    ASSERT_NE((std::string *) NULL, list->get(0));
                    ASSERT_NE((std::string *) NULL, list->get(1));
                    ASSERT_NE((std::string *) NULL, list->get(2));
                    ASSERT_EQ("item3", *list->get(0));
                    ASSERT_EQ("item4", *list->get(1));
                    ASSERT_EQ("item5", *list->get(2));

                    ASSERT_TRUE(q->offer("item1"));
                    ASSERT_TRUE(q->offer("item2"));
                    ASSERT_TRUE(q->offer("item3"));
                    list = q->drainTo(5);
                    ASSERT_EQ((size_t) 3U, list->size());
                    ASSERT_NE((std::string *) NULL, list->get(0));
                    ASSERT_NE((std::string *) NULL, list->get(1));
                    ASSERT_NE((std::string *) NULL, list->get(2));
                    ASSERT_EQ("item1", *list->get(0));
                    ASSERT_EQ("item2", *list->get(1));
                    ASSERT_EQ("item3", *list->get(2));
                }

                TEST_F(RawPointerQueueTest, testToArray) {
                    ASSERT_TRUE(q->offer("item1"));
                    ASSERT_TRUE(q->offer("item2"));
                    ASSERT_TRUE(q->offer("item3"));
                    ASSERT_TRUE(q->offer("item4"));
                    ASSERT_TRUE(q->offer("item5"));

                    std::unique_ptr<client::DataArray<std::string> > array = q->toArray();
                    size_t size = array->size();
                    ASSERT_EQ(5U, size);
                    for (size_t i = 0; i < size; i++) {
                        const std::string *item = (*array)[i];
                        ASSERT_NE((std::string *) NULL, item);
                        ASSERT_EQ(std::string("item") + hazelcast::util::IOUtil::to_string(i + 1), *item);
                    }
                }

                TEST_F(RawPointerQueueTest, testAddAll) {
                    std::vector<std::string> coll;
                    coll.push_back("item1");
                    coll.push_back("item2");
                    coll.push_back("item3");
                    coll.push_back("item4");

                    ASSERT_TRUE(q->addAll(coll));
                    int size = q->size();
                    ASSERT_EQ(size, (int) coll.size());
                }

                TEST_F(RawPointerQueueTest, testRemoveRetain) {
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

                TEST_F(RawPointerQueueTest, testClear) {
                    ASSERT_TRUE(q->offer("item1"));
                    ASSERT_TRUE(q->offer("item2"));
                    ASSERT_TRUE(q->offer("item3"));
                    ASSERT_TRUE(q->offer("item4"));
                    ASSERT_TRUE(q->offer("item5"));

                    q->clear();

                    ASSERT_EQ(0, q->size());
                    ASSERT_EQ(q->poll().get(), (std::string *) NULL);
                }
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace test {
            class RawPointerClientTxnMapTest : public ClientTestSupport {
            protected:
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

            HazelcastServer *RawPointerClientTxnMapTest::instance = NULL;
            HazelcastClient *RawPointerClientTxnMapTest::client = NULL;

            TEST_F(RawPointerClientTxnMapTest, testPutGet) {
                std::string name = "defMap";

                TransactionContext context = client->newTransactionContext();
                context.beginTransaction();

                TransactionalMap<std::string, std::string> originalMap = context.getMap<std::string, std::string>(name);
                client::adaptor::RawPointerTransactionalMap<std::string, std::string> map(originalMap);

                ASSERT_EQ(map.put("key1", "value1").get(), (std::string *) NULL);
                ASSERT_EQ("value1", *(map.get("key1")));
                std::shared_ptr<std::string> val = client->getMap<std::string, std::string>(name).get("key1");
                ASSERT_EQ(val.get(), (std::string *) NULL);

                context.commitTransaction();

                ASSERT_EQ("value1", *(client->getMap<std::string, std::string>(name).get("key1")));
            }


//            @Test MTODO
//            public void testGetForUpdate() throws TransactionException {
//            final IMap<String, Integer> map = hz.getMap("testTxnGetForUpdate");
//            final CountDownLatch latch1 = new CountDownLatch(1);
//            final CountDownLatch latch2 = new CountDownLatch(1);
//            map.put("var", 0);
//            final AtomicBoolean pass = new AtomicBoolean(true);
//
//
//            Runnable incrementor = new Runnable() {
//                public void run() {
//                    try {
//                        latch1.await(100, TimeUnit.SECONDS);
//                        pass.set(map.tryPut("var", 1, 0, TimeUnit.SECONDS) == false);
//                        latch2.countDown();
//                    } catch (Exception e) {
//                    }
//                }
//            }
//            new Thread(incrementor).start();
//            boolean b = hz.executeTransaction(new TransactionalTask<Boolean>() {
//                public Boolean execute(TransactionalTaskContext context) throws TransactionException {
//                    try {
//                        final TransactionalMap<String, Integer> txMap = context.getMap("testTxnGetForUpdate");
//                        txMap.getForUpdate("var");
//                        latch1.countDown();
//                        latch2.await(100, TimeUnit.SECONDS);
//                    } catch (Exception e) {
//                    }
//                    return true;
//                }
//            });
//            assertTrue(b);
//            assertTrue(pass.get());
//            assertTrue(map.tryPut("var", 1, 0, TimeUnit.SECONDS));
//        }

            TEST_F(RawPointerClientTxnMapTest, testKeySetValues) {
                std::string name = "testKeySetValues";
                IMap<std::string, std::string> map = client->getMap<std::string, std::string>(name);
                map.put("key1", "value1");
                map.put("key2", "value2");

                TransactionContext context = client->newTransactionContext();
                context.beginTransaction();
                TransactionalMap<std::string, std::string> originalMap = context.getMap<std::string, std::string>(name);
                client::adaptor::RawPointerTransactionalMap<std::string, std::string> txMap(originalMap);
                ASSERT_EQ(txMap.put("key3", "value3").get(), (std::string *) NULL);


                ASSERT_EQ(3, (int) txMap.size());
                ASSERT_EQ(3, (int) txMap.keySet()->size());
                ASSERT_EQ(3, (int) txMap.values()->size());
                context.commitTransaction();

                ASSERT_EQ(3, (int) map.size());
                ASSERT_EQ(3, (int) map.keySet().size());
                ASSERT_EQ(3, (int) map.values().size());

            }

            TEST_F(RawPointerClientTxnMapTest, testKeySetAndValuesWithPredicates) {
                std::string name = "testKeysetAndValuesWithPredicates";
                IMap<Employee, Employee> map = client->getMap<Employee, Employee>(name);

                Employee emp1("abc-123-xvz", 34);
                Employee emp2("abc-123-xvz", 20);

                map.put(emp1, emp1);

                TransactionContext context = client->newTransactionContext();
                context.beginTransaction();

                TransactionalMap<Employee, Employee> originalMap = context.getMap<Employee, Employee>(name);
                client::adaptor::RawPointerTransactionalMap<Employee, Employee> txMap(originalMap);

                ASSERT_EQ(txMap.put(emp2, emp2).get(), (Employee *) NULL);

                ASSERT_EQ(2, (int) txMap.size());
                ASSERT_EQ(2, (int) txMap.keySet()->size());
                query::SqlPredicate predicate("a = 10");
                ASSERT_EQ(0, (int) txMap.keySet(&predicate)->size());
                ASSERT_EQ(0, (int) txMap.values(&predicate)->size());
                query::SqlPredicate predicate2("a >= 10");
                ASSERT_EQ(2, (int) txMap.keySet(&predicate2)->size());
                ASSERT_EQ(2, (int) txMap.values(&predicate2)->size());

                context.commitTransaction();

                ASSERT_EQ(2, (int) map.size());
                ASSERT_EQ(2, (int) map.values().size());
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace test {
            namespace adaptor {
                class RawPointerMultiMapTest : public ClientTestSupport {
                protected:
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

                    static void lockTtlThread(hazelcast::util::ThreadArgs &args) {
                        client::adaptor::RawPointerMultiMap<std::string, std::string> *map = (client::adaptor::RawPointerMultiMap<std::string, std::string> *) args.arg0;
                        hazelcast::util::CountDownLatch *latch = (hazelcast::util::CountDownLatch *) args.arg1;

                        if (!map->tryLock("key1")) {
                            latch->countDown();
                        }

                        if (map->tryLock("key1", 5 * 1000)) {
                            latch->countDown();
                        }
                    }

                    virtual void TearDown() {
                        // clear mm
                        mm->clear();
                    }

                    static void SetUpTestCase() {
                        instance = new HazelcastServer(*g_srvFactory);
                        client = new HazelcastClient;
                        legacy = new MultiMap<std::string, std::string>(
                                client->getMultiMap<std::string, std::string>("MyMultiMap"));
                        mm = new client::adaptor::RawPointerMultiMap<std::string, std::string>(*legacy);
                    }

                    static void TearDownTestCase() {
                        delete mm;
                        delete legacy;
                        delete client;
                        delete clientConfig;
                        delete instance;

                        mm = NULL;
                        legacy = NULL;
                        client = NULL;
                        clientConfig = NULL;
                        instance = NULL;
                    }

                    static HazelcastServer *instance;
                    static ClientConfig *clientConfig;
                    static HazelcastClient *client;
                    static MultiMap<std::string, std::string> *legacy;
                    static client::adaptor::RawPointerMultiMap<std::string, std::string> *mm;
                };

                HazelcastServer *RawPointerMultiMapTest::instance = NULL;
                ClientConfig *RawPointerMultiMapTest::clientConfig = NULL;
                HazelcastClient *RawPointerMultiMapTest::client = NULL;
                MultiMap<std::string, std::string> *RawPointerMultiMapTest::legacy = NULL;
                client::adaptor::RawPointerMultiMap<std::string, std::string> *RawPointerMultiMapTest::mm = NULL;

                TEST_F(RawPointerMultiMapTest, testPutGetRemove) {
                    ASSERT_TRUE(mm->put("key1", "value1"));
                    ASSERT_TRUE(mm->put("key1", "value2"));
                    ASSERT_TRUE(mm->put("key1", "value3"));

                    ASSERT_TRUE(mm->put("key2", "value4"));
                    ASSERT_TRUE(mm->put("key2", "value5"));

                    ASSERT_EQ(3, mm->valueCount("key1"));
                    ASSERT_EQ(2, mm->valueCount("key2"));
                    ASSERT_EQ(5, mm->size());

                    std::unique_ptr<hazelcast::client::DataArray<std::string> > coll = mm->get("key1");
                    ASSERT_EQ(3, (int) coll->size());

                    coll = mm->remove("key2");
                    ASSERT_EQ(2, (int) coll->size());
                    ASSERT_EQ(0, mm->valueCount("key2"));
                    ASSERT_EQ(0, (int) mm->get("key2")->size());

                    ASSERT_FALSE(mm->remove("key1", "value4"));
                    ASSERT_EQ(3, mm->size());

                    ASSERT_TRUE(mm->remove("key1", "value2"));
                    ASSERT_EQ(2, mm->size());

                    ASSERT_TRUE(mm->remove("key1", "value1"));
                    ASSERT_EQ(1, mm->size());
                    coll = mm->get("key1");
                    std::unique_ptr<std::string> val = coll->release(0);
                    ASSERT_NE((std::string *) NULL, val.get());
                    ASSERT_EQ("value3", *val);
                }

                TEST_F(RawPointerMultiMapTest, testKeySetEntrySetAndValues) {
                    ASSERT_TRUE(mm->put("key1", "value1"));
                    ASSERT_TRUE(mm->put("key1", "value2"));
                    ASSERT_TRUE(mm->put("key1", "value3"));

                    ASSERT_TRUE(mm->put("key2", "value4"));
                    ASSERT_TRUE(mm->put("key2", "value5"));


                    ASSERT_EQ(2, (int) mm->keySet()->size());
                    ASSERT_EQ(5, (int) mm->values()->size());
                    ASSERT_EQ(5, (int) mm->entrySet()->size());
                }

                TEST_F(RawPointerMultiMapTest, testContains) {
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

                TEST_F(RawPointerMultiMapTest, testListener) {
                    hazelcast::util::CountDownLatch latch1Add(8);
                    hazelcast::util::CountDownLatch latch1Remove(4);

                    hazelcast::util::CountDownLatch latch2Add(3);
                    hazelcast::util::CountDownLatch latch2Remove(3);

                    MyMultiMapListener mmener1(latch1Add, latch1Remove);
                    MyMultiMapListener mmener2(latch2Add, latch2Remove);

                    std::string id1 = mm->addEntryListener(mmener1, true);
                    std::string id2 = mm->addEntryListener(mmener2, "key3", true);

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
                    client::adaptor::RawPointerMultiMap<std::string, std::string> *mm = (client::adaptor::RawPointerMultiMap<std::string, std::string> *) args.arg0;
                    hazelcast::util::CountDownLatch *latch = (hazelcast::util::CountDownLatch *) args.arg1;
                    if (!mm->tryLock("key1")) {
                        latch->countDown();
                    }
                }

                TEST_F(RawPointerMultiMapTest, testLock) {
                    mm->lock("key1");
                    hazelcast::util::CountDownLatch latch(1);
                    hazelcast::util::StartedThread t(lockThread, mm, &latch);
                    ASSERT_TRUE(latch.await(5));
                    mm->forceUnlock("key1");
                    t.join();
                }

                TEST_F(RawPointerMultiMapTest, testLockTtl) {
                    mm->lock("key1", 3 * 1000);
                    hazelcast::util::CountDownLatch latch(2);
                    hazelcast::util::StartedThread t(lockTtlThread, mm, &latch);
                    ASSERT_TRUE(latch.await(10));
                    mm->forceUnlock("key1");
                    t.join();
                }


                void tryLockThread(hazelcast::util::ThreadArgs &args) {
                    client::adaptor::RawPointerMultiMap<std::string, std::string> *mm = (client::adaptor::RawPointerMultiMap<std::string, std::string> *) args.arg0;
                    hazelcast::util::CountDownLatch *latch = (hazelcast::util::CountDownLatch *) args.arg1;
                    try {
                        if (!mm->tryLock("key1", 2)) {
                            latch->countDown();
                        }
                    } catch (...) {
                        std::cerr << "Unexpected exception at RawPointerMultiMapTest tryLockThread" << std::endl;
                    }
                }

                void tryLockThread2(hazelcast::util::ThreadArgs &args) {
                    client::adaptor::RawPointerMultiMap<std::string, std::string> *mm = (client::adaptor::RawPointerMultiMap<std::string, std::string> *) args.arg0;
                    hazelcast::util::CountDownLatch *latch = (hazelcast::util::CountDownLatch *) args.arg1;
                    try {
                        if (mm->tryLock("key1", 20 * 1000)) {
                            latch->countDown();
                        }
                    } catch (...) {
                        std::cerr << "Unexpected exception at RawPointerMultiMapTest lockThread2" << std::endl;
                    }
                }

                TEST_F(RawPointerMultiMapTest, testTryLock) {
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
                    client::adaptor::RawPointerMultiMap<std::string, std::string> *mm = (client::adaptor::RawPointerMultiMap<std::string, std::string> *) args.arg0;
                    hazelcast::util::CountDownLatch *latch = (hazelcast::util::CountDownLatch *) args.arg1;
                    mm->forceUnlock("key1");
                    latch->countDown();
                }

                TEST_F(RawPointerMultiMapTest, testForceUnlock) {
                    mm->lock("key1");
                    hazelcast::util::CountDownLatch latch(1);
                    hazelcast::util::StartedThread t(forceUnlockThread, mm, &latch);
                    ASSERT_TRUE(latch.await(100));
                    ASSERT_FALSE(mm->isLocked("key1"));
                }
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace test {
            namespace adaptor {
                class RawPointerTxnMultiMapTest : public ClientTestSupport {
                protected:
                    static void SetUpTestCase() {
                        instance = new HazelcastServer(*g_srvFactory);
                        client = new HazelcastClient(getConfig());
                    }

                    static void TearDownTestCase() {
                        delete client;
                        delete clientConfig;
                        delete instance;

                        client = NULL;
                        clientConfig = NULL;
                        instance = NULL;
                    }

                    class GetRemoveTestTask : public hazelcast::util::Runnable {
                    public:
                        GetRemoveTestTask(MultiMap<std::string, std::string> &mm,
                                          hazelcast::util::CountDownLatch &latch) : mm(
                                mm),
                                                                                    latch(latch) {}

                        virtual void run() {
                            std::string key = hazelcast::util::IOUtil::to_string(hazelcast::util::getCurrentThreadId());
                            client->getMultiMap<std::string, std::string>("testPutGetRemove").put(key, "value");
                            TransactionContext context = client->newTransactionContext();
                            context.beginTransaction();
                            TransactionalMultiMap<std::string, std::string> originalMultiMap = context.getMultiMap<std::string, std::string>(
                                    "testPutGetRemove");
                            client::adaptor::RawPointerTransactionalMultiMap<std::string, std::string> multiMap(
                                    originalMultiMap);
                            ASSERT_FALSE(multiMap.put(key, "value"));
                            ASSERT_TRUE(multiMap.put(key, "value1"));
                            ASSERT_TRUE(multiMap.put(key, "value2"));
                            ASSERT_EQ(3, (int) multiMap.get(key)->size());
                            context.commitTransaction();

                            ASSERT_EQ(3, (int) mm.get(key).size());

                            latch.countDown();
                        }

                        virtual const std::string getName() const {
                            return "GetRemoveTestTask";
                        }

                    private:
                        MultiMap<std::string, std::string> &mm;
                        hazelcast::util::CountDownLatch &latch;
                    };

                    static HazelcastServer *instance;
                    static ClientConfig *clientConfig;
                    static HazelcastClient *client;
                };

                HazelcastServer *RawPointerTxnMultiMapTest::instance = NULL;
                ClientConfig *RawPointerTxnMultiMapTest::clientConfig = NULL;
                HazelcastClient *RawPointerTxnMultiMapTest::client = NULL;

                TEST_F(RawPointerTxnMultiMapTest, testPutGetRemove) {
                    MultiMap<std::string, std::string> mm = client->getMultiMap<std::string, std::string>(
                            "testPutGetRemove");
                    constexpr int n = 10;
                    hazelcast::util::CountDownLatch latch(n);

                    std::array<std::future<void>, n> allFutures;
                    for (int i = 0; i < n; i++) {
                        allFutures[i] = std::async([&]() { GetRemoveTestTask(mm, latch).run(); });
                    }
                    ASSERT_OPEN_EVENTUALLY(latch);
                }
            }
        }
    }
}


//
// Created by İhsan Demir on 17/05/15.
//



namespace hazelcast {
    namespace client {
        namespace test {
            namespace common {
                namespace containers {
                    class LittleEndianBufferTest : public ::testing::Test,
                                                   public hazelcast::util::LittleEndianBufferWrapper /* Need this in order to test*/
                    {
                    public:
                        LittleEndianBufferTest() : LittleEndianBufferWrapper(0) {}
                    };

                    TEST_F (LittleEndianBufferTest, testBinaryFormat) {
#define TEST_DATA_SIZE 8
#define LARGE_BUFFER_SIZE 20
#define START_BYTE_NUMBER  5

                        uint64_t ONE = 1;
                        uint64_t oneByteFactor = ONE << 8;
                        uint64_t twoBytesFactor = ONE << 16;
                        uint64_t threeBytesFactor = ONE << 24;
                        uint64_t fourBytesFactor = ONE << 32;
                        uint64_t fiveBytesFactor = ONE << 40;
                        uint64_t sixBytesFactor = ONE << 48;
                        uint64_t sevenBytesFactor = ONE << 56;

                        byte buf[TEST_DATA_SIZE] = {0x8A, 0x9A, 0xAA, 0xBA, 0xCA, 0xDA, 0xEA, 0x8B};
                        buffer.resize(LARGE_BUFFER_SIZE);
                        memcpy(&buffer[START_BYTE_NUMBER], buf, TEST_DATA_SIZE);

                        // ----- Test unsigned get starts ---------------------------------
                        // NOTE: When the first bit of the highest byte is equal to 1, than the number is negative,
                        // and the value is (-1 * (1s complement + 1)) (i.e. this is twos complement)
                        {
                            wrapForRead(LARGE_BUFFER_SIZE, START_BYTE_NUMBER);
                            uint8_t result = getUint8();
                            ASSERT_EQ(0x8A, result);
                        }

                        {
                            wrapForRead(LARGE_BUFFER_SIZE, START_BYTE_NUMBER);
                            uint16_t result = getUint16();
                            ASSERT_EQ(0x8A + 0x9A * oneByteFactor, result);
                        }

                        {
                            wrapForRead(LARGE_BUFFER_SIZE, START_BYTE_NUMBER);
                            uint32_t result = getUint32();
                            ASSERT_EQ(0x8A +
                                      0x9A * oneByteFactor +
                                      0xAA * twoBytesFactor +
                                      0xBA * threeBytesFactor, result);
                        }

                        {
                            wrapForRead(LARGE_BUFFER_SIZE, START_BYTE_NUMBER);
                            uint64_t result = getUint64();
                            ASSERT_EQ(0x8A +
                                      0x9A * oneByteFactor +
                                      0xAA * twoBytesFactor +
                                      0xBA * threeBytesFactor +
                                      0xCA * fourBytesFactor +
                                      0xDA * fiveBytesFactor +
                                      0xEA * sixBytesFactor +
                                      0x8B * sevenBytesFactor, result);
                        }
                        // ----- Test unsigned get ends ---------------------------------

                        // ----- Test signed get starts ---------------------------------

                        {
                            wrapForRead(LARGE_BUFFER_SIZE, START_BYTE_NUMBER);
                            int16_t result = getInt16();
                            ASSERT_EQ(-1 * (~((int16_t) (0x8A +
                                                         0x9A * oneByteFactor)) + 1), result);
                        }

                        {
                            wrapForRead(LARGE_BUFFER_SIZE, START_BYTE_NUMBER);
                            int32_t result = getInt32();
                            ASSERT_EQ(-1 * (~((int32_t) (
                                    0x8A +
                                    0x9A * oneByteFactor +
                                    0xAA * twoBytesFactor +
                                    0xBA * threeBytesFactor)) + 1), result);
                        }

                        {
                            wrapForRead(LARGE_BUFFER_SIZE, START_BYTE_NUMBER);
                            int64_t result = getInt64();
                            ASSERT_EQ(-1 * (~((int64_t) (
                                    0x8A +
                                    0x9A * oneByteFactor +
                                    0xAA * twoBytesFactor +
                                    0xBA * threeBytesFactor +
                                    0xCA * fourBytesFactor +
                                    0xDA * fiveBytesFactor +
                                    0xEA * sixBytesFactor +
                                    0x8B * sevenBytesFactor)) + 1), result);
                        }
// ----- Test signed get ends ---------------------------------

                        const byte firstChar = 'B';
                        byte strBytes[8] = {4, 0, 0, 0, /* This part is the len field which is 4 bytes */
                                            firstChar, firstChar + 1, firstChar + 2,
                                            firstChar + 3}; // This is string BCDE

                        buffer.clear();
                        buffer.insert(buffer.begin(), strBytes, strBytes + 8);
                        {
                            wrapForRead(8, 0);
                            ASSERT_EQ("BCDE", getStringUtf8());
                        }

// ---- Test consecutive gets starts ---------------------------
                        {
                            byte continousBuffer[45] = {0x8A, 0x9A, 0xAA, 0xBA, 0xCA, 0xDA, 0xEA, 0x8B,
                                                        0x8A, 0x9A, 0xAA, 0xBA, 0xCA, 0xDA, 0xEA, 0x8B,
                                                        0x8A, 0x9A, 0xAA, 0xBA, 0xCA, 0xDA, 0xEA, 0x8B,
                                                        0x8A, 0x9A, 0xAA, 0xBA, 0xCA,
                                                        4, 0, 0, 0, /* This part is the len field which is 4 bytes */
                                                        firstChar, firstChar + 1, firstChar + 2, firstChar + 3,
                                                        0x8A, 0x01, 0x00, 0xBA, 0xCA, 0xDA, 0xEA, 0x8B};

                            buffer.clear();
                            buffer.insert(buffer.begin(), continousBuffer, continousBuffer + 45);

                            wrapForRead(8 * 10, 0);

                            {
                                uint8_t result = getUint8();
                                ASSERT_EQ(0x8A, result);
                            }

                            {
                                uint16_t result = getUint16();
                                ASSERT_EQ(0x9A +
                                          0xAA * oneByteFactor, result);
                            }

                            {
                                uint32_t result = getUint32();
                                ASSERT_EQ(0xBA +
                                          0xCA * oneByteFactor +
                                          0xDA * twoBytesFactor +
                                          0xEA * threeBytesFactor, result);
                            }

                            {
                                uint64_t result = getUint64();
                                ASSERT_EQ(0x8B +
                                          0x8A * oneByteFactor +
                                          0x9A * twoBytesFactor +
                                          0xAA * threeBytesFactor +
                                          0xBA * fourBytesFactor +
                                          0xCA * fiveBytesFactor +
                                          0xDA * sixBytesFactor +
                                          0xEA * sevenBytesFactor, result);
                            }
// ----- Test unsigned get ends ---------------------------------

// ----- Test signed get starts ---------------------------------

                            {
                                int16_t result = getInt16();
                                ASSERT_EQ(-1 * (~((int16_t) (0x8B +
                                                             0x8A * oneByteFactor)) + 1), result);
                            }

                            {
                                int32_t result = getInt32();
                                ASSERT_EQ(-1 * (~((int32_t) (
                                        0x9A +
                                        0xAA * oneByteFactor +
                                        0xBA * twoBytesFactor +
                                        0xCA * threeBytesFactor)) + 1), result);
                            }

                            {
                                int64_t result = getInt64();
                                ASSERT_EQ(-1 * (~((int64_t) (
                                        0xDA +
                                        0xEA * oneByteFactor +
                                        0x8B * twoBytesFactor +
                                        0x8A * threeBytesFactor +
                                        0x9A * fourBytesFactor +
                                        0xAA * fiveBytesFactor +
                                        0xBA * sixBytesFactor +
                                        0xCA * sevenBytesFactor)) + 1), result);
                            }
// ----- Test signed get ends ---------------------------------

                            {
                                ASSERT_EQ("BCDE", getStringUtf8());
                            }

                            {
                                bool result = getBoolean();
                                ASSERT_TRUE(result);

                                result = getBoolean();
                                ASSERT_TRUE(result);

                                result = getBoolean();
                                ASSERT_FALSE(result);
                            }

                        }
// ---- Test consecutive gets ends ---------------------------

// ---- Write related tests starts --------------------------
                        buffer.clear();
                        buffer.resize(30, 0);
                        wrapForWrite(30, 0);

                        set((uint8_t) 0x8A);
                        ASSERT_EQ(0x8A, buffer[0]);

                        set(true);
                        ASSERT_EQ(0x01, buffer[1]);

                        set(false);
                        ASSERT_EQ(0x00, buffer[2]);

                        set('C');
                        ASSERT_EQ('C', buffer[3]);

                        int16_t int16Val = 0x7BCD;
                        set(int16Val);
                        ASSERT_EQ(0xCD, buffer[4]);
                        ASSERT_EQ(0x7B, buffer[5]);

                        uint16_t uInt16Val = 0xABCD;
                        set(uInt16Val);
                        ASSERT_EQ(0xCD, buffer[6]);
                        ASSERT_EQ(0xAB, buffer[7]);

                        int32_t int32Val = 0xAEBCEEFF;
                        set(int32Val);
                        ASSERT_EQ(0xFF, buffer[8]);
                        ASSERT_EQ(0xEE, buffer[9]);
                        ASSERT_EQ(0xBC, buffer[10]);
                        ASSERT_EQ(0xAE, buffer[11]);


                        set(std::string("Test Data"));
                        ASSERT_EQ(0x09, (int) buffer[12]);
                        ASSERT_EQ(0x0, buffer[13]);
                        ASSERT_EQ(0x0, buffer[14]);
                        ASSERT_EQ(0x0, buffer[15]);
                        ASSERT_EQ('T', buffer[16]);
                        ASSERT_EQ('e', buffer[17]);
                        ASSERT_EQ('a', buffer[24]);
                    }
                }
            }
        }
    }
}


namespace hazelcast {
    namespace client {

        namespace test {
            class CallIdSequenceWithoutBackpressureTest : public ClientTestSupport {
            protected:
                spi::impl::sequence::CallIdSequenceWithoutBackpressure sequence;

                void next(bool isUrgent) {
                    int64_t oldSequence = sequence.getLastCallId();
                    int64_t result = nextCallId(sequence, isUrgent);
                    ASSERT_EQ(oldSequence + 1, result);
                    ASSERT_EQ(oldSequence + 1, sequence.getLastCallId());
                }

                int64_t nextCallId(spi::impl::sequence::CallIdSequence &seq, bool isUrgent) {
                    return isUrgent ? seq.forceNext() : seq.next();
                }
            };

            TEST_F(CallIdSequenceWithoutBackpressureTest, testInit) {
                ASSERT_EQ(0, sequence.getLastCallId());
                ASSERT_EQ(INT32_MAX, sequence.getMaxConcurrentInvocations());
            }

            TEST_F(CallIdSequenceWithoutBackpressureTest, testNext) {
                // regular operation
                next(false);
                next(true);
            }

            TEST_F(CallIdSequenceWithoutBackpressureTest, whenNextRepeated_thenKeepSucceeding) {
                for (int64_t k = 1; k < 10000; k++) {
                    ASSERT_EQ(k, nextCallId(sequence, false));
                }
            }

            TEST_F(CallIdSequenceWithoutBackpressureTest, complete) {
                nextCallId(sequence, false);
                int64_t oldSequence = sequence.getLastCallId();
                sequence.complete();
                ASSERT_EQ(oldSequence, sequence.getLastCallId());
            }
        }
    }
}


namespace hazelcast {
    namespace client {

        namespace test {
            class CallIdSequenceWithBackpressureTest : public ClientTestSupport {
            public:
                CallIdSequenceWithBackpressureTest() {}

            protected:
                class ThreeSecondDelayCompleteOperation : public hazelcast::util::Runnable {
                public:
                    ThreeSecondDelayCompleteOperation(spi::impl::sequence::CallIdSequenceWithBackpressure &sequence,
                                                      hazelcast::util::CountDownLatch &nextCalledLatch) : sequence(
                            sequence),
                                                                                                          nextCalledLatch(
                                                                                                                  nextCalledLatch) {}

                    virtual const std::string getName() const {
                        return "ThreeSecondDelayCompleteOperation";
                    }

                    virtual void run() {
                        sequence.next();
                        nextCalledLatch.countDown();
                        sleepSeconds(3);
                        sequence.complete();
                    }

                private:
                    spi::impl::sequence::CallIdSequenceWithBackpressure &sequence;
                    hazelcast::util::CountDownLatch &nextCalledLatch;
                };

                int64_t nextCallId(spi::impl::sequence::CallIdSequence &seq, bool isUrgent) {
                    return isUrgent ? seq.forceNext() : seq.next();
                }
            };

            TEST_F(CallIdSequenceWithBackpressureTest, testInit) {
                spi::impl::sequence::CallIdSequenceWithBackpressure sequence(100, 60000);
                ASSERT_EQ(0, sequence.getLastCallId());
                ASSERT_EQ(100, sequence.getMaxConcurrentInvocations());
            }

            TEST_F(CallIdSequenceWithBackpressureTest, whenNext_thenSequenceIncrements) {
                spi::impl::sequence::CallIdSequenceWithBackpressure sequence(100, 60000);
                int64_t oldSequence = sequence.getLastCallId();
                int64_t result = sequence.next();
                ASSERT_EQ(oldSequence + 1, result);
                ASSERT_EQ(oldSequence + 1, sequence.getLastCallId());

                oldSequence = sequence.getLastCallId();
                result = sequence.forceNext();
                ASSERT_EQ(oldSequence + 1, result);
                ASSERT_EQ(oldSequence + 1, sequence.getLastCallId());
            }

            TEST_F(CallIdSequenceWithBackpressureTest, next_whenNoCapacity_thenBlockTillCapacity) {
                spi::impl::sequence::CallIdSequenceWithBackpressure sequence(1, 60000);
                int64_t oldLastCallId = sequence.getLastCallId();

                hazelcast::util::CountDownLatch nextCalledLatch(1);

                auto f = std::async(std::packaged_task<void()>(
                        [&]() { ThreeSecondDelayCompleteOperation(sequence, nextCalledLatch).run(); }));

                ASSERT_OPEN_EVENTUALLY(nextCalledLatch);

                int64_t result = sequence.next();
                ASSERT_EQ(oldLastCallId + 2, result);
                ASSERT_EQ(oldLastCallId + 2, sequence.getLastCallId());
                f.get();
            }

            TEST_F(CallIdSequenceWithBackpressureTest, next_whenNoCapacity_thenBlockTillTimeout) {
                spi::impl::sequence::CallIdSequenceWithBackpressure sequence(1, 2000);
// first invocation consumes the available call ID
                nextCallId(sequence, false);

                int64_t oldLastCallId = sequence.getLastCallId();
                ASSERT_THROW(sequence.next(), exception::HazelcastOverloadException);

                ASSERT_EQ(oldLastCallId, sequence.getLastCallId());
            }

            TEST_F(CallIdSequenceWithBackpressureTest, when_overCapacityButPriorityItem_then_noBackpressure) {
                spi::impl::sequence::CallIdSequenceWithBackpressure sequence(1, 60000);

// occupy the single call ID slot
                nextCallId(sequence, true);

                int64_t oldLastCallId = sequence.getLastCallId();

                int64_t result = nextCallId(sequence, true);
                ASSERT_EQ(oldLastCallId + 1, result);
                ASSERT_EQ(oldLastCallId + 1, sequence.getLastCallId());
            }

            TEST_F(CallIdSequenceWithBackpressureTest, whenComplete_thenTailIncrements) {
                spi::impl::sequence::CallIdSequenceWithBackpressure sequence(1, 60000);

                nextCallId(sequence, false);

                int64_t oldSequence = sequence.getLastCallId();
                int64_t oldTail = sequence.getTail();
                sequence.complete();

                ASSERT_EQ(oldSequence, sequence.getLastCallId());
                ASSERT_EQ(oldTail + 1, sequence.getTail());
            }

        }
    }
}


namespace hazelcast {
    namespace client {

        namespace test {
            class FailFastCallIdSequenceTest : public ClientTestSupport {
            public:
            };

            TEST_F(FailFastCallIdSequenceTest, testGettersAndDefaults) {
                spi::impl::sequence::FailFastCallIdSequence sequence(100);
                ASSERT_EQ(0, sequence.getLastCallId());
                ASSERT_EQ(100, sequence.getMaxConcurrentInvocations());
            }

            TEST_F(FailFastCallIdSequenceTest, whenNext_thenSequenceIncrements) {
                spi::impl::sequence::FailFastCallIdSequence sequence(100);
                int64_t oldSequence = sequence.getLastCallId();
                int64_t result = sequence.next();
                ASSERT_EQ(oldSequence + 1, result);
                ASSERT_EQ(oldSequence + 1, sequence.getLastCallId());
            }

            TEST_F(FailFastCallIdSequenceTest, next_whenNoCapacity_thenThrowException) {
                spi::impl::sequence::FailFastCallIdSequence sequence(1);

                // take the only slot available
                sequence.next();

                // this next is going to fail with an exception
                ASSERT_THROW(sequence.next(), exception::HazelcastOverloadException);
            }

            TEST_F(FailFastCallIdSequenceTest, when_overCapacityButPriorityItem_then_noException) {
                spi::impl::sequence::FailFastCallIdSequence sequence(1);

// take the only slot available
                ASSERT_EQ(1, sequence.next());

                ASSERT_EQ(2, sequence.forceNext());
            }

            TEST_F(FailFastCallIdSequenceTest, whenComplete_thenTailIncrements) {
                spi::impl::sequence::FailFastCallIdSequence sequence(100);
                sequence.next();

                int64_t oldSequence = sequence.getLastCallId();
                int64_t oldTail = sequence.getTail();
                sequence.complete();

                ASSERT_EQ(oldSequence, sequence.getLastCallId());
                ASSERT_EQ(oldTail + 1, sequence.getTail());
            }

        }
    }
}

namespace hazelcast {
    namespace client {
        namespace test {
            extern HazelcastServerFactory *g_srvFactory;
            extern std::shared_ptr<RemoteControllerClient> remoteController;

            HazelcastServerFactory::HazelcastServerFactory(const std::string &serverXmlConfigFilePath)
                    : HazelcastServerFactory::HazelcastServerFactory(g_srvFactory->getServerAddress(),
                                                                     serverXmlConfigFilePath) {
            }

            HazelcastServerFactory::HazelcastServerFactory(const std::string &serverAddress,
                                                           const std::string &serverXmlConfigFilePath)
                    : logger("HazelcastServerFactory", "HazelcastServerFactory", "testversion", config::LoggerConfig()),
                      serverAddress(serverAddress) {

                if (!logger.start()) {
                    throw (client::exception::ExceptionBuilder<client::exception::IllegalStateException>(
                            "HazelcastServerFactory::HazelcastServerFactory") << "Could not start logger "
                                                                              << logger.getInstanceName()).build();
                }

                std::string xmlConfig = readFromXmlFile(serverXmlConfigFilePath);

                remote::Cluster cluster;
                remoteController->createCluster(cluster, HAZELCAST_VERSION, xmlConfig);

                this->clusterId = cluster.id;
            }

            HazelcastServerFactory::~HazelcastServerFactory() {
                remoteController->shutdownCluster(clusterId);
            }

            remote::Member HazelcastServerFactory::startServer() {
                remote::Member member;
                remoteController->startMember(member, clusterId);
                return member;
            }

            bool HazelcastServerFactory::setAttributes(int memberStartOrder) {
                std::ostringstream script;
                script << "function attrs() { "
                          "var member = instance_" << memberStartOrder << ".getCluster().getLocalMember(); "
                                                                          "member.setIntAttribute(\"intAttr\", 211); "
                                                                          "member.setBooleanAttribute(\"boolAttr\", true); "
                                                                          "member.setByteAttribute(\"byteAttr\", 7); "
                                                                          "member.setDoubleAttribute(\"doubleAttr\", 2.0); "
                                                                          "member.setFloatAttribute(\"floatAttr\", 1.2); "
                                                                          "member.setShortAttribute(\"shortAttr\", 3); "
                                                                          "return member.setStringAttribute(\"strAttr\", \"strAttr\");} "
                                                                          " result=attrs(); ";


                Response response;
                remoteController->executeOnController(response, clusterId, script.str().c_str(), Lang::JAVASCRIPT);
                return response.success;
            }

            bool HazelcastServerFactory::shutdownServer(const remote::Member &member) {
                return remoteController->shutdownMember(clusterId, member.uuid);
            }

            bool HazelcastServerFactory::terminateServer(const remote::Member &member) {
                return remoteController->terminateMember(clusterId, member.uuid);
            }

            const std::string &HazelcastServerFactory::getServerAddress() {
                return serverAddress;
            }

            std::string HazelcastServerFactory::readFromXmlFile(const std::string &xmlFilePath) {
                std::ifstream xmlFile(xmlFilePath.c_str());
                if (!xmlFile) {
                    std::ostringstream out;
                    out << "Failed to read from xml file to at " << xmlFilePath;
                    BOOST_THROW_EXCEPTION(
                            exception::IllegalStateException("HazelcastServerFactory::readFromXmlFile", out.str()));
                }

                std::ostringstream buffer;

                buffer << xmlFile.rdbuf();

                xmlFile.close();

                return buffer.str();
            }

        }
    }
}


using namespace hazelcast::client::mixedtype;

namespace hazelcast {
    namespace client {
        namespace test {
            class BaseCustom {
            public:
                BaseCustom() {
                    value = 3;
                }

                BaseCustom(int value) : value(value) {}

                int getValue() const {
                    return value;
                }

                void setValue(int value) {
                    BaseCustom::value = value;
                }

                bool operator<(const BaseCustom &rhs) const {
                    return getValue() < rhs.getValue();
                }

            private:
                int value;
            };

            class Derived1Custom : public BaseCustom {
            public:
                Derived1Custom() : BaseCustom(4) {}

                Derived1Custom(int value) : BaseCustom(value) {}
            };

            class Derived2Custom : public Derived1Custom {
            public:
                Derived2Custom() : Derived1Custom(5) {}
            };

            int32_t getHazelcastTypeId(const BaseCustom *) {
                return 3;
            }

            int32_t getHazelcastTypeId(const Derived1Custom *) {
                return 4;
            }

            int32_t getHazelcastTypeId(const Derived2Custom *) {
                return 5;
            }

            class MixedMapTest : public ClientTestSupport {
            protected:
                class BaseCustomSerializer : public serialization::StreamSerializer {
                public:
                    virtual int32_t getHazelcastTypeId() const {
                        return 3;
                    }

                    virtual void write(serialization::ObjectDataOutput &out, const void *object) {
                        out.writeInt(static_cast<const BaseCustom *>(object)->getValue());
                    }

                    virtual void *read(serialization::ObjectDataInput &in) {
                        std::unique_ptr<BaseCustom> object(new BaseCustom);
                        object->setValue(in.readInt());
                        return object.release();
                    }
                };

                class Derived1CustomSerializer : public BaseCustomSerializer {
                public:
                    virtual int32_t getHazelcastTypeId() const {
                        return 4;
                    }

                    virtual void *read(serialization::ObjectDataInput &in) {
                        std::unique_ptr<Derived1Custom> object(new Derived1Custom);
                        object->setValue(in.readInt());
                        return object.release();
                    }
                };

                class Derived2CustomSerializer : public BaseCustomSerializer {
                public:
                    virtual int32_t getHazelcastTypeId() const {
                        return 5;
                    }

                    virtual void *read(serialization::ObjectDataInput &in) {
                        std::unique_ptr<Derived2Custom> object(new Derived2Custom);
                        object->setValue(in.readInt());
                        return object.release();
                    }
                };

                class BaseDataSerializable : public serialization::IdentifiedDataSerializable {
                public:
                    virtual ~BaseDataSerializable() {}

                    virtual int getFactoryId() const {
                        return 666;
                    }

                    virtual int getClassId() const {
                        return 10;
                    }

                    virtual void writeData(serialization::ObjectDataOutput &writer) const {
                    }

                    virtual void readData(serialization::ObjectDataInput &reader) {
                    }

                    virtual bool operator<(const BaseDataSerializable &rhs) const {
                        return getClassId() < rhs.getClassId();
                    }
                };

                class Derived1DataSerializable : public BaseDataSerializable {
                public:
                    virtual int getClassId() const {
                        return 11;
                    }
                };

                class Derived2DataSerializable : public Derived1DataSerializable {
                public:
                    virtual int getClassId() const {
                        return 12;
                    }
                };

                class BasePortable : public serialization::Portable {
                public:
                    virtual ~BasePortable() {}

                    virtual int getFactoryId() const {
                        return 666;
                    }

                    virtual int getClassId() const {
                        return 3;
                    }

                    virtual void writePortable(serialization::PortableWriter &writer) const {
                    }

                    virtual void readPortable(serialization::PortableReader &reader) {
                    }

                    bool operator<(const BasePortable &rhs) const {
                        return getClassId() < rhs.getClassId();
                    }
                };

                class Derived1Portable : public BasePortable {
                public:
                    virtual int getClassId() const {
                        return 4;
                    }
                };

                class Derived2Portable : public BasePortable {
                public:
                    virtual int getClassId() const {
                        return 5;
                    }
                };

                class PolymorphicDataSerializableFactory : public serialization::DataSerializableFactory {
                public:
                    virtual std::unique_ptr<serialization::IdentifiedDataSerializable> create(int32_t typeId) {
                        switch (typeId) {
                            case 10:
                                return std::unique_ptr<serialization::IdentifiedDataSerializable>(
                                        new BaseDataSerializable);
                            case 11:
                                return std::unique_ptr<serialization::IdentifiedDataSerializable>(
                                        new Derived1DataSerializable);
                            case 12:
                                return std::unique_ptr<serialization::IdentifiedDataSerializable>(
                                        new Derived2DataSerializable);
                            default:
                                return std::unique_ptr<serialization::IdentifiedDataSerializable>();
                        }
                    }
                };

                class PolymorphicPortableFactory : public serialization::PortableFactory {
                public:
                    virtual std::unique_ptr<serialization::Portable> create(int32_t classId) const {
                        switch (classId) {
                            case 3:
                                return std::unique_ptr<serialization::Portable>(new BasePortable);
                            case 4:
                                return std::unique_ptr<serialization::Portable>(new Derived1Portable);
                            case 5:
                                return std::unique_ptr<serialization::Portable>(new Derived2Portable);
                            default:
                                return std::unique_ptr<serialization::Portable>();
                        }
                    }
                };

                virtual void TearDown() {
                    // clear maps
                    mixedMap->clear();
                }

                static void SetUpTestCase() {
                    instance = new HazelcastServer(*g_srvFactory);
                    client = new HazelcastClient(getConfig());
                    mixedMap = new mixedtype::IMap(client->toMixedType().getMap("MyMap"));

                    ClientConfig config;
                    SerializationConfig &serializationConfig = config.getSerializationConfig();
                    serializationConfig.addDataSerializableFactory(666,
                                                                   std::shared_ptr<serialization::DataSerializableFactory>(
                                                                           new PolymorphicDataSerializableFactory()));
                    serializationConfig.addPortableFactory(666, std::shared_ptr<serialization::PortableFactory>(
                            new PolymorphicPortableFactory));

                    serializationConfig.registerSerializer(
                            std::shared_ptr<serialization::SerializerBase>(new BaseCustomSerializer));

                    serializationConfig.registerSerializer(
                            std::shared_ptr<serialization::SerializerBase>(new Derived1CustomSerializer));

                    serializationConfig.registerSerializer(
                            std::shared_ptr<serialization::SerializerBase>(new Derived2CustomSerializer));

                    client2 = new HazelcastClient(config);
                    imap = new IMap<int, BaseDataSerializable>(client2->getMap<int, BaseDataSerializable>("MyMap"));
                    rawPointerMap = new hazelcast::client::adaptor::RawPointerMap<int, BaseDataSerializable>(*imap);
                    imapPortable = new IMap<int, BasePortable>(client2->getMap<int, BasePortable>("MyMap"));
                    rawPointerMapPortable = new hazelcast::client::adaptor::RawPointerMap<int, BasePortable>(
                            *imapPortable);
                    imapCustom = new IMap<int, BaseCustom>(client2->getMap<int, BaseCustom>("MyMap"));
                    rawPointerMapCustom = new hazelcast::client::adaptor::RawPointerMap<int, BaseCustom>(*imapCustom);
                }

                static void TearDownTestCase() {
                    //delete mixedMap;
                    delete rawPointerMap;
                    delete imap;
                    delete rawPointerMapPortable;
                    delete imapPortable;
                    delete rawPointerMapCustom;
                    delete imapCustom;
                    delete client;
                    delete client2;
                    delete instance2;
                    delete instance;

                    mixedMap = NULL;
                    rawPointerMap = NULL;
                    imap = NULL;
                    rawPointerMapPortable = NULL;
                    imapPortable = NULL;
                    rawPointerMapCustom = NULL;
                    imapCustom = NULL;
                    client = NULL;
                    client2 = NULL;
                    instance2 = NULL;
                    instance = NULL;
                }

                static HazelcastServer *instance;
                static HazelcastServer *instance2;
                static HazelcastClient *client;
                static mixedtype::IMap *mixedMap;
                static HazelcastClient *client2;
                static IMap<int, BaseDataSerializable> *imap;
                static hazelcast::client::adaptor::RawPointerMap<int, BaseDataSerializable> *rawPointerMap;
                static IMap<int, BasePortable> *imapPortable;
                static hazelcast::client::adaptor::RawPointerMap<int, BasePortable> *rawPointerMapPortable;
                static IMap<int, BaseCustom> *imapCustom;
                static hazelcast::client::adaptor::RawPointerMap<int, BaseCustom> *rawPointerMapCustom;
            };

            HazelcastServer *MixedMapTest::instance = NULL;
            HazelcastServer *MixedMapTest::instance2 = NULL;
            HazelcastClient *MixedMapTest::client = NULL;
            mixedtype::IMap *MixedMapTest::mixedMap = NULL;
            HazelcastClient *MixedMapTest::client2 = NULL;
            IMap<int, MixedMapTest::BaseDataSerializable> *MixedMapTest::imap = NULL;
            hazelcast::client::adaptor::RawPointerMap<int, MixedMapTest::BaseDataSerializable> *MixedMapTest::rawPointerMap = NULL;
            IMap<int, MixedMapTest::BasePortable> *MixedMapTest::imapPortable = NULL;
            hazelcast::client::adaptor::RawPointerMap<int, MixedMapTest::BasePortable> *MixedMapTest::rawPointerMapPortable = NULL;
            IMap<int, BaseCustom> *MixedMapTest::imapCustom = NULL;
            hazelcast::client::adaptor::RawPointerMap<int, BaseCustom> *MixedMapTest::rawPointerMapCustom = NULL;

            TEST_F(MixedMapTest, testPutDifferentTypes) {
                mixedMap->put<int, int>(3, 5);
                TypedData oldData = mixedMap->put<int, std::string>(10, "MyStringValue");

                ASSERT_EQ(NULL, oldData.getData().get());

                TypedData result = mixedMap->get<int>(3);
                ASSERT_EQ(-7, result.getType().typeId);
                std::unique_ptr<int> value = result.get<int>();
                ASSERT_NE((int *) NULL, value.get());
                ASSERT_EQ(5, *value);

                result = mixedMap->get<int>(10);
                ASSERT_EQ(-11, result.getType().typeId);
                std::unique_ptr<std::string> strValue = result.get<std::string>();
                ASSERT_NE((std::string *) NULL, strValue.get());
                ASSERT_EQ("MyStringValue", *strValue);
            }

            TEST_F(MixedMapTest, testPolymorphismWithIdentifiedDataSerializable) {
                BaseDataSerializable base;
                Derived1DataSerializable derived1;
                Derived2DataSerializable derived2;
                mixedMap->put<int, BaseDataSerializable>(1, base);
                mixedMap->put<int, Derived1DataSerializable>(2, derived1);
                mixedMap->put<int, Derived2DataSerializable>(3, derived2);

                std::set<int> keys;
                keys.insert(1);
                keys.insert(2);
                keys.insert(3);

                std::vector<std::pair<TypedData, TypedData> > values = mixedMap->getAll<int>(keys);
                for (std::vector<std::pair<TypedData, TypedData> >::iterator it = values.begin();
                     it != values.end(); ++it) {
                    TypedData &keyData = (*it).first;
                    TypedData &valueData = (*it).second;
                    std::unique_ptr<int> key = keyData.get<int>();
                    ASSERT_NE((int *) NULL, key.get());
                    serialization::pimpl::ObjectType objectType = valueData.getType();
                    switch (*key) {
                        case 1: {
                            // serialization::pimpl::SerializationConstants::CONSTANT_TYPE_DATA, using -2 since static
                            // variable is not exported into the library
                            ASSERT_EQ(-2, objectType.typeId);
                            ASSERT_EQ(666, objectType.factoryId);
                            ASSERT_EQ(10, objectType.classId);
                            std::unique_ptr<BaseDataSerializable> value = valueData.get<BaseDataSerializable>();
                            ASSERT_NE((BaseDataSerializable *) NULL, value.get());
                            break;
                        }
                        case 2: {
                            ASSERT_EQ(-2, objectType.typeId);
                            ASSERT_EQ(666, objectType.factoryId);
                            ASSERT_EQ(11, objectType.classId);
                            std::unique_ptr<BaseDataSerializable> value(valueData.get<Derived1DataSerializable>());
                            ASSERT_NE((BaseDataSerializable *) NULL, value.get());
                            break;
                        }
                        case 3: {
                            ASSERT_EQ(-2, objectType.typeId);
                            ASSERT_EQ(666, objectType.factoryId);
                            ASSERT_EQ(12, objectType.classId);
                            std::unique_ptr<BaseDataSerializable> value(valueData.get<Derived2DataSerializable>());
                            ASSERT_NE((BaseDataSerializable *) NULL, value.get());
                            break;
                        }
                        default:
                            FAIL();
                    }
                }
            }

            TEST_F(MixedMapTest, testPolymorphismUsingBaseClassWithIdentifiedDataSerializable) {
                BaseDataSerializable base;
                Derived1DataSerializable derived1;
                Derived2DataSerializable derived2;
                rawPointerMap->put(1, base);
                rawPointerMap->put(2, derived1);
                rawPointerMap->put(3, derived2);

                std::set<int> keys;
                keys.insert(1);
                keys.insert(2);
                keys.insert(3);

                std::unique_ptr<EntryArray<int, BaseDataSerializable> > entries = rawPointerMap->getAll(keys);
                ASSERT_NE((EntryArray<int, BaseDataSerializable> *) NULL, entries.get());
                ASSERT_EQ((size_t) 3, entries->size());
                for (size_t i = 0; i < entries->size(); ++i) {
                    std::pair<const int *, const BaseDataSerializable *> entry = (*entries)[i];
                    const int *key = entry.first;
                    ASSERT_NE((const int *) NULL, key);
                    const BaseDataSerializable *value = entry.second;
                    ASSERT_NE((const BaseDataSerializable *) NULL, value);
                    switch (*key) {
                        case 1:
                            ASSERT_EQ(base.getClassId(), value->getClassId());
                            break;
                        case 2:
                            ASSERT_EQ(derived1.getClassId(), value->getClassId());
                            break;
                        case 3:
                            ASSERT_EQ(derived2.getClassId(), value->getClassId());
                            break;
                        default:
                            FAIL();
                    }
                }
            }

            TEST_F(MixedMapTest, testPolymorphismWithPortable) {
                BasePortable base;
                Derived1Portable derived1;
                Derived2Portable derived2;
                mixedMap->put<int, BasePortable>(1, base);
                mixedMap->put<int, Derived1Portable>(2, derived1);
                mixedMap->put<int, Derived2Portable>(3, derived2);

                TypedData secondData = mixedMap->get<int>(2);
                serialization::pimpl::ObjectType secondType = secondData.getType();
                ASSERT_EQ(-1, secondType.typeId);
                secondData.get<Derived1Portable>();

                std::set<int> keys;
                keys.insert(1);
                keys.insert(2);
                keys.insert(3);

                std::vector<std::pair<TypedData, TypedData> > values = mixedMap->getAll<int>(keys);
                for (std::vector<std::pair<TypedData, TypedData> >::iterator it = values.begin();
                     it != values.end(); ++it) {
                    TypedData &keyData = (*it).first;
                    TypedData &valueData = (*it).second;
                    std::unique_ptr<int> key = keyData.get<int>();
                    ASSERT_NE((int *) NULL, key.get());
                    serialization::pimpl::ObjectType objectType = valueData.getType();
                    switch (*key) {
                        case 1: {
// serialization::pimpl::SerializationConstants::CONSTANT_TYPE_PORTABLE, using -1 since static
// variable is not exported into the library
                            ASSERT_EQ(-1, objectType.typeId);
                            ASSERT_EQ(666, objectType.factoryId);
                            ASSERT_EQ(3, objectType.classId);
                            std::unique_ptr<BasePortable> value = valueData.get<BasePortable>();
                            ASSERT_NE((BasePortable *) NULL, value.get());
                            break;
                        }
                        case 2: {
                            ASSERT_EQ(-1, objectType.typeId);
                            ASSERT_EQ(666, objectType.factoryId);
                            ASSERT_EQ(4, objectType.classId);
                            std::unique_ptr<BasePortable> value(valueData.get<Derived1Portable>());
                            ASSERT_NE((BasePortable *) NULL, value.get());
                            break;
                        }
                        case 3: {
                            ASSERT_EQ(-1, objectType.typeId);
                            ASSERT_EQ(666, objectType.factoryId);
                            ASSERT_EQ(5, objectType.classId);
                            std::unique_ptr<BasePortable> value(valueData.get<Derived2Portable>());
                            ASSERT_NE((BasePortable *) NULL, value.get());
                            break;
                        }
                        default:
                            FAIL();
                    }
                }
            }

            TEST_F(MixedMapTest, testPolymorphismUsingBaseClassWithPortable) {
                BasePortable basePortable;
                Derived1Portable derived1Portable;
                Derived2Portable derived2Portable;
                rawPointerMapPortable->put(1, basePortable);
                rawPointerMapPortable->put(2, derived1Portable);
                rawPointerMapPortable->put(3, derived2Portable);

                std::set<int> keys;
                keys.insert(1);
                keys.insert(2);
                keys.insert(3);

                std::unique_ptr<EntryArray<int, BasePortable> > entries = rawPointerMapPortable->getAll(keys);
                ASSERT_NE((EntryArray<int, BasePortable> *) NULL, entries.get());
                ASSERT_EQ((size_t) 3, entries->size());
                for (size_t i = 0; i < entries->size(); ++i) {
                    std::pair<const int *, const BasePortable *> entry = (*entries)[i];
                    const int *key = entry.first;
                    ASSERT_NE((const int *) NULL, key);
                    const BasePortable *value = entry.second;
                    ASSERT_NE((const BasePortable *) NULL, value);
                    switch (*key) {
                        case 1:
                            ASSERT_EQ(basePortable.getClassId(), value->getClassId());
                            break;
                        case 2:
                            ASSERT_EQ(derived1Portable.getClassId(), value->getClassId());
                            break;
                        case 3:
                            ASSERT_EQ(derived2Portable.getClassId(), value->getClassId());
                            break;
                        default:
                            FAIL();
                    }
                }
            }

            TEST_F(MixedMapTest, testPolymorphismUsingBaseClass) {
                BaseCustom baseCustom;
                Derived1Custom derived1Custom;
                Derived2Custom derived2Custom;
                rawPointerMapCustom->put(1, baseCustom);
                rawPointerMapCustom->put(2, derived1Custom);
                rawPointerMapCustom->put(3, derived2Custom);

                std::set<int> keys;
                keys.insert(1);
                keys.insert(2);
                keys.insert(3);

                std::unique_ptr<EntryArray<int, BaseCustom> > entries = rawPointerMapCustom->getAll(keys);
                ASSERT_NE((EntryArray<int, BaseCustom> *) NULL, entries.get());
                ASSERT_EQ((size_t) 3, entries->size());
                for (size_t i = 0; i < entries->size(); ++i) {
                    std::pair<const int *, const BaseCustom *> entry = (*entries)[i];
                    const int *key = entry.first;
                    ASSERT_NE((const int *) NULL, key);
                    const BaseCustom *value = entry.second;
                    ASSERT_NE((const BaseCustom *) NULL, value);
                    switch (*key) {
                        case 1:
                            ASSERT_EQ(baseCustom.getValue(), value->getValue());
                            break;
                        case 2:
                            ASSERT_EQ(derived1Custom.getValue(), value->getValue());
                            break;
                        case 3:
                            ASSERT_EQ(derived2Custom.getValue(), value->getValue());
                            break;
                        default:
                            FAIL();
                    }
                }
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace test {
            class MapGlobalSerializerTest : public ClientTestSupport {
            public:
                class UnknownObject {
                };

                class WriteReadIntGlobalSerializer : public serialization::StreamSerializer {
                public:
                    virtual int32_t getHazelcastTypeId() const {
                        return 123;
                    }

                    virtual void write(serialization::ObjectDataOutput &out, const void *object) {
                    }

                    virtual void *read(serialization::ObjectDataInput &in) {
                        return new int(5);
                    }
                };

            protected:

                static void SetUpTestCase() {
                    instance = new HazelcastServer(*g_srvFactory);
                    ClientConfig clientConfig;
                    clientConfig.getSerializationConfig().setGlobalSerializer(
                            std::shared_ptr<serialization::StreamSerializer>(new WriteReadIntGlobalSerializer()));
                    client = new HazelcastClient(clientConfig);
                    imap = new mixedtype::IMap(client->toMixedType().getMap("UnknownObject"));
                }

                static void TearDownTestCase() {
                    delete imap;
                    delete client;
                    delete instance;

                    imap = NULL;
                    client = NULL;
                    instance = NULL;
                }

                static HazelcastServer *instance;
                static HazelcastClient *client;
                static mixedtype::IMap *imap;
            };

            HazelcastServer *MapGlobalSerializerTest::instance = NULL;
            HazelcastClient *MapGlobalSerializerTest::client = NULL;
            mixedtype::IMap *MapGlobalSerializerTest::imap = NULL;

            TEST_F(MapGlobalSerializerTest, testPutUnknownObjectButGetIntegerValue) {
                MapGlobalSerializerTest::UnknownObject myObject;
                imap->put<int, MapGlobalSerializerTest::UnknownObject>(2, myObject);

                TypedData data = imap->get<int>(2);
                ASSERT_EQ(123, data.getType().typeId);
                std::ostringstream out;
                out << data.getType();
                ASSERT_TRUE(out.str().find("123") != std::string::npos);
                std::unique_ptr<int> value = data.get<int>();
                ASSERT_NE((int *) NULL, value.get());
                ASSERT_EQ(5, *value);
            }

        }
    }
}


namespace hazelcast {
    namespace client {
        namespace test {
            class ClientExpirationListenerTest : public ClientTestSupport {
            protected:
                virtual void TearDown() {
                    // clear maps
                    imap->clear();
                }

                static void SetUpTestCase() {
                    instance = new HazelcastServer(*g_srvFactory);
                    instance2 = new HazelcastServer(*g_srvFactory);
                    ClientConfig clientConfig(getConfig());
                    clientConfig.getProperties()[ClientProperties::PROP_HEARTBEAT_TIMEOUT] = "20";
                    client = new HazelcastClient(clientConfig);
                    imap = new IMap<int, int>(client->getMap<int, int>("IntMap"));
                }

                static void TearDownTestCase() {
                    delete imap;
                    delete client;
                    delete instance2;
                    delete instance;

                    imap = NULL;
                    client = NULL;
                    instance2 = NULL;
                    instance = NULL;
                }

                static HazelcastServer *instance;
                static HazelcastServer *instance2;
                static HazelcastClient *client;
                static IMap<int, int> *imap;
            };

            HazelcastServer *ClientExpirationListenerTest::instance = NULL;
            HazelcastServer *ClientExpirationListenerTest::instance2 = NULL;
            HazelcastClient *ClientExpirationListenerTest::client = NULL;
            IMap<int, int> *ClientExpirationListenerTest::imap = NULL;

            class ExpirationListener : public EntryAdapter<int, int> {
            public:
                ExpirationListener(hazelcast::util::CountDownLatch &latch)
                        : latch(latch) {

                }

                void entryExpired(const EntryEvent<int, int> &event) {
                    latch.countDown();
                }

            private:
                hazelcast::util::CountDownLatch &latch;
            };

            TEST_F(ClientExpirationListenerTest, notified_afterExpirationOfEntries) {
                int numberOfPutOperations = 10000;
                hazelcast::util::CountDownLatch expirationEventArrivalCount(numberOfPutOperations);

                ExpirationListener expirationListener(expirationEventArrivalCount);
                std::string registrationId = imap->addEntryListener(expirationListener, true);

                for (int i = 0; i < numberOfPutOperations; i++) {
                    imap->put(i, i, 100);
                }

                // wait expiration of entries.
                hazelcast::util::sleep(1);

                // trigger immediate fire of expiration events by touching them.
                for (int i = 0; i < numberOfPutOperations; ++i) {
                    imap->get(i);
                }

                ASSERT_OPEN_EVENTUALLY(expirationEventArrivalCount);
                ASSERT_TRUE(imap->removeEntryListener(registrationId));
            }


            class ExpirationAndEvictionListener : public EntryAdapter<int, int> {
            public:
                ExpirationAndEvictionListener(hazelcast::util::CountDownLatch &evictedLatch,
                                              hazelcast::util::CountDownLatch &expiredLatch)
                        : evictedLatch(evictedLatch), expiredLatch(expiredLatch) {

                }

                void entryEvicted(const EntryEvent<int, int> &event) {
                    evictedLatch.countDown();
                }

                void entryExpired(const EntryEvent<int, int> &event) {
                    expiredLatch.countDown();
                }

            private:
                hazelcast::util::CountDownLatch &evictedLatch;
                hazelcast::util::CountDownLatch &expiredLatch;
            };

            TEST_F(ClientExpirationListenerTest, bothNotified_afterExpirationOfEntries) {
                int numberOfPutOperations = 1000;
                hazelcast::util::CountDownLatch expirationEventArrivalCount(numberOfPutOperations);
                hazelcast::util::CountDownLatch evictedEventArrivalCount(numberOfPutOperations);

                ExpirationAndEvictionListener expirationListener(expirationEventArrivalCount, evictedEventArrivalCount);
                std::string registrationId = imap->addEntryListener(expirationListener, true);

                for (int i = 0; i < numberOfPutOperations; i++) {
                    imap->put(i, i, 100);
                }

// wait expiration of entries.
                hazelcast::util::sleep(1);

// trigger immediate fire of expiration events by touching them.
                for (int i = 0; i < numberOfPutOperations; i++) {
                    imap->get(i);
                }

                ASSERT_TRUE(expirationEventArrivalCount.await(120));
                ASSERT_TRUE(evictedEventArrivalCount.await(120));
                ASSERT_TRUE(imap->removeEntryListener(registrationId));
            }
        }
    }
}


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
                    addNearCacheConfig<int, int>(std::shared_ptr<config::NearCacheConfig<int, int> >(
                            new config::NearCacheConfig<int, int>(intMapName)));

                    addNearCacheConfig<int, Employee>(std::shared_ptr<config::NearCacheConfig<int, Employee> >(
                            new config::NearCacheConfig<int, Employee>(employeesMapName)));

                    addNearCacheConfig<std::string, std::string>(
                            std::shared_ptr<config::NearCacheConfig<std::string, std::string> >(
                                    new config::NearCacheConfig<std::string, std::string>(imapName)));

                    addNearCacheConfig<std::string, std::string>(
                            std::shared_ptr<config::NearCacheConfig<std::string, std::string> >(
                                    new config::NearCacheConfig<std::string, std::string>(ONE_SECOND_MAP_NAME)));
                }
            };

            class NearCachedObjectMapClientConfig : public MapClientConfig {
            public:
                NearCachedObjectMapClientConfig() {
                    addNearCacheConfig<int, int>(std::shared_ptr<config::NearCacheConfig<int, int> >(
                            new config::NearCacheConfig<int, int>(intMapName, config::OBJECT)));

                    addNearCacheConfig<int, Employee>(std::shared_ptr<config::NearCacheConfig<int, Employee> >(
                            new config::NearCacheConfig<int, Employee>(employeesMapName, config::OBJECT)));

                    addNearCacheConfig<std::string, std::string>(
                            std::shared_ptr<config::NearCacheConfig<std::string, std::string> >(
                                    new config::NearCacheConfig<std::string, std::string>(imapName, config::OBJECT)));

                    addNearCacheConfig<std::string, std::string>(
                            std::shared_ptr<config::NearCacheConfig<std::string, std::string> >(
                                    new config::NearCacheConfig<std::string, std::string>(ONE_SECOND_MAP_NAME,
                                                                                          config::OBJECT)));
                }
            };

            class ClientMapTest : public ClientTestSupport, public ::testing::WithParamInterface<ClientConfig> {
            public:
                ClientMapTest() : client(HazelcastClient(GetParam())),
                                  imap(client.getMap<std::string, std::string>(MapClientConfig::imapName)),
                                  intMap(client.getMap<int, int>(MapClientConfig::intMapName)),
                                  employees(client.getMap<int, Employee>(MapClientConfig::employeesMapName)) {
                }

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

            protected:
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

                virtual void TearDown() {
                    // clear maps
                    employees.destroy();
                    intMap.destroy();
                    imap.destroy();
                    client.getMap<std::string, std::string>(MapClientConfig::ONE_SECOND_MAP_NAME).destroy();
                    client.getMap<Employee, int>("tradeMap").destroy();
                }

                void fillMap() {
                    for (int i = 0; i < 10; i++) {
                        std::string key = "key";
                        key += hazelcast::util::IOUtil::to_string(i);
                        std::string value = "value";
                        value += hazelcast::util::IOUtil::to_string(i);
                        imap.put(key, value);
                    }
                }

                static void tryPutThread(hazelcast::util::ThreadArgs &args) {
                    hazelcast::util::CountDownLatch *latch = (hazelcast::util::CountDownLatch *) args.arg0;
                    IMap<std::string, std::string> *pMap = (IMap<std::string, std::string> *) args.arg1;
                    bool result = pMap->tryPut("key1", "value3", 1 * 1000);
                    if (!result) {
                        latch->countDown();
                    }
                }

                static void tryRemoveThread(hazelcast::util::ThreadArgs &args) {
                    hazelcast::util::CountDownLatch *latch = (hazelcast::util::CountDownLatch *) args.arg0;
                    IMap<std::string, std::string> *pMap = (IMap<std::string, std::string> *) args.arg1;
                    bool result = pMap->tryRemove("key2", 1 * 1000);
                    if (!result) {
                        latch->countDown();
                    }
                }

                static void testLockThread(hazelcast::util::ThreadArgs &args) {
                    hazelcast::util::CountDownLatch *latch = (hazelcast::util::CountDownLatch *) args.arg0;
                    IMap<std::string, std::string> *pMap = (IMap<std::string, std::string> *) args.arg1;
                    pMap->tryPut("key1", "value2", 1);
                    latch->countDown();
                }

                static void testLockTTLThread(hazelcast::util::ThreadArgs &args) {
                    hazelcast::util::CountDownLatch *latch = (hazelcast::util::CountDownLatch *) args.arg0;
                    IMap<std::string, std::string> *pMap = (IMap<std::string, std::string> *) args.arg1;
                    pMap->tryPut("key1", "value2", 5 * 1000);
                    latch->countDown();
                }

                static void testLockTTL2Thread(hazelcast::util::ThreadArgs &args) {
                    hazelcast::util::CountDownLatch *latch = (hazelcast::util::CountDownLatch *) args.arg0;
                    IMap<std::string, std::string> *pMap = (IMap<std::string, std::string> *) args.arg1;
                    if (!pMap->tryLock("key1")) {
                        latch->countDown();
                    }
                    if (pMap->tryLock("key1", 5 * 1000)) {
                        latch->countDown();
                    }
                }

                static void testMapTryLockThread1(hazelcast::util::ThreadArgs &args) {
                    hazelcast::util::CountDownLatch *latch = (hazelcast::util::CountDownLatch *) args.arg0;
                    IMap<std::string, std::string> *pMap = (IMap<std::string, std::string> *) args.arg1;
                    if (!pMap->tryLock("key1", 2)) {
                        latch->countDown();
                    }
                }

                static void testMapTryLockThread2(hazelcast::util::ThreadArgs &args) {
                    hazelcast::util::CountDownLatch *latch = (hazelcast::util::CountDownLatch *) args.arg0;
                    IMap<std::string, std::string> *pMap = (IMap<std::string, std::string> *) args.arg1;
                    if (pMap->tryLock("key1", 20 * 1000)) {
                        latch->countDown();
                    }
                }

                static void testMapForceUnlockThread(hazelcast::util::ThreadArgs &args) {
                    hazelcast::util::CountDownLatch *latch = (hazelcast::util::CountDownLatch *) args.arg0;
                    IMap<std::string, std::string> *pMap = (IMap<std::string, std::string> *) args.arg1;
                    pMap->forceUnlock("key1");
                    latch->countDown();
                }

                template<typename K, typename V>
                class EvictedEntryListener : public EntryAdapter<K, V> {
                public:
                    EvictedEntryListener(const std::shared_ptr<util::CountDownLatch> &evictLatch) : evictLatch(
                            evictLatch) {}

                    virtual void entryEvicted(const EntryEvent<K, V> &event) {
                        evictLatch->countDown();
                    }

                private:
                    std::shared_ptr<hazelcast::util::CountDownLatch> evictLatch;
                };

                template<typename K, typename V>
                class CountdownListener : public EntryAdapter<K, V> {
                public:
                    CountdownListener(hazelcast::util::CountDownLatch &addLatch,
                                      hazelcast::util::CountDownLatch &removeLatch,
                                      hazelcast::util::CountDownLatch &updateLatch,
                                      hazelcast::util::CountDownLatch &evictLatch)
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
                    hazelcast::util::CountDownLatch &addLatch;
                    hazelcast::util::CountDownLatch &removeLatch;
                    hazelcast::util::CountDownLatch &updateLatch;
                    hazelcast::util::CountDownLatch &evictLatch;
                };

                class MyListener : public EntryAdapter<std::string, std::string> {
                public:
                    MyListener(hazelcast::util::CountDownLatch &latch, hazelcast::util::CountDownLatch &nullLatch)
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
                    hazelcast::util::CountDownLatch &latch;
                    hazelcast::util::CountDownLatch &nullLatch;
                };

                class ClearListener : public EntryAdapter<std::string, std::string> {
                public:
                    ClearListener(hazelcast::util::CountDownLatch &latch) : latch(latch) {
                    }

                    void mapCleared(const MapEvent &event) {
                        latch.countDown();
                    }

                private:
                    hazelcast::util::CountDownLatch &latch;
                };

                class EvictListener : public EntryAdapter<std::string, std::string> {
                public:
                    EvictListener(hazelcast::util::CountDownLatch &latch) : latch(latch) {
                    }

                    void mapEvicted(const MapEvent &event) {
                        latch.countDown();
                    }

                private:
                    hazelcast::util::CountDownLatch &latch;
                };

                class SampleEntryListenerForPortableKey : public EntryAdapter<Employee, int> {
                public:
                    SampleEntryListenerForPortableKey(hazelcast::util::CountDownLatch &latch,
                                                      hazelcast::util::AtomicInt &atomicInteger)
                            : latch(latch), atomicInteger(atomicInteger) {

                    }

                    void entryAdded(const EntryEvent<Employee, int> &event) {
                        ++atomicInteger;
                        latch.countDown();
                    }

                private:
                    hazelcast::util::CountDownLatch &latch;
                    hazelcast::util::AtomicInt &atomicInteger;
                };

                class EntryMultiplier : public serialization::IdentifiedDataSerializable {
                public:
                    EntryMultiplier(int multiplier) : multiplier(multiplier) {}

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

                HazelcastClient client;
                IMap<std::string, std::string> imap;
                IMap<int, int> intMap;
                IMap<int, Employee> employees;

                static HazelcastServer *instance;
                static HazelcastServer *instance2;
            };

            HazelcastServer *ClientMapTest::instance = NULL;
            HazelcastServer *ClientMapTest::instance2 = NULL;

            INSTANTIATE_TEST_SUITE_P(ClientMapTestWithDifferentConfigs, ClientMapTest,
                                     ::testing::Values(MapClientConfig(), NearCachedDataMapClientConfig(),
                                                       NearCachedObjectMapClientConfig()));

            TEST_P(ClientMapTest, testIssue537) {
                hazelcast::util::CountDownLatch latch(2);
                hazelcast::util::CountDownLatch nullLatch(1);
                MyListener myListener(latch, nullLatch);
                std::string id = imap.addEntryListener(myListener, true);

                imap.put("key1", "value1", 2 * 1000);

                ASSERT_TRUE(latch.await(10));
                ASSERT_TRUE(nullLatch.await(1));

                ASSERT_TRUE(imap.removeEntryListener(id));

                imap.put("key2", "value2");
                ASSERT_EQ(1, imap.size());
            }

            TEST_P(ClientMapTest, testContains) {
                fillMap();

                ASSERT_FALSE(imap.containsKey("key10"));
                ASSERT_TRUE(imap.containsKey("key1"));

                ASSERT_FALSE(imap.containsValue("value10"));
                ASSERT_TRUE(imap.containsValue("value1"));
            }

            TEST_P(ClientMapTest, testGet) {
                fillMap();
                for (int i = 0; i < 10; i++) {
                    std::string key = "key";
                    key += hazelcast::util::IOUtil::to_string(i);
                    std::shared_ptr<std::string> temp = imap.get(key);

                    std::string value = "value";
                    value += hazelcast::util::IOUtil::to_string(i);
                    ASSERT_EQ(*temp, value);
                }
            }

            TEST_P(ClientMapTest, testAsyncGet) {
                fillMap();
                auto future = imap.getAsync("key1");
                std::shared_ptr<std::string> value = future.get();
                ASSERT_NOTNULL(value.get(), std::string);
                ASSERT_EQ("value1", *value);
            }

            TEST_P(ClientMapTest, testAsyncPut) {
                fillMap();
                auto future = imap.putAsync(
                        "key3", "value");
                std::shared_ptr<std::string> value = future.get();
                ASSERT_NOTNULL(value.get(), std::string);
                ASSERT_EQ("value3", *value);
                value = imap.get("key3");
                ASSERT_NOTNULL(value.get(), std::string);
                ASSERT_EQ("value", *value);
            }

            TEST_P(ClientMapTest, testAsyncPutWithTtl) {
                fillMap();
                std::shared_ptr<hazelcast::util::CountDownLatch> evictLatch(new hazelcast::util::CountDownLatch(1));
                EvictedEntryListener <std::string, std::string> listener(
                        evictLatch);
                std::string listenerId = imap.addEntryListener(listener, true);

                auto future = imap.putAsync(
                        "key", "value1", 3, hazelcast::util::concurrent::TimeUnit::SECONDS());
                std::shared_ptr<std::string> value = future.get();
                ASSERT_NULL("no value for key should exist", value.get(), std::string);
                value = imap.get("key");
                ASSERT_NOTNULL(value.get(), std::string);
                ASSERT_EQ("value1", *value);

                ASSERT_OPEN_EVENTUALLY(*evictLatch);

                monitor::impl::NearCacheStatsImpl *nearCacheStatsImpl =
                        (monitor::impl::NearCacheStatsImpl *) imap.getLocalMapStats().getNearCacheStats();

// When ttl expires at server, the server does not send near cache invalidation, hence below is for
// non-near cache test case.
                if (!nearCacheStatsImpl) {
                    value = imap.get("key");
                    ASSERT_NULL("The value for key should have expired and not exist", value.get(), std::string);
                }

                ASSERT_TRUE(imap.removeEntryListener(listenerId));
            }

            TEST_P(ClientMapTest, testAsyncPutWithMaxIdle) {
                fillMap();
                std::shared_ptr<hazelcast::util::CountDownLatch> evictLatch(new hazelcast::util::CountDownLatch(1));
                EvictedEntryListener <std::string, std::string> listener(
                        evictLatch);
                std::string listenerId = imap.addEntryListener(listener, true);

                auto future = imap.putAsync(
                        "key", "value1", 0, hazelcast::util::concurrent::TimeUnit::SECONDS(), 3,
                        hazelcast::util::concurrent::TimeUnit::SECONDS());
                std::shared_ptr<std::string> value = future.get();
                ASSERT_NULL("no value for key should exist", value.get(), std::string);
                value = imap.get("key");
                ASSERT_NOTNULL(value.get(), std::string);
                ASSERT_EQ("value1", *value);

                ASSERT_OPEN_EVENTUALLY(*evictLatch);

                monitor::impl::NearCacheStatsImpl *nearCacheStatsImpl =
                        (monitor::impl::NearCacheStatsImpl *) imap.getLocalMapStats().getNearCacheStats();

// When ttl expires at server, the server does not send near cache invalidation, hence below is for
// non-near cache test case.
                if (!nearCacheStatsImpl) {
                    value = imap.get("key");
                    ASSERT_NULL("The value for key should have expired and not exist", value.get(), std::string);
                }

                ASSERT_TRUE(imap.removeEntryListener(listenerId));
            }

            TEST_P(ClientMapTest, testAsyncSet) {
                fillMap();
                auto future = imap.setAsync("key3", "value");
                future.get();
                std::shared_ptr<std::string> value = imap.get("key3");
                ASSERT_NOTNULL(value.get(), std::string);
                ASSERT_EQ("value", *value);
            }

            TEST_P(ClientMapTest, testAsyncSetWithTtl) {
                fillMap();
                std::shared_ptr<hazelcast::util::CountDownLatch> evictLatch(new hazelcast::util::CountDownLatch(1));
                EvictedEntryListener <std::string, std::string> listener(
                        evictLatch);
                std::string listenerId = imap.addEntryListener(listener, true);

                auto future = imap.setAsync("key", "value1", 3, hazelcast::util::concurrent::TimeUnit::SECONDS());
                future.get();
                std::shared_ptr<std::string> value = imap.get("key");
                ASSERT_NOTNULL(value.get(), std::string);
                ASSERT_EQ("value1", *value);

                ASSERT_OPEN_EVENTUALLY(*evictLatch);

                monitor::impl::NearCacheStatsImpl *nearCacheStatsImpl =
                        (monitor::impl::NearCacheStatsImpl *) imap.getLocalMapStats().getNearCacheStats();

// When ttl expires at server, the server does not send near cache invalidation, hence below is for
// non-near cache test case.
                if (!nearCacheStatsImpl) {
                    value = imap.get("key");
                    ASSERT_NULL("The value for key should have expired and not exist", value.get(), std::string);
                }

                ASSERT_TRUE(imap.removeEntryListener(listenerId));
            }

            TEST_P(ClientMapTest, testAsyncSetWithMaxIdle) {
                fillMap();
                std::shared_ptr<hazelcast::util::CountDownLatch> evictLatch(new hazelcast::util::CountDownLatch(1));
                EvictedEntryListener <std::string, std::string> listener(
                        evictLatch);
                std::string listenerId = imap.addEntryListener(listener, true);

                auto future = imap.setAsync("key",
                                            "value1",
                                            0,
                                            hazelcast::util::concurrent::TimeUnit::SECONDS(),
                                            3,
                                            hazelcast::util::concurrent::TimeUnit::SECONDS());
                future.get();
                std::shared_ptr<std::string> value = imap.get("key");
                ASSERT_NOTNULL(value.get(), std::string);
                ASSERT_EQ("value1", *value);

                ASSERT_OPEN_EVENTUALLY(*evictLatch);

                monitor::impl::NearCacheStatsImpl *nearCacheStatsImpl =
                        (monitor::impl::NearCacheStatsImpl *) imap.getLocalMapStats().getNearCacheStats();

// When ttl expires at server, the server does not send near cache invalidation, hence below is for
// non-near cache test case.
                if (!nearCacheStatsImpl) {
                    value = imap.get("key");
                    ASSERT_NULL("The value for key should have expired and not exist", value.get(), std::string);
                }

                ASSERT_TRUE(imap.removeEntryListener(listenerId));
            }

            TEST_P(ClientMapTest, testAsyncRemove) {
                fillMap();
                auto future = imap.removeAsync("key4");
                std::shared_ptr<std::string> value = future.get();
                ASSERT_NOTNULL(value.get(), std::string);
                ASSERT_EQ("value4", *value);
            }

            TEST_P(ClientMapTest, testPartitionAwareKey) {
                int partitionKey = 5;
                int value = 25;
                IMap<PartitionAwareInt, int> map =
                        client.getMap<PartitionAwareInt, int>(MapClientConfig::intMapName);
                PartitionAwareInt partitionAwareInt(partitionKey, 7);
                map.put(partitionAwareInt, value);
                std::shared_ptr<int> val = map.get(partitionAwareInt);
                ASSERT_NOTNULL(val.get(), int);
                ASSERT_EQ(*val, value);
            }

            TEST_P(ClientMapTest, testRemoveAndDelete) {
                fillMap();
                std::shared_ptr<std::string> temp = imap.remove("key10");
                ASSERT_EQ(temp.get(), (std::string *) NULL);
                imap.deleteEntry("key9");
                ASSERT_EQ(imap.size(), 9);
                for (int i = 0; i < 9; i++) {
                    std::string key = "key";
                    key += hazelcast::util::IOUtil::to_string(i);
                    std::shared_ptr<std::string> temp2 = imap.remove(key);
                    std::string value = "value";
                    value += hazelcast::util::IOUtil::to_string(i);
                    ASSERT_EQ(*temp2, value);
                }
                ASSERT_EQ(imap.size(), 0);
            }

            TEST_P(ClientMapTest, testRemoveIfSame) {
                fillMap();

                ASSERT_FALSE(imap.remove("key2", "value"));
                ASSERT_EQ(10, imap.size());

                ASSERT_TRUE((imap.remove("key2", "value2")));
                ASSERT_EQ(9, imap.size());

            }

            TEST_P(ClientMapTest, testRemoveAll) {
                fillMap();

                imap.removeAll(
                        query::EqualPredicate<std::string>(query::QueryConstants::getKeyAttributeName(), "key5"));

                std::shared_ptr<std::string> value = imap.get("key5");

                ASSERT_NULL("key5 should not exist", value.get(), std::string);

                imap.removeAll(
                        query::LikePredicate(query::QueryConstants::getValueAttributeName(), "value%"));

                ASSERT_TRUE(imap.isEmpty());
            }

            TEST_P(ClientMapTest, testGetAllPutAll) {

                std::map<std::string, std::string> mapTemp;

                for (int i = 0; i < 100; i++) {
                    mapTemp[hazelcast::util::IOUtil::to_string(i)] = hazelcast::util::IOUtil::to_string(i);
                }
                ASSERT_EQ(imap.size(), 0);
                imap.putAll(mapTemp);
                ASSERT_EQ(imap.size(), 100);

                for (int i = 0; i < 100; i++) {
                    std::string expected = hazelcast::util::IOUtil::to_string(i);
                    std::shared_ptr<std::string> actual = imap.get(
                            hazelcast::util::IOUtil::to_string(i));
                    ASSERT_EQ(expected, *actual);
                }

                std::set<std::string> tempSet;
                tempSet.insert(hazelcast::util::IOUtil::to_string(1));
                tempSet.insert(hazelcast::util::IOUtil::to_string(3));

                std::map<std::string, std::string> m2 = imap.getAll(tempSet);

                ASSERT_EQ(2U, m2.size());
                ASSERT_EQ(m2[hazelcast::util::IOUtil::to_string(1)], "1");
                ASSERT_EQ(m2[hazelcast::util::IOUtil::to_string(3)], "3");

            }

            TEST_P(ClientMapTest, testTryPutRemove) {
                ASSERT_TRUE(imap.tryPut("key1", "value1", 1 * 1000));
                ASSERT_TRUE(imap.tryPut("key2", "value2", 1 * 1000));
                imap.lock("key1");
                imap.lock("key2");

                hazelcast::util::CountDownLatch latch(2);

                hazelcast::util::StartedThread t1(tryPutThread, &latch, &imap);
                hazelcast::util::StartedThread t2(tryRemoveThread, &latch, &imap);

                ASSERT_TRUE(latch.await(20));
                ASSERT_EQ("value1", *(imap.get("key1")));
                ASSERT_EQ("value2", *(imap.get("key2")));
                imap.forceUnlock("key1");
                imap.forceUnlock("key2");
            }

            TEST_P(ClientMapTest, testPutTtl) {
                hazelcast::util::CountDownLatch dummy(10);
                hazelcast::util::CountDownLatch evict(1);
                CountdownListener <std::string, std::string> sampleEntryListener(
                        dummy, dummy, dummy, evict);
                std::string id = imap.addEntryListener(sampleEntryListener, false);


                auto nearCacheStatsImpl = (monitor::impl::NearCacheStatsImpl *) imap.getLocalMapStats().getNearCacheStats();

                int64_t initialInvalidationRequests = 0;
                if (nearCacheStatsImpl) {
                    initialInvalidationRequests = nearCacheStatsImpl->getInvalidationRequests();
                }

// put will cause an invalidation event sent from the server to the client
                imap.put("key1", "value1", 1000);

// if near cache is enabled
                if (nearCacheStatsImpl) {
                    ASSERT_EQ_EVENTUALLY(initialInvalidationRequests + 1,
                                         nearCacheStatsImpl->getInvalidationRequests());

// populate near cache
                    imap.get("key1").get();

// When ttl expires at server, the server does not send near cache invalidation.
                    ASSERT_TRUE_ALL_THE_TIME((imap.get("key1").get() && nearCacheStatsImpl->getInvalidationRequests() ==
                                                                        initialInvalidationRequests + 1), 2);
                } else {
// trigger eviction
                    ASSERT_NULL_EVENTUALLY(imap.get("key1").get(), std::string);
                }

                ASSERT_TRUE(evict.await(5));

                ASSERT_TRUE(imap.removeEntryListener(id));
            }

            TEST_P(ClientMapTest, testPutConfigTtl) {
                IMap<std::string, std::string> map = client.getMap<std::string, std::string>(
                        MapClientConfig::ONE_SECOND_MAP_NAME);
                hazelcast::util::CountDownLatch dummy(10);
                hazelcast::util::CountDownLatch evict(1);
                CountdownListener <std::string, std::string> sampleEntryListener(
                        dummy, dummy, dummy, evict);
                std::string id = map.addEntryListener(sampleEntryListener, false);

                auto nearCacheStatsImpl = (monitor::impl::NearCacheStatsImpl *) map.getLocalMapStats().getNearCacheStats();

                int64_t initialInvalidationRequests = 0;
                if (nearCacheStatsImpl) {
                    initialInvalidationRequests = nearCacheStatsImpl->getInvalidationRequests();
                }

// put will cause an invalidation event sent from the server to the client
                map.put("key1", "value1");

// if near cache is enabled
                if (nearCacheStatsImpl) {
                    ASSERT_EQ_EVENTUALLY(initialInvalidationRequests + 1,
                                         nearCacheStatsImpl->getInvalidationRequests());

// populate near cache
                    imap.get("key1").get();

// When ttl expires at server, the server does not send near cache invalidation.
                    ASSERT_TRUE_ALL_THE_TIME((map.get("key1").get() && nearCacheStatsImpl->getInvalidationRequests() ==
                                                                       initialInvalidationRequests + 1), 2);
                } else {
// trigger eviction
                    ASSERT_NULL_EVENTUALLY(map.get("key1").get(), std::string);
                }

                ASSERT_TRUE(evict.await(5));

                ASSERT_TRUE(map.removeEntryListener(id));
            }

            TEST_P(ClientMapTest, testPutIfAbsent) {
                std::shared_ptr<std::string> o = imap.putIfAbsent("key1", "value1");
                ASSERT_EQ(o.get(), (std::string *) NULL);
                ASSERT_EQ("value1", *(imap.putIfAbsent("key1", "value3")));
            }

            TEST_P(ClientMapTest, testPutIfAbsentTtl) {
                ASSERT_EQ(imap.putIfAbsent("key1", "value1", 1000).get(),
                          (std::string *) NULL);
                ASSERT_EQ("value1", *(imap.putIfAbsent("key1", "value3", 1000)));

                ASSERT_NULL_EVENTUALLY(imap.putIfAbsent("key1", "value3", 1000).get(), std::string);
                ASSERT_EQ("value3", *(imap.putIfAbsent("key1", "value4", 1000)));
            }

            TEST_P(ClientMapTest, testSet) {
                imap.set("key1", "value1");
                ASSERT_EQ("value1", *(imap.get("key1")));

                imap.set("key1", "value2");
                ASSERT_EQ("value2", *(imap.get("key1")));
            }

            TEST_P(ClientMapTest, testSetTtl) {
                hazelcast::util::CountDownLatch dummy(10);
                hazelcast::util::CountDownLatch evict(1);
                CountdownListener <std::string, std::string> sampleEntryListener(
                        dummy, dummy, dummy, evict);
                std::string id = imap.addEntryListener(sampleEntryListener, false);

                monitor::impl::NearCacheStatsImpl *nearCacheStatsImpl = (monitor::impl::NearCacheStatsImpl *) imap.getLocalMapStats().getNearCacheStats();

                int64_t initialInvalidationRequests = 0;
                if (nearCacheStatsImpl) {
                    initialInvalidationRequests = nearCacheStatsImpl->getInvalidationRequests();
                }

// set will cause an invalidation event sent from the server to the client
                imap.set("key1", "value1", 1000);

// if near cache is enabled
                if (nearCacheStatsImpl) {
                    ASSERT_EQ_EVENTUALLY(initialInvalidationRequests + 1,
                                         nearCacheStatsImpl->getInvalidationRequests());

// populate near cache
                    imap.get("key1").get();

// When ttl expires at server, the server does not send near cache invalidation.
                    ASSERT_TRUE_ALL_THE_TIME((imap.get("key1").get() && nearCacheStatsImpl->getInvalidationRequests() ==
                                                                        initialInvalidationRequests + 1), 2);
                } else {
// trigger eviction
                    ASSERT_NULL_EVENTUALLY(imap.get("key1").get(), std::string);
                }

                ASSERT_TRUE(evict.await(5));

                ASSERT_TRUE(imap.removeEntryListener(id));
            }

            TEST_P(ClientMapTest, testSetConfigTtl) {
                IMap<std::string, std::string> map = client.getMap<std::string, std::string>(
                        MapClientConfig::ONE_SECOND_MAP_NAME);
                hazelcast::util::CountDownLatch dummy(10);
                hazelcast::util::CountDownLatch evict(1);
                CountdownListener <std::string, std::string> sampleEntryListener(
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
                    ASSERT_EQ_EVENTUALLY(initialInvalidationRequests + 1,
                                         nearCacheStatsImpl->getInvalidationRequests());

// populate near cache
                    imap.get("key1").get();

// When ttl expires at server, the server does not send near cache invalidation.
                    ASSERT_TRUE_ALL_THE_TIME((map.get("key1").get() && nearCacheStatsImpl->getInvalidationRequests() ==
                                                                       initialInvalidationRequests + 1), 2);
                } else {
// trigger eviction
                    ASSERT_NULL_EVENTUALLY(map.get("key1").get(), std::string);
                }

                ASSERT_TRUE(evict.await(5));

                ASSERT_TRUE(map.removeEntryListener(id));
            }

            TEST_P(ClientMapTest, testLock) {
                imap.put("key1", "value1");
                ASSERT_EQ("value1", *(imap.get("key1")));
                imap.lock("key1");
                hazelcast::util::CountDownLatch latch(1);
                hazelcast::util::StartedThread t1(testLockThread, &latch, &imap);
                ASSERT_TRUE(latch.await(5));
                ASSERT_EQ("value1", *(imap.get("key1")));
                imap.forceUnlock("key1");
            }

            TEST_P(ClientMapTest, testLockTtl) {
                imap.put("key1", "value1");
                ASSERT_EQ("value1", *(imap.get("key1")));
                imap.lock("key1", 2 * 1000);
                hazelcast::util::CountDownLatch latch(1);
                hazelcast::util::StartedThread t1(testLockTTLThread, &latch, &imap);
                ASSERT_TRUE(latch.await(10));
                ASSERT_FALSE(imap.isLocked("key1"));
                ASSERT_EQ("value2", *(imap.get("key1")));
                imap.forceUnlock("key1");
            }

            TEST_P(ClientMapTest, testLockTtl2) {
                imap.lock("key1", 3 * 1000);
                hazelcast::util::CountDownLatch latch(2);
                hazelcast::util::StartedThread t1(testLockTTL2Thread, &latch, &imap);
                ASSERT_TRUE(latch.await(10));
                imap.forceUnlock("key1");
            }

            TEST_P(ClientMapTest, testTryLock) {
                ASSERT_TRUE(imap.tryLock("key1", 2 * 1000));
                hazelcast::util::CountDownLatch latch(1);
                hazelcast::util::StartedThread t1(testMapTryLockThread1, &latch, &imap);

                ASSERT_TRUE(latch.await(100));

                ASSERT_TRUE(imap.isLocked("key1"));

                hazelcast::util::CountDownLatch latch2(1);
                hazelcast::util::StartedThread t2(testMapTryLockThread2, &latch2, &imap);

                hazelcast::util::sleep(1);
                imap.unlock("key1");
                ASSERT_TRUE(latch2.await(100));
                ASSERT_TRUE(imap.isLocked("key1"));
                imap.forceUnlock("key1");
            }

            TEST_P(ClientMapTest, testForceUnlock) {
                imap.lock("key1");
                hazelcast::util::CountDownLatch latch(1);
                hazelcast::util::StartedThread t2(testMapForceUnlockThread, &latch, &imap);
                ASSERT_TRUE(latch.await(100));
                t2.join();
                ASSERT_FALSE(imap.isLocked("key1"));

            }

            TEST_P(ClientMapTest, testValues) {
                fillMap();
                std::vector<std::string> tempVector;
                query::SqlPredicate predicate("this == value1");
                tempVector = imap.values(predicate);
                ASSERT_EQ(1U, tempVector.size());

                std::vector<std::string>::iterator it = tempVector.begin();
                ASSERT_EQ("value1", *it);
            }

            TEST_P(ClientMapTest, testValuesWithPredicate) {
                const int numItems = 20;
                for (int i = 0; i < numItems; ++i) {
                    intMap.put(i, 2 * i);
                }

                std::vector<int> values = intMap.values();
                ASSERT_EQ(numItems, (int) values.size());
                std::sort(values.begin(), values.end());
                for (int i = 0; i < numItems; ++i) {
                    ASSERT_EQ(2 * i, values[i]);
                }

// EqualPredicate
// key == 5
                values = intMap.values(
                        query::EqualPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5));
                ASSERT_EQ(1, (int) values.size());
                ASSERT_EQ(2 * 5, values[0]);

// value == 8
                values = intMap.values(
                        query::EqualPredicate<int>(query::QueryConstants::getValueAttributeName(), 8));
                ASSERT_EQ(1, (int) values.size());
                ASSERT_EQ(8, values[0]);

// key == numItems
                values = intMap.values(
                        query::EqualPredicate<int>(query::QueryConstants::getKeyAttributeName(), numItems));
                ASSERT_EQ(0, (int) values.size());

// NotEqual Predicate
// key != 5
                values = intMap.values(
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
                values = intMap.values(
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
                values = intMap.values(query::TruePredicate());
                ASSERT_EQ(numItems, (int) values.size());
                std::sort(values.begin(), values.end());
                for (int i = 0; i < numItems; ++i) {
                    ASSERT_EQ(2 * i, values[i]);
                }

// FalsePredicate
                values = intMap.values(query::FalsePredicate());
                ASSERT_EQ(0, (int) values.size());

// BetweenPredicate
// 5 <= key <= 10
                values = intMap.values(
                        query::BetweenPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5, 10));
                std::sort(values.begin(), values.end());
                ASSERT_EQ(6, (int) values.size());
                for (int i = 0; i < 6; ++i) {
                    ASSERT_EQ(2 * (i + 5), values[i]);
                }

// 20 <= key <=30
                values = intMap.values(
                        query::BetweenPredicate<int>(query::QueryConstants::getKeyAttributeName(), 20, 30));
                ASSERT_EQ(0, (int) values.size());

// GreaterLessPredicate
// value <= 10
                values = intMap.values(
                        query::GreaterLessPredicate<int>(query::QueryConstants::getValueAttributeName(), 10, true,
                                                         true));
                ASSERT_EQ(6, (int) values.size());
                std::sort(values.begin(), values.end());
                for (int i = 0; i < 6; ++i) {
                    ASSERT_EQ(2 * i, values[i]);
                }

// key < 7
                values = intMap.values(
                        query::GreaterLessPredicate<int>(query::QueryConstants::getKeyAttributeName(), 7, false, true));
                ASSERT_EQ(7, (int) values.size());
                std::sort(values.begin(), values.end());
                for (int i = 0; i < 7; ++i) {
                    ASSERT_EQ(2 * i, values[i]);
                }

// value >= 15
                values = intMap.values(
                        query::GreaterLessPredicate<int>(query::QueryConstants::getValueAttributeName(), 15, true,
                                                         false));
                ASSERT_EQ(12, (int) values.size());
                std::sort(values.begin(), values.end());
                for (int i = 0; i < 12; ++i) {
                    ASSERT_EQ(2 * (i + 8), values[i]);
                }

// key > 5
                values = intMap.values(
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
                values = intMap.values(
                        query::InPredicate<int>(query::QueryConstants::getKeyAttributeName(), inVals));
                ASSERT_EQ(3, (int) values.size());
                std::sort(values.begin(), values.end());
                ASSERT_EQ(2 * 4, values[0]);
                ASSERT_EQ(2 * 10, values[1]);
                ASSERT_EQ(2 * 19, values[2]);

// value in {4, 10, 19}
                values = intMap.values(
                        query::InPredicate<int>(query::QueryConstants::getValueAttributeName(), inVals));
                ASSERT_EQ(2, (int) values.size());
                std::sort(values.begin(), values.end());
                ASSERT_EQ(4, values[0]);
                ASSERT_EQ(10, values[1]);

// InstanceOfPredicate
// value instanceof Integer
                values = intMap.values(query::InstanceOfPredicate("java.lang.Integer"));
                ASSERT_EQ(20, (int) values.size());
                std::sort(values.begin(), values.end());
                for (int i = 0; i < numItems; ++i) {
                    ASSERT_EQ(2 * i, values[i]);
                }

                values = intMap.values(query::InstanceOfPredicate("java.lang.String"));
                ASSERT_EQ(0, (int) values.size());

// NotPredicate
// !(5 <= key <= 10)
                std::unique_ptr<query::Predicate> bp = std::unique_ptr<query::Predicate>(
                        new query::BetweenPredicate<int>(
                                query::QueryConstants::getKeyAttributeName(), 5, 10));
                query::NotPredicate notPredicate(bp);
                values = intMap.values(notPredicate);
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
                bp = std::unique_ptr<query::Predicate>(
                        new query::BetweenPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5, 10));
                std::unique_ptr<query::Predicate> inPred = std::unique_ptr<query::Predicate>(
                        new query::InPredicate<int>(query::QueryConstants::getValueAttributeName(), inVals));
                values = intMap.values(query::AndPredicate().add(bp).add(inPred));
                ASSERT_EQ(1, (int) values.size());
                std::sort(values.begin(), values.end());
                ASSERT_EQ(10, values[0]);

// OrPredicate
// 5 <= key <= 10 OR Values in {4, 10, 19} = values {4, 10, 12, 14, 16, 18, 20}
                bp = std::unique_ptr<query::Predicate>(
                        new query::BetweenPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5, 10));
                inPred = std::unique_ptr<query::Predicate>(
                        new query::InPredicate<int>(query::QueryConstants::getValueAttributeName(), inVals));
                values = intMap.values(query::OrPredicate().add(bp).add(inPred));
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
                    key += hazelcast::util::IOUtil::to_string(i);
                    std::string value = "value";
                    value += hazelcast::util::IOUtil::to_string(i);
                    imap.put(key, value);
                }
                imap.put("key_111_test", "myvalue_111_test");
                imap.put("key_22_test", "myvalue_22_test");

// LikePredicate
// value LIKE "value1" : {"value1"}
                std::vector<std::string> strValues = imap.values(
                        query::LikePredicate(query::QueryConstants::getValueAttributeName(), "value1"));
                ASSERT_EQ(1, (int) strValues.size());
                ASSERT_EQ("value1", strValues[0]);

// ILikePredicate
// value ILIKE "%VALue%1%" : {"myvalue_111_test", "value1", "value10", "value11"}
                strValues = imap.values(
                        query::ILikePredicate(query::QueryConstants::getValueAttributeName(), "%VALue%1%"));
                ASSERT_EQ(4, (int) strValues.size());
                std::sort(strValues.begin(), strValues.end());
                ASSERT_EQ("myvalue_111_test", strValues[0]);
                ASSERT_EQ("value1", strValues[1]);
                ASSERT_EQ("value10", strValues[2]);
                ASSERT_EQ("value11", strValues[3]);

// value ILIKE "%VAL%2%" : {"myvalue_22_test", "value2"}
                strValues = imap.values(
                        query::ILikePredicate(query::QueryConstants::getValueAttributeName(), "%VAL%2%"));
                ASSERT_EQ(2, (int) strValues.size());
                std::sort(strValues.begin(), strValues.end());
                ASSERT_EQ("myvalue_22_test", strValues[0]);
                ASSERT_EQ("value2", strValues[1]);

// SqlPredicate
// __key BETWEEN 4 and 7 : {4, 5, 6, 7} -> {8, 10, 12, 14}
                char sql[100];
                hazelcast::util::hz_snprintf(sql, 50, "%s BETWEEN 4 and 7",
                                             query::QueryConstants::getKeyAttributeName());
                values = intMap.values(query::SqlPredicate(sql));
                ASSERT_EQ(4, (int) values.size());
                std::sort(values.begin(), values.end());
                for (int i = 0; i < 4; ++i) {
                    ASSERT_EQ(2 * (i + 4), values[i]);
                }

// RegexPredicate
// value matches the regex ".*value.*2.*" : {myvalue_22_test, value2}
                strValues = imap.values(
                        query::RegexPredicate(query::QueryConstants::getValueAttributeName(), ".*value.*2.*"));
                ASSERT_EQ(2, (int) strValues.size());
                std::sort(strValues.begin(), strValues.end());
                ASSERT_EQ("myvalue_22_test", strValues[0]);
                ASSERT_EQ("value2", strValues[1]);
            }

            TEST_P(ClientMapTest, testValuesWithPagingPredicate) {
                int predSize = 5;
                const int totalEntries = 25;

                for (int i = 0; i < totalEntries; ++i) {
                    intMap.put(i, i);
                }

                query::PagingPredicate<int, int> predicate((size_t) predSize);

                std::vector<int> values = intMap.values(predicate);
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    ASSERT_EQ(i, values[i]);
                }

                values = intMap.values(predicate);
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    ASSERT_EQ(i, values[i]);
                }

                predicate.nextPage();
                values = intMap.values(predicate);
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

                values = intMap.values(predicate);
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
                values = intMap.values(predicate);
                ASSERT_EQ(0, (int) values.size());

                predicate.setPage(0);
                values = intMap.values(predicate);
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    ASSERT_EQ(i, values[i]);
                }

                predicate.previousPage();
                ASSERT_EQ(0, (int) predicate.getPage());

                predicate.setPage(5);
                values = intMap.values(predicate);
                ASSERT_EQ(0, (int) values.size());

                predicate.setPage(3);
                values = intMap.values(predicate);
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    ASSERT_EQ(3 * predSize + i, values[i]);
                }

                predicate.previousPage();
                values = intMap.values(predicate);
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    ASSERT_EQ(2 * predSize + i, values[i]);
                }

// test PagingPredicate with inner predicate (value < 10)
                std::unique_ptr<query::Predicate> lessThanTenPredicate(std::unique_ptr<query::Predicate>(
                        new query::GreaterLessPredicate<int>(query::QueryConstants::getValueAttributeName(), 9, false,
                                                             true)));
                query::PagingPredicate<int, int> predicate2(lessThanTenPredicate, 5);
                values = intMap.values(predicate2);
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    ASSERT_EQ(i, values[i]);
                }

                predicate2.nextPage();
// match values 5,6, 7, 8
                values = intMap.values(predicate2);
                ASSERT_EQ(predSize - 1, (int) values.size());
                for (int i = 0; i < predSize - 1; ++i) {
                    ASSERT_EQ(predSize + i, values[i]);
                }

                predicate2.nextPage();
                values = intMap.values(predicate2);
                ASSERT_EQ(0, (int) values.size());

// test paging predicate with comparator
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
                        std::unique_ptr<query::EntryComparator<int, Employee> >(new EmployeeEntryComparator()),
                        (size_t) predSize);
                std::vector<Employee> result = employees.values(predicate3);
                ASSERT_EQ(2, (int) result.size());
                ASSERT_EQ(empl6, result[0]);
                ASSERT_EQ(empl2, result[1]);

                predicate3.nextPage();
                result = employees.values(predicate3);
                ASSERT_EQ(2, (int) result.size());
                ASSERT_EQ(empl3, result[0]);
                ASSERT_EQ(empl4, result[1]);
            }

            TEST_P(ClientMapTest, testKeySetWithPredicate) {
                const int numItems = 20;
                for (int i = 0; i < numItems; ++i) {
                    intMap.put(i, 2 * i);
                }

                std::vector<int> keys = intMap.keySet();
                ASSERT_EQ(numItems, (int) keys.size());
                std::sort(keys.begin(), keys.end());
                for (int i = 0; i < numItems; ++i) {
                    ASSERT_EQ(i, keys[i]);
                }

// EqualPredicate
// key == 5
                keys = intMap.keySet(
                        query::EqualPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5));
                ASSERT_EQ(1, (int) keys.size());
                ASSERT_EQ(5, keys[0]);

// value == 8
                keys = intMap.keySet(
                        query::EqualPredicate<int>(query::QueryConstants::getValueAttributeName(), 8));
                ASSERT_EQ(1, (int) keys.size());
                ASSERT_EQ(4, keys[0]);

// key == numItems
                keys = intMap.keySet(
                        query::EqualPredicate<int>(query::QueryConstants::getKeyAttributeName(), numItems));
                ASSERT_EQ(0, (int) keys.size());

// NotEqual Predicate
// key != 5
                keys = intMap.keySet(
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
                keys = intMap.keySet(
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
                keys = intMap.keySet(query::TruePredicate());
                ASSERT_EQ(numItems, (int) keys.size());
                std::sort(keys.begin(), keys.end());
                for (int i = 0; i < numItems; ++i) {
                    ASSERT_EQ(i, keys[i]);
                }

// FalsePredicate
                keys = intMap.keySet(query::FalsePredicate());
                ASSERT_EQ(0, (int) keys.size());

// BetweenPredicate
// 5 <= key <= 10
                keys = intMap.keySet(
                        query::BetweenPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5, 10));
                std::sort(keys.begin(), keys.end());
                ASSERT_EQ(6, (int) keys.size());
                for (int i = 0; i < 6; ++i) {
                    ASSERT_EQ((i + 5), keys[i]);
                }

// 20 <= key <=30
                keys = intMap.keySet(
                        query::BetweenPredicate<int>(query::QueryConstants::getKeyAttributeName(), 20, 30));
                ASSERT_EQ(0, (int) keys.size());

// GreaterLessPredicate
// value <= 10
                keys = intMap.keySet(
                        query::GreaterLessPredicate<int>(query::QueryConstants::getValueAttributeName(), 10, true,
                                                         true));
                ASSERT_EQ(6, (int) keys.size());
                std::sort(keys.begin(), keys.end());
                for (int i = 0; i < 6; ++i) {
                    ASSERT_EQ(i, keys[i]);
                }

// key < 7
                keys = intMap.keySet(
                        query::GreaterLessPredicate<int>(query::QueryConstants::getKeyAttributeName(), 7, false, true));
                ASSERT_EQ(7, (int) keys.size());
                std::sort(keys.begin(), keys.end());
                for (int i = 0; i < 7; ++i) {
                    ASSERT_EQ(i, keys[i]);
                }

// value >= 15
                keys = intMap.keySet(
                        query::GreaterLessPredicate<int>(query::QueryConstants::getValueAttributeName(), 15, true,
                                                         false));
                ASSERT_EQ(12, (int) keys.size());
                std::sort(keys.begin(), keys.end());
                for (int i = 0; i < 12; ++i) {
                    ASSERT_EQ(i + 8, keys[i]);
                }

// key > 5
                keys = intMap.keySet(
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
                keys = intMap.keySet(
                        query::InPredicate<int>(query::QueryConstants::getKeyAttributeName(), inVals));
                ASSERT_EQ(3, (int) keys.size());
                std::sort(keys.begin(), keys.end());
                ASSERT_EQ(4, keys[0]);
                ASSERT_EQ(10, keys[1]);
                ASSERT_EQ(19, keys[2]);

// value in {4, 10, 19}
                keys = intMap.keySet(
                        query::InPredicate<int>(query::QueryConstants::getValueAttributeName(), inVals));
                ASSERT_EQ(2, (int) keys.size());
                std::sort(keys.begin(), keys.end());
                ASSERT_EQ(2, keys[0]);
                ASSERT_EQ(5, keys[1]);

// InstanceOfPredicate
// value instanceof Integer
                keys = intMap.keySet(query::InstanceOfPredicate("java.lang.Integer"));
                ASSERT_EQ(20, (int) keys.size());
                std::sort(keys.begin(), keys.end());
                for (int i = 0; i < numItems; ++i) {
                    ASSERT_EQ(i, keys[i]);
                }

                keys = intMap.keySet(query::InstanceOfPredicate("java.lang.String"));
                ASSERT_EQ(0, (int) keys.size());

// NotPredicate
// !(5 <= key <= 10)
                std::unique_ptr<query::Predicate> bp = std::unique_ptr<query::Predicate>(
                        new query::BetweenPredicate<int>(
                                query::QueryConstants::getKeyAttributeName(), 5, 10));
                query::NotPredicate notPredicate(bp);
                keys = intMap.keySet(notPredicate);
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
                bp = std::unique_ptr<query::Predicate>(
                        new query::BetweenPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5, 10));
                std::unique_ptr<query::Predicate> inPred = std::unique_ptr<query::Predicate>(
                        new query::InPredicate<int>(query::QueryConstants::getValueAttributeName(), inVals));
                keys = intMap.keySet(query::AndPredicate().add(bp).add(inPred));
                ASSERT_EQ(1, (int) keys.size());
                std::sort(keys.begin(), keys.end());
                ASSERT_EQ(5, keys[0]);

// OrPredicate
// 5 <= key <= 10 OR Values in {4, 10, 19} = keys {2, 5, 6, 7, 8, 9, 10}
                bp = std::unique_ptr<query::Predicate>(
                        new query::BetweenPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5, 10));
                inPred = std::unique_ptr<query::Predicate>(
                        new query::InPredicate<int>(query::QueryConstants::getValueAttributeName(), inVals));
                keys = intMap.keySet(query::OrPredicate().add(bp).add(inPred));
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
                    key += hazelcast::util::IOUtil::to_string(i);
                    std::string value = "value";
                    value += hazelcast::util::IOUtil::to_string(i);
                    imap.put(key, value);
                }
                imap.put("key_111_test", "myvalue_111_test");
                imap.put("key_22_test", "myvalue_22_test");

// LikePredicate
// value LIKE "value1" : {"value1"}
                std::vector<std::string> strKeys = imap.keySet(
                        query::LikePredicate(query::QueryConstants::getValueAttributeName(), "value1"));
                ASSERT_EQ(1, (int) strKeys.size());
                ASSERT_EQ("key1", strKeys[0]);

// ILikePredicate
// value ILIKE "%VALue%1%" : {"key_111_test", "key1", "key10", "key11"}
                strKeys = imap.keySet(
                        query::ILikePredicate(query::QueryConstants::getValueAttributeName(), "%VALue%1%"));
                ASSERT_EQ(4, (int) strKeys.size());
                std::sort(strKeys.begin(), strKeys.end());
                ASSERT_EQ("key1", strKeys[0]);
                ASSERT_EQ("key10", strKeys[1]);
                ASSERT_EQ("key11", strKeys[2]);
                ASSERT_EQ("key_111_test", strKeys[3]);

// key ILIKE "%VAL%2%" : {"key_22_test", "key2"}
                strKeys = imap.keySet(
                        query::ILikePredicate(query::QueryConstants::getValueAttributeName(), "%VAL%2%"));
                ASSERT_EQ(2, (int) strKeys.size());
                std::sort(strKeys.begin(), strKeys.end());
                ASSERT_EQ("key2", strKeys[0]);
                ASSERT_EQ("key_22_test", strKeys[1]);

// SqlPredicate
// __key BETWEEN 4 and 7 : {4, 5, 6, 7} -> {8, 10, 12, 14}
                char sql[100];
                hazelcast::util::hz_snprintf(sql, 50, "%s BETWEEN 4 and 7",
                                             query::QueryConstants::getKeyAttributeName());
                keys = intMap.keySet(query::SqlPredicate(sql));
                ASSERT_EQ(4, (int) keys.size());
                std::sort(keys.begin(), keys.end());
                for (int i = 0; i < 4; ++i) {
                    ASSERT_EQ(i + 4, keys[i]);
                }

// RegexPredicate
// value matches the regex ".*value.*2.*" : {key_22_test, value2}
                strKeys = imap.keySet(
                        query::RegexPredicate(query::QueryConstants::getValueAttributeName(), ".*value.*2.*"));
                ASSERT_EQ(2, (int) strKeys.size());
                std::sort(strKeys.begin(), strKeys.end());
                ASSERT_EQ("key2", strKeys[0]);
                ASSERT_EQ("key_22_test", strKeys[1]);
            }

            TEST_P(ClientMapTest, testKeySetWithPagingPredicate) {
                int predSize = 5;
                const int totalEntries = 25;

                for (int i = 0; i < totalEntries; ++i) {
                    intMap.put(i, i);
                }

                query::PagingPredicate<int, int> predicate((size_t) predSize);

                std::vector<int> values = intMap.keySet(predicate);
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    ASSERT_EQ(i, values[i]);
                }

                values = intMap.keySet(predicate);
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    ASSERT_EQ(i, values[i]);
                }

                predicate.nextPage();
                values = intMap.keySet(predicate);
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

                values = intMap.keySet(predicate);
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
                values = intMap.keySet(predicate);
                ASSERT_EQ(0, (int) values.size());

                predicate.setPage(0);
                values = intMap.keySet(predicate);
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    ASSERT_EQ(i, values[i]);
                }

                predicate.previousPage();
                ASSERT_EQ(0, (int) predicate.getPage());

                predicate.setPage(5);
                values = intMap.keySet(predicate);
                ASSERT_EQ(0, (int) values.size());

                predicate.setPage(3);
                values = intMap.keySet(predicate);
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    ASSERT_EQ(3 * predSize + i, values[i]);
                }

                predicate.previousPage();
                values = intMap.keySet(predicate);
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    ASSERT_EQ(2 * predSize + i, values[i]);
                }

// test PagingPredicate with inner predicate (value < 10)
                std::unique_ptr<query::Predicate> lessThanTenPredicate(std::unique_ptr<query::Predicate>(
                        new query::GreaterLessPredicate<int>(query::QueryConstants::getValueAttributeName(), 9, false,
                                                             true)));
                query::PagingPredicate<int, int> predicate2(lessThanTenPredicate, 5);
                values = intMap.keySet(predicate2);
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    ASSERT_EQ(i, values[i]);
                }

                predicate2.nextPage();
// match values 5,6, 7, 8
                values = intMap.keySet(predicate2);
                ASSERT_EQ(predSize - 1, (int) values.size());
                for (int i = 0; i < predSize - 1; ++i) {
                    ASSERT_EQ(predSize + i, values[i]);
                }

                predicate2.nextPage();
                values = intMap.keySet(predicate2);
                ASSERT_EQ(0, (int) values.size());

// test paging predicate with comparator
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
                        std::unique_ptr<query::EntryComparator<int, Employee> >(new EmployeeEntryKeyComparator()),
                        (size_t) predSize);
                std::vector<int> result = employees.keySet(predicate3);
// since keyset result only returns keys from the server, no ordering based on the value but ordered based on the keys
                ASSERT_EQ(2, (int) result.size());
                ASSERT_EQ(3, result[0]);
                ASSERT_EQ(4, result[1]);

                predicate3.nextPage();
                result = employees.keySet(predicate3);
                ASSERT_EQ(2, (int) result.size());
                ASSERT_EQ(5, result[0]);
                ASSERT_EQ(6, result[1]);
            }

            TEST_P(ClientMapTest, testEntrySetWithPredicate) {
                const int numItems = 20;
                std::vector<std::pair<int, int> > expected(numItems);
                for (int i = 0; i < numItems; ++i) {
                    intMap.put(i, 2 * i);
                    expected[i] = std::pair<int, int>(i, 2 * i);
                }

                std::vector<std::pair<int, int> > entries = intMap.entrySet();
                ASSERT_EQ(numItems, (int) entries.size());
                std::sort(entries.begin(), entries.end());
                for (int i = 0; i < numItems; ++i) {
                    ASSERT_EQ(expected[i], entries[i]);
                }

// EqualPredicate
// key == 5
                entries = intMap.entrySet(
                        query::EqualPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5));
                ASSERT_EQ(1, (int) entries.size());
                ASSERT_EQ(expected[5], entries[0]);

// value == 8
                entries = intMap.entrySet(
                        query::EqualPredicate<int>(query::QueryConstants::getValueAttributeName(), 8));
                ASSERT_EQ(1, (int) entries.size());
                ASSERT_EQ(expected[4], entries[0]);

// key == numItems
                entries = intMap.entrySet(
                        query::EqualPredicate<int>(query::QueryConstants::getKeyAttributeName(), numItems));
                ASSERT_EQ(0, (int) entries.size());

// NotEqual Predicate
// key != 5
                entries = intMap.entrySet(
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
                entries = intMap.entrySet(
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
                entries = intMap.entrySet(query::TruePredicate());
                ASSERT_EQ(numItems, (int) entries.size());
                std::sort(entries.begin(), entries.end());
                for (int i = 0; i < numItems; ++i) {
                    ASSERT_EQ(expected[i], entries[i]);
                }

// FalsePredicate
                entries = intMap.entrySet(query::FalsePredicate());
                ASSERT_EQ(0, (int) entries.size());

// BetweenPredicate
// 5 <= key <= 10
                entries = intMap.entrySet(
                        query::BetweenPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5, 10));
                std::sort(entries.begin(), entries.end());
                ASSERT_EQ(6, (int) entries.size());
                for (int i = 0; i < 6; ++i) {
                    ASSERT_EQ(expected[i + 5], entries[i]);
                }

// 20 <= key <=30
                entries = intMap.entrySet(
                        query::BetweenPredicate<int>(query::QueryConstants::getKeyAttributeName(), 20, 30));
                ASSERT_EQ(0, (int) entries.size());

// GreaterLessPredicate
// value <= 10
                entries = intMap.entrySet(
                        query::GreaterLessPredicate<int>(query::QueryConstants::getValueAttributeName(), 10, true,
                                                         true));
                ASSERT_EQ(6, (int) entries.size());
                std::sort(entries.begin(), entries.end());
                for (int i = 0; i < 6; ++i) {
                    ASSERT_EQ(expected[i], entries[i]);
                }

// key < 7
                entries = intMap.entrySet(
                        query::GreaterLessPredicate<int>(query::QueryConstants::getKeyAttributeName(), 7, false, true));
                ASSERT_EQ(7, (int) entries.size());
                std::sort(entries.begin(), entries.end());
                for (int i = 0; i < 7; ++i) {
                    ASSERT_EQ(expected[i], entries[i]);
                }

// value >= 15
                entries = intMap.entrySet(
                        query::GreaterLessPredicate<int>(query::QueryConstants::getValueAttributeName(), 15, true,
                                                         false));
                ASSERT_EQ(12, (int) entries.size());
                std::sort(entries.begin(), entries.end());
                for (int i = 0; i < 12; ++i) {
                    ASSERT_EQ(expected[i + 8], entries[i]);
                }

// key > 5
                entries = intMap.entrySet(
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
                entries = intMap.entrySet(
                        query::InPredicate<int>(query::QueryConstants::getKeyAttributeName(), inVals));
                ASSERT_EQ(3, (int) entries.size());
                std::sort(entries.begin(), entries.end());
                ASSERT_EQ(expected[4], entries[0]);
                ASSERT_EQ(expected[10], entries[1]);
                ASSERT_EQ(expected[19], entries[2]);

// value in {4, 10, 19}
                entries = intMap.entrySet(
                        query::InPredicate<int>(query::QueryConstants::getValueAttributeName(), inVals));
                ASSERT_EQ(2, (int) entries.size());
                std::sort(entries.begin(), entries.end());
                ASSERT_EQ(expected[2], entries[0]);
                ASSERT_EQ(expected[5], entries[1]);

// InstanceOfPredicate
// value instanceof Integer
                entries = intMap.entrySet(query::InstanceOfPredicate("java.lang.Integer"));
                ASSERT_EQ(20, (int) entries.size());
                std::sort(entries.begin(), entries.end());
                for (int i = 0; i < numItems; ++i) {
                    ASSERT_EQ(expected[i], entries[i]);
                }

                entries = intMap.entrySet(query::InstanceOfPredicate("java.lang.String"));
                ASSERT_EQ(0, (int) entries.size());

// NotPredicate
// !(5 <= key <= 10)
                std::unique_ptr<query::Predicate> bp = std::unique_ptr<query::Predicate>(
                        new query::BetweenPredicate<int>(
                                query::QueryConstants::getKeyAttributeName(), 5, 10));
                query::NotPredicate notPredicate(bp);
                entries = intMap.entrySet(notPredicate);
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
                bp = std::unique_ptr<query::Predicate>(
                        new query::BetweenPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5, 10));
                std::unique_ptr<query::Predicate> inPred = std::unique_ptr<query::Predicate>(
                        new query::InPredicate<int>(query::QueryConstants::getValueAttributeName(), inVals));
                entries = intMap.entrySet(query::AndPredicate().add(bp).add(inPred));
                ASSERT_EQ(1, (int) entries.size());
                std::sort(entries.begin(), entries.end());
                ASSERT_EQ(expected[5], entries[0]);

// OrPredicate
// 5 <= key <= 10 OR Values in {4, 10, 19} = entries {2, 5, 6, 7, 8, 9, 10}
                bp = std::unique_ptr<query::Predicate>(
                        new query::BetweenPredicate<int>(query::QueryConstants::getKeyAttributeName(), 5, 10));
                inPred = std::unique_ptr<query::Predicate>(
                        new query::InPredicate<int>(query::QueryConstants::getValueAttributeName(), inVals));
                entries = intMap.entrySet(query::OrPredicate().add(bp).add(inPred));
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
                    key += hazelcast::util::IOUtil::to_string(i);
                    std::string value = "value";
                    value += hazelcast::util::IOUtil::to_string(i);
                    imap.put(key, value);
                    expectedStrEntries[i] = std::pair<std::string, std::string>(key, value);
                }
                imap.put("key_111_test", "myvalue_111_test");
                expectedStrEntries[12] = std::pair<std::string, std::string>("key_111_test", "myvalue_111_test");
                imap.put("key_22_test", "myvalue_22_test");
                expectedStrEntries[13] = std::pair<std::string, std::string>("key_22_test", "myvalue_22_test");

// LikePredicate
// value LIKE "value1" : {"value1"}
                std::vector<std::pair<std::string, std::string> > strEntries = imap.entrySet(
                        query::LikePredicate(query::QueryConstants::getValueAttributeName(), "value1"));
                ASSERT_EQ(1, (int) strEntries.size());
                ASSERT_EQ(expectedStrEntries[1], strEntries[0]);

// ILikePredicate
// value ILIKE "%VALue%1%" : {"key_111_test", "key1", "key10", "key11"}
                strEntries = imap.entrySet(
                        query::ILikePredicate(query::QueryConstants::getValueAttributeName(), "%VALue%1%"));
                ASSERT_EQ(4, (int) strEntries.size());
                std::sort(strEntries.begin(), strEntries.end());
                ASSERT_EQ(expectedStrEntries[1], strEntries[0]);
                ASSERT_EQ(expectedStrEntries[10], strEntries[1]);
                ASSERT_EQ(expectedStrEntries[11], strEntries[2]);
                ASSERT_EQ(expectedStrEntries[12], strEntries[3]);

// key ILIKE "%VAL%2%" : {"key_22_test", "key2"}
                strEntries = imap.entrySet(
                        query::ILikePredicate(query::QueryConstants::getValueAttributeName(), "%VAL%2%"));
                ASSERT_EQ(2, (int) strEntries.size());
                std::sort(strEntries.begin(), strEntries.end());
                ASSERT_EQ(expectedStrEntries[2], strEntries[0]);
                ASSERT_EQ(expectedStrEntries[13], strEntries[1]);

// SqlPredicate
// __key BETWEEN 4 and 7 : {4, 5, 6, 7} -> {8, 10, 12, 14}
                char sql[100];
                hazelcast::util::hz_snprintf(sql, 50, "%s BETWEEN 4 and 7",
                                             query::QueryConstants::getKeyAttributeName());
                entries = intMap.entrySet(query::SqlPredicate(sql));
                ASSERT_EQ(4, (int) entries.size());
                std::sort(entries.begin(), entries.end());
                for (int i = 0; i < 4; ++i) {
                    ASSERT_EQ(expected[i + 4], entries[i]);
                }

// RegexPredicate
// value matches the regex ".*value.*2.*" : {key_22_test, value2}
                strEntries = imap.entrySet(
                        query::RegexPredicate(query::QueryConstants::getValueAttributeName(), ".*value.*2.*"));
                ASSERT_EQ(2, (int) strEntries.size());
                std::sort(strEntries.begin(), strEntries.end());
                ASSERT_EQ(expectedStrEntries[2], strEntries[0]);
                ASSERT_EQ(expectedStrEntries[13], strEntries[1]);
            }

            TEST_P(ClientMapTest, testEntrySetWithPagingPredicate) {
                int predSize = 5;
                const int totalEntries = 25;

                for (int i = 0; i < totalEntries; ++i) {
                    intMap.put(i, i);
                }

                query::PagingPredicate<int, int> predicate((size_t) predSize);

                std::vector<std::pair<int, int> > values = intMap.entrySet(predicate);
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    std::pair<int, int> value(i, i);
                    ASSERT_EQ(value, values[i]);
                }

                values = intMap.entrySet(predicate);
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    std::pair<int, int> value(i, i);
                    ASSERT_EQ(value, values[i]);
                }

                predicate.nextPage();
                values = intMap.entrySet(predicate);
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

                values = intMap.entrySet(predicate);
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
                values = intMap.entrySet(predicate);
                ASSERT_EQ(0, (int) values.size());

                predicate.setPage(0);
                values = intMap.entrySet(predicate);
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    std::pair<int, int> value(i, i);
                    ASSERT_EQ(value, values[i]);
                }

                predicate.previousPage();
                ASSERT_EQ(0, (int) predicate.getPage());

                predicate.setPage(5);
                values = intMap.entrySet(predicate);
                ASSERT_EQ(0, (int) values.size());

                predicate.setPage(3);
                values = intMap.entrySet(predicate);
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    std::pair<int, int> value(3 * predSize + i, 3 * predSize + i);
                    ASSERT_EQ(value, values[i]);
                }

                predicate.previousPage();
                values = intMap.entrySet(predicate);
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    std::pair<int, int> value(2 * predSize + i, 2 * predSize + i);
                    ASSERT_EQ(value, values[i]);
                }

// test PagingPredicate with inner predicate (value < 10)
                std::unique_ptr<query::Predicate> lessThanTenPredicate(std::unique_ptr<query::Predicate>(
                        new query::GreaterLessPredicate<int>(query::QueryConstants::getValueAttributeName(), 9, false,
                                                             true)));
                query::PagingPredicate<int, int> predicate2(lessThanTenPredicate, 5);
                values = intMap.entrySet(predicate2);
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    std::pair<int, int> value(i, i);
                    ASSERT_EQ(value, values[i]);
                }

                predicate2.nextPage();
// match values 5,6, 7, 8
                values = intMap.entrySet(predicate2);
                ASSERT_EQ(predSize - 1, (int) values.size());
                for (int i = 0; i < predSize - 1; ++i) {
                    std::pair<int, int> value(predSize + i, predSize + i);
                    ASSERT_EQ(value, values[i]);
                }

                predicate2.nextPage();
                values = intMap.entrySet(predicate2);
                ASSERT_EQ(0, (int) values.size());

// test paging predicate with comparator
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
                        std::unique_ptr<query::EntryComparator<int, Employee> >(new EmployeeEntryComparator()),
                        (size_t) predSize);
                std::vector<std::pair<int, Employee> > result = employees.entrySet(
                        predicate3);
                ASSERT_EQ(2, (int) result.size());
                std::pair<int, Employee> value(8, empl6);
                ASSERT_EQ(value, result[0]);
                value = std::pair<int, Employee>(4, empl2);
                ASSERT_EQ(value, result[1]);

                predicate3.nextPage();
                result = employees.entrySet(predicate3);
                ASSERT_EQ(2, (int) result.size());
                value = std::pair<int, Employee>(5, empl3);
                ASSERT_EQ(value, result[0]);
                value = std::pair<int, Employee>(6, empl4);
                ASSERT_EQ(value, result[1]);
            }

            TEST_P(ClientMapTest, testReplace) {
                std::shared_ptr<std::string> temp = imap.replace("key1", "value");
                ASSERT_EQ(temp.get(), (std::string *) NULL);

                std::string tempKey = "key1";
                std::string tempValue = "value1";
                imap.put(tempKey, tempValue);

                ASSERT_EQ("value1", *(imap.replace("key1", "value2")));
                ASSERT_EQ("value2", *(imap.get("key1")));

                ASSERT_FALSE(imap.replace("key1", "value1", "value3"));
                ASSERT_EQ("value2", *(imap.get("key1")));

                ASSERT_TRUE(imap.replace("key1", "value2", "value3"));
                ASSERT_EQ("value3", *(imap.get("key1")));
            }

            TEST_P(ClientMapTest, testListenerWithPortableKey) {
                IMap<Employee, int> tradeMap = client.getMap<Employee, int>("tradeMap");
                hazelcast::util::CountDownLatch countDownLatch(1);
                hazelcast::util::AtomicInt atomicInteger(0);
                SampleEntryListenerForPortableKey listener(countDownLatch,
                                                           atomicInteger);
                Employee key("a", 1);
                std::string id = tradeMap.addEntryListener(listener, key, true);
                Employee key2("a", 2);
                tradeMap.put(key2, 1);
                tradeMap.put(key, 3);
                ASSERT_OPEN_EVENTUALLY(countDownLatch);
                ASSERT_EQ(1, (int) atomicInteger);

                ASSERT_TRUE(tradeMap.removeEntryListener(id));
            }

            TEST_P(ClientMapTest, testListener) {
                hazelcast::util::CountDownLatch latch1Add(5);
                hazelcast::util::CountDownLatch latch1Remove(2);
                hazelcast::util::CountDownLatch dummy(10);
                hazelcast::util::CountDownLatch latch2Add(1);
                hazelcast::util::CountDownLatch latch2Remove(1);

                CountdownListener <std::string, std::string> listener1(
                        latch1Add, latch1Remove, dummy, dummy);
                CountdownListener <std::string, std::string> listener2(
                        latch2Add, latch2Remove, dummy, dummy);

                std::string listener1ID = imap.addEntryListener(listener1, false);
                std::string listener2ID = imap.addEntryListener(listener2, "key3", true);

                hazelcast::util::sleep(2);

                imap.put("key1", "value1");
                imap.put("key2", "value2");
                imap.put("key3", "value3");
                imap.put("key4", "value4");
                imap.put("key5", "value5");

                imap.remove("key1");
                imap.remove("key3");

                ASSERT_TRUE(latch1Add.await(10));
                ASSERT_TRUE(latch1Remove.await(10));
                ASSERT_TRUE(latch2Add.await(5));
                ASSERT_TRUE(latch2Remove.await(5));

                ASSERT_TRUE(imap.removeEntryListener(listener1ID));
                ASSERT_TRUE(imap.removeEntryListener(listener2ID));

            }

            TEST_P(ClientMapTest, testListenerWithTruePredicate) {
                hazelcast::util::CountDownLatch latchAdd(3);
                hazelcast::util::CountDownLatch latchRemove(1);
                hazelcast::util::CountDownLatch latchEvict(1);
                hazelcast::util::CountDownLatch latchUpdate(1);

                CountdownListener<int, int> listener(latchAdd, latchRemove,
                                                     latchUpdate,
                                                     latchEvict);

                std::string listenerId = intMap.addEntryListener(listener,
                                                                 query::TruePredicate(),
                                                                 false);

                intMap.put(1, 1);
                intMap.put(2, 2);
                intMap.put(3, 3, 1000); // evict after 1 second
                intMap.remove(2);

                hazelcast::util::sleep(2);

                ASSERT_EQ(NULL, intMap.get(3).get()); // trigger eviction

// update an entry
                intMap.set(1, 5);
                std::shared_ptr<int> value = intMap.get(1);
                ASSERT_NE((int *) NULL, value.get());
                ASSERT_EQ(5, *value);

                hazelcast::util::CountDownLatchWaiter latches;
                latches.add(latchAdd).add(latchRemove).add(latchUpdate).add(latchEvict);
                ASSERT_TRUE(latches.awaitMillis(2000));

                ASSERT_TRUE(intMap.removeEntryListener(listenerId));
            }

            TEST_P(ClientMapTest, testListenerWithFalsePredicate) {
                hazelcast::util::CountDownLatch latchAdd(3);
                hazelcast::util::CountDownLatch latchRemove(1);
                hazelcast::util::CountDownLatch latchEvict(1);
                hazelcast::util::CountDownLatch latchUpdate(1);

                CountdownListener<int, int> listener(latchAdd, latchRemove,
                                                     latchUpdate,
                                                     latchEvict);

                std::string listenerId = intMap.addEntryListener(listener,
                                                                 query::FalsePredicate(),
                                                                 false);

                intMap.put(1, 1);
                intMap.put(2, 2);
                intMap.put(3, 3, 1000); // evict after 1 second
                intMap.remove(2);

                hazelcast::util::sleep(2);

                ASSERT_EQ(NULL, intMap.get(3).get()); // trigger eviction

// update an entry
                intMap.set(1, 5);
                std::shared_ptr<int> value = intMap.get(1);
                ASSERT_NE((int *) NULL, value.get());
                ASSERT_EQ(5, *value);

                hazelcast::util::CountDownLatchWaiter latches;
                latches.add(latchAdd).add(latchRemove).add(latchUpdate).add(latchEvict);
                ASSERT_FALSE(latches.awaitMillis(2000));

                ASSERT_TRUE(intMap.removeEntryListener(listenerId));
            }

            TEST_P(ClientMapTest, testListenerWithEqualPredicate) {
                hazelcast::util::CountDownLatch latchAdd(1);
                hazelcast::util::CountDownLatch latchRemove(1);
                hazelcast::util::CountDownLatch latchEvict(1);
                hazelcast::util::CountDownLatch latchUpdate(1);

                CountdownListener<int, int> listener(latchAdd, latchRemove,
                                                     latchUpdate,
                                                     latchEvict);

                std::string listenerId = intMap.addEntryListener(listener,
                                                                 query::EqualPredicate<int>(
                                                                         query::QueryConstants::getKeyAttributeName(),
                                                                         3), true);

                intMap.put(1, 1);
                intMap.put(2, 2);
                intMap.put(3, 3, 1000); // evict after 1 second
                intMap.remove(2);

                hazelcast::util::sleep(2);

                ASSERT_EQ(NULL, intMap.get(3).get()); // trigger eviction

// update an entry
                intMap.set(1, 5);
                std::shared_ptr<int> value = intMap.get(1);
                ASSERT_NE((int *) NULL, value.get());
                ASSERT_EQ(5, *value);

                hazelcast::util::CountDownLatchWaiter latches;
                latches.add(latchAdd).add(latchEvict);
                ASSERT_TRUE(latches.awaitMillis(2000));

                latches.reset();
                latches.add(latchUpdate).add(latchRemove);
                ASSERT_FALSE(latches.awaitMillis(2000));

                ASSERT_TRUE(intMap.removeEntryListener(listenerId));
            }

            TEST_P(ClientMapTest, testListenerWithNotEqualPredicate) {
                hazelcast::util::CountDownLatch latchAdd(2);
                hazelcast::util::CountDownLatch latchRemove(1);
                hazelcast::util::CountDownLatch latchEvict(1);
                hazelcast::util::CountDownLatch latchUpdate(1);

                CountdownListener<int, int> listener(latchAdd, latchRemove,
                                                     latchUpdate,
                                                     latchEvict);

                std::string listenerId = intMap.addEntryListener(listener,
                                                                 query::NotEqualPredicate<int>(
                                                                         query::QueryConstants::getKeyAttributeName(),
                                                                         3), true);

                intMap.put(1, 1);
                intMap.put(2, 2);
                intMap.put(3, 3, 1000); // evict after 1 second
                intMap.remove(2);

                hazelcast::util::sleep(2);

                ASSERT_EQ(NULL, intMap.get(3).get()); // trigger eviction

// update an entry
                intMap.set(1, 5);
                std::shared_ptr<int> value = intMap.get(1);
                ASSERT_NE((int *) NULL, value.get());
                ASSERT_EQ(5, *value);

                hazelcast::util::CountDownLatchWaiter latches;
                latches.add(latchAdd).add(latchRemove).add(latchUpdate);
                ASSERT_TRUE(latches.awaitMillis(2000));

                latches.reset();
                latches.add(latchEvict);
                ASSERT_FALSE(latches.awaitMillis(2000));

                ASSERT_TRUE(intMap.removeEntryListener(listenerId));
            }

            TEST_P(ClientMapTest, testListenerWithGreaterLessPredicate) {
                hazelcast::util::CountDownLatch latchAdd(2);
                hazelcast::util::CountDownLatch latchRemove(1);
                hazelcast::util::CountDownLatch latchEvict(1);
                hazelcast::util::CountDownLatch latchUpdate(1);

                CountdownListener<int, int> listener(latchAdd, latchRemove,
                                                     latchUpdate,
                                                     latchEvict);

// key <= 2
                std::string listenerId = intMap.addEntryListener(listener,
                                                                 query::GreaterLessPredicate<int>(
                                                                         query::QueryConstants::getKeyAttributeName(),
                                                                         2, true, true),
                                                                 false);

                intMap.put(1, 1);
                intMap.put(2, 2);
                intMap.put(3, 3, 1000); // evict after 1 second
                intMap.remove(2);

                hazelcast::util::sleep(2);

                ASSERT_EQ(NULL, intMap.get(3).get()); // trigger eviction

// update an entry
                intMap.set(1, 5);
                std::shared_ptr<int> value = intMap.get(1);
                ASSERT_NE((int *) NULL, value.get());
                ASSERT_EQ(5, *value);

                hazelcast::util::CountDownLatchWaiter latches;
                latches.add(latchAdd).add(latchRemove).add(latchUpdate);
                ASSERT_TRUE(latches.awaitMillis(2000));

                ASSERT_FALSE(latchEvict.awaitMillis(2000));

                ASSERT_TRUE(intMap.removeEntryListener(listenerId));
            }

            TEST_P(ClientMapTest, testListenerWithBetweenPredicate) {
                hazelcast::util::CountDownLatch latchAdd(2);
                hazelcast::util::CountDownLatch latchRemove(1);
                hazelcast::util::CountDownLatch latchEvict(1);
                hazelcast::util::CountDownLatch latchUpdate(1);

                CountdownListener<int, int> listener(latchAdd, latchRemove,
                                                     latchUpdate,
                                                     latchEvict);

// 1 <=key <= 2
                std::string listenerId = intMap.addEntryListener(listener,
                                                                 query::BetweenPredicate<int>(
                                                                         query::QueryConstants::getKeyAttributeName(),
                                                                         1, 2), true);

                intMap.put(1, 1);
                intMap.put(2, 2);
                intMap.put(3, 3, 1000); // evict after 1 second
                intMap.remove(2);

                hazelcast::util::sleep(2);

                ASSERT_EQ(NULL, intMap.get(3).get()); // trigger eviction

// update an entry
                intMap.set(1, 5);
                std::shared_ptr<int> value = intMap.get(1);
                ASSERT_NE((int *) NULL, value.get());
                ASSERT_EQ(5, *value);

                hazelcast::util::CountDownLatchWaiter latches;
                latches.add(latchAdd).add(latchRemove).add(latchUpdate);
                ASSERT_TRUE(latches.awaitMillis(2000));

                ASSERT_FALSE(latchEvict.awaitMillis(2000));

                ASSERT_TRUE(intMap.removeEntryListener(listenerId));
            }

            TEST_P(ClientMapTest, testListenerWithSqlPredicate) {
                hazelcast::util::CountDownLatch latchAdd(1);
                hazelcast::util::CountDownLatch latchRemove(1);
                hazelcast::util::CountDownLatch latchEvict(1);
                hazelcast::util::CountDownLatch latchUpdate(1);

                CountdownListener<int, int> listener(latchAdd, latchRemove,
                                                     latchUpdate,
                                                     latchEvict);

// 1 <=key <= 2
                std::string listenerId = intMap.addEntryListener(listener,
                                                                 query::SqlPredicate(
                                                                         "__key < 2"), true);

                intMap.put(1, 1);
                intMap.put(2, 2);
                intMap.put(3, 3, 1000); // evict after 1 second
                intMap.remove(2);

                hazelcast::util::sleep(2);

                ASSERT_EQ(NULL, intMap.get(3).get()); // trigger eviction

// update an entry
                intMap.set(1, 5);
                std::shared_ptr<int> value = intMap.get(1);
                ASSERT_NE((int *) NULL, value.get());
                ASSERT_EQ(5, *value);

                hazelcast::util::CountDownLatchWaiter latches;
                latches.add(latchAdd).add(latchUpdate);
                ASSERT_TRUE(latches.awaitMillis(2000));

                latches.reset();
                latches.add(latchRemove).add(latchEvict);
                ASSERT_FALSE(latches.awaitMillis(2000));

                ASSERT_TRUE(intMap.removeEntryListener(listenerId));
            }

            TEST_P(ClientMapTest, testListenerWithRegExPredicate) {
                hazelcast::util::CountDownLatch latchAdd(2);
                hazelcast::util::CountDownLatch latchRemove(1);
                hazelcast::util::CountDownLatch latchEvict(1);
                hazelcast::util::CountDownLatch latchUpdate(1);

                CountdownListener <std::string, std::string> listener(
                        latchAdd, latchRemove, latchUpdate, latchEvict);

// key matches any word containing ".*met.*"
                std::string listenerId = imap.addEntryListener(listener,
                                                               query::RegexPredicate(
                                                                       query::QueryConstants::getKeyAttributeName(),
                                                                       ".*met.*"), true);

                imap.put("ilkay", "yasar");
                imap.put("mehmet", "demir");
                imap.put("metin", "ozen", 1000); // evict after 1 second
                imap.put("hasan", "can");
                imap.remove("mehmet");

                hazelcast::util::sleep(2);

                ASSERT_EQ((std::string *) NULL, imap.get("metin").get()); // trigger eviction

// update an entry
                imap.set("hasan", "suphi");
                std::shared_ptr<std::string> value = imap.get("hasan");
                ASSERT_NE((std::string *) NULL, value.get());
                ASSERT_EQ("suphi", *value);

                hazelcast::util::CountDownLatchWaiter latches;
                latches.add(latchAdd).add(latchRemove).add(latchEvict);
                ASSERT_TRUE(latches.awaitMillis(2000));

                ASSERT_FALSE(latchUpdate.awaitMillis(2000));

                ASSERT_TRUE(imap.removeEntryListener(listenerId));
            }

            TEST_P(ClientMapTest, testListenerWithInstanceOfPredicate) {
                hazelcast::util::CountDownLatch latchAdd(3);
                hazelcast::util::CountDownLatch latchRemove(1);
                hazelcast::util::CountDownLatch latchEvict(1);
                hazelcast::util::CountDownLatch latchUpdate(1);

                CountdownListener<int, int> listener(latchAdd, latchRemove,
                                                     latchUpdate,
                                                     latchEvict);

// 1 <=key <= 2
                std::string listenerId = intMap.addEntryListener(listener,
                                                                 query::InstanceOfPredicate(
                                                                         "java.lang.Integer"),
                                                                 false);

                intMap.put(1, 1);
                intMap.put(2, 2);
                intMap.put(3, 3, 1000); // evict after 1 second
                intMap.remove(2);

                hazelcast::util::sleep(2);

                ASSERT_EQ(NULL, intMap.get(3).get()); // trigger eviction

// update an entry
                intMap.set(1, 5);
                std::shared_ptr<int> value = intMap.get(1);
                ASSERT_NE((int *) NULL, value.get());
                ASSERT_EQ(5, *value);

                hazelcast::util::CountDownLatchWaiter latches;
                latches.add(latchAdd).add(latchRemove).add(latchUpdate).add(latchEvict);
                ASSERT_TRUE(latches.awaitMillis(2000));

                ASSERT_TRUE(intMap.removeEntryListener(listenerId));
            }

            TEST_P(ClientMapTest, testListenerWithNotPredicate) {
                hazelcast::util::CountDownLatch latchAdd(2);
                hazelcast::util::CountDownLatch latchRemove(1);
                hazelcast::util::CountDownLatch latchEvict(1);
                hazelcast::util::CountDownLatch latchUpdate(1);

                CountdownListener<int, int> listener(latchAdd, latchRemove,
                                                     latchUpdate,
                                                     latchEvict);

// key >= 3
                std::unique_ptr<query::Predicate> greaterLessPred = std::unique_ptr<query::Predicate>(
                        new query::GreaterLessPredicate<int>(query::QueryConstants::getKeyAttributeName(), 3, true,
                                                             false));
                query::NotPredicate notPredicate(greaterLessPred);
                std::string listenerId = intMap.addEntryListener(listener, notPredicate,
                                                                 false);

                intMap.put(1, 1);
                intMap.put(2, 2);
                intMap.put(3, 3, 1000); // evict after 1 second
                intMap.remove(2);

                hazelcast::util::sleep(2);

                ASSERT_EQ(NULL, intMap.get(3).get()); // trigger eviction

// update an entry
                intMap.set(1, 5);
                std::shared_ptr<int> value = intMap.get(1);
                ASSERT_NE((int *) NULL, value.get());
                ASSERT_EQ(5, *value);

                hazelcast::util::CountDownLatchWaiter latches;
                latches.add(latchAdd).add(latchRemove).add(latchUpdate);
                ASSERT_TRUE(latches.awaitMillis(2000));

                latches.reset();
                latches.add(latchEvict);
                ASSERT_FALSE(latches.awaitMillis(1000));

                ASSERT_TRUE(intMap.removeEntryListener(listenerId));
            }

            TEST_P(ClientMapTest, testListenerWithAndPredicate) {
                hazelcast::util::CountDownLatch latchAdd(1);
                hazelcast::util::CountDownLatch latchRemove(1);
                hazelcast::util::CountDownLatch latchEvict(1);
                hazelcast::util::CountDownLatch latchUpdate(1);

                CountdownListener<int, int> listener(latchAdd, latchRemove,
                                                     latchUpdate,
                                                     latchEvict);

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
                std::string listenerId = intMap.addEntryListener(listener, predicate, false);

                intMap.put(1, 1);
                intMap.put(2, 2);
                intMap.put(3, 3, 1000); // evict after 1 second
                intMap.remove(2);

                hazelcast::util::sleep(2);

                ASSERT_EQ(NULL, intMap.get(3).get()); // trigger eviction

// update an entry
                intMap.set(1, 5);
                std::shared_ptr<int> value = intMap.get(1);
                ASSERT_NE((int *) NULL, value.get());
                ASSERT_EQ(5, *value);

                hazelcast::util::CountDownLatchWaiter latches;
                latches.add(latchAdd).add(latchUpdate);
                ASSERT_TRUE(latches.awaitMillis(2000));

                latches.reset();
                latches.add(latchEvict).add(latchRemove);
                ASSERT_FALSE(latches.awaitMillis(1000));

                ASSERT_TRUE(intMap.removeEntryListener(listenerId));
            }

            TEST_P(ClientMapTest, testListenerWithOrPredicate) {
                hazelcast::util::CountDownLatch latchAdd(2);
                hazelcast::util::CountDownLatch latchRemove(1);
                hazelcast::util::CountDownLatch latchEvict(1);
                hazelcast::util::CountDownLatch latchUpdate(1);

                CountdownListener<int, int> listener(latchAdd, latchRemove,
                                                     latchUpdate,
                                                     latchEvict);

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
                std::string listenerId = intMap.addEntryListener(listener, predicate, true);

                intMap.put(1, 1);
                intMap.put(2, 2);
                intMap.put(3, 3, 1000); // evict after 1 second
                intMap.remove(2);

                hazelcast::util::sleep(2);

                ASSERT_EQ(NULL, intMap.get(3).get()); // trigger eviction

// update an entry
                intMap.set(1, 5);
                std::shared_ptr<int> value = intMap.get(1);
                ASSERT_NE((int *) NULL, value.get());
                ASSERT_EQ(5, *value);

                hazelcast::util::CountDownLatchWaiter latches;
                latches.add(latchAdd).add(latchEvict).add(latchRemove);
                ASSERT_TRUE(latches.awaitMillis(2000));

                ASSERT_FALSE(latchUpdate.awaitMillis(2000));

                ASSERT_TRUE(intMap.removeEntryListener(listenerId));
            }

            TEST_P(ClientMapTest, testClearEvent) {
                hazelcast::util::CountDownLatch latch(1);
                ClearListener clearListener(latch);
                std::string listenerId = imap.addEntryListener(clearListener, false);
                imap.put("key1", "value1");
                imap.clear();
                ASSERT_TRUE(latch.await(120));
                imap.removeEntryListener(listenerId);
            }

            TEST_P(ClientMapTest, testEvictAllEvent) {
                hazelcast::util::CountDownLatch latch(1);
                EvictListener evictListener(latch);
                std::string listenerId = imap.addEntryListener(evictListener, false);
                imap.put("key1", "value1");
                imap.evictAll();
                ASSERT_TRUE(latch.await(120));
                imap.removeEntryListener(listenerId);
            }

            TEST_P(ClientMapTest, testMapWithPortable) {
                std::shared_ptr<Employee> n1 = employees.get(1);
                ASSERT_EQ(n1.get(), (Employee *) NULL);
                Employee employee("sancar", 24);
                std::shared_ptr<Employee> ptr = employees.put(1, employee);
                ASSERT_EQ(ptr.get(), (Employee *) NULL);
                ASSERT_FALSE(employees.isEmpty());
                EntryView<int, Employee> view = employees.getEntryView(1);
                ASSERT_EQ(view.value, employee);
                ASSERT_EQ(view.key, 1);

                employees.addIndex("a", true);
                employees.addIndex("n", false);
            }

            TEST_P(ClientMapTest, testMapStoreRelatedRequests) {
                imap.putTransient("ali", "veli", 1100);
                imap.flush();
                ASSERT_EQ(1, imap.size());
                ASSERT_FALSE(imap.evict("deli"));
                ASSERT_TRUE(imap.evict("ali"));
                ASSERT_EQ(imap.get("ali").get(), (std::string *) NULL);
            }

            TEST_P(ClientMapTest, testExecuteOnKey) {
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);

                employees.put(3, empl1);
                employees.put(4, empl2);

                EntryMultiplier processor(4);

                std::shared_ptr<int> result = employees.executeOnKey<int, EntryMultiplier>(
                        4, processor);

                ASSERT_NE((int *) NULL, result.get());
                ASSERT_EQ(4 * processor.getMultiplier(), *result);
            }

            TEST_P(ClientMapTest, testSubmitToKey) {
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);

                employees.put(3, empl1);
                employees.put(4, empl2);

                EntryMultiplier processor(4);

                auto future = employees.submitToKey<int, EntryMultiplier>(4, processor);

                future_status status = future.wait_for(chrono::seconds(2));
                ASSERT_EQ(future_status::ready, status);
                auto result = future.get();
                ASSERT_NE((int *) NULL, result.get());
                ASSERT_EQ(4 * processor.getMultiplier(), *result);
            }

            TEST_P(ClientMapTest, testExecuteOnNonExistentKey) {
                EntryMultiplier processor(4);

                std::shared_ptr<int> result = employees.executeOnKey<int, EntryMultiplier>(
                        17, processor);

                ASSERT_NE((int *) NULL, result.get());
                ASSERT_EQ(-1, *result);
            }

            TEST_P(ClientMapTest, testExecuteOnKeys) {
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);
                Employee empl3("deniz", 25);

                employees.put(3, empl1);
                employees.put(4, empl2);
                employees.put(5, empl3);

                EntryMultiplier processor(4);

                std::set<int> keys;
                keys.insert(3);
                keys.insert(5);
// put non existent key
                keys.insert(999);

                std::map<int, std::shared_ptr<int> > result = employees.executeOnKeys<int, EntryMultiplier>(
                        keys, processor);

                ASSERT_EQ(3, (int) result.size());
                ASSERT_NE(result.end(), result.find(3));
                ASSERT_NE(result.end(), result.find(5));
                ASSERT_NE(result.end(), result.find(999));
                ASSERT_EQ(3 * processor.getMultiplier(), *result[3]);
                ASSERT_EQ(5 * processor.getMultiplier(), *result[5]);
                ASSERT_EQ(-1, *result[999]);
            }

            TEST_P(ClientMapTest, testExecuteOnEntries) {
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);
                Employee empl3("deniz", 25);

                employees.put(3, empl1);
                employees.put(4, empl2);
                employees.put(5, empl3);

                EntryMultiplier processor(4);

                std::map<int, std::shared_ptr<int> > result = employees.executeOnEntries<int, EntryMultiplier>(
                        processor);

                ASSERT_EQ(3, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(3)));
                ASSERT_TRUE((result.end() != result.find(4)));
                ASSERT_TRUE((result.end() != result.find(5)));
                ASSERT_EQ(3 * processor.getMultiplier(), *result[3]);
                ASSERT_EQ(4 * processor.getMultiplier(), *result[4]);
                ASSERT_EQ(5 * processor.getMultiplier(), *result[5]);
            }

            TEST_P(ClientMapTest, testExecuteOnEntriesWithTruePredicate) {
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);
                Employee empl3("deniz", 25);

                employees.put(3, empl1);
                employees.put(4, empl2);
                employees.put(5, empl3);

                EntryMultiplier processor(4);

                std::map<int, std::shared_ptr<int> > result = employees.executeOnEntries<int, EntryMultiplier>(
                        processor, query::TruePredicate());

                ASSERT_EQ(3, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(3)));
                ASSERT_TRUE((result.end() != result.find(4)));
                ASSERT_TRUE((result.end() != result.find(5)));
                ASSERT_EQ(3 * processor.getMultiplier(), *result[3]);
                ASSERT_EQ(4 * processor.getMultiplier(), *result[4]);
                ASSERT_EQ(5 * processor.getMultiplier(), *result[5]);
            }

            TEST_P(ClientMapTest, testExecuteOnEntriesWithFalsePredicate) {
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);
                Employee empl3("deniz", 25);

                employees.put(3, empl1);
                employees.put(4, empl2);
                employees.put(5, empl3);

                EntryMultiplier processor(4);

                std::map<int, std::shared_ptr<int> > result = employees.executeOnEntries<int, EntryMultiplier>(
                        processor, query::FalsePredicate());

                ASSERT_EQ(0, (int) result.size());
            }

            TEST_P(ClientMapTest, testExecuteOnEntriesWithAndPredicate) {
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);
                Employee empl3("deniz", 25);

                employees.put(3, empl1);
                employees.put(4, empl2);
                employees.put(5, empl3);

                query::AndPredicate andPredicate;
/* 25 <= age <= 35 AND age = 35 */
                andPredicate.add(
                        std::unique_ptr<query::Predicate>(new query::BetweenPredicate<int>("a", 25, 35))).add(
                        std::unique_ptr<query::Predicate>(
                                new query::NotPredicate(
                                        std::unique_ptr<query::Predicate>(new query::EqualPredicate<int>("a", 35)))));

                EntryMultiplier processor(4);

                std::map<int, std::shared_ptr<int> > result = employees.executeOnEntries<int, EntryMultiplier>(
                        processor, andPredicate);

                ASSERT_EQ(1, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(5)));
                ASSERT_EQ(5 * processor.getMultiplier(), *result[5]);
            }

            TEST_P(ClientMapTest, testExecuteOnEntriesWithOrPredicate) {
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);
                Employee empl3("deniz", 25);

                employees.put(3, empl1);
                employees.put(4, empl2);
                employees.put(5, empl3);

                query::OrPredicate orPredicate;
/* age == 21 OR age > 25 */
                orPredicate.add(
                        std::unique_ptr<query::Predicate>(new query::EqualPredicate<int>("a", 21))).add(
                        std::unique_ptr<query::Predicate>(new query::GreaterLessPredicate<int>("a", 25, false, false)));

                EntryMultiplier processor(4);

                std::map<int, std::shared_ptr<int> > result = employees.executeOnEntries<int, EntryMultiplier>(
                        processor, orPredicate);

                ASSERT_EQ(2, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(3)));
                ASSERT_TRUE((result.end() != result.find(4)));
                ASSERT_EQ(3 * processor.getMultiplier(), *result[3]);
                ASSERT_EQ(4 * processor.getMultiplier(), *result[4]);
            }

            TEST_P(ClientMapTest, testExecuteOnEntriesWithBetweenPredicate) {
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);
                Employee empl3("deniz", 25);

                employees.put(3, empl1);
                employees.put(4, empl2);
                employees.put(5, empl3);

                EntryMultiplier processor(4);

                std::map<int, std::shared_ptr<int> > result = employees.executeOnEntries<int, EntryMultiplier>(
                        processor, query::BetweenPredicate<int>("a", 25, 35));

                ASSERT_EQ(2, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(3)));
                ASSERT_TRUE((result.end() != result.find(5)));
                ASSERT_EQ(3 * processor.getMultiplier(), *result[3]);
                ASSERT_EQ(5 * processor.getMultiplier(), *result[5]);
            }

            TEST_P(ClientMapTest, testExecuteOnEntriesWithEqualPredicate) {
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);
                Employee empl3("deniz", 25);

                employees.put(3, empl1);
                employees.put(4, empl2);
                employees.put(5, empl3);

                EntryMultiplier processor(4);

                std::map<int, std::shared_ptr<int> > result = employees.executeOnEntries<int, EntryMultiplier>(
                        processor, query::EqualPredicate<int>("a", 25));

                ASSERT_EQ(1, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(5)));

                result = employees.executeOnEntries<int, EntryMultiplier>(
                        processor, query::EqualPredicate<int>("a", 10));

                ASSERT_EQ(0, (int) result.size());
            }

            TEST_P(ClientMapTest, testExecuteOnEntriesWithNotEqualPredicate) {
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);
                Employee empl3("deniz", 25);

                employees.put(3, empl1);
                employees.put(4, empl2);
                employees.put(5, empl3);

                EntryMultiplier processor(4);

                std::map<int, std::shared_ptr<int> > result = employees.executeOnEntries<int, EntryMultiplier>(
                        processor, query::NotEqualPredicate<int>("a", 25));

                ASSERT_EQ(2, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(3)));
                ASSERT_TRUE((result.end() != result.find(4)));
            }

            TEST_P(ClientMapTest, testExecuteOnEntriesWithGreaterLessPredicate) {
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);
                Employee empl3("deniz", 25);

                employees.put(3, empl1);
                employees.put(4, empl2);
                employees.put(5, empl3);

                EntryMultiplier processor(4);

                std::map<int, std::shared_ptr<int> > result = employees.executeOnEntries<int, EntryMultiplier>(
                        processor, query::GreaterLessPredicate<int>("a", 25, false, true)); // <25 matching

                ASSERT_EQ(1, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(4)));

                result = employees.executeOnEntries<int, EntryMultiplier>(
                        processor, query::GreaterLessPredicate<int>("a", 25, true, true)); // <=25 matching

                ASSERT_EQ(2, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(4)));
                ASSERT_TRUE((result.end() != result.find(5)));

                result = employees.executeOnEntries<int, EntryMultiplier>(
                        processor, query::GreaterLessPredicate<int>("a", 25, false, false)); // >25 matching

                ASSERT_EQ(1, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(3)));

                result = employees.executeOnEntries<int, EntryMultiplier>(
                        processor, query::GreaterLessPredicate<int>("a", 25, true, false)); // >=25 matching

                ASSERT_EQ(2, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(3)));
                ASSERT_TRUE((result.end() != result.find(5)));
            }

            TEST_P(ClientMapTest, testExecuteOnEntriesWithLikePredicate) {
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);
                Employee empl3("deniz", 25);

                employees.put(3, empl1);
                employees.put(4, empl2);
                employees.put(5, empl3);

                EntryMultiplier processor(4);

                std::map<int, std::shared_ptr<int> > result = employees.executeOnEntries<int, EntryMultiplier>(
                        processor, query::LikePredicate("n", "deniz"));

                ASSERT_EQ(1, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(5)));
            }

            TEST_P(ClientMapTest, testExecuteOnEntriesWithILikePredicate) {
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);
                Employee empl3("deniz", 25);

                employees.put(3, empl1);
                employees.put(4, empl2);
                employees.put(5, empl3);

                EntryMultiplier processor(4);

                std::map<int, std::shared_ptr<int> > result = employees.executeOnEntries<int, EntryMultiplier>(
                        processor, query::ILikePredicate("n", "deniz"));

                ASSERT_EQ(1, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(5)));
            }

            TEST_P(ClientMapTest, testExecuteOnEntriesWithInPredicate) {
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
                std::map<int, std::shared_ptr<int> > result = employees.executeOnEntries<int, EntryMultiplier>(
                        processor, predicate);

                ASSERT_EQ(2, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(3)));
                ASSERT_TRUE((result.end() != result.find(4)));
            }

            TEST_P(ClientMapTest, testExecuteOnEntriesWithInstanceOfPredicate) {
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);
                Employee empl3("deniz", 25);

                employees.put(3, empl1);
                employees.put(4, empl2);
                employees.put(5, empl3);

                EntryMultiplier processor(4);
                std::map<int, std::shared_ptr<int> > result = employees.executeOnEntries<int, EntryMultiplier>(
                        processor, query::InstanceOfPredicate("com.hazelcast.client.test.Employee"));

                ASSERT_EQ(3, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(3)));
                ASSERT_TRUE((result.end() != result.find(4)));
                ASSERT_TRUE((result.end() != result.find(5)));
            }

            TEST_P(ClientMapTest, testExecuteOnEntriesWithNotPredicate) {
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);
                Employee empl3("deniz", 25);

                employees.put(3, empl1);
                employees.put(4, empl2);
                employees.put(5, empl3);

                EntryMultiplier processor(4);
                std::unique_ptr<query::Predicate> eqPredicate(new query::EqualPredicate<int>("a", 25));
                query::NotPredicate notPredicate(eqPredicate);
                std::map<int, std::shared_ptr<int> > result = employees.executeOnEntries<int, EntryMultiplier>(
                        processor, notPredicate);

                ASSERT_EQ(2, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(3)));
                ASSERT_TRUE((result.end() != result.find(4)));

                query::NotPredicate notFalsePredicate(std::unique_ptr<query::Predicate>(new query::FalsePredicate()));
                result = employees.executeOnEntries<int, EntryMultiplier>(
                        processor, notFalsePredicate);

                ASSERT_EQ(3, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(3)));
                ASSERT_TRUE((result.end() != result.find(4)));
                ASSERT_TRUE((result.end() != result.find(5)));

                query::NotPredicate notBetweenPredicate(
                        std::unique_ptr<query::Predicate>(new query::BetweenPredicate<int>("a", 25, 35)));
                result = employees.executeOnEntries<int, EntryMultiplier>(
                        processor, notBetweenPredicate);

                ASSERT_EQ(1, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(4)));
            }

            TEST_P(ClientMapTest, testExecuteOnEntriesWithRegexPredicate) {
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);
                Employee empl3("deniz", 25);

                employees.put(3, empl1);
                employees.put(4, empl2);
                employees.put(5, empl3);

                EntryMultiplier processor(4);

                std::map<int, std::shared_ptr<int> > result = employees.executeOnEntries<int, EntryMultiplier>(
                        processor, query::RegexPredicate("n", ".*met"));

                ASSERT_EQ(2, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(3)));
                ASSERT_TRUE((result.end() != result.find(4)));
            }

            TEST_P(ClientMapTest, testAddInterceptor) {
                std::string prefix("My Prefix");
                MapGetInterceptor interceptor(prefix);
                std::string interceptorId = imap.
                        addInterceptor<MapGetInterceptor>(interceptor);

                std::shared_ptr<std::string> val = imap.get("nonexistent");
                ASSERT_NE((std::string *) NULL, val.get());
                ASSERT_EQ(prefix, *val);

                val = imap.put("key1", "value1");
                ASSERT_EQ((std::string *) NULL, val.get());

                val = imap.get("key1");
                ASSERT_NE((std::string *) NULL, val.get());
                ASSERT_EQ(prefix + "value1", *val);

                imap.removeInterceptor(interceptorId);
            }

            TEST_P(ClientMapTest, testJsonPutGet) {
                IMap<std::string, HazelcastJsonValue> map = client.getMap<std::string, HazelcastJsonValue>(
                        getTestName());
                HazelcastJsonValue value("{ \"age\": 4 }");
                map.put("item1", value);
                std::shared_ptr<HazelcastJsonValue> retrieved = map.get("item1");

                ASSERT_EQ_PTR(value, retrieved.get(), HazelcastJsonValue);
            }

            TEST_P(ClientMapTest, testQueryOverJsonObject) {
                IMap<std::string, HazelcastJsonValue> map = client.getMap<std::string, HazelcastJsonValue>(
                        getTestName());
                HazelcastJsonValue young("{ \"age\": 4 }");
                HazelcastJsonValue old("{ \"age\": 20 }");
                map.put("item1", young);
                map.put("item2", old);

                ASSERT_EQ(2, map.size());

// Get the objects whose age is less than 6
                std::vector<HazelcastJsonValue> result = map.values(
                        query::GreaterLessPredicate<int>("age", 6, false, true));
                ASSERT_EQ(1U, result.size());
                ASSERT_EQ(young, result[0]);
            }

            TEST_P(ClientMapTest, testExtendedAsciiString) {
                std::string key = "Num\xc3\xa9ro key";
                std::string value = "Num\xc3\xa9ro value";
                imap.put(key, value);

                std::shared_ptr<std::string> actualValue = imap.get(key);

                ASSERT_EQ_PTR(value, actualValue.get(), std::string);
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

