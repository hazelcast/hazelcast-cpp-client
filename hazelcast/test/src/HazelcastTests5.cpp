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
#include <hazelcast/client/EntryListener.h>
#include <hazelcast/client/ItemListener.h>
#include "serialization/Serializables.h"
#include <regex>
#include <vector>
#include <fstream>
#include "ClientTestSupportBase.h"
#include <hazelcast/client/ClientConfig.h>
#include <hazelcast/client/HazelcastClient.h>
#include <hazelcast/client/serialization/serialization.h>
#include <hazelcast/client/impl/Partition.h>
#include <gtest/gtest.h>
#include <hazelcast/client/connection/ClientConnectionManagerImpl.h>
#include <memory>
#include <hazelcast/client/proxy/PNCounterImpl.h>
#include <hazelcast/util/AddressHelper.h>
#include <hazelcast/util/Util.h>
#include <TestHelperFunctions.h>
#include <ostream>
#include <ctime>
#include <hazelcast/client/LifecycleListener.h>
#include <hazelcast/client/HazelcastJsonValue.h>
#include <hazelcast/client/internal/nearcache/impl/store/NearCacheObjectRecordStore.h>
#include <hazelcast/client/query/Predicates.h>
#include <cmath>
#include <hazelcast/client/spi/impl/sequence/CallIdSequenceWithoutBackpressure.h>
#include <hazelcast/client/spi/impl/sequence/CallIdSequenceWithBackpressure.h>
#include <hazelcast/client/spi/impl/sequence/FailFastCallIdSequence.h>
#include <string>
#include <boost/asio.hpp>


#ifdef HZ_BUILD_WITH_SSL

#include <openssl/crypto.h>

#endif

#include "hazelcast/client/exception/ProtocolExceptions.h"
#include "hazelcast/client/internal/socket/SSLSocket.h"
#include "hazelcast/client/InitialMembershipEvent.h"
#include "hazelcast/client/SocketInterceptor.h"
#include "hazelcast/client/Socket.h"
#include "hazelcast/client/IMap.h"
#include "hazelcast/util/SyncHttpsClient.h"
#include "hazelcast/client/Pipelining.h"
#include "hazelcast/util/MurmurHash3.h"
#include "hazelcast/client/protocol/ClientProtocolErrorCodes.h"
#include "hazelcast/client/MultiMap.h"
#include "hazelcast/client/EntryEvent.h"
#include "hazelcast/client/ClientProperties.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4996) //for unsafe getenv
#endif

/*
namespace hazelcast {
    namespace client {
        namespace test {
            namespace common {
                namespace containers {
                    class LittleEndianBufferTest : public ::testing::Test,
                                    public protocol::ClientMessage */
/* Need this in order to test*//*

                    {
                    public:
                        LittleEndianBufferTest() : protocol::ClientMessage(0) {}
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
                        byte strBytes[8] = {4, 0, 0, 0, */
/* This part is the len field which is 4 bytes *//*

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
                                                        4, 0, 0, 0, */
/* This part is the len field which is 4 bytes *//*

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
*/


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
                CallIdSequenceWithBackpressureTest() = default;

            protected:
                class ThreeSecondDelayCompleteOperation {
                public:
                    ThreeSecondDelayCompleteOperation(spi::impl::sequence::CallIdSequenceWithBackpressure &sequence,
                                                      boost::latch &nextCalledLatch) : sequence(
                            sequence),
                                                                                       nextCalledLatch(
                                                                                               nextCalledLatch) {}

                    virtual const std::string getName() const {
                        return "ThreeSecondDelayCompleteOperation";
                    }

                    virtual void run() {
                        sequence.next();
                        nextCalledLatch.count_down();
                        sleepSeconds(3);
                        sequence.complete();
                    }

                private:
                    spi::impl::sequence::CallIdSequenceWithBackpressure &sequence;
                    boost::latch &nextCalledLatch;
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

                boost::latch nextCalledLatch(1);

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
                    : logger_(config::LoggerConfig().logger_factory()("HazelcastServerFactory", "HazelcastServerFactory")),
                      serverAddress(serverAddress) {
                std::string xmlConfig = readFromXmlFile(serverXmlConfigFilePath);

                remote::Cluster cluster;
                remoteController->createClusterKeepClusterName(cluster, HAZELCAST_VERSION, xmlConfig);

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

            const std::string &HazelcastServerFactory::getClusterId() const {
                return clusterId;
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
                public:
                    UnknownObject(int value) : value_(value) {}

                    int getValue() const {
                        return value_;
                    }

                private:
                    int value_;
                };

                class WriteReadIntGlobalSerializer : public serialization::global_serializer {
                public:
                    void write(const boost::any &object, serialization::ObjectDataOutput &out) override {
                        auto const &obj = boost::any_cast<UnknownObject>(object);
                        out.write<int32_t>(obj.getValue());
                    }

                    boost::any read(serialization::ObjectDataInput &in) override {
                        return boost::any(UnknownObject(in.read<int32_t>()));
                    }
                };
            protected:

                static void SetUpTestCase() {
                    instance = new HazelcastServer(*g_srvFactory);
                    ClientConfig clientConfig = getConfig();
                    clientConfig.getSerializationConfig().setGlobalSerializer(
                            std::make_shared<WriteReadIntGlobalSerializer>());
                    client = new HazelcastClient(clientConfig);
                    imap = client->getMap("UnknownObject");
                }

                static void TearDownTestCase() {
                    delete client;
                    delete instance;

                    imap = nullptr;
                    client = nullptr;
                    instance = nullptr;
                }

                static HazelcastServer *instance;
                static HazelcastClient *client;
                static std::shared_ptr<IMap> imap;
            };

            HazelcastServer *MapGlobalSerializerTest::instance = nullptr;
            HazelcastClient *MapGlobalSerializerTest::client = nullptr;
            std::shared_ptr<IMap> MapGlobalSerializerTest::imap;

            TEST_F(MapGlobalSerializerTest, testPutGetUnserializableObject) {
                MapGlobalSerializerTest::UnknownObject myObject(8);
                imap->put<int, MapGlobalSerializerTest::UnknownObject>(2, myObject).get();

                boost::optional<MapGlobalSerializerTest::UnknownObject> data =
                        imap->get<int, MapGlobalSerializerTest::UnknownObject>(2).get();
                ASSERT_TRUE(data.has_value());
                ASSERT_EQ(8, data.value().getValue());
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace test {
            class ClientExpirationListenerTest : public ClientTestSupport {
            protected:
                void TearDown() override {
                    // clear maps
                    imap->clear().get();
                }

                static void SetUpTestCase() {
                    instance = new HazelcastServer(*g_srvFactory);
                    instance2 = new HazelcastServer(*g_srvFactory);
                    client = new HazelcastClient(getConfig());
                    imap = client->getMap("IntMap");
                }

                static void TearDownTestCase() {
                    delete client;
                    delete instance2;
                    delete instance;

                    imap = nullptr;
                    client = nullptr;
                    instance2 = nullptr;
                    instance = nullptr;
                }

                static HazelcastServer *instance;
                static HazelcastServer *instance2;
                static HazelcastClient *client;
                static std::shared_ptr<IMap> imap;
            };

            HazelcastServer *ClientExpirationListenerTest::instance = nullptr;
            HazelcastServer *ClientExpirationListenerTest::instance2 = nullptr;
            HazelcastClient *ClientExpirationListenerTest::client = nullptr;
            std::shared_ptr<IMap> ClientExpirationListenerTest::imap = nullptr;

            TEST_F(ClientExpirationListenerTest, notified_afterExpirationOfEntries) {
                int numberOfPutOperations = 10000;
                boost::latch expirationEventArrivalCount(numberOfPutOperations);

                EntryListener expirationListener;

                expirationListener.
                    on_expired([&expirationEventArrivalCount](EntryEvent &&) {
                        expirationEventArrivalCount.count_down();
                    });

                auto registrationId = imap->addEntryListener(std::move(expirationListener), true).get();

                for (int i = 0; i < numberOfPutOperations; i++) {
                    imap->put<int, int>(i, i, std::chrono::milliseconds(100)).get();
                }

                // wait expiration of entries.
                std::this_thread::sleep_for(std::chrono::seconds(1));

                // trigger immediate fire of expiration events by touching them.
                for (int i = 0; i < numberOfPutOperations; ++i) {
                    imap->get<int, int>(i).get();
                }

                ASSERT_OPEN_EVENTUALLY(expirationEventArrivalCount);
                ASSERT_TRUE(imap->removeEntryListener(registrationId).get());
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace test {
            class ClientListenerFunctionObjectsTest : public ClientTestSupport {
            public:
                static void SetUpTestSuite() {
                    instance = new HazelcastServer(*g_srvFactory);
                    ClientConfig clientConfig(getConfig());
                    clientConfig.setProperty(ClientProperties::PROP_HEARTBEAT_TIMEOUT, "20");
                    client = new HazelcastClient(clientConfig);
                    map = client->getMap("map");
                }

                static void TearDownTestSuite() {
                    delete client;
                    delete instance;

                    map.reset();
                    instance = nullptr;
                    client = nullptr;
                }

                void TearDown() override {
                    map->clear().get();
                }

                static HazelcastServer *instance;
                static HazelcastClient *client;
                static std::shared_ptr<IMap> map;
            };

            HazelcastServer *ClientListenerFunctionObjectsTest::instance = nullptr;
            HazelcastClient *ClientListenerFunctionObjectsTest::client = nullptr;
            std::shared_ptr<IMap> ClientListenerFunctionObjectsTest::map = nullptr;

            TEST_F(ClientListenerFunctionObjectsTest, lambda) {
                boost::latch called {1};

                auto id = map->addEntryListener(
                    EntryListener().
                        on_added([&called](EntryEvent &&e) {
                            ASSERT_EQ(1, e.getKey().get<int>().get());
                            ASSERT_EQ(2, e.getValue().get<int>().get());
                            called.count_down();
                        })
                , true).get();

                map->put(1, 2).get();

                ASSERT_OPEN_EVENTUALLY(called);

                map->removeEntryListener(id).get();
            }

            TEST_F(ClientListenerFunctionObjectsTest, functor) {
                struct OnEntryAdded {
                    boost::latch& latch;

                    void operator()(EntryEvent &&e) {
                        ASSERT_EQ(1, e.getKey().get<int>().get());
                        ASSERT_EQ(2, e.getValue().get<int>().get());
                        latch.count_down();
                    }
                };

                boost::latch called {1};
                OnEntryAdded handler {called};

                auto id = map->addEntryListener(
                    EntryListener().on_added(std::move(handler))
                , true).get();

                map->put(1, 2).get();

                ASSERT_OPEN_EVENTUALLY(called);

                map->removeEntryListener(id).get();
            }

            TEST_F(ClientListenerFunctionObjectsTest, staticFunction) {
                static boost::latch called {1};

                struct OnEntryAdded {
                    static void handler(EntryEvent &&e) {
                        ASSERT_EQ(1, e.getKey().get<int>().get());
                        ASSERT_EQ(2, e.getValue().get<int>().get());
                        called.count_down();
                    }
                };

                auto id = map->addEntryListener(
                    EntryListener().on_added(&OnEntryAdded::handler)
                , true).get();

                map->put(1, 2).get();

                ASSERT_OPEN_EVENTUALLY(called);

                map->removeEntryListener(id).get();
            }

            TEST_F(ClientListenerFunctionObjectsTest, bind) {
                boost::latch called {1};

                struct MyListener {
                    boost::latch &latch;

                    void added(EntryEvent &&e) {
                        ASSERT_EQ(1, e.getKey().get<int>().get());
                        ASSERT_EQ(2, e.getValue().get<int>().get());
                        latch.count_down();
                    }
                };

                MyListener listener {called};
                auto handler = std::bind(&MyListener::added, &listener, std::placeholders::_1);

                auto id = map->addEntryListener(
                    EntryListener().on_added(std::move(handler))
                , true).get();

                map->put(1, 2).get();

                ASSERT_OPEN_EVENTUALLY(called);

                map->removeEntryListener(id).get();
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace test {
            class PartitionAwareInt : public PartitionAware<int> {
            public:
                PartitionAwareInt() : partitionKey(0), actualKey(0) {}

                PartitionAwareInt(int partitionKey, int actualKey)
                        : partitionKey(partitionKey), actualKey(actualKey) {}

                const int *getPartitionKey() const override {
                    return &partitionKey;
                }

                int getActualKey() const {
                    return actualKey;
                }
            private:
                int partitionKey;
                int actualKey;
            };

            class MapClientConfig : public ClientConfig {
            public:
                static constexpr const char *intMapName = "IntMap";
                static constexpr const char *employeesMapName = "EmployeesMap";
                static constexpr const char *imapName = "clientMapTest";
                static constexpr const char *ONE_SECOND_MAP_NAME = "OneSecondTtlMap";

                MapClientConfig() {
                    getNetworkConfig().addAddress(Address(g_srvFactory->getServerAddress(), 5701));
                }

                virtual ~MapClientConfig() = default;
            };

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
                                  imap(client.getMap(MapClientConfig::imapName)),
                                  intMap(client.getMap(MapClientConfig::intMapName)),
                                  employees(client.getMap(MapClientConfig::employeesMapName)) {
                }

                static void SetUpTestCase() {
                    instance = new HazelcastServer(*g_srvFactory);
                    instance2 = new HazelcastServer(*g_srvFactory);
                }

                static void TearDownTestCase() {
                    delete instance2;
                    delete instance;

                    instance2 = nullptr;
                    instance = nullptr;
                }

                class MapGetInterceptor {
                    friend struct serialization::hz_serializer<MapGetInterceptor>;
                public:
                    MapGetInterceptor(const std::string &prefix) : prefix_(prefix) {}
                private:
                    std::string prefix_;
                };

                class EntryMultiplier {
                public:
                    EntryMultiplier(int multiplier) : multiplier(multiplier) {}

                    int getMultiplier() const {
                        return multiplier;
                    }
                private:
                    int multiplier;
                };
            protected:
                void SetUp() override {
                    spi::ClientContext context(client);
                    ASSERT_EQ_EVENTUALLY(2, context.getConnectionManager().getActiveConnections().size());
                }

                void TearDown() override {
                    // clear maps
                    employees->destroy().get();
                    intMap->destroy().get();
                    imap->destroy().get();
                    client.getMap(MapClientConfig::ONE_SECOND_MAP_NAME)->destroy().get();
                    client.getMap("tradeMap")->destroy().get();
                }

                void fillMap() {
                    for (int i = 0; i < 10; i++) {
                        std::string key = "key";
                        key += std::to_string(i);
                        std::string value = "value";
                        value += std::to_string(i);
                        imap->put(key, value).get();
                    }
                }

                static void tryPutThread(hazelcast::util::ThreadArgs &args) {
                    auto *latch1 = (boost::latch *) args.arg0;
                    IMap *pMap = (IMap *) args.arg1;
                    bool result = pMap->tryPut("key1", "value3", std::chrono::seconds(1)).get();
                    if (!result) {
                        latch1->count_down();
                    }
                }

                static void tryRemoveThread(hazelcast::util::ThreadArgs &args) {
                    auto *latch1 = (boost::latch *) args.arg0;
                    IMap *pMap = (IMap *) args.arg1;
                    bool result = pMap->tryRemove("key2", std::chrono::seconds(1)).get();
                    if (!result) {
                        latch1->count_down();
                    }
                }

                static void testLockThread(hazelcast::util::ThreadArgs &args) {
                    auto *latch1 = (boost::latch *) args.arg0;
                    IMap *pMap = (IMap *) args.arg1;
                    pMap->tryPut("key1", "value2", std::chrono::milliseconds(1)).get();
                    latch1->count_down();
                }

                static void testLockTTLThread(hazelcast::util::ThreadArgs &args) {
                    auto *latch1 = (boost::latch *) args.arg0;
                    IMap *pMap = (IMap *) args.arg1;
                    pMap->tryPut("key1", "value2", std::chrono::seconds(5)).get();
                    latch1->count_down();
                }

                static void testLockTTL2Thread(hazelcast::util::ThreadArgs &args) {
                    auto *latch1 = (boost::latch *) args.arg0;
                    IMap *pMap = (IMap *) args.arg1;
                    if (!pMap->tryLock("key1").get()) {
                        latch1->count_down();
                    }
                    if (pMap->tryLock("key1", std::chrono::seconds(5)).get()) {
                        latch1->count_down();
                    }
                }

                static void testMapTryLockThread1(hazelcast::util::ThreadArgs &args) {
                    auto *latch1 = (boost::latch *) args.arg0;
                    IMap *pMap = (IMap *) args.arg1;
                    if (!pMap->tryLock("key1", std::chrono::milliseconds(2)).get()) {
                        latch1->count_down();
                    }
                }

                static void testMapTryLockThread2(hazelcast::util::ThreadArgs &args) {
                    auto *latch1 = (boost::latch *) args.arg0;
                    IMap *pMap = (IMap *) args.arg1;
                    if (pMap->tryLock("key1", std::chrono::seconds(20)).get()) {
                        latch1->count_down();
                    }
                }

                static void testMapForceUnlockThread(hazelcast::util::ThreadArgs &args) {
                    auto *latch1 = (boost::latch *) args.arg0;
                    IMap *pMap = (IMap *) args.arg1;
                    pMap->forceUnlock("key1").get();
                    latch1->count_down();
                }

                EntryListener makeCountdownListener(boost::latch &addLatch,
                                                    boost::latch &removeLatch,
                                                    boost::latch &updateLatch,
                                                    boost::latch &expiryLatch,
                                                    boost::latch *evictLatch = nullptr) {
                    return EntryListener().
                        on_added([&addLatch](EntryEvent &&) {
                            addLatch.count_down();
                        }).
                        on_removed([&removeLatch](EntryEvent &&) {
                            removeLatch.count_down();
                        }).
                        on_updated([&updateLatch](EntryEvent &&) {
                            updateLatch.count_down();
                        }).
                        on_expired([&expiryLatch](EntryEvent &&) {
                            expiryLatch.count_down();
                        }).
                        on_evicted([evictLatch](EntryEvent &&) {
                            if (!evictLatch) {
                                return;
                            }
                            evictLatch->count_down();
                        });
                }

                void validate_expiry_invalidations(std::shared_ptr<IMap> map, std::function<void()> f) {
                    boost::latch dummy(10), expiry(1);
                    auto id = map->addEntryListener(makeCountdownListener(dummy, dummy, dummy, expiry), false).get();

                    auto nearCacheStatsImpl = std::static_pointer_cast<monitor::impl::NearCacheStatsImpl>(
                            map->getLocalMapStats().getNearCacheStats());

                    int64_t initialInvalidationRequests = 0;
                    if (nearCacheStatsImpl) {
                        initialInvalidationRequests = nearCacheStatsImpl->getInvalidationRequests();
                    }

                    f();

                    // populate near cache
                    map->get<std::string, std::string>("key1").get();

                    // if near cache is enabled
                    if (nearCacheStatsImpl) {
                        ASSERT_TRUE_EVENTUALLY(
                                initialInvalidationRequests < nearCacheStatsImpl->getInvalidationRequests());
                        // When ttl expires at server, the server sends another invalidation.
                        ASSERT_EQ_EVENTUALLY(
                                initialInvalidationRequests + 2, nearCacheStatsImpl->getInvalidationRequests());
                    } else {
                        // trigger eviction
                        ASSERT_TRUE_EVENTUALLY((!map->get<std::string, std::string>("key1").get().has_value()));
                    }

                    ASSERT_OPEN_EVENTUALLY(expiry);
                    ASSERT_TRUE(map->removeEntryListener(id).get());
                }

                HazelcastClient client;
                std::shared_ptr<IMap> imap;
                std::shared_ptr<IMap> intMap;
                std::shared_ptr<IMap> employees;

                static HazelcastServer *instance;
                static HazelcastServer *instance2;
            };

            HazelcastServer *ClientMapTest::instance = nullptr;
            HazelcastServer *ClientMapTest::instance2 = nullptr;

            INSTANTIATE_TEST_SUITE_P(ClientMapTestWithDifferentConfigs, ClientMapTest,
                                     ::testing::Values(MapClientConfig(), NearCachedDataMapClientConfig(),
                                                       NearCachedObjectMapClientConfig()));

            TEST_P(ClientMapTest, testIssue537) {
                boost::latch latch1(2);
                boost::latch nullLatch(1);

                EntryListener listener;

                listener.
                    on_added([&latch1](EntryEvent &&) {
                        latch1.count_down();
                    }).
                    on_expired([&latch1, &nullLatch](EntryEvent &&event) {
                        auto oldValue = event.getOldValue().get<std::string>();
                        if (!oldValue.has_value() || oldValue.value().compare("")) {
                            nullLatch.count_down();
                        }
                        latch1.count_down();
                    });

                auto id = imap->addEntryListener(std::move(listener), true).get();

                imap->put<std::string, std::string>("key1", "value1", std::chrono::seconds(2)).get();

                ASSERT_OPEN_EVENTUALLY(latch1);
                ASSERT_OPEN_EVENTUALLY(nullLatch);

                ASSERT_TRUE(imap->removeEntryListener(id).get());

                imap->put<std::string, std::string>("key2", "value2").get();
                ASSERT_EQ(1, imap->size().get());
            }

            TEST_P(ClientMapTest, testContains) {
                fillMap();

                ASSERT_FALSE(imap->containsKey("key10").get());
                ASSERT_TRUE(imap->containsKey("key1").get());

                ASSERT_FALSE(imap->containsValue("value10").get());
                ASSERT_TRUE(imap->containsValue("value1").get());
            }

            TEST_P(ClientMapTest, testGet) {
                fillMap();
                for (int i = 0; i < 10; i++) {
                    std::string key = "key";
                    key += std::to_string(i);
                    boost::optional<std::string> temp = imap->get<std::string, std::string>(key).get();
                    ASSERT_TRUE(temp.has_value());
                    std::string value = "value";
                    value += std::to_string(i);
                    ASSERT_EQ(temp.value(), value);
                }
            }

            TEST_P(ClientMapTest, testPartitionAwareKey) {
                int partitionKey = 5;
                int value = 25;
                std::shared_ptr<IMap> map = client.getMap(MapClientConfig::intMapName);
                PartitionAwareInt partitionAwareInt(partitionKey, 7);
                map->put(partitionAwareInt, value).get();
                boost::optional<int> val = map->get<PartitionAwareInt, int>(partitionAwareInt).get();
                ASSERT_TRUE(val.has_value());
                ASSERT_EQ(*val, value);
            }

            TEST_P(ClientMapTest, testRemoveAndDelete) {
                fillMap();
                boost::optional<std::string> temp = imap->remove<std::string, std::string>("key10").get();
                ASSERT_FALSE(temp.has_value());
                imap->deleteEntry("key9").get();
                ASSERT_EQ(imap->size().get(), 9);
                for (int i = 0; i < 9; i++) {
                    std::string key = "key";
                    key += std::to_string(i);
                    boost::optional<std::string> temp2 = imap->remove<std::string, std::string>(key).get();
                    std::string value = "value";
                    value += std::to_string(i);
                    ASSERT_EQ(*temp2, value);
                }
                ASSERT_EQ(imap->size().get(), 0);
            }

            TEST_P(ClientMapTest, testRemoveIfSame) {
                fillMap();

                ASSERT_FALSE(imap->remove("key2", "value").get());
                ASSERT_EQ(10, imap->size().get());

                ASSERT_TRUE((imap->remove("key2", "value2").get()));
                ASSERT_EQ(9, imap->size().get());
            }

            TEST_P(ClientMapTest, testRemoveAll) {
                fillMap();

                imap->removeAll(
                        query::EqualPredicate(client, query::QueryConstants::KEY_ATTRIBUTE_NAME, "key5")).get();

                boost::optional<std::string> value = imap->get<std::string, std::string>("key5").get();

                ASSERT_FALSE(value.has_value()) << "key5 should not exist";

                imap->removeAll(
                        query::LikePredicate(client, query::QueryConstants::THIS_ATTRIBUTE_NAME, "value%")).get();

                ASSERT_TRUE(imap->isEmpty().get());
            }

            TEST_P(ClientMapTest, testGetAllPutAll) {
                std::unordered_map<std::string, std::string> mapTemp;

                for (int i = 0; i < 100; i++) {
                    mapTemp[std::to_string(i)] = std::to_string(i);
                }
                ASSERT_EQ(imap->size().get(), 0);
                imap->putAll(mapTemp).get();
                ASSERT_EQ(imap->size().get(), 100);

                for (int i = 0; i < 100; i++) {
                    std::string expected = std::to_string(i);
                    boost::optional<std::string> actual = imap->get<std::string, std::string>(
                            std::to_string(i)).get();
                    ASSERT_EQ(expected, *actual);
                }

                std::unordered_set<std::string> tempSet;
                tempSet.insert(std::to_string(1));
                tempSet.insert(std::to_string(3));
                std::unordered_map<std::string, std::string> m2 = imap->getAll<std::string, std::string>(tempSet).get();

                ASSERT_EQ(2U, m2.size());
                ASSERT_EQ(m2[std::to_string(1)], "1");
                ASSERT_EQ(m2[std::to_string(3)], "3");

            }

            TEST_P(ClientMapTest, testTryPutRemove) {
                ASSERT_TRUE(imap->tryPut("key1", "value1", std::chrono::seconds(1)).get());
                ASSERT_TRUE(imap->tryPut("key2", "value2", std::chrono::seconds(1)).get());
                imap->lock("key1").get();
                imap->lock("key2").get();

                boost::latch latch1(2);

                hazelcast::util::StartedThread t1(tryPutThread, &latch1, imap.get());
                hazelcast::util::StartedThread t2(tryRemoveThread, &latch1, imap.get());

                ASSERT_OPEN_EVENTUALLY(latch1);
                ASSERT_EQ("value1", (imap->get<std::string, std::string>("key1").get().value()));
                ASSERT_EQ("value2", (imap->get<std::string, std::string>("key2").get().value()));
                imap->forceUnlock("key1").get();
                imap->forceUnlock("key2").get();
            }

            TEST_P(ClientMapTest, testPutTtl) {
                validate_expiry_invalidations(imap, [=] () { imap->put<std::string, std::string>("key1", "value1", std::chrono::seconds(1)).get(); });
            }

            TEST_P(ClientMapTest, testPutConfigTtl) {
                std::shared_ptr<IMap> map = client.getMap(MapClientConfig::ONE_SECOND_MAP_NAME);
                validate_expiry_invalidations(map, [=] () { map->put<std::string, std::string>("key1", "value1").get(); });
            }

            TEST_P(ClientMapTest, testPutIfAbsent) {
                boost::optional<std::string> o = imap->putIfAbsent<std::string, std::string>("key1", "value1").get();
                ASSERT_FALSE(o.has_value());
                ASSERT_EQ("value1", (imap->putIfAbsent<std::string, std::string>("key1", "value3").get().value()));
            }

            TEST_P(ClientMapTest, testPutIfAbsentTtl) {
                ASSERT_FALSE((imap->putIfAbsent<std::string, std::string>("key1", "value1", std::chrono::seconds(1)).get().has_value()));
                ASSERT_EQ("value1", (imap->putIfAbsent<std::string, std::string>("key1", "value3", std::chrono::seconds(1)).get().value()));

                ASSERT_FALSE_EVENTUALLY((imap->putIfAbsent<std::string, std::string>("key1", "value3", std::chrono::seconds(1)).get().has_value()));
                ASSERT_EQ("value3", (imap->putIfAbsent<std::string, std::string>("key1", "value4", std::chrono::seconds(1)).get().value()));
            }

            TEST_P(ClientMapTest, testSet) {
                imap->set("key1", "value1").get();
                ASSERT_EQ("value1", (imap->get<std::string, std::string>("key1").get().value()));

                imap->set("key1", "value2").get();
                ASSERT_EQ("value2", (imap->get<std::string, std::string>("key1").get().value()));
            }

            TEST_P(ClientMapTest, testSetTtl) {
                validate_expiry_invalidations(imap, [=] () { imap->set("key1", "value1", std::chrono::seconds(1)).get(); });
            }

            TEST_P(ClientMapTest, testSetConfigTtl) {
                std::shared_ptr<IMap> map = client.getMap(MapClientConfig::ONE_SECOND_MAP_NAME);
                validate_expiry_invalidations(map, [=] () { map->set("key1", "value1").get(); });
            }

            TEST_P(ClientMapTest, testLock) {
                imap->put<std::string, std::string>("key1", "value1").get();
                ASSERT_EQ("value1", (imap->get<std::string, std::string>("key1").get().value()));
                imap->lock("key1").get();
                boost::latch latch1(1);
                hazelcast::util::StartedThread t1(testLockThread, &latch1, imap.get());
                ASSERT_OPEN_EVENTUALLY(latch1);
                ASSERT_EQ("value1", (imap->get<std::string, std::string>("key1").get().value()));
                imap->forceUnlock("key1").get();
            }

            TEST_P(ClientMapTest, testLockTtl) {
                imap->put<std::string, std::string>("key1", "value1").get();
                ASSERT_EQ("value1", (imap->get<std::string, std::string>("key1").get().value()));
                imap->lock("key1", std::chrono::seconds(2)).get();
                boost::latch latch1(1);
                hazelcast::util::StartedThread t1(testLockTTLThread, &latch1, imap.get());
                ASSERT_OPEN_EVENTUALLY(latch1);
                ASSERT_FALSE(imap->isLocked("key1").get());
                ASSERT_EQ("value2", (imap->get<std::string, std::string>("key1").get().value()));
                imap->forceUnlock("key1").get();
            }

            TEST_P(ClientMapTest, testLockTtl2) {
                imap->lock("key1", std::chrono::seconds(3)).get();
                boost::latch latch1(2);
                hazelcast::util::StartedThread t1(testLockTTL2Thread, &latch1, imap.get());
                ASSERT_OPEN_EVENTUALLY(latch1);
                imap->forceUnlock("key1").get();
            }

            TEST_P(ClientMapTest, testTryLock) {
                ASSERT_TRUE(imap->tryLock("key1", std::chrono::seconds(2)).get());
                boost::latch latch1(1);
                hazelcast::util::StartedThread t1(testMapTryLockThread1, &latch1, imap.get());

                ASSERT_OPEN_EVENTUALLY(latch1);

                ASSERT_TRUE(imap->isLocked("key1").get());

                boost::latch latch2(1);
                hazelcast::util::StartedThread t2(testMapTryLockThread2, &latch2, imap.get());

                std::this_thread::sleep_for(std::chrono::seconds(1));
                imap->unlock("key1").get();
                ASSERT_OPEN_EVENTUALLY(latch2);
                ASSERT_TRUE(imap->isLocked("key1").get());
                imap->forceUnlock("key1").get();
            }

            TEST_P(ClientMapTest, testTryLockTtl) {
                ASSERT_TRUE(imap->tryLock("key1", std::chrono::seconds(2), std::chrono::seconds(1)).get());
                boost::latch latch1(1);
                hazelcast::util::StartedThread t1(testMapTryLockThread1, &latch1, imap.get());

                ASSERT_OPEN_EVENTUALLY(latch1);

                ASSERT_TRUE(imap->isLocked("key1").get());

                boost::latch latch2(1);
                hazelcast::util::StartedThread t2(testMapTryLockThread2, &latch2, imap.get());

                ASSERT_OPEN_EVENTUALLY(latch2);
                ASSERT_TRUE(imap->isLocked("key1").get());
                imap->forceUnlock("key1").get();
            }

            TEST_P(ClientMapTest, testTryLockTtlTimeout) {
                ASSERT_TRUE(imap->tryLock("key1", std::chrono::seconds(2), std::chrono::seconds(200)).get());
                boost::latch latch1(1);
                hazelcast::util::StartedThread t1(testMapTryLockThread1, &latch1, imap.get());

                ASSERT_OPEN_EVENTUALLY(latch1);

                ASSERT_TRUE(imap->isLocked("key1").get());
                imap->forceUnlock("key1").get();
            }

            TEST_P(ClientMapTest, testForceUnlock) {
                imap->lock("key1").get();
                boost::latch latch1(1);
                hazelcast::util::StartedThread t2(testMapForceUnlockThread, &latch1, imap.get());
                ASSERT_OPEN_EVENTUALLY(latch1);
                t2.join();
                ASSERT_FALSE(imap->isLocked("key1").get());
            }

            TEST_P(ClientMapTest, testJsonValues) {
                const int numItems = 5;
                for (int i = 0; i < numItems; ++i) {
                    imap->put("key_" + std::to_string(i), HazelcastJsonValue("{ \"value\"=\"value_" + std::to_string(i) + "\"}")).get();
                }
                auto values = imap->values<HazelcastJsonValue>().get();
                ASSERT_EQ(numItems, (int) values.size());
            }

            /**
             * Fails with `HazelcastSerializationException {Not comparable { "value"="value_2"}`
             * The HazelcastJsonValue should be comparable at Java server side to make it work.
             */
            TEST_P(ClientMapTest, DISABLED_testJsonValuesWithPagingPredicate) {
                const int numItems = 5;
                const int predSize = 3;
                for (int i = 0; i < numItems; ++i) {
                    imap->put<std::string, HazelcastJsonValue>("key_" + std::to_string(i), HazelcastJsonValue(
                            "{ \"value\"=\"value_" + std::to_string(i) + "\"}")).get();
                }
                auto predicate = imap->newPagingPredicate<std::string, HazelcastJsonValue>((size_t) predSize);
                auto values = imap->values<std::string, HazelcastJsonValue>(predicate).get();
                ASSERT_EQ(predSize, (int) values.size());
            }


            TEST_P(ClientMapTest, testValues) {
                fillMap();
                query::SqlPredicate predicate(client, "this == value1");
                std::vector<std::string> tempVector = imap->values<std::string>(predicate).get();
                ASSERT_EQ(1U, tempVector.size());
                ASSERT_EQ("value1", tempVector[0]);
            }

            TEST_P(ClientMapTest, testValuesWithPredicate) {
                const int numItems = 20;
                for (int i = 0; i < numItems; ++i) {
                    intMap->put(i, 2 * i).get();
                }

                auto values = intMap->values<int>().get();
                ASSERT_EQ(numItems, (int) values.size());
                std::sort(values.begin(), values.end());
                for (int i = 0; i < numItems; ++i) {
                    ASSERT_EQ(2 * i, values[i]);
                }

                // EqualPredicate
                // key == 5
                values = intMap->values<int>(
                        query::EqualPredicate(client, query::QueryConstants::KEY_ATTRIBUTE_NAME, 5)).get();
                ASSERT_EQ(1, (int) values.size());
                ASSERT_EQ(2 * 5, values[0]);

                // value == 8
                values = intMap->values<int>(
                        query::EqualPredicate(client, query::QueryConstants::THIS_ATTRIBUTE_NAME, 8)).get();
                ASSERT_EQ(1, (int) values.size());
                ASSERT_EQ(8, values[0]);

                // key == numItems
                values = intMap->values<int>(
                        query::EqualPredicate(client, query::QueryConstants::KEY_ATTRIBUTE_NAME, numItems)).get();
                ASSERT_EQ(0, (int) values.size());

                // NotEqual Predicate
                // key != 5
                values = intMap->values<int>(
                        query::NotEqualPredicate(client, query::QueryConstants::KEY_ATTRIBUTE_NAME, 5)).get();
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
                values = intMap->values<int>(
                        query::NotEqualPredicate(client, query::QueryConstants::THIS_ATTRIBUTE_NAME, 8)).get();
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
                values = intMap->values<int>(query::TruePredicate(client)).get();
                ASSERT_EQ(numItems, (int) values.size());
                std::sort(values.begin(), values.end());
                for (int i = 0; i < numItems; ++i) {
                    ASSERT_EQ(2 * i, values[i]);
                }

                // FalsePredicate
                values = intMap->values<int>(query::FalsePredicate(client)).get();
                ASSERT_EQ(0, (int) values.size());

                // BetweenPredicate
                // 5 <= key <= 10
                values = intMap->values<int>(
                        query::BetweenPredicate(client, query::QueryConstants::KEY_ATTRIBUTE_NAME, 5, 10)).get();
                std::sort(values.begin(), values.end());
                ASSERT_EQ(6, (int) values.size());
                for (int i = 0; i < 6; ++i) {
                    ASSERT_EQ(2 * (i + 5), values[i]);
                }

                // 20 <= key <=30
                values = intMap->values<int>(
                        query::BetweenPredicate(client, query::QueryConstants::KEY_ATTRIBUTE_NAME, 20, 30)).get();
                ASSERT_EQ(0, (int) values.size());

                // GreaterLessPredicate
                // value <= 10
                values = intMap->values<int>(
                        query::GreaterLessPredicate(client, query::QueryConstants::THIS_ATTRIBUTE_NAME, 10, true,
                                                         true)).get();
                ASSERT_EQ(6, (int) values.size());
                std::sort(values.begin(), values.end());
                for (int i = 0; i < 6; ++i) {
                    ASSERT_EQ(2 * i, values[i]);
                }

                // key < 7
                values = intMap->values<int>(
                        query::GreaterLessPredicate(client, query::QueryConstants::KEY_ATTRIBUTE_NAME, 7, false, true)).get();
                ASSERT_EQ(7, (int) values.size());
                std::sort(values.begin(), values.end());
                for (int i = 0; i < 7; ++i) {
                    ASSERT_EQ(2 * i, values[i]);
                }

                // value >= 15
                values = intMap->values<int>(
                        query::GreaterLessPredicate(client, query::QueryConstants::THIS_ATTRIBUTE_NAME, 15, true,
                                                         false)).get();
                ASSERT_EQ(12, (int) values.size());
                std::sort(values.begin(), values.end());
                for (int i = 0; i < 12; ++i) {
                    ASSERT_EQ(2 * (i + 8), values[i]);
                }

                // key > 5
                values = intMap->values<int>(
                        query::GreaterLessPredicate(client, query::QueryConstants::KEY_ATTRIBUTE_NAME, 5, false,
                                                         false)).get();
                ASSERT_EQ(14, (int) values.size());
                std::sort(values.begin(), values.end());
                for (int i = 0; i < 14; ++i) {
                    ASSERT_EQ(2 * (i + 6), values[i]);
                }

                // InPredicate
                // key in {4, 10, 19}
                values = intMap->values<int>(
                        query::InPredicate(client, query::QueryConstants::KEY_ATTRIBUTE_NAME, 4, 10, 19)).get();
                ASSERT_EQ(3, (int) values.size());
                std::sort(values.begin(), values.end());
                ASSERT_EQ(2 * 4, values[0]);
                ASSERT_EQ(2 * 10, values[1]);
                ASSERT_EQ(2 * 19, values[2]);

                // value in {4, 10, 19}
                values = intMap->values<int>(
                        query::InPredicate(client, query::QueryConstants::THIS_ATTRIBUTE_NAME, 4, 10, 19)).get();
                ASSERT_EQ(2, (int) values.size());
                std::sort(values.begin(), values.end());
                ASSERT_EQ(4, values[0]);
                ASSERT_EQ(10, values[1]);

                // InstanceOfPredicate
                // value instanceof Integer
                values = intMap->values<int>(query::InstanceOfPredicate(client, "java.lang.Integer")).get();
                ASSERT_EQ(20, (int) values.size());
                std::sort(values.begin(), values.end());
                for (int i = 0; i < numItems; ++i) {
                    ASSERT_EQ(2 * i, values[i]);
                }

                values = intMap->values<int>(query::InstanceOfPredicate(client, "java.lang.String")).get();
                ASSERT_EQ(0, (int) values.size());

                // NotPredicate
                // !(5 <= key <= 10)
                query::NotPredicate notPredicate(client, query::BetweenPredicate(client,
                                                                                 query::QueryConstants::KEY_ATTRIBUTE_NAME,
                                                                                 5, 10));
                values = intMap->values<int>(notPredicate).get();
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
                values = intMap->values<int>(query::AndPredicate(client, query::BetweenPredicate(client,
                                                                                                 query::QueryConstants::KEY_ATTRIBUTE_NAME,
                                                                                                 5, 10),
                                                                 query::InPredicate(client,
                                                                                    query::QueryConstants::THIS_ATTRIBUTE_NAME,
                                                                                    4, 10, 19))).get();
                ASSERT_EQ(1, (int) values.size());
                std::sort(values.begin(), values.end());
                ASSERT_EQ(10, values[0]);

                // OrPredicate
                // 5 <= key <= 10 OR Values in {4, 10, 19} = values {4, 10, 12, 14, 16, 18, 20}
                values = intMap->values<int>(query::OrPredicate(client, query::BetweenPredicate(client,
                                                                                                query::QueryConstants::KEY_ATTRIBUTE_NAME,
                                                                                                5, 10),
                                                                query::InPredicate(client,
                                                                                   query::QueryConstants::THIS_ATTRIBUTE_NAME,
                                                                                   4, 10, 19))).get();
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
                    key += std::to_string(i);
                    std::string value = "value";
                    value += std::to_string(i);
                    imap->put(key, value).get();
                }
                imap->put<std::string, std::string>("key_111_test", "myvalue_111_test").get();
                imap->put<std::string, std::string>("key_22_test", "myvalue_22_test").get();

                // LikePredicate
                // value LIKE "value1" : {"value1"}
                std::vector<std::string> strValues = imap->values<std::string>(
                        query::LikePredicate(client, query::QueryConstants::THIS_ATTRIBUTE_NAME, "value1")).get();
                ASSERT_EQ(1, (int) strValues.size());
                ASSERT_EQ("value1", strValues[0]);

                // ILikePredicate
                // value ILIKE "%VALue%1%" : {"myvalue_111_test", "value1", "value10", "value11"}
                strValues = imap->values<std::string>(
                        query::ILikePredicate(client, query::QueryConstants::THIS_ATTRIBUTE_NAME, "%VALue%1%")).get();
                ASSERT_EQ(4, (int) strValues.size());
                std::sort(strValues.begin(), strValues.end());
                ASSERT_EQ("myvalue_111_test", strValues[0]);
                ASSERT_EQ("value1", strValues[1]);
                ASSERT_EQ("value10", strValues[2]);
                ASSERT_EQ("value11", strValues[3]);

                // value ILIKE "%VAL%2%" : {"myvalue_22_test", "value2"}
                strValues = imap->values<std::string>(
                        query::ILikePredicate(client, query::QueryConstants::THIS_ATTRIBUTE_NAME, "%VAL%2%")).get();
                ASSERT_EQ(2, (int) strValues.size());
                std::sort(strValues.begin(), strValues.end());
                ASSERT_EQ("myvalue_22_test", strValues[0]);
                ASSERT_EQ("value2", strValues[1]);

                // SqlPredicate
                // __key BETWEEN 4 and 7 : {4, 5, 6, 7} -> {8, 10, 12, 14}
                char sql[100];
                hazelcast::util::hz_snprintf(sql, 50, "%s BETWEEN 4 and 7",
                                             query::QueryConstants::KEY_ATTRIBUTE_NAME);
                values = intMap->values<int>(query::SqlPredicate(client, sql)).get();
                ASSERT_EQ(4, (int) values.size());
                std::sort(values.begin(), values.end());
                for (int i = 0; i < 4; ++i) {
                    ASSERT_EQ(2 * (i + 4), values[i]);
                }

                // RegexPredicate
                // value matches the regex ".*value.*2.*" : {myvalue_22_test, value2}
                strValues = imap->values<std::string>(
                        query::RegexPredicate(client, query::QueryConstants::THIS_ATTRIBUTE_NAME, ".*value.*2.*")).get();
                ASSERT_EQ(2, (int) strValues.size());
                std::sort(strValues.begin(), strValues.end());
                ASSERT_EQ("myvalue_22_test", strValues[0]);
                ASSERT_EQ("value2", strValues[1]);
            }

            TEST_P(ClientMapTest, testValuesWithPagingPredicate) {
                int predSize = 5;
                const int totalEntries = 25;

                for (int i = 0; i < totalEntries; ++i) {
                    intMap->put(i, i).get();
                }

                auto predicate = intMap->newPagingPredicate<int, int>((size_t) predSize);

                std::vector<int> values = intMap->values<int>(predicate).get();
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    ASSERT_EQ(i, values[i]);
                }

                values = intMap->values<int>(predicate).get();
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    ASSERT_EQ(i, values[i]);
                }

                predicate.nextPage();
                values = intMap->values<int>(predicate).get();
                ASSERT_EQ(predSize, (int) values.size());

                for (int i = 0; i < predSize; ++i) {
                    ASSERT_EQ(predSize + i, values[i]);
                }

                ASSERT_EQ(1, (int) predicate.getPage());

                predicate.setPage(4);

                values = intMap->values<int>(predicate).get();
                ASSERT_EQ(predSize, (int) values.size());

                for (int i = 0; i < predSize; ++i) {
                    ASSERT_EQ(predSize * 4 + i, values[i]);
                }

                predicate.nextPage();
                values = intMap->values<int>(predicate).get();
                ASSERT_EQ(0, (int) values.size());

                predicate.setPage(0);
                values = intMap->values<int>(predicate).get();
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    ASSERT_EQ(i, values[i]);
                }

                predicate.previousPage();
                ASSERT_EQ(0, (int) predicate.getPage());

                predicate.setPage(5);
                values = intMap->values<int>(predicate).get();
                ASSERT_EQ(0, (int) values.size());

                predicate.setPage(3);
                values = intMap->values<int>(predicate).get();
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    ASSERT_EQ(3 * predSize + i, values[i]);
                }

                predicate.previousPage();
                values = intMap->values<int>(predicate).get();
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    ASSERT_EQ(2 * predSize + i, values[i]);
                }

                // test PagingPredicate with inner predicate (value < 10)
                query::GreaterLessPredicate lessThanTenPredicate(client, query::QueryConstants::THIS_ATTRIBUTE_NAME, 9, false, true);
                auto predicate2 = intMap->newPagingPredicate<int, int>(5, lessThanTenPredicate);
                values = intMap->values<int>(predicate2).get();
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    ASSERT_EQ(i, values[i]);
                }

                predicate2.nextPage();
                // match values 5,6, 7, 8
                values = intMap->values<int>(predicate2).get();
                ASSERT_EQ(predSize - 1, (int) values.size());
                for (int i = 0; i < predSize - 1; ++i) {
                    ASSERT_EQ(predSize + i, values[i]);
                }

                predicate2.nextPage();
                values = intMap->values<int>(predicate2).get();
                ASSERT_EQ(0, (int) values.size());

                // test paging predicate with comparator
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);
                Employee empl3("deniz", 25);
                Employee empl4("ali", 33);
                Employee empl5("veli", 44);
                Employee empl6("aylin", 5);

                employees->put(3, empl1).get();
                employees->put(4, empl2).get();
                employees->put(5, empl3).get();
                employees->put(6, empl4).get();
                employees->put(7, empl5).get();
                employees->put(8, empl6).get();

                predSize = 2;
                auto predicate3 = intMap->newPagingPredicate<int, Employee>(EmployeeEntryComparator(), (size_t) predSize);
                std::vector<Employee> result = employees->values<int, Employee>(predicate3).get();
                ASSERT_EQ(2, (int) result.size());
                ASSERT_EQ(empl6, result[0]);
                ASSERT_EQ(empl2, result[1]);

                predicate3.nextPage();
                result = employees->values<int, Employee>(predicate3).get();
                ASSERT_EQ(2, (int) result.size());
                ASSERT_EQ(empl3, result[0]);
                ASSERT_EQ(empl4, result[1]);
            }

            TEST_P(ClientMapTest, testKeySetWithPredicate) {
                const int numItems = 20;
                for (int i = 0; i < numItems; ++i) {
                    intMap->put(i, 2 * i).get();
                }

                std::vector<int> keys = intMap->keySet<int>().get();
                ASSERT_EQ(numItems, (int) keys.size());
                std::sort(keys.begin(), keys.end());
                for (int i = 0; i < numItems; ++i) {
                    ASSERT_EQ(i, keys[i]);
                }

                // EqualPredicate
                // key == 5
                keys = intMap->keySet<int>(
                        query::EqualPredicate(client, query::QueryConstants::KEY_ATTRIBUTE_NAME, 5)).get();
                ASSERT_EQ(1, (int) keys.size());
                ASSERT_EQ(5, keys[0]);

                // value == 8
                keys = intMap->keySet<int>(
                        query::EqualPredicate(client, query::QueryConstants::THIS_ATTRIBUTE_NAME, 8)).get();
                ASSERT_EQ(1, (int) keys.size());
                ASSERT_EQ(4, keys[0]);

// key == numItems
                keys = intMap->keySet<int>(
                        query::EqualPredicate(client, query::QueryConstants::KEY_ATTRIBUTE_NAME, numItems)).get();
                ASSERT_EQ(0, (int) keys.size());

// NotEqual Predicate
// key != 5
                keys = intMap->keySet<int>(
                        query::NotEqualPredicate(client, query::QueryConstants::KEY_ATTRIBUTE_NAME, 5)).get();
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
                keys = intMap->keySet<int>(
                        query::NotEqualPredicate(client, query::QueryConstants::THIS_ATTRIBUTE_NAME, 8)).get();
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
                keys = intMap->keySet<int>(query::TruePredicate(client)).get();
                ASSERT_EQ(numItems, (int) keys.size());
                std::sort(keys.begin(), keys.end());
                for (int i = 0; i < numItems; ++i) {
                    ASSERT_EQ(i, keys[i]);
                }

// FalsePredicate
                keys = intMap->keySet<int>(query::FalsePredicate(client)).get();
                ASSERT_EQ(0, (int) keys.size());

// BetweenPredicate
// 5 <= key <= 10
                keys = intMap->keySet<int>(
                        query::BetweenPredicate(client, query::QueryConstants::KEY_ATTRIBUTE_NAME, 5, 10)).get();
                std::sort(keys.begin(), keys.end());
                ASSERT_EQ(6, (int) keys.size());
                for (int i = 0; i < 6; ++i) {
                    ASSERT_EQ((i + 5), keys[i]);
                }

// 20 <= key <=30
                keys = intMap->keySet<int>(
                        query::BetweenPredicate(client, query::QueryConstants::KEY_ATTRIBUTE_NAME, 20, 30)).get();
                ASSERT_EQ(0, (int) keys.size());

// GreaterLessPredicate
// value <= 10
                keys = intMap->keySet<int>(
                        query::GreaterLessPredicate(client, query::QueryConstants::THIS_ATTRIBUTE_NAME, 10, true,
                                                         true)).get();
                ASSERT_EQ(6, (int) keys.size());
                std::sort(keys.begin(), keys.end());
                for (int i = 0; i < 6; ++i) {
                    ASSERT_EQ(i, keys[i]);
                }

// key < 7
                keys = intMap->keySet<int>(
                        query::GreaterLessPredicate(client, query::QueryConstants::KEY_ATTRIBUTE_NAME, 7, false, true)).get();
                ASSERT_EQ(7, (int) keys.size());
                std::sort(keys.begin(), keys.end());
                for (int i = 0; i < 7; ++i) {
                    ASSERT_EQ(i, keys[i]);
                }

// value >= 15
                keys = intMap->keySet<int>(
                        query::GreaterLessPredicate(client, query::QueryConstants::THIS_ATTRIBUTE_NAME, 15, true,
                                                         false)).get();
                ASSERT_EQ(12, (int) keys.size());
                std::sort(keys.begin(), keys.end());
                for (int i = 0; i < 12; ++i) {
                    ASSERT_EQ(i + 8, keys[i]);
                }

// key > 5
                keys = intMap->keySet<int>(
                        query::GreaterLessPredicate(client, query::QueryConstants::KEY_ATTRIBUTE_NAME, 5, false,
                                                         false)).get();
                ASSERT_EQ(14, (int) keys.size());
                std::sort(keys.begin(), keys.end());
                for (int i = 0; i < 14; ++i) {
                    ASSERT_EQ(i + 6, keys[i]);
                }

// InPredicate
// key in {4, 10, 19}
                keys = intMap->keySet<int>(
                        query::InPredicate(client, query::QueryConstants::KEY_ATTRIBUTE_NAME, 4, 10, 19)).get();
                ASSERT_EQ(3, (int) keys.size());
                std::sort(keys.begin(), keys.end());
                ASSERT_EQ(4, keys[0]);
                ASSERT_EQ(10, keys[1]);
                ASSERT_EQ(19, keys[2]);

// value in {4, 10, 19}
                keys = intMap->keySet<int>(
                        query::InPredicate(client, query::QueryConstants::THIS_ATTRIBUTE_NAME, 4, 10, 19)).get();
                ASSERT_EQ(2, (int) keys.size());
                std::sort(keys.begin(), keys.end());
                ASSERT_EQ(2, keys[0]);
                ASSERT_EQ(5, keys[1]);

// InstanceOfPredicate
// value instanceof Integer
                keys = intMap->keySet<int>(query::InstanceOfPredicate(client, "java.lang.Integer")).get();
                ASSERT_EQ(20, (int) keys.size());
                std::sort(keys.begin(), keys.end());
                for (int i = 0; i < numItems; ++i) {
                    ASSERT_EQ(i, keys[i]);
                }

                keys = intMap->keySet<int>(query::InstanceOfPredicate(client, "java.lang.String")).get();
                ASSERT_EQ(0, (int) keys.size());

                // NotPredicate
                // !(5 <= key <= 10)
                query::NotPredicate notPredicate(client, query::BetweenPredicate(client, query::QueryConstants::KEY_ATTRIBUTE_NAME, 5, 10));
                keys = intMap->keySet<int>(notPredicate).get();
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
                keys = intMap->keySet<int>(query::AndPredicate(client,
                        query::BetweenPredicate(client, query::QueryConstants::KEY_ATTRIBUTE_NAME, 5, 10),
                        query::InPredicate(client, query::QueryConstants::THIS_ATTRIBUTE_NAME, 4, 10, 19))).get();
                ASSERT_EQ(1, (int) keys.size());
                std::sort(keys.begin(), keys.end());
                ASSERT_EQ(5, keys[0]);

// OrPredicate
// 5 <= key <= 10 OR Values in {4, 10, 19} = keys {2, 5, 6, 7, 8, 9, 10}
                keys = intMap->keySet<int>(query::OrPredicate(client, query::BetweenPredicate(client, query::QueryConstants::KEY_ATTRIBUTE_NAME, 5, 10), query::InPredicate(client, query::QueryConstants::THIS_ATTRIBUTE_NAME, 4, 10, 19))).get();
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
                    key += std::to_string(i);
                    std::string value = "value";
                    value += std::to_string(i);
                    imap->put<std::string, std::string>(key, value).get();
                }
                imap->put<std::string, std::string>("key_111_test", "myvalue_111_test").get();
                imap->put<std::string, std::string>("key_22_test", "myvalue_22_test").get();

// LikePredicate
// value LIKE "value1" : {"value1"}
                std::vector<std::string> strKeys = imap->keySet<std::string>(
                        query::LikePredicate(client, query::QueryConstants::THIS_ATTRIBUTE_NAME, "value1")).get();
                ASSERT_EQ(1, (int) strKeys.size());
                ASSERT_EQ("key1", strKeys[0]);

// ILikePredicate
// value ILIKE "%VALue%1%" : {"key_111_test", "key1", "key10", "key11"}
                strKeys = imap->keySet<std::string>(
                        query::ILikePredicate(client, query::QueryConstants::THIS_ATTRIBUTE_NAME, "%VALue%1%")).get();
                ASSERT_EQ(4, (int) strKeys.size());
                std::sort(strKeys.begin(), strKeys.end());
                ASSERT_EQ("key1", strKeys[0]);
                ASSERT_EQ("key10", strKeys[1]);
                ASSERT_EQ("key11", strKeys[2]);
                ASSERT_EQ("key_111_test", strKeys[3]);

// key ILIKE "%VAL%2%" : {"key_22_test", "key2"}
                strKeys = imap->keySet<std::string>(
                        query::ILikePredicate(client, query::QueryConstants::THIS_ATTRIBUTE_NAME, "%VAL%2%")).get();
                ASSERT_EQ(2, (int) strKeys.size());
                std::sort(strKeys.begin(), strKeys.end());
                ASSERT_EQ("key2", strKeys[0]);
                ASSERT_EQ("key_22_test", strKeys[1]);

// SqlPredicate
// __key BETWEEN 4 and 7 : {4, 5, 6, 7} -> {8, 10, 12, 14}
                char sql[100];
                hazelcast::util::hz_snprintf(sql, 50, "%s BETWEEN 4 and 7",
                                             query::QueryConstants::KEY_ATTRIBUTE_NAME);
                keys = intMap->keySet<int>(query::SqlPredicate(client, sql)).get();
                ASSERT_EQ(4, (int) keys.size());
                std::sort(keys.begin(), keys.end());
                for (int i = 0; i < 4; ++i) {
                    ASSERT_EQ(i + 4, keys[i]);
                }

// RegexPredicate
// value matches the regex ".*value.*2.*" : {key_22_test, value2}
                strKeys = imap->keySet<std::string>(
                        query::RegexPredicate(client, query::QueryConstants::THIS_ATTRIBUTE_NAME, ".*value.*2.*")).get();
                ASSERT_EQ(2, (int) strKeys.size());
                std::sort(strKeys.begin(), strKeys.end());
                ASSERT_EQ("key2", strKeys[0]);
                ASSERT_EQ("key_22_test", strKeys[1]);
            }

            TEST_P(ClientMapTest, testKeySetWithPagingPredicate) {
                int predSize = 5;
                const int totalEntries = 25;

                for (int i = 0; i < totalEntries; ++i) {
                    intMap->put(i, i).get();
                }

                auto predicate = intMap->newPagingPredicate<int, int>((size_t) predSize);

                std::vector<int> values = intMap->keySet<int>(predicate).get();
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    ASSERT_EQ(i, values[i]);
                }

                values = intMap->keySet<int>(predicate).get();
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    ASSERT_EQ(i, values[i]);
                }

                predicate.nextPage();
                values = intMap->keySet<int>(predicate).get();
                ASSERT_EQ(predSize, (int) values.size());

                for (int i = 0; i < predSize; ++i) {
                    ASSERT_EQ(predSize + i, values[i]);
                }

                ASSERT_EQ(1, (int) predicate.getPage());

                predicate.setPage(4);

                values = intMap->keySet<int>(predicate).get();
                ASSERT_EQ(predSize, (int) values.size());

                for (int i = 0; i < predSize; ++i) {
                    ASSERT_EQ(predSize * 4 + i, values[i]);
                }

                predicate.nextPage();
                values = intMap->keySet<int>(predicate).get();
                ASSERT_EQ(0, (int) values.size());

                predicate.setPage(0);
                values = intMap->keySet<int>(predicate).get();
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    ASSERT_EQ(i, values[i]);
                }

                predicate.previousPage();
                ASSERT_EQ(0, (int) predicate.getPage());

                predicate.setPage(5);
                values = intMap->keySet<int>(predicate).get();
                ASSERT_EQ(0, (int) values.size());

                predicate.setPage(3);
                values = intMap->keySet<int>(predicate).get();
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    ASSERT_EQ(3 * predSize + i, values[i]);
                }

                predicate.previousPage();
                values = intMap->keySet<int>(predicate).get();
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    ASSERT_EQ(2 * predSize + i, values[i]);
                }

// test PagingPredicate with inner predicate (value < 10)
                query::GreaterLessPredicate lessThanTenPredicate(client, query::QueryConstants::THIS_ATTRIBUTE_NAME, 9, false, true);
                auto predicate2 = intMap->newPagingPredicate<int, int>(5, lessThanTenPredicate);
                values = intMap->keySet<int>(predicate2).get();
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    ASSERT_EQ(i, values[i]);
                }

                predicate2.nextPage();
// match values 5,6, 7, 8
                values = intMap->keySet<int>(predicate2).get();
                ASSERT_EQ(predSize - 1, (int) values.size());
                for (int i = 0; i < predSize - 1; ++i) {
                    ASSERT_EQ(predSize + i, values[i]);
                }

                predicate2.nextPage();
                values = intMap->keySet<int>(predicate2).get();
                ASSERT_EQ(0, (int) values.size());

                // test paging predicate with comparator
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);
                Employee empl3("deniz", 25);
                Employee empl4("ali", 33);
                Employee empl5("veli", 44);
                Employee empl6("aylin", 5);

                employees->put(3, empl1).get();
                employees->put(4, empl2).get();
                employees->put(5, empl3).get();
                employees->put(6, empl4).get();
                employees->put(7, empl5).get();
                employees->put(8, empl6).get();

                predSize = 2;
                auto predicate3 = intMap->newPagingPredicate<int, Employee>(EmployeeEntryKeyComparator(), (size_t) predSize);
                std::vector<int> result = employees->keySet<int>(predicate3).get();
                // since keyset result only returns keys from the server, no ordering based on the value but ordered based on the keys
                ASSERT_EQ(2, (int) result.size());
                ASSERT_EQ(3, result[0]);
                ASSERT_EQ(4, result[1]);

                predicate3.nextPage();
                result = employees->keySet<int>(predicate3).get();
                ASSERT_EQ(2, (int) result.size());
                ASSERT_EQ(5, result[0]);
                ASSERT_EQ(6, result[1]);
            }

            TEST_P(ClientMapTest, testEntrySetWithPredicate) {
                const int numItems = 20;
                std::vector<std::pair<int, int> > expected(numItems);
                for (int i = 0; i < numItems; ++i) {
                    intMap->put(i, 2 * i).get();
                    expected[i] = std::pair<int, int>(i, 2 * i);
                }

                std::vector<std::pair<int, int> > entries = intMap->entrySet<int, int>().get();
                ASSERT_EQ(numItems, (int) entries.size());
                std::sort(entries.begin(), entries.end());
                for (int i = 0; i < numItems; ++i) {
                    ASSERT_EQ(expected[i], entries[i]);
                }

// EqualPredicate
// key == 5
                entries = intMap->entrySet<int, int>(
                        query::EqualPredicate(client, query::QueryConstants::KEY_ATTRIBUTE_NAME, 5)).get();
                ASSERT_EQ(1, (int) entries.size());
                ASSERT_EQ(expected[5], entries[0]);

// value == 8
                entries = intMap->entrySet<int, int>(
                        query::EqualPredicate(client, query::QueryConstants::THIS_ATTRIBUTE_NAME, 8)).get();
                ASSERT_EQ(1, (int) entries.size());
                ASSERT_EQ(expected[4], entries[0]);

// key == numItems
                entries = intMap->entrySet<int, int>(
                        query::EqualPredicate(client, query::QueryConstants::KEY_ATTRIBUTE_NAME, numItems)).get();
                ASSERT_EQ(0, (int) entries.size());

// NotEqual Predicate
// key != 5
                entries = intMap->entrySet<int, int>(
                        query::NotEqualPredicate(client, query::QueryConstants::KEY_ATTRIBUTE_NAME, 5)).get();
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
                entries = intMap->entrySet<int, int>(
                        query::NotEqualPredicate(client, query::QueryConstants::THIS_ATTRIBUTE_NAME, 8)).get();
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
                entries = intMap->entrySet<int, int>(query::TruePredicate(client)).get();
                ASSERT_EQ(numItems, (int) entries.size());
                std::sort(entries.begin(), entries.end());
                for (int i = 0; i < numItems; ++i) {
                    ASSERT_EQ(expected[i], entries[i]);
                }

// FalsePredicate
                entries = intMap->entrySet<int, int>(query::FalsePredicate(client)).get();
                ASSERT_EQ(0, (int) entries.size());

// BetweenPredicate
// 5 <= key <= 10
                entries = intMap->entrySet<int, int>(
                        query::BetweenPredicate(client, query::QueryConstants::KEY_ATTRIBUTE_NAME, 5, 10)).get();
                std::sort(entries.begin(), entries.end());
                ASSERT_EQ(6, (int) entries.size());
                for (int i = 0; i < 6; ++i) {
                    ASSERT_EQ(expected[i + 5], entries[i]);
                }

// 20 <= key <=30
                entries = intMap->entrySet<int, int>(
                        query::BetweenPredicate(client, query::QueryConstants::KEY_ATTRIBUTE_NAME, 20, 30)).get();
                ASSERT_EQ(0, (int) entries.size());

// GreaterLessPredicate
// value <= 10
                entries = intMap->entrySet<int, int>(
                        query::GreaterLessPredicate(client, query::QueryConstants::THIS_ATTRIBUTE_NAME, 10, true,
                                                         true)).get();
                ASSERT_EQ(6, (int) entries.size());
                std::sort(entries.begin(), entries.end());
                for (int i = 0; i < 6; ++i) {
                    ASSERT_EQ(expected[i], entries[i]);
                }

// key < 7
                entries = intMap->entrySet<int, int>(
                        query::GreaterLessPredicate(client, query::QueryConstants::KEY_ATTRIBUTE_NAME, 7, false, true)).get();
                ASSERT_EQ(7, (int) entries.size());
                std::sort(entries.begin(), entries.end());
                for (int i = 0; i < 7; ++i) {
                    ASSERT_EQ(expected[i], entries[i]);
                }

// value >= 15
                entries = intMap->entrySet<int, int>(
                        query::GreaterLessPredicate(client, query::QueryConstants::THIS_ATTRIBUTE_NAME, 15, true,
                                                         false)).get();
                ASSERT_EQ(12, (int) entries.size());
                std::sort(entries.begin(), entries.end());
                for (int i = 0; i < 12; ++i) {
                    ASSERT_EQ(expected[i + 8], entries[i]);
                }

// key > 5
                entries = intMap->entrySet<int, int>(
                        query::GreaterLessPredicate(client, query::QueryConstants::KEY_ATTRIBUTE_NAME, 5, false,
                                                         false)).get();
                ASSERT_EQ(14, (int) entries.size());
                std::sort(entries.begin(), entries.end());
                for (int i = 0; i < 14; ++i) {
                    ASSERT_EQ(expected[i + 6], entries[i]);
                }

                // InPredicate
                // key in {4, 10, 19}
                entries = intMap->entrySet<int, int>(
                        query::InPredicate(client, query::QueryConstants::KEY_ATTRIBUTE_NAME, 4, 10, 19)).get();
                ASSERT_EQ(3, (int) entries.size());
                std::sort(entries.begin(), entries.end());
                ASSERT_EQ(expected[4], entries[0]);
                ASSERT_EQ(expected[10], entries[1]);
                ASSERT_EQ(expected[19], entries[2]);

// value in {4, 10, 19}
                entries = intMap->entrySet<int, int>(
                        query::InPredicate(client, query::QueryConstants::THIS_ATTRIBUTE_NAME, 4, 10, 19)).get();
                ASSERT_EQ(2, (int) entries.size());
                std::sort(entries.begin(), entries.end());
                ASSERT_EQ(expected[2], entries[0]);
                ASSERT_EQ(expected[5], entries[1]);

// InstanceOfPredicate
// value instanceof Integer
                entries = intMap->entrySet<int, int>(query::InstanceOfPredicate(client, "java.lang.Integer")).get();
                ASSERT_EQ(20, (int) entries.size());
                std::sort(entries.begin(), entries.end());
                for (int i = 0; i < numItems; ++i) {
                    ASSERT_EQ(expected[i], entries[i]);
                }

                entries = intMap->entrySet<int, int>(query::InstanceOfPredicate(client, "java.lang.String")).get();
                ASSERT_EQ(0, (int) entries.size());

                // NotPredicate
                // !(5 <= key <= 10)
                query::NotPredicate notPredicate(client, query::BetweenPredicate(client, query::QueryConstants::KEY_ATTRIBUTE_NAME, 5, 10));
                entries = intMap->entrySet<int, int>(notPredicate).get();
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
                entries = intMap->entrySet<int, int>(query::AndPredicate(client, query::BetweenPredicate(client,
                        query::QueryConstants::KEY_ATTRIBUTE_NAME, 5, 10),
                                query::InPredicate(client, query::QueryConstants::THIS_ATTRIBUTE_NAME, 4, 10, 19))).get();
                ASSERT_EQ(1, (int) entries.size());
                std::sort(entries.begin(), entries.end());
                ASSERT_EQ(expected[5], entries[0]);

// OrPredicate
// 5 <= key <= 10 OR Values in {4, 10, 19} = entries {2, 5, 6, 7, 8, 9, 10}
                entries = intMap->entrySet<int, int>(query::OrPredicate(client,
                        query::BetweenPredicate(client, query::QueryConstants::KEY_ATTRIBUTE_NAME, 5, 10),
                        query::InPredicate(client, query::QueryConstants::THIS_ATTRIBUTE_NAME, 4, 10, 19))).get();
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
                    key += std::to_string(i);
                    std::string value = "value";
                    value += std::to_string(i);
                    imap->put<std::string, std::string>(key, value).get();
                    expectedStrEntries[i] = std::pair<std::string, std::string>(key, value);
                }
                imap->put<std::string, std::string>("key_111_test", "myvalue_111_test").get();
                expectedStrEntries[12] = std::pair<std::string, std::string>("key_111_test", "myvalue_111_test");
                imap->put<std::string, std::string>("key_22_test", "myvalue_22_test").get();
                expectedStrEntries[13] = std::pair<std::string, std::string>("key_22_test", "myvalue_22_test");

// LikePredicate
// value LIKE "value1" : {"value1"}
                std::vector<std::pair<std::string, std::string> > strEntries = imap->entrySet<std::string, std::string>(
                        query::LikePredicate(client, query::QueryConstants::THIS_ATTRIBUTE_NAME, "value1")).get();
                ASSERT_EQ(1, (int) strEntries.size());
                ASSERT_EQ(expectedStrEntries[1], strEntries[0]);

// ILikePredicate
// value ILIKE "%VALue%1%" : {"key_111_test", "key1", "key10", "key11"}
                strEntries = imap->entrySet<std::string, std::string>(
                        query::ILikePredicate(client, query::QueryConstants::THIS_ATTRIBUTE_NAME, "%VALue%1%")).get();
                ASSERT_EQ(4, (int) strEntries.size());
                std::sort(strEntries.begin(), strEntries.end());
                ASSERT_EQ(expectedStrEntries[1], strEntries[0]);
                ASSERT_EQ(expectedStrEntries[10], strEntries[1]);
                ASSERT_EQ(expectedStrEntries[11], strEntries[2]);
                ASSERT_EQ(expectedStrEntries[12], strEntries[3]);

// key ILIKE "%VAL%2%" : {"key_22_test", "key2"}
                strEntries = imap->entrySet<std::string, std::string>(
                        query::ILikePredicate(client, query::QueryConstants::THIS_ATTRIBUTE_NAME, "%VAL%2%")).get();
                ASSERT_EQ(2, (int) strEntries.size());
                std::sort(strEntries.begin(), strEntries.end());
                ASSERT_EQ(expectedStrEntries[2], strEntries[0]);
                ASSERT_EQ(expectedStrEntries[13], strEntries[1]);

// SqlPredicate
// __key BETWEEN 4 and 7 : {4, 5, 6, 7} -> {8, 10, 12, 14}
                char sql[100];
                hazelcast::util::hz_snprintf(sql, 50, "%s BETWEEN 4 and 7",
                                             query::QueryConstants::KEY_ATTRIBUTE_NAME);
                entries = intMap->entrySet<int, int>(query::SqlPredicate(client, sql)).get();
                ASSERT_EQ(4, (int) entries.size());
                std::sort(entries.begin(), entries.end());
                for (int i = 0; i < 4; ++i) {
                    ASSERT_EQ(expected[i + 4], entries[i]);
                }

// RegexPredicate
// value matches the regex ".*value.*2.*" : {key_22_test, value2}
                strEntries = imap->entrySet<std::string, std::string>(
                        query::RegexPredicate(client, query::QueryConstants::THIS_ATTRIBUTE_NAME, ".*value.*2.*")).get();
                ASSERT_EQ(2, (int) strEntries.size());
                std::sort(strEntries.begin(), strEntries.end());
                ASSERT_EQ(expectedStrEntries[2], strEntries[0]);
                ASSERT_EQ(expectedStrEntries[13], strEntries[1]);
            }

            TEST_P(ClientMapTest, testEntrySetWithPagingPredicate) {
                int predSize = 5;
                const int totalEntries = 25;

                for (int i = 0; i < totalEntries; ++i) {
                    intMap->put(i, i).get();
                }

                auto predicate = intMap->newPagingPredicate<int, int>((size_t) predSize);

                std::vector<std::pair<int, int> > values = intMap->entrySet<int, int>(predicate).get();
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    std::pair<int, int> value(i, i);
                    ASSERT_EQ(value, values[i]);
                }

                values = intMap->entrySet<int, int>(predicate).get();
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    std::pair<int, int> value(i, i);
                    ASSERT_EQ(value, values[i]);
                }

                predicate.nextPage();
                values = intMap->entrySet<int, int>(predicate).get();
                ASSERT_EQ(predSize, (int) values.size());

                for (int i = 0; i < predSize; ++i) {
                    std::pair<int, int> value(predSize + i, predSize + i);
                    ASSERT_EQ(value, values[i]);
                }

                ASSERT_EQ(1, (int) predicate.getPage());

                predicate.setPage(4);

                values = intMap->entrySet<int, int>(predicate).get();
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    std::pair<int, int> value(predSize * 4 + i, predSize * 4 + i);
                    ASSERT_EQ(value, values[i]);
                }

                predicate.nextPage();
                values = intMap->entrySet<int, int>(predicate).get();
                ASSERT_EQ(0, (int) values.size());

                predicate.setPage(0);
                values = intMap->entrySet<int, int>(predicate).get();
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    std::pair<int, int> value(i, i);
                    ASSERT_EQ(value, values[i]);
                }

                predicate.previousPage();
                ASSERT_EQ(0, (int) predicate.getPage());

                predicate.setPage(5);
                values = intMap->entrySet<int, int>(predicate).get();
                ASSERT_EQ(0, (int) values.size());

                predicate.setPage(3);
                values = intMap->entrySet<int, int>(predicate).get();
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    std::pair<int, int> value(3 * predSize + i, 3 * predSize + i);
                    ASSERT_EQ(value, values[i]);
                }

                predicate.previousPage();
                values = intMap->entrySet<int, int>(predicate).get();
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    std::pair<int, int> value(2 * predSize + i, 2 * predSize + i);
                    ASSERT_EQ(value, values[i]);
                }

// test PagingPredicate with inner predicate (value < 10)
                query::GreaterLessPredicate lessThanTenPredicate(client, query::QueryConstants::THIS_ATTRIBUTE_NAME, 9, false, true);
                auto predicate2 = intMap->newPagingPredicate<int, int>(5, lessThanTenPredicate);
                values = intMap->entrySet<int, int>(predicate2).get();
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    std::pair<int, int> value(i, i);
                    ASSERT_EQ(value, values[i]);
                }

                predicate2.nextPage();
// match values 5,6, 7, 8
                values = intMap->entrySet<int, int>(predicate2).get();
                ASSERT_EQ(predSize - 1, (int) values.size());
                for (int i = 0; i < predSize - 1; ++i) {
                    std::pair<int, int> value(predSize + i, predSize + i);
                    ASSERT_EQ(value, values[i]);
                }

                predicate2.nextPage();
                values = intMap->entrySet<int, int>(predicate2).get();
                ASSERT_EQ(0, (int) values.size());

// test paging predicate with comparator
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);
                Employee empl3("deniz", 25);
                Employee empl4("ali", 33);
                Employee empl5("veli", 44);
                Employee empl6("aylin", 5);

                employees->put(3, empl1).get();
                employees->put(4, empl2).get();
                employees->put(5, empl3).get();
                employees->put(6, empl4).get();
                employees->put(7, empl5).get();
                employees->put(8, empl6).get();

                predSize = 2;
                auto predicate3 = intMap->newPagingPredicate<int, Employee>(EmployeeEntryComparator(), (size_t) predSize);
                std::vector<std::pair<int, Employee> > result = employees->entrySet<int, Employee>(
                        predicate3).get();
                ASSERT_EQ(2, (int) result.size());
                std::pair<int, Employee> value(8, empl6);
                ASSERT_EQ(value, result[0]);
                value = std::pair<int, Employee>(4, empl2);
                ASSERT_EQ(value, result[1]);

                predicate3.nextPage();
                result = employees->entrySet<int, Employee>(predicate3).get();
                ASSERT_EQ(2, (int) result.size());
                value = std::pair<int, Employee>(5, empl3);
                ASSERT_EQ(value, result[0]);
                value = std::pair<int, Employee>(6, empl4);
                ASSERT_EQ(value, result[1]);
            }

            TEST_P(ClientMapTest, testReplace) {
                boost::optional<std::string> temp = imap->replace<std::string, std::string>("key1", "value").get();
                ASSERT_FALSE(temp);

                std::string tempKey = "key1";
                std::string tempValue = "value1";
                imap->put<std::string, std::string>(tempKey, tempValue).get();

                ASSERT_EQ("value1", (imap->replace<std::string, std::string>("key1", "value2").get().value()));
                ASSERT_EQ("value2", (imap->get<std::string, std::string>("key1").get().value()));

                ASSERT_FALSE((imap->replace<std::string, std::string>("key1", "value1", "value3").get()));
                ASSERT_EQ("value2", (imap->get<std::string, std::string>("key1").get().value()));

                ASSERT_TRUE((imap->replace<std::string, std::string>("key1", "value2", "value3").get()));
                ASSERT_EQ("value3", (imap->get<std::string, std::string>("key1").get().value()));
            }

            TEST_P(ClientMapTest, testListenerWithPortableKey) {
                std::shared_ptr<IMap> tradeMap = client.getMap("tradeMap");
                boost::latch countDownLatch(1);
                std::atomic<int> atomicInteger(0);

                EntryListener listener;
                listener.
                    on_added([&countDownLatch, &atomicInteger](EntryEvent &&event) {
                        ++atomicInteger;
                        countDownLatch.count_down();
                    });

                Employee key("a", 1);
                auto id = tradeMap->addEntryListener(std::move(listener), true, key).get();
                Employee key2("a", 2);
                tradeMap->put<Employee, int>(key2, 1).get();
                tradeMap->put<Employee, int>(key, 3).get();
                ASSERT_OPEN_EVENTUALLY(countDownLatch);
                ASSERT_EQ(1, (int) atomicInteger);

                ASSERT_TRUE(tradeMap->removeEntryListener(id).get());
            }

            TEST_P(ClientMapTest, testListener) {
                boost::latch latch1Add(5);
                boost::latch latch1Remove(2);
                boost::latch dummy(10);
                boost::latch latch2Add(1);
                boost::latch latch2Remove(1);

                auto listener1 = makeCountdownListener(
                        latch1Add, latch1Remove, dummy, dummy);
                auto listener2 = makeCountdownListener(
                        latch2Add, latch2Remove, dummy, dummy);

                auto listener1ID = imap->addEntryListener(std::move(listener1), false).get();
                auto listener2ID = imap->addEntryListener(std::move(listener2), true, "key3").get();

                std::this_thread::sleep_for(std::chrono::seconds(2));

                imap->put<std::string, std::string>("key1", "value1").get();
                imap->put<std::string, std::string>("key2", "value2").get();
                imap->put<std::string, std::string>("key3", "value3").get();
                imap->put<std::string, std::string>("key4", "value4").get();
                imap->put<std::string, std::string>("key5", "value5").get();

                imap->remove<std::string, std::string>("key1").get();
                imap->remove<std::string, std::string>("key3").get();

                ASSERT_OPEN_EVENTUALLY(latch1Add);
                ASSERT_OPEN_EVENTUALLY(latch1Remove);
                ASSERT_OPEN_EVENTUALLY(latch2Add);
                ASSERT_OPEN_EVENTUALLY(latch2Remove);

                ASSERT_TRUE(imap->removeEntryListener(listener1ID).get());
                ASSERT_TRUE(imap->removeEntryListener(listener2ID).get());
            }

            TEST_P(ClientMapTest, testListenerWithTruePredicate) {
                boost::latch latchAdd(3), latchRemove(1), latchEvict(1), latchUpdate(1);

                auto listener = makeCountdownListener(latchAdd, latchRemove, latchUpdate, latchEvict);

                auto listenerId = intMap->addEntryListener(std::move(listener), query::TruePredicate(client), false).get();

                intMap->put(1, 1).get();
                intMap->put(2, 2).get();
                intMap->put(3, 3, std::chrono::seconds(1)).get(); // evict after 1 second
                intMap->remove<int, int>(2).get();

                std::this_thread::sleep_for(std::chrono::seconds(2));

                ASSERT_FALSE((intMap->get<int, int>(3).get().has_value())); // trigger eviction

                // update an entry
                intMap->set(1, 5).get();
                boost::optional<int> value = intMap->get<int, int>(1).get();
                ASSERT_TRUE(value.has_value());
                ASSERT_EQ(5, value.value());

                CountDownLatchWaiter latches;
                latches.add(latchAdd).add(latchRemove).add(latchUpdate).add(latchEvict);
                ASSERT_EQ(boost::cv_status::no_timeout, latches.wait_for(boost::chrono::seconds(2)));

                ASSERT_TRUE(intMap->removeEntryListener(listenerId).get());
            }

            TEST_P(ClientMapTest, testListenerWithFalsePredicate) {
                boost::latch latchAdd(3), latchRemove(1), latchEvict(1), latchUpdate(1);

                auto listener = makeCountdownListener(latchAdd, latchRemove, latchUpdate, latchEvict);

                auto listenerId = intMap->addEntryListener(std::move(listener), query::FalsePredicate(client), false).get();

                intMap->put(1, 1).get();
                intMap->put(2, 2).get();
                intMap->put(3, 3, std::chrono::seconds(1)).get(); // evict after 1 second
                intMap->remove<int, int>(2).get();

                std::this_thread::sleep_for(std::chrono::seconds(2));

                ASSERT_FALSE((intMap->get<int, int>(3).get().has_value())); // trigger eviction

// update an entry
                intMap->set(1, 5).get();
                boost::optional<int> value = intMap->get<int, int>(1).get();
                ASSERT_TRUE(value.has_value());
                ASSERT_EQ(5, value.value());

                CountDownLatchWaiter latches;
                latches.add(latchAdd).add(latchRemove).add(latchUpdate).add(latchEvict);
                ASSERT_EQ(boost::cv_status::timeout, latches.wait_for(boost::chrono::seconds(2)));

                ASSERT_TRUE(intMap->removeEntryListener(listenerId).get());
            }

            TEST_P(ClientMapTest, testListenerWithEqualPredicate) {
                boost::latch latchAdd(1), latchRemove(1), latchEvict(1), latchUpdate(1);

                auto listener = makeCountdownListener(latchAdd, latchRemove, latchUpdate, latchEvict);

                auto listenerId = intMap->addEntryListener(std::move(listener),
                                                                 query::EqualPredicate(client, 
                                                                         query::QueryConstants::KEY_ATTRIBUTE_NAME,
                                                                         3), true).get();

                intMap->put(1, 1).get();
                intMap->put(2, 2).get();
                intMap->put(3, 3, std::chrono::seconds(1)).get(); // evict after 1 second
                intMap->remove<int, int>(2).get();

                std::this_thread::sleep_for(std::chrono::seconds(2));

                ASSERT_FALSE((intMap->get<int, int>(3).get().has_value())); // trigger eviction

// update an entry
                intMap->set(1, 5).get();
                boost::optional<int> value = intMap->get<int, int>(1).get();
                ASSERT_TRUE(value.has_value());
                ASSERT_EQ(5, value.value());

                CountDownLatchWaiter latches;
                latches.add(latchAdd).add(latchEvict);
                ASSERT_EQ(boost::cv_status::no_timeout, latches.wait_for(boost::chrono::seconds(2)));

                latches.reset();
                latches.add(latchUpdate).add(latchRemove);
                ASSERT_EQ(boost::cv_status::timeout, latches.wait_for(boost::chrono::seconds(2)));

                ASSERT_TRUE(intMap->removeEntryListener(listenerId).get());
            }

            TEST_P(ClientMapTest, testListenerWithNotEqualPredicate) {
                boost::latch latchAdd(2), latchRemove(1), latchEvict(1), latchUpdate(1);

                auto listener = makeCountdownListener(latchAdd, latchRemove, latchUpdate, latchEvict);

                auto listenerId = intMap->addEntryListener(std::move(listener),
                                                                 query::NotEqualPredicate(client, 
                                                                         query::QueryConstants::KEY_ATTRIBUTE_NAME,
                                                                         3), true).get();

                intMap->put(1, 1).get();
                intMap->put(2, 2).get();
                intMap->put(3, 3, std::chrono::seconds(1)).get(); // evict after 1 second
                intMap->remove<int, int>(2).get();

                std::this_thread::sleep_for(std::chrono::seconds(2));

                ASSERT_FALSE((intMap->get<int, int>(3).get().has_value())); // trigger eviction

// update an entry
                intMap->set(1, 5).get();
                boost::optional<int> value = intMap->get<int, int>(1).get();
                ASSERT_TRUE(value.has_value());
                ASSERT_EQ(5, value.value());

                CountDownLatchWaiter latches;
                latches.add(latchAdd).add(latchRemove).add(latchUpdate);
                ASSERT_EQ(boost::cv_status::no_timeout, latches.wait_for(boost::chrono::seconds(2)));

                latches.reset();
                latches.add(latchEvict);
                ASSERT_EQ(boost::cv_status::timeout, latches.wait_for(boost::chrono::seconds(2)));

                ASSERT_TRUE(intMap->removeEntryListener(listenerId).get());
            }

            TEST_P(ClientMapTest, testListenerWithGreaterLessPredicate) {
                boost::latch latchAdd(2), latchRemove(1), latchEvict(1), latchUpdate(1);

                auto listener = makeCountdownListener(latchAdd, latchRemove,
                                                     latchUpdate,
                                                     latchEvict);
// key <= 2
                auto listenerId = intMap->addEntryListener(std::move(listener),
                                                                 query::GreaterLessPredicate(client, 
                                                                         query::QueryConstants::KEY_ATTRIBUTE_NAME,
                                                                         2, true, true),
                                                                 false).get();

                intMap->put(1, 1).get();
                intMap->put(2, 2).get();
                intMap->put(3, 3, std::chrono::seconds(1)).get(); // evict after 1 second
                intMap->remove<int, int>(2).get();

                std::this_thread::sleep_for(std::chrono::seconds(2));

                ASSERT_FALSE((intMap->get<int, int>(3).get().has_value())); // trigger eviction

// update an entry
                intMap->set(1, 5).get();
                boost::optional<int> value = intMap->get<int, int>(1).get();
                ASSERT_TRUE(value.has_value());
                ASSERT_EQ(5, value.value());

                CountDownLatchWaiter latches;
                latches.add(latchAdd).add(latchRemove).add(latchUpdate);
                ASSERT_EQ(boost::cv_status::no_timeout, latches.wait_for(boost::chrono::seconds(2)));

                ASSERT_EQ(boost::cv_status::timeout, latchEvict.wait_for(boost::chrono::seconds(2)));

                ASSERT_TRUE(intMap->removeEntryListener(listenerId).get());
            }

            TEST_P(ClientMapTest, testListenerWithBetweenPredicate) {
                boost::latch latchAdd(2), latchRemove(1), latchEvict(1), latchUpdate(1);

                auto listener = makeCountdownListener(latchAdd, latchRemove, latchUpdate, latchEvict);

// 1 <=key <= 2
                auto listenerId = intMap->addEntryListener(std::move(listener),
                                                                 query::BetweenPredicate(client, 
                                                                         query::QueryConstants::KEY_ATTRIBUTE_NAME,
                                                                         1, 2), true).get();

                intMap->put(1, 1).get();
                intMap->put(2, 2).get();
                intMap->put(3, 3, std::chrono::seconds(1)).get(); // evict after 1 second
                intMap->remove<int, int>(2).get();

                std::this_thread::sleep_for(std::chrono::seconds(2));

                ASSERT_FALSE((intMap->get<int, int>(3).get().has_value())); // trigger eviction

// update an entry
                intMap->set(1, 5).get();
                boost::optional<int> value = intMap->get<int, int>(1).get();
                ASSERT_TRUE(value.has_value());
                ASSERT_EQ(5, value.value());

                CountDownLatchWaiter latches;
                latches.add(latchAdd).add(latchRemove).add(latchUpdate);
                ASSERT_EQ(boost::cv_status::no_timeout, latches.wait_for(boost::chrono::seconds(2)));

                ASSERT_EQ(boost::cv_status::timeout, latchEvict.wait_for(boost::chrono::seconds(2)));

                ASSERT_TRUE(intMap->removeEntryListener(listenerId).get());
            }

            TEST_P(ClientMapTest, testListenerWithSqlPredicate) {
                boost::latch latchAdd(1), latchRemove(1), latchEvict(1), latchUpdate(1);

                auto listener = makeCountdownListener(latchAdd, latchRemove, latchUpdate, latchEvict);

// 1 <=key <= 2
                auto listenerId = intMap->addEntryListener(std::move(listener), query::SqlPredicate(client, "__key < 2"), true).get();

                intMap->put(1, 1).get();
                intMap->put(2, 2).get();
                intMap->put(3, 3, std::chrono::seconds(1)).get(); // evict after 1 second
                intMap->remove<int, int>(2).get();

                std::this_thread::sleep_for(std::chrono::seconds(2));

                ASSERT_FALSE((intMap->get<int, int>(3).get().has_value())); // trigger eviction

// update an entry
                intMap->set(1, 5).get();
                boost::optional<int> value = intMap->get<int, int>(1).get();
                ASSERT_TRUE(value.has_value());
                ASSERT_EQ(5, value.value());

                CountDownLatchWaiter latches;
                latches.add(latchAdd).add(latchUpdate);
                ASSERT_EQ(boost::cv_status::no_timeout, latches.wait_for(boost::chrono::seconds(2)));

                latches.reset();
                latches.add(latchRemove).add(latchEvict);
                ASSERT_EQ(boost::cv_status::timeout, latches.wait_for(boost::chrono::seconds(2)));

                ASSERT_TRUE(intMap->removeEntryListener(listenerId).get());
            }

            TEST_P(ClientMapTest, testListenerWithRegExPredicate) {
                boost::latch latchAdd(2), latchRemove(1), latchEvict(1), latchUpdate(1);

                auto listener = makeCountdownListener(
                        latchAdd, latchRemove, latchUpdate, latchEvict);

// key matches any word containing ".*met.*"
                auto listenerId = imap->addEntryListener(std::move(listener),
                                                               query::RegexPredicate(client, 
                                                                       query::QueryConstants::KEY_ATTRIBUTE_NAME,
                                                                       ".*met.*"), true).get();

                imap->put<std::string, std::string>("ilkay", "yasar").get();
                imap->put<std::string, std::string>("mehmet", "demir").get();
                imap->put<std::string, std::string>("metin", "ozen", std::chrono::seconds(1)).get(); // evict after 1 second
                imap->put<std::string, std::string>("hasan", "can").get();
                imap->remove<std::string, std::string>("mehmet").get();

                std::this_thread::sleep_for(std::chrono::seconds(2));

                ASSERT_FALSE((imap->get<std::string, std::string>("metin").get().has_value())); // trigger eviction

// update an entry
                imap->set("hasan", "suphi").get();
                boost::optional<std::string> value = imap->get<std::string, std::string>("hasan").get();
                ASSERT_TRUE(value.has_value());
                ASSERT_EQ("suphi", value.value());

                CountDownLatchWaiter latches;
                latches.add(latchAdd).add(latchRemove).add(latchEvict);
                ASSERT_EQ(boost::cv_status::no_timeout, latches.wait_for(boost::chrono::seconds(2)));

                ASSERT_EQ(boost::cv_status::timeout, latchUpdate.wait_for(boost::chrono::seconds(2)));

                ASSERT_TRUE(imap->removeEntryListener(listenerId).get());
            }

            TEST_P(ClientMapTest, testListenerWithInstanceOfPredicate) {
                boost::latch latchAdd(3), latchRemove(1), latchEvict(1), latchUpdate(1);
                auto listener = makeCountdownListener(latchAdd, latchRemove,latchUpdate,latchEvict);
// 1 <=key <= 2
                auto listenerId = intMap->addEntryListener(std::move(listener),
                                                                 query::InstanceOfPredicate(client, 
                                                                         "java.lang.Integer"),
                                                                 false).get();

                intMap->put(1, 1).get();
                intMap->put(2, 2).get();
                intMap->put(3, 3, std::chrono::seconds(1)).get(); // evict after 1 second
                intMap->remove<int, int>(2).get();

                std::this_thread::sleep_for(std::chrono::seconds(2));

                ASSERT_FALSE((intMap->get<int, int>(3).get().has_value())); // trigger eviction

// update an entry
                intMap->set(1, 5).get();
                boost::optional<int> value = intMap->get<int, int>(1).get();
                ASSERT_TRUE(value.has_value());
                ASSERT_EQ(5, value.value());

                CountDownLatchWaiter latches;
                latches.add(latchAdd).add(latchRemove).add(latchUpdate).add(latchEvict);
                ASSERT_EQ(boost::cv_status::no_timeout, latches.wait_for(boost::chrono::seconds(2)));

                ASSERT_TRUE(intMap->removeEntryListener(listenerId).get());
            }

            TEST_P(ClientMapTest, testListenerWithNotPredicate) {
                boost::latch latchAdd(2), latchRemove(1), latchEvict(1), latchUpdate(1);
                auto listener = makeCountdownListener(latchAdd, latchRemove,latchUpdate,latchEvict);
                // key >= 3
                query::NotPredicate notPredicate(client, query::GreaterLessPredicate(client,
                                                                                     query::QueryConstants::KEY_ATTRIBUTE_NAME,
                                                                                     3, true, false));
                auto listenerId = intMap->addEntryListener(std::move(listener), notPredicate,false).get();

                intMap->put(1, 1).get();
                intMap->put(2, 2).get();
                intMap->put(3, 3, std::chrono::seconds(1)).get(); // evict after 1 second
                intMap->remove<int, int>(2).get();

                std::this_thread::sleep_for(std::chrono::seconds(2));

                ASSERT_FALSE((intMap->get<int, int>(3).get().has_value())); // trigger eviction

// update an entry
                intMap->set(1, 5).get();
                boost::optional<int> value = intMap->get<int, int>(1).get();
                ASSERT_TRUE(value.has_value());
                ASSERT_EQ(5, value.value());

                CountDownLatchWaiter latches;
                latches.add(latchAdd).add(latchRemove).add(latchUpdate);
                ASSERT_EQ(boost::cv_status::no_timeout, latches.wait_for(boost::chrono::seconds(2)));

                latches.reset();
                latches.add(latchEvict);
                ASSERT_EQ(boost::cv_status::timeout, latches.wait_for(boost::chrono::seconds(1)));

                ASSERT_TRUE(intMap->removeEntryListener(listenerId).get());
            }

            TEST_P(ClientMapTest, testListenerWithAndPredicate) {
                boost::latch latchAdd(1), latchRemove(1), latchEvict(1), latchUpdate(1);
                auto listener = makeCountdownListener(latchAdd, latchRemove,latchUpdate,latchEvict);

// key < 3
                query::GreaterLessPredicate greaterLessPred(client, query::QueryConstants::KEY_ATTRIBUTE_NAME, 3, false, true);
// value == 1
                query::EqualPredicate equalPred(client, query::QueryConstants::KEY_ATTRIBUTE_NAME, 1);
// key < 3 AND key == 1 --> (1, 1)
                query::AndPredicate predicate(client, greaterLessPred, equalPred);
                auto listenerId = intMap->addEntryListener(std::move(listener), predicate, false).get();

                intMap->put(1, 1).get();
                intMap->put(2, 2).get();
                intMap->put(3, 3, std::chrono::seconds(1)).get(); // evict after 1 second
                intMap->remove<int, int>(2).get();

                std::this_thread::sleep_for(std::chrono::seconds(2));

                ASSERT_FALSE((intMap->get<int, int>(3).get().has_value())); // trigger eviction

// update an entry
                intMap->set(1, 5).get();
                boost::optional<int> value = intMap->get<int, int>(1).get();
                ASSERT_TRUE(value.has_value());
                ASSERT_EQ(5, value.value());

                CountDownLatchWaiter latches;
                latches.add(latchAdd).add(latchUpdate);
                ASSERT_EQ(boost::cv_status::no_timeout, latches.wait_for(boost::chrono::seconds(2)));

                latches.reset();
                latches.add(latchEvict).add(latchRemove);
                ASSERT_EQ(boost::cv_status::timeout, latches.wait_for(boost::chrono::seconds(1)));

                ASSERT_TRUE(intMap->removeEntryListener(listenerId).get());
            }

            TEST_P(ClientMapTest, testListenerWithOrPredicate) {
                boost::latch latchAdd(2), latchRemove(1), latchEvict(1), latchUpdate(1);

                auto listener = makeCountdownListener(latchAdd, latchRemove,
                                                     latchUpdate,
                                                     latchEvict);

                // key < 3
                query::GreaterLessPredicate greaterLessPred(client, query::QueryConstants::KEY_ATTRIBUTE_NAME, 3, true, false);
                // value == 1
                query::EqualPredicate equalPred(client, query::QueryConstants::THIS_ATTRIBUTE_NAME, 2);
                // key >= 3 OR value == 2 --> (1, 1), (2, 2)
                query::OrPredicate predicate(client, greaterLessPred, equalPred);
                auto listenerId = intMap->addEntryListener(std::move(listener), predicate, true).get();

                intMap->put(1, 1).get();
                intMap->put(2, 2).get();
                intMap->put(3, 3, std::chrono::seconds(1)).get(); // evict after 1 second
                intMap->remove<int, int>(2).get();

                std::this_thread::sleep_for(std::chrono::seconds(2));

                ASSERT_FALSE((intMap->get<int, int>(3).get().has_value())); // trigger eviction

// update an entry
                intMap->set(1, 5).get();
                boost::optional<int> value = intMap->get<int, int>(1).get();
                ASSERT_TRUE(value.has_value());
                ASSERT_EQ(5, value.value());

                CountDownLatchWaiter latches;
                latches.add(latchAdd).add(latchEvict).add(latchRemove);
                ASSERT_EQ(boost::cv_status::no_timeout, latches.wait_for(boost::chrono::seconds(2)));

                ASSERT_EQ(boost::cv_status::timeout, latchUpdate.wait_for(boost::chrono::seconds(2)));

                ASSERT_TRUE(intMap->removeEntryListener(listenerId).get());
            }

            TEST_P(ClientMapTest, testClearEvent) {
                boost::latch latch1(1);

                EntryListener clearListener;
                clearListener.
                    on_map_cleared([&latch1](MapEvent &&) {
                        latch1.count_down();
                    });

                auto listenerId = imap->addEntryListener(std::move(clearListener), false).get();
                imap->put<std::string, std::string>("key1", "value1").get();
                imap->clear().get();
                ASSERT_OPEN_EVENTUALLY(latch1);
                imap->removeEntryListener(listenerId).get();
            }

            TEST_P(ClientMapTest, testEvictAllEvent) {
                boost::latch latch1(1);
                EntryListener evictListener;

                evictListener.
                    on_map_evicted([&latch1](MapEvent &&event) {
                        latch1.count_down();
                    });

                auto listenerId = imap->addEntryListener(std::move(evictListener), false).get();
                imap->put<std::string, std::string>("key1", "value1").get();
                imap->evictAll().get();
                ASSERT_OPEN_EVENTUALLY(latch1);
                imap->removeEntryListener(listenerId).get();
            }

            TEST_P(ClientMapTest, testMapWithPortable) {
                boost::optional<Employee> n1 = employees->get<int, Employee>(1).get();
                ASSERT_FALSE(n1);
                Employee employee("sancar", 24);
                boost::optional<Employee> ptr = employees->put(1, employee).get();
                ASSERT_FALSE(ptr);
                ASSERT_FALSE(employees->isEmpty().get());
                EntryView<int, Employee> view = employees->getEntryView<int, Employee>(1).get().value();
                ASSERT_EQ(view.value, employee);
                ASSERT_EQ(view.key, 1);

                employees->addIndex(config::index_config::index_type::SORTED, std::string("a")).get();
                employees->addIndex(config::index_config::index_type::HASH, std::string("n")).get();
            }

            TEST_P(ClientMapTest, testMapStoreRelatedRequests) {
                imap->putTransient<std::string, std::string>("ali", "veli", std::chrono::milliseconds(1100)).get();
                imap->flush().get();
                ASSERT_EQ(1, imap->size().get());
                ASSERT_FALSE(imap->evict("deli").get());
                ASSERT_TRUE(imap->evict("ali").get());
                ASSERT_FALSE((imap->get<std::string, std::string>("ali").get().has_value()));
            }

            TEST_P(ClientMapTest, testExecuteOnKey) {
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);

                employees->put(3, empl1).get();
                employees->put(4, empl2).get();

                EntryMultiplier processor(4);

                boost::optional<int> result = employees->executeOnKey<int, int, EntryMultiplier>(4, processor).get();

                ASSERT_TRUE(result.has_value());
                ASSERT_EQ(4 * processor.getMultiplier(), result.value());
            }

            TEST_P(ClientMapTest, testSubmitToKey) {
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);

                employees->put(3, empl1).get();
                employees->put(4, empl2).get();

                EntryMultiplier processor(4);

                auto result = employees->submitToKey<int, int, EntryMultiplier>(4, processor).get();
                ASSERT_TRUE(result.has_value());
                ASSERT_EQ(4 * processor.getMultiplier(), result.value());
            }

            TEST_P(ClientMapTest, testExecuteOnNonExistentKey) {
                EntryMultiplier processor(4);

                boost::optional<int> result = employees->executeOnKey<int, int, EntryMultiplier>(
                        17, processor).get();

                ASSERT_TRUE(result.has_value());
                ASSERT_EQ(-1, result.value());
            }

            TEST_P(ClientMapTest, testExecuteOnKeys) {
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);
                Employee empl3("deniz", 25);

                employees->put(3, empl1).get();
                employees->put(4, empl2).get();
                employees->put(5, empl3).get();

                EntryMultiplier processor(4);

                std::unordered_set<int> keys;
                keys.insert(3);
                keys.insert(5);
// put non existent key
                keys.insert(999);

                std::unordered_map<int, boost::optional<int> > result = employees->executeOnKeys<int, int, EntryMultiplier>(
                        keys, processor).get();

                ASSERT_EQ(3, (int) result.size());
                ASSERT_NE(result.end(), result.find(3));
                ASSERT_NE(result.end(), result.find(5));
                ASSERT_NE(result.end(), result.find(999));
                ASSERT_EQ(3 * processor.getMultiplier(), result[3].value());
                ASSERT_EQ(5 * processor.getMultiplier(), result[5].value());
                ASSERT_EQ(-1, result[999].value());
            }

            TEST_P(ClientMapTest, testExecuteOnEntries) {
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);
                Employee empl3("deniz", 25);

                employees->put(3, empl1).get();
                employees->put(4, empl2).get();
                employees->put(5, empl3).get();

                EntryMultiplier processor(4);

                std::unordered_map<int, boost::optional<int> > result = employees->executeOnEntries<int, int, EntryMultiplier>(
                        processor).get();

                ASSERT_EQ(3, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(3)));
                ASSERT_TRUE((result.end() != result.find(4)));
                ASSERT_TRUE((result.end() != result.find(5)));
                ASSERT_EQ(3 * processor.getMultiplier(), result[3].value());
                ASSERT_EQ(4 * processor.getMultiplier(), result[4].value());
                ASSERT_EQ(5 * processor.getMultiplier(), result[5].value());
            }

            TEST_P(ClientMapTest, testExecuteOnEntriesWithTruePredicate) {
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);
                Employee empl3("deniz", 25);

                employees->put(3, empl1).get();
                employees->put(4, empl2).get();
                employees->put(5, empl3).get();

                EntryMultiplier processor(4);

                std::unordered_map<int, boost::optional<int>> result = employees->executeOnEntries<int, int, EntryMultiplier>(
                        processor, query::TruePredicate(client)).get();

                ASSERT_EQ(3, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(3)));
                ASSERT_TRUE((result.end() != result.find(4)));
                ASSERT_TRUE((result.end() != result.find(5)));
                ASSERT_EQ(3 * processor.getMultiplier(), result[3].value());
                ASSERT_EQ(4 * processor.getMultiplier(), result[4].value());
                ASSERT_EQ(5 * processor.getMultiplier(), result[5].value());
            }

            TEST_P(ClientMapTest, testExecuteOnEntriesWithFalsePredicate) {
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);
                Employee empl3("deniz", 25);

                employees->put(3, empl1).get();
                employees->put(4, empl2).get();
                employees->put(5, empl3).get();

                EntryMultiplier processor(4);

                std::unordered_map<int, boost::optional<int> > result = employees->executeOnEntries<int, int, EntryMultiplier>(
                        processor, query::FalsePredicate(client)).get();

                ASSERT_EQ(0, (int) result.size());
            }

            TEST_P(ClientMapTest, testExecuteOnEntriesWithAndPredicate) {
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);
                Employee empl3("deniz", 25);

                employees->put(3, empl1).get();
                employees->put(4, empl2).get();
                employees->put(5, empl3).get();

                /* 25 <= age <= 35 AND age = 35 */
                query::AndPredicate andPredicate(client, query::BetweenPredicate(client, "a", 25, 35),
                                                 query::NotPredicate(client, query::EqualPredicate(client, "a", 35)));

                EntryMultiplier processor(4);

                std::unordered_map<int, boost::optional<int> > result = employees->executeOnEntries<int, int, EntryMultiplier>(
                        processor, andPredicate).get();

                ASSERT_EQ(1, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(5)));
                ASSERT_EQ(5 * processor.getMultiplier(), result[5].value());
            }

            TEST_P(ClientMapTest, testExecuteOnEntriesWithOrPredicate) {
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);
                Employee empl3("deniz", 25);

                employees->put(3, empl1).get();
                employees->put(4, empl2).get();
                employees->put(5, empl3).get();

                /* age == 21 OR age > 25 */
                query::OrPredicate orPredicate(client, query::EqualPredicate(client, "a", 21), query::GreaterLessPredicate(client, "a", 25, false, false));

                EntryMultiplier processor(4);

                std::unordered_map<int, boost::optional<int> > result = employees->executeOnEntries<int, int, EntryMultiplier>(
                        processor, orPredicate).get();

                ASSERT_EQ(2, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(3)));
                ASSERT_TRUE((result.end() != result.find(4)));
                ASSERT_EQ(3 * processor.getMultiplier(), result[3].value());
                ASSERT_EQ(4 * processor.getMultiplier(), result[4].value());
            }

            TEST_P(ClientMapTest, testExecuteOnEntriesWithBetweenPredicate) {
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);
                Employee empl3("deniz", 25);

                employees->put(3, empl1).get();
                employees->put(4, empl2).get();
                employees->put(5, empl3).get();

                EntryMultiplier processor(4);

                std::unordered_map<int, boost::optional<int> > result = employees->executeOnEntries<int, int, EntryMultiplier>(
                        processor, query::BetweenPredicate(client, "a", 25, 35)).get();

                ASSERT_EQ(2, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(3)));
                ASSERT_TRUE((result.end() != result.find(5)));
                ASSERT_EQ(3 * processor.getMultiplier(), result[3].value());
                ASSERT_EQ(5 * processor.getMultiplier(), result[5].value());
            }

            TEST_P(ClientMapTest, testExecuteOnEntriesWithEqualPredicate) {
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);
                Employee empl3("deniz", 25);

                employees->put(3, empl1).get();
                employees->put(4, empl2).get();
                employees->put(5, empl3).get();

                EntryMultiplier processor(4);

                std::unordered_map<int, boost::optional<int> > result = employees->executeOnEntries<int, int, EntryMultiplier>(
                        processor, query::EqualPredicate(client, "a", 25)).get();

                ASSERT_EQ(1, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(5)));

                result = employees->executeOnEntries<int, int, EntryMultiplier>(
                        processor, query::EqualPredicate(client, "a", 10)).get();

                ASSERT_EQ(0, (int) result.size());
            }

            TEST_P(ClientMapTest, testExecuteOnEntriesWithNotEqualPredicate) {
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);
                Employee empl3("deniz", 25);

                employees->put(3, empl1).get();
                employees->put(4, empl2).get();
                employees->put(5, empl3).get();

                EntryMultiplier processor(4);

                std::unordered_map<int, boost::optional<int> > result = employees->executeOnEntries<int, int, EntryMultiplier>(
                        processor, query::NotEqualPredicate(client, "a", 25)).get();

                ASSERT_EQ(2, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(3)));
                ASSERT_TRUE((result.end() != result.find(4)));
            }

            TEST_P(ClientMapTest, testExecuteOnEntriesWithGreaterLessPredicate) {
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);
                Employee empl3("deniz", 25);

                employees->put(3, empl1).get();
                employees->put(4, empl2).get();
                employees->put(5, empl3).get();

                EntryMultiplier processor(4);

                std::unordered_map<int, boost::optional<int> > result = employees->executeOnEntries<int, int, EntryMultiplier>(
                        processor, query::GreaterLessPredicate(client, "a", 25, false, true)).get(); // <25 matching

                ASSERT_EQ(1, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(4)));

                result = employees->executeOnEntries<int, int, EntryMultiplier>(
                        processor, query::GreaterLessPredicate(client, "a", 25, true, true)).get(); // <=25 matching

                ASSERT_EQ(2, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(4)));
                ASSERT_TRUE((result.end() != result.find(5)));

                result = employees->executeOnEntries<int, int, EntryMultiplier>(
                        processor, query::GreaterLessPredicate(client, "a", 25, false, false)).get(); // >25 matching

                ASSERT_EQ(1, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(3)));

                result = employees->executeOnEntries<int, int, EntryMultiplier>(
                        processor, query::GreaterLessPredicate(client, "a", 25, true, false)).get(); // >=25 matching

                ASSERT_EQ(2, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(3)));
                ASSERT_TRUE((result.end() != result.find(5)));
            }

            TEST_P(ClientMapTest, testExecuteOnEntriesWithLikePredicate) {
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);
                Employee empl3("deniz", 25);

                employees->put(3, empl1).get();
                employees->put(4, empl2).get();
                employees->put(5, empl3).get();

                EntryMultiplier processor(4);

                std::unordered_map<int, boost::optional<int> > result = employees->executeOnEntries<int, int, EntryMultiplier>(
                        processor, query::LikePredicate(client, "n", "deniz")).get();

                ASSERT_EQ(1, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(5)));
            }

            TEST_P(ClientMapTest, testExecuteOnEntriesWithILikePredicate) {
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);
                Employee empl3("deniz", 25);

                employees->put(3, empl1).get();
                employees->put(4, empl2).get();
                employees->put(5, empl3).get();

                EntryMultiplier processor(4);

                std::unordered_map<int, boost::optional<int> > result = employees->executeOnEntries<int, int, EntryMultiplier>(
                        processor, query::ILikePredicate(client, "n", "deniz")).get();

                ASSERT_EQ(1, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(5)));
            }

            TEST_P(ClientMapTest, testExecuteOnEntriesWithInPredicate) {
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);
                Employee empl3("deniz", 25);

                employees->put(3, empl1).get();
                employees->put(4, empl2).get();
                employees->put(5, empl3).get();

                EntryMultiplier processor(4);

                query::InPredicate predicate(client, "n", "ahmet", "mehmet");
                std::unordered_map<int, boost::optional<int> > result = employees->executeOnEntries<int, int, EntryMultiplier>(
                        processor, predicate).get();

                ASSERT_EQ(2, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(3)));
                ASSERT_TRUE((result.end() != result.find(4)));
            }

            TEST_P(ClientMapTest, testExecuteOnEntriesWithInstanceOfPredicate) {
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);
                Employee empl3("deniz", 25);

                employees->put(3, empl1).get();
                employees->put(4, empl2).get();
                employees->put(5, empl3).get();

                EntryMultiplier processor(4);
                std::unordered_map<int, boost::optional<int> > result = employees->executeOnEntries<int, int, EntryMultiplier>(
                        processor, query::InstanceOfPredicate(client, "com.hazelcast.client.test.Employee")).get();

                ASSERT_EQ(3, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(3)));
                ASSERT_TRUE((result.end() != result.find(4)));
                ASSERT_TRUE((result.end() != result.find(5)));
            }

            TEST_P(ClientMapTest, testExecuteOnEntriesWithNotPredicate) {
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);
                Employee empl3("deniz", 25);

                employees->put(3, empl1).get();
                employees->put(4, empl2).get();
                employees->put(5, empl3).get();

                EntryMultiplier processor(4);
                query::EqualPredicate eqPredicate(client, "a", 25);
                query::NotPredicate notPredicate(client, eqPredicate);
                std::unordered_map<int, boost::optional<int> > result = employees->executeOnEntries<int, int, EntryMultiplier>(
                        processor, notPredicate).get();

                ASSERT_EQ(2, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(3)));
                ASSERT_TRUE((result.end() != result.find(4)));

                query::NotPredicate notFalsePredicate(client, query::FalsePredicate(client));
                result = employees->executeOnEntries<int, int, EntryMultiplier>(processor, notFalsePredicate).get();

                ASSERT_EQ(3, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(3)));
                ASSERT_TRUE((result.end() != result.find(4)));
                ASSERT_TRUE((result.end() != result.find(5)));

                query::NotPredicate notBetweenPredicate(client, query::BetweenPredicate(client, "a", 25, 35));
                result = employees->executeOnEntries<int, int, EntryMultiplier>(processor, notBetweenPredicate).get();

                ASSERT_EQ(1, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(4)));
            }

            TEST_P(ClientMapTest, testExecuteOnEntriesWithRegexPredicate) {
                Employee empl1("ahmet", 35);
                Employee empl2("mehmet", 21);
                Employee empl3("deniz", 25);

                employees->put(3, empl1).get();
                employees->put(4, empl2).get();
                employees->put(5, empl3).get();

                EntryMultiplier processor(4);

                std::unordered_map<int, boost::optional<int> > result = employees->executeOnEntries<int, int, EntryMultiplier>(
                        processor, query::RegexPredicate(client, "n", ".*met")).get();

                ASSERT_EQ(2, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(3)));
                ASSERT_TRUE((result.end() != result.find(4)));
            }

            TEST_P(ClientMapTest, testAddInterceptor) {
                std::string prefix("My Prefix");
                MapGetInterceptor interceptor(prefix);
                std::string interceptorId = imap->addInterceptor<MapGetInterceptor>(interceptor).get();

                boost::optional<std::string> val = imap->get<std::string, std::string>("nonexistent").get();
                ASSERT_TRUE(val);
                ASSERT_EQ(prefix, *val);

                val = imap->put<std::string, std::string>("key1", "value1").get();
                ASSERT_FALSE(val);

                val = imap->get<std::string, std::string>("key1").get();
                ASSERT_TRUE(val);
                ASSERT_EQ(prefix + "value1", *val);

                imap->removeInterceptor(interceptorId).get();
            }

            TEST_P(ClientMapTest, testJsonPutGet) {
                std::shared_ptr<IMap> map = client.getMap(getTestName());
                HazelcastJsonValue value("{ \"age\": 4 }");
                map->put("item1", value).get();
                boost::optional<HazelcastJsonValue> retrieved = map->get<std::string, HazelcastJsonValue>("item1").get();

                ASSERT_TRUE(retrieved.has_value());
                ASSERT_EQ(value, retrieved.value());
            }

            TEST_P(ClientMapTest, testQueryOverJsonObject) {
                std::shared_ptr<IMap> map = client.getMap(getTestName());
                HazelcastJsonValue young("{ \"age\": 4 }");
                HazelcastJsonValue old("{ \"age\": 20 }");
                map->put("item1", young).get();
                map->put("item2", old).get();

                ASSERT_EQ(2, map->size().get());

// Get the objects whose age is less than 6
                std::vector<HazelcastJsonValue> result = map->values<HazelcastJsonValue>(
                        query::GreaterLessPredicate(client, "age", 6, false, true)).get();
                ASSERT_EQ(1U, result.size());
                ASSERT_EQ(young, result[0]);
            }

            TEST_P(ClientMapTest, testExtendedAsciiString) {
                std::string key = "Num\xc3\xa9ro key";
                std::string value = "Num\xc3\xa9ro value";
                imap->put<std::string, std::string>(key, value).get();

                boost::optional<std::string> actualValue = imap->get<std::string, std::string>(key).get();
                ASSERT_TRUE(actualValue.has_value());
                ASSERT_EQ(value, actualValue.value());
            }
        }

        namespace serialization {
            template<>
            struct hz_serializer<test::ClientMapTest::EntryMultiplier> : public identified_data_serializer {
                static int getFactoryId() {
                    return 666;
                }

                static int getClassId() {
                    return 3;
                }

                static void writeData(const test::ClientMapTest::EntryMultiplier &object, ObjectDataOutput &writer) {
                    writer.write<int32_t>(object.getMultiplier());
                }

                static test::ClientMapTest::EntryMultiplier readData(ObjectDataInput &reader) {
                    return test::ClientMapTest::EntryMultiplier(reader.read<int32_t>());
                }
            };

            template<>
            struct hz_serializer<test::PartitionAwareInt> : public identified_data_serializer {
            public:
                static int32_t getFactoryId() {
                    return 666;
                }

                static int32_t getClassId() {
                    return 9;
                }

                static void writeData(const test::PartitionAwareInt &object, ObjectDataOutput &out) {
                    out.write<int32_t>(object.getActualKey());
                }

                static test::PartitionAwareInt readData(ObjectDataInput &in) {
                    int value = in.read<int32_t>();
                    return test::PartitionAwareInt(value, value);
                }
            };

            template<>
            struct hz_serializer<test::ClientMapTest::MapGetInterceptor> : public identified_data_serializer {
                static int getFactoryId() {
                    return 666;
                }

                static int getClassId() {
                    return 6;
                }

                static void writeData(const test::ClientMapTest::MapGetInterceptor &object, ObjectDataOutput &writer) {
                    writer.write(object.prefix_);
                }

                static test::ClientMapTest::MapGetInterceptor readData(ObjectDataInput &reader) {
                    return test::ClientMapTest::MapGetInterceptor(reader.read<std::string>());
                }
            };
        }
    }
}

namespace std {
    template<> struct hash<hazelcast::client::test::PartitionAwareInt> {
        std::size_t operator()(const hazelcast::client::test::PartitionAwareInt &object) const noexcept {
            return std::hash<int>{}(object.getActualKey());
        }
    };
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

