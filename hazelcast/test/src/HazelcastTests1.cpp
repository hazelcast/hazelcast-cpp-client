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
#include "hazelcast/client/ReliableTopic.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(disable: 4996) //for unsafe getenv
#endif

namespace hazelcast {
    namespace client {
        namespace test {
            /**
            class ClientStatisticsTest : public ClientTestSupport {
            protected:
                static const int STATS_PERIOD_SECONDS = 1;

                static void SetUpTestCase() {
                    instance = new HazelcastServer(*g_srvFactory);
                }

                static void TearDownTestCase() {
                    delete instance;

                    instance = NULL;
                }

                Response getClientStatsFromServer() {
                    const char *script = "client0=instance_0.getClientService().getConnectedClients()."
                                         "toArray()[0]\nresult=client0.getClientStatistics();";

                    Response response;
                    RemoteControllerClient &controllerClient = g_srvFactory->getRemoteController();
                    controllerClient.executeOnController(response, g_srvFactory->getClusterId(), script, Lang::PYTHON);
                    return response;
                }

                std::string unescapeSpecialCharacters(const std::string &value) {
                    std::regex reBackslah("\\,");
                    auto escapedValue = std::regex_replace(value, reBackslah, ",");
                    std::regex reEqual("\\=");
                    escapedValue = std::regex_replace(escapedValue, reEqual, "=");
                    std::regex reDoubleBackslash("\\\\");
                    escapedValue = std::regex_replace(escapedValue, reDoubleBackslash, "\\");
                    return escapedValue;
                }

                std::map<std::string, std::string>
                getStatsFromResponse(const Response &statsResponse) {
                    std::map<std::string, std::string> statsMap;
                    if (statsResponse.success && !statsResponse.result.empty()) {
                        // passing -1 as the submatch index parameter performs splitting
                        std::regex re(",");
                        std::sregex_token_iterator first{statsResponse.result.begin(), statsResponse.result.end(), re,
                                                         -1}, last;

                        while (first != last) {
                            string input = unescapeSpecialCharacters(*first);
                            std::regex reEqual("=");
                            std::sregex_token_iterator tokenIt{input.begin(), input.end(), reEqual,
                                                             -1}, tokenLast;

                            if (tokenIt->length() > 1) {
                                auto pair = *tokenIt;
                                pair.
                                statsMap[keyValuePair[0]] = keyValuePair[1];
                            } else {
                                statsMap[keyValuePair[0]] = "";
                            }
                        }
                    }

                    return statsMap;
                }

                std::map<std::string, std::string> getStats() {
                    HazelcastServerFactory::Response statsResponse = getClientStatsFromServer();

                    return getStatsFromResponse(statsResponse);
                }

                bool verifyClientStatsFromServerIsNotEmpty() {
                    HazelcastServerFactory::Response response = getClientStatsFromServer();
                    return response.success && !response.result.empty();
                }

                std::unique_ptr<HazelcastClient> createHazelcastClient() {
                    ClientConfig clientConfig;
                    clientConfig.setProperty(ClientProperties::STATISTICS_ENABLED, "true")
                            .setProperty(ClientProperties::STATISTICS_PERIOD_SECONDS,
                                         hazelcast::util::IOUtil::to_string<int>(STATS_PERIOD_SECONDS))
                                    // add IMap Near Cache config
                            .addNearCacheConfig(std::shared_ptr<config::NearCacheConfig<int, int> >(
                                    new config::NearCacheConfig<int, int>(getTestName())));

                    clientConfig.getNetworkConfig().setConnectionAttemptLimit(20);

                    return std::unique_ptr<HazelcastClient>(new HazelcastClient(clientConfig));
                }

                void waitForFirstStatisticsCollection() {
                    ASSERT_TRUE_EVENTUALLY_WITH_TIMEOUT(verifyClientStatsFromServerIsNotEmpty(),
                                                        3 * STATS_PERIOD_SECONDS);
                }

                std::string getClientLocalAddress(HazelcastClient &client) {
                    spi::ClientContext clientContext(client);
                    connection::ClientConnectionManagerImpl &connectionManager = clientContext.getConnectionManager();
                    std::shared_ptr<connection::Connection> ownerConnection = connectionManager.getOwnerConnection();
                    std::unique_ptr<Address> localSocketAddress = ownerConnection->getLocalSocketAddress();
                    std::ostringstream localAddressString;
                    localAddressString << localSocketAddress->getHost() << ":" << localSocketAddress->getPort();
                    return localAddressString.str();
                }

                bool isStatsUpdated(const std::string &lastStatisticsCollectionTime) {
                    std::map<string, string> stats = getStats();
                    if (stats["lastStatisticsCollectionTime"] != lastStatisticsCollectionTime) {
                        return true;
                    }
                    return false;
                }

                void produceSomeStats(HazelcastClient &client) {
                    IMap<int, int> map = client.getMap<int, int>(getTestName());
                    produceSomeStats(map);
                }

                void produceSomeStats(IMap<int, int> &map) {
                    map.put(5, 10);
                    ASSERT_EQ(10, *map.get(5));
                    ASSERT_EQ(10, *map.get(5));
                }

                std::string toString(const std::map<std::string, std::string> &map) {
                    std::ostringstream out;
                    typedef std::map<std::string, std::string> StringMap;
                    out << "Map {" << std::endl;
                    for (const StringMap::value_type &entry : map) {
                        out << "\t\t(" << entry.first << " , " << entry.second << ")" << std::endl;
                    }
                    out << "}" << std::endl;

                    return out.str();
                }

                static HazelcastServer *instance;
            };

            HazelcastServer *ClientStatisticsTest::instance = NULL;

            TEST_F(ClientStatisticsTest, testClientStatisticsDisabledByDefault) {

                ClientConfig clientConfig;
                clientConfig.setProperty(ClientProperties::STATISTICS_PERIOD_SECONDS, "1");

                HazelcastClient client(clientConfig);

                // sleep twice the collection period
                sleepSeconds(2);

                HazelcastServerFactory::Response statsFromServer = getClientStatsFromServer();
                ASSERT_TRUE(statsFromServer.success);
                ASSERT_TRUE(statsFromServer.message.empty()) << "Statistics should be disabled by default.";
            }

            TEST_F(ClientStatisticsTest, testNoUpdateWhenDisabled) {
                ClientConfig clientConfig;
                clientConfig.setProperty(ClientProperties::STATISTICS_ENABLED, "false").setProperty(
                        ClientProperties::STATISTICS_PERIOD_SECONDS, "1");

                HazelcastClient client(clientConfig);

                ASSERT_TRUE_ALL_THE_TIME(getStats().empty(), 2);
            }

            TEST_F(ClientStatisticsTest, testClientStatisticsDisabledWithWrongValue) {

                ClientConfig clientConfig;
                clientConfig.setProperty(ClientProperties::STATISTICS_ENABLED, "trueee");

                HazelcastClient client(clientConfig);

                // sleep twice the collection period
                sleepSeconds(2);

                HazelcastServerFactory::Response statsFromServer = getClientStatsFromServer();
                ASSERT_TRUE(statsFromServer.success);
                ASSERT_TRUE(statsFromServer.message.empty()) << "Statistics should not be enabled with wrong value.";
            }

            TEST_F(ClientStatisticsTest, testClientStatisticsContent) {
                ClientConfig clientConfig;
                std::string mapName = getTestName();
                clientConfig.addNearCacheConfig(
                        std::shared_ptr<config::NearCacheConfig<int, int> >(new config::NearCacheConfig<int, int>(
                                mapName.c_str())));
                clientConfig.setProperty(ClientProperties::STATISTICS_ENABLED, "true").setProperty(
                        ClientProperties::STATISTICS_PERIOD_SECONDS, "1");

                HazelcastClient client(clientConfig);

                // initialize near cache
                client.getMap<int, int>(mapName);

                // sleep twice the collection period
                sleepSeconds(2);

                HazelcastServerFactory::Response statsFromServer = getClientStatsFromServer();
                ASSERT_TRUE(statsFromServer.success);
                ASSERT_FALSE(statsFromServer.result.empty());

                std::string localAddress = getClientLocalAddress(client);

                ASSERT_NE(std::string::npos,
                          statsFromServer.result.find(std::string("clientName=") + client.getName()));
                ASSERT_NE(std::string::npos, statsFromServer.result.find("lastStatisticsCollectionTime="));
                ASSERT_NE(std::string::npos, statsFromServer.result.find("enterprise=false"));
                ASSERT_NE(std::string::npos, statsFromServer.result.find("clientType=CPP"));
                ASSERT_NE(std::string::npos,
                          statsFromServer.result.find(std::string("clientVersion=") + HAZELCAST_VERSION));
                ASSERT_NE(std::string::npos, statsFromServer.result.find("clusterConnectionTimestamp="));
                ASSERT_NE(std::string::npos,
                          statsFromServer.result.find(std::string("clientAddress=") + localAddress));

                ASSERT_NE(std::string::npos,
                          statsFromServer.result.find(std::string("nc.") + mapName + "." + "creationTime"));
                ASSERT_NE(std::string::npos,
                          statsFromServer.result.find(std::string("nc.") + mapName + "." + "evictions"));
                ASSERT_NE(std::string::npos,
                          statsFromServer.result.find(std::string("nc.") + mapName + "." + "hits"));
                ASSERT_NE(std::string::npos,
                          statsFromServer.result.find(std::string("nc.") + mapName + "." + "lastPersistenceDuration"));
                ASSERT_NE(std::string::npos,
                          statsFromServer.result.find(std::string("nc.") + mapName + "." + "lastPersistenceKeyCount"));
                ASSERT_NE(std::string::npos,
                          statsFromServer.result.find(std::string("nc.") + mapName + "." + "lastPersistenceTime"));
                ASSERT_NE(std::string::npos,
                          statsFromServer.result.find(
                                  std::string("nc.") + mapName + "." + "lastPersistenceWrittenBytes"));
                ASSERT_NE(std::string::npos,
                          statsFromServer.result.find(std::string("nc.") + mapName + "." + "misses"));
                ASSERT_NE(std::string::npos,
                          statsFromServer.result.find(std::string("nc.") + mapName + "." + "ownedEntryCount"));
                ASSERT_NE(std::string::npos,
                          statsFromServer.result.find(std::string("nc.") + mapName + "." + "expirations"));
                ASSERT_NE(std::string::npos,
                          statsFromServer.result.find(std::string("nc.") + mapName + "." + "invalidations"));
                ASSERT_NE(std::string::npos,
                          statsFromServer.result.find(std::string("nc.") + mapName + "." + "invalidationRequests"));
                ASSERT_NE(std::string::npos,
                          statsFromServer.result.find(std::string("nc.") + mapName + "." + "ownedEntryMemoryCost"));
                ASSERT_NE(std::string::npos,
                          statsFromServer.result.find(std::string("nc.") + mapName + "." + "creationTime"));
                ASSERT_NE(std::string::npos,
                          statsFromServer.result.find(std::string("nc.") + mapName + "." + "creationTime"));
            }

            TEST_F(ClientStatisticsTest, testStatisticsCollectionNonDefaultPeriod) {
                std::unique_ptr<HazelcastClient> client = createHazelcastClient();

                int64_t clientConnectionTime = hazelcast::util::currentTimeMillis();

                // wait enough time for statistics collection
                waitForFirstStatisticsCollection();

                Response statsResponse = getClientStatsFromServer();
                ASSERT_TRUE(statsResponse.success);
                string &stats = statsResponse.result;
                ASSERT_TRUE(!stats.empty());

                std::map<std::string, std::string> statsMap = getStatsFromResponse(statsResponse);

                ASSERT_EQ(1U, statsMap.count("clusterConnectionTimestamp"))
                                            << "clusterConnectionTimestamp stat should exist (" << stats << ")";
                int64_t connectionTimeStat;
                ASSERT_NO_THROW(
                        (connectionTimeStat = std::stoll(statsMap["clusterConnectionTimestamp"])))
                                            << "connectionTimeStat value is not in correct (" << stats << ")";

                ASSERT_EQ(1U, statsMap.count("clientAddress")) << "clientAddress stat should exist (" << stats << ")";
                std::string expectedClientAddress = getClientLocalAddress(*client);
                ASSERT_EQ(expectedClientAddress, statsMap["clientAddress"]);

                ASSERT_EQ(1U, statsMap.count("clientVersion")) << "clientVersion stat should exist (" << stats << ")";
                ASSERT_EQ(HAZELCAST_VERSION, statsMap["clientVersion"]);

                // time measured by us after client connection should be greater than the connection time reported by the statistics
                ASSERT_GE(clientConnectionTime, connectionTimeStat) << "connectionTimeStat was " << connectionTimeStat
                                                                    << ", clientConnectionTime was "
                                                                    << clientConnectionTime << "(" << stats << ")";

                std::string mapHitsKey = "nc." + getTestName() + ".hits";
                ASSERT_EQ(0U, statsMap.count(mapHitsKey)) << mapHitsKey << " stat should not exist (" << stats << ")";

                ASSERT_EQ(1U, statsMap.count("lastStatisticsCollectionTime"))
                                            << "lastStatisticsCollectionTime stat should exist (" << stats << ")";
                std::string lastStatisticsCollectionTimeString = statsMap["lastStatisticsCollectionTime"];
                ASSERT_NO_THROW((std::stoll(lastStatisticsCollectionTimeString)))
                                            << "lastStatisticsCollectionTime value is not in correct (" << stats << ")";

                // this creates empty map statistics
                IMap<int, int> map = client->getMap<int, int>(getTestName());

                statsMap = getStats();
                lastStatisticsCollectionTimeString = statsMap["lastStatisticsCollectionTime"];

                // wait enough time for statistics collection
                ASSERT_TRUE_EVENTUALLY(isStatsUpdated(lastStatisticsCollectionTimeString));

                statsMap = getStats();
                ASSERT_EQ(1U, statsMap.count(mapHitsKey))
                                            << mapHitsKey << " stat should exist (" << toString(statsMap) << ")";
                ASSERT_EQ("0", statsMap[mapHitsKey]) << "Expected 0 map hits (" << toString(statsMap) << ")";

                // produce map stat
                produceSomeStats(map);

                statsMap = getStats();
                lastStatisticsCollectionTimeString = statsMap["lastStatisticsCollectionTime"];

                // wait enough time for statistics collection
                ASSERT_TRUE_EVENTUALLY(isStatsUpdated(lastStatisticsCollectionTimeString));

                statsMap = getStats();
                ASSERT_EQ(1U, statsMap.count(mapHitsKey))
                                            << mapHitsKey << " stat should exist (" << toString(statsMap) << ")";
                ASSERT_EQ("1", statsMap[mapHitsKey]) << "Expected 1 map hits (" << toString(statsMap) << ")";
            }

            TEST_F(ClientStatisticsTest, testStatisticsPeriod) {
                std::unique_ptr<HazelcastClient> client = createHazelcastClient();

                // wait enough time for statistics collection
                waitForFirstStatisticsCollection();

                std::map<std::string, std::string> initialStats = getStats();

                // produce map stat
                produceSomeStats(*client);

                // wait enough time for statistics collection
                ASSERT_TRUE_EVENTUALLY(isStatsUpdated(initialStats["lastStatisticsCollectionTime"]));

                ASSERT_NE(initialStats, getStats()) << "initial statistics should not be the same as current stats";
            }
*/
        }
    }
}





namespace hazelcast {
    namespace client {
        namespace test {
            namespace ringbuffer {
                StartsWithStringFilter::StartsWithStringFilter(const std::string &startString) : startString(
                        startString) {}

                int StartsWithStringFilter::getFactoryId() const {
                    return 666;
                }

                int StartsWithStringFilter::getClassId() const {
                    return 14;
                }

                void StartsWithStringFilter::writeData(serialization::ObjectDataOutput &writer) const {
                    writer.writeUTF(&startString);
                }

                void StartsWithStringFilter::readData(serialization::ObjectDataInput &reader) {
                    startString = *reader.readUTF();
                }
            }
        }
    }
}

using namespace hazelcast::client::mixedtype;

namespace hazelcast {
    namespace client {
        namespace test {
            class MixedRingbufferTest : public ClientTestSupport {
            public:
                MixedRingbufferTest() : rb(client->toMixedType().getRingbuffer(getTestName())) {
                }

            protected:
                virtual void TearDown() {
                }

                static void SetUpTestCase() {
                    instance = new HazelcastServer(*g_srvFactory);
                    client = new HazelcastClient;
                }

                static void TearDownTestCase() {
                    delete client;
                    delete instance;

                    client = NULL;
                    instance = NULL;
                }

                mixedtype::Ringbuffer rb;
                static HazelcastServer *instance;
                static HazelcastClient *client;

                static const int64_t CAPACITY;
            };

            const int64_t MixedRingbufferTest::CAPACITY = 10;

            HazelcastServer *MixedRingbufferTest::instance = NULL;
            HazelcastClient *MixedRingbufferTest::client = NULL;

            TEST_F(MixedRingbufferTest, testAPI) {
                ASSERT_EQ(CAPACITY, rb.capacity());
                ASSERT_EQ(0, rb.headSequence());
                ASSERT_EQ(-1, rb.tailSequence());
                ASSERT_EQ(0, rb.size());
                ASSERT_EQ(CAPACITY, rb.remainingCapacity());
                ASSERT_THROW(rb.readOne(-1), exception::IllegalArgumentException);
                ASSERT_THROW(rb.readOne(1), exception::IllegalArgumentException);

                Employee employee1("First", 10);
                Employee employee2("Second", 20);

                ASSERT_EQ(0, rb.add<Employee>(employee1));
                ASSERT_EQ(CAPACITY, rb.capacity());
                ASSERT_EQ(CAPACITY, rb.remainingCapacity());
                ASSERT_EQ(0, rb.headSequence());
                ASSERT_EQ(0, rb.tailSequence());
                ASSERT_EQ(1, rb.size());
                ASSERT_EQ(employee1, *rb.readOne(0).get<Employee>());
                ASSERT_THROW(rb.readOne(2), exception::IllegalArgumentException);

                ASSERT_EQ(1, rb.add<Employee>(employee2));
                ASSERT_EQ(CAPACITY, rb.capacity());
                ASSERT_EQ(CAPACITY, rb.remainingCapacity());
                ASSERT_EQ(0, rb.headSequence());
                ASSERT_EQ(1, rb.tailSequence());
                ASSERT_EQ(2, rb.size());
                ASSERT_EQ(employee1, *rb.readOne(0).get<Employee>());
                ASSERT_EQ(employee2, *rb.readOne(1).get<Employee>());
                ASSERT_THROW(rb.readOne(3), exception::IllegalArgumentException);

                // insert many employees to fill the ringbuffer capacity
                for (int i = 0; i < CAPACITY - 2; ++i) {
                    Employee eleman("name", 10 * (i + 2));
                    ASSERT_EQ(i + 2, rb.add<Employee>(eleman));
                    ASSERT_EQ(CAPACITY, rb.capacity());
                    ASSERT_EQ(CAPACITY, rb.remainingCapacity());
                    ASSERT_EQ(0, rb.headSequence());
                    ASSERT_EQ(i + 2, rb.tailSequence());
                    ASSERT_EQ(i + 3, rb.size());
                    ASSERT_EQ(eleman, *rb.readOne(i + 2).get<Employee>());
                }

                // verify that the head element is overriden on the first add
                Employee latestEmployee("latest employee", 100);
                ASSERT_EQ(CAPACITY, rb.add<Employee>(latestEmployee));
                ASSERT_EQ(CAPACITY, rb.capacity());
                ASSERT_EQ(CAPACITY, rb.remainingCapacity());
                ASSERT_EQ(1, rb.headSequence());
                ASSERT_EQ(CAPACITY, rb.tailSequence());
                ASSERT_EQ(CAPACITY, rb.size());
                ASSERT_EQ(latestEmployee, *rb.readOne(CAPACITY).get<Employee>());
            }
        }
    }
}




namespace hazelcast {
    namespace client {
        namespace test {
            namespace ringbuffer {
                class RingbufferTest : public ClientTestSupport {
                public:
                    RingbufferTest() {
                        for (int i = 0; i < 11; ++i) {
                            std::ostringstream out;
                            out << i;
                            items.push_back(out.str());
                        }
                    }

                protected:
                    class ReadOneWithLatchTask : public hazelcast::util::Runnable {
                    public:
                        ReadOneWithLatchTask(const std::shared_ptr<Ringbuffer<std::string> > &clientRingbuffer,
                                             const std::shared_ptr<boost::latch> &latch1) : clientRingbuffer(
                                clientRingbuffer), latch1(latch1) {}

                        virtual const std::string getName() const {
                            return "ReadOneWithLatchTask";
                        }

                        virtual void run() {
                            try {
                                clientRingbuffer->readOne(0);
                            } catch (exception::StaleSequenceException &) {
                                latch1->count_down();
                            } catch (std::exception &e) {
                                std::cerr << e.what();
                            }
                        }

                    private:
                        const std::shared_ptr<Ringbuffer<std::string> > clientRingbuffer;
                        const std::shared_ptr<boost::latch> latch1;
                        static const int CAPACITY;
                    };

                    virtual void SetUp() {
                        std::string testName = getTestName();
                        clientRingbuffer = client->getRingbuffer<std::string>(testName);
                        client2Ringbuffer = client2->getRingbuffer<std::string>(testName);
                    }

                    static void SetUpTestCase() {
                        instance = new HazelcastServer(*g_srvFactory);
                        client = new HazelcastClient(getConfig());
                        client2 = new HazelcastClient(getConfig());
                    }

                    static void TearDownTestCase() {
                        delete client;
                        delete client2;
                        delete instance;

                        client = NULL;
                        client2 = NULL;
                        instance = NULL;
                    }

                    static HazelcastServer *instance;
                    static HazelcastClient *client;
                    static HazelcastClient *client2;
                    std::shared_ptr<Ringbuffer<std::string> > clientRingbuffer;
                    std::shared_ptr<Ringbuffer<std::string> > client2Ringbuffer;
                    std::vector<std::string> items;

                    static const int64_t CAPACITY;
                };

                const int64_t RingbufferTest::CAPACITY = 10;

                HazelcastServer *RingbufferTest::instance = NULL;
                HazelcastClient *RingbufferTest::client = NULL;
                HazelcastClient *RingbufferTest::client2 = NULL;

                TEST_F(RingbufferTest, testAPI) {
                    std::shared_ptr<Ringbuffer<Employee> > rb = client->getRingbuffer<Employee>(getTestName() + "2");
                    ASSERT_EQ(CAPACITY, rb->capacity());
                    ASSERT_EQ(0, rb->headSequence());
                    ASSERT_EQ(-1, rb->tailSequence());
                    ASSERT_EQ(0, rb->size());
                    ASSERT_EQ(CAPACITY, rb->remainingCapacity());
                    ASSERT_THROW(rb->readOne(-1), exception::IllegalArgumentException);
                    ASSERT_THROW(rb->readOne(1), exception::IllegalArgumentException);

                    Employee employee1("First", 10);
                    Employee employee2("Second", 20);

                    ASSERT_EQ(0, rb->add(employee1));
                    ASSERT_EQ(CAPACITY, rb->capacity());
                    ASSERT_EQ(CAPACITY, rb->remainingCapacity());
                    ASSERT_EQ(0, rb->headSequence());
                    ASSERT_EQ(0, rb->tailSequence());
                    ASSERT_EQ(1, rb->size());
                    ASSERT_EQ(employee1, *rb->readOne(0));
                    ASSERT_THROW(rb->readOne(2), exception::IllegalArgumentException);

                    ASSERT_EQ(1, rb->add(employee2));
                    ASSERT_EQ(CAPACITY, rb->capacity());
                    ASSERT_EQ(CAPACITY, rb->remainingCapacity());
                    ASSERT_EQ(0, rb->headSequence());
                    ASSERT_EQ(1, rb->tailSequence());
                    ASSERT_EQ(2, rb->size());
                    ASSERT_EQ(employee1, *rb->readOne(0));
                    ASSERT_EQ(employee2, *rb->readOne(1));
                    ASSERT_THROW(*rb->readOne(3), exception::IllegalArgumentException);

                    // insert many employees to fill the ringbuffer capacity
                    for (int i = 0; i < CAPACITY - 2; ++i) {
                        Employee eleman("name", 10 * (i + 2));
                        ASSERT_EQ(i + 2, rb->add(eleman));
                        ASSERT_EQ(CAPACITY, rb->capacity());
                        ASSERT_EQ(CAPACITY, rb->remainingCapacity());
                        ASSERT_EQ(0, rb->headSequence());
                        ASSERT_EQ(i + 2, rb->tailSequence());
                        ASSERT_EQ(i + 3, rb->size());
                        ASSERT_EQ(eleman, *rb->readOne(i + 2));
                    }

                    // verify that the head element is overriden on the first add
                    Employee latestEmployee("latest employee", 100);
                    ASSERT_EQ(CAPACITY, rb->add(latestEmployee));
                    ASSERT_EQ(CAPACITY, rb->capacity());
                    ASSERT_EQ(CAPACITY, rb->remainingCapacity());
                    ASSERT_EQ(1, rb->headSequence());
                    ASSERT_EQ(CAPACITY, rb->tailSequence());
                    ASSERT_EQ(CAPACITY, rb->size());
                    ASSERT_EQ(latestEmployee, *rb->readOne(CAPACITY));
                }

                TEST_F(RingbufferTest, readManyAsync_whenHitsStale_shouldNotBeBlocked) {
                    auto f = clientRingbuffer->readManyAsync<void>(0, 1, 10, NULL);
                    client2Ringbuffer->addAllAsync(items, Ringbuffer<std::string>::OVERWRITE);
                    ASSERT_THROW(f.get(), exception::StaleSequenceException);
                }

                TEST_F(RingbufferTest, readOne_whenHitsStale_shouldNotBeBlocked) {
                    std::shared_ptr<boost::latch> latch1 = std::make_shared<boost::latch>(1);
                    std::thread t(std::packaged_task<void()>([ =] ()
                    { ReadOneWithLatchTask(clientRingbuffer, latch1).run(); }));
                    client2Ringbuffer->addAllAsync(items, Ringbuffer<std::string>::OVERWRITE);
                    ASSERT_OPEN_EVENTUALLY(*latch1);
                    t.join();
                }

                TEST_F(RingbufferTest, headSequence) {
                    for (int k = 0; k < 2 * CAPACITY; k++) {
                        client2Ringbuffer->add("foo");
                    }

                    ASSERT_EQ(client2Ringbuffer->headSequence(), clientRingbuffer->headSequence());
                }

                TEST_F(RingbufferTest, tailSequence) {
                    for (int k = 0; k < 2 * CAPACITY; k++) {
                        client2Ringbuffer->add("foo");
                    }

                    ASSERT_EQ(client2Ringbuffer->tailSequence(), clientRingbuffer->tailSequence());
                }

                TEST_F(RingbufferTest, size) {
                    client2Ringbuffer->add("foo");

                    ASSERT_EQ(client2Ringbuffer->tailSequence(), clientRingbuffer->tailSequence());
                }

                TEST_F(RingbufferTest, capacity) {
                    ASSERT_EQ(client2Ringbuffer->capacity(), clientRingbuffer->capacity());
                }

                TEST_F(RingbufferTest, remainingCapacity) {
                    client2Ringbuffer->add("foo");

                    ASSERT_EQ(client2Ringbuffer->remainingCapacity(), clientRingbuffer->remainingCapacity());
                }

                TEST_F(RingbufferTest, add) {
                    clientRingbuffer->add("foo");

                    std::unique_ptr<std::string> value = client2Ringbuffer->readOne(0);
                    ASSERT_EQ_PTR("foo", value.get(), std::string);
                }

                TEST_F(RingbufferTest, addAsync) {
                    auto f = clientRingbuffer->addAsync("foo", Ringbuffer<std::string>::OVERWRITE);
                    auto result = f.get();

                    ASSERT_EQ_PTR(client2Ringbuffer->headSequence(), result.get(), int64_t);
                    ASSERT_EQ_PTR("foo", client2Ringbuffer->readOne(0).get(), std::string);
                    ASSERT_EQ(0, client2Ringbuffer->headSequence());
                    ASSERT_EQ(0, client2Ringbuffer->tailSequence());
                }

                TEST_F(RingbufferTest, addAllAsync) {
                    std::vector<std::string> items;
                    items.push_back("foo");
                    items.push_back("bar");
                    auto f = clientRingbuffer->addAllAsync(items, Ringbuffer<std::string>::OVERWRITE);
                    auto result = f.get();

                    ASSERT_EQ_PTR(client2Ringbuffer->tailSequence(), result.get(), int64_t);
                    ASSERT_EQ_PTR("foo", client2Ringbuffer->readOne(0).get(), std::string);
                    ASSERT_EQ_PTR("bar", client2Ringbuffer->readOne(1).get(), std::string);
                    ASSERT_EQ(0, client2Ringbuffer->headSequence());
                    ASSERT_EQ(1, client2Ringbuffer->tailSequence());
                }

                TEST_F(RingbufferTest, readOne) {
                    client2Ringbuffer->add("foo");
                    ASSERT_EQ_PTR("foo", clientRingbuffer->readOne(0).get(), std::string);
                }

                TEST_F(RingbufferTest, readManyAsync_noFilter) {
                    client2Ringbuffer->add("1");
                    client2Ringbuffer->add("2");
                    client2Ringbuffer->add("3");

                    auto f = clientRingbuffer->readManyAsync<void>(0, 3, 3, NULL);
                    auto rs = f.get();

                    ASSERT_EQ(3, rs->readCount());
                    ASSERT_EQ_PTR("1", rs->getItems().get(0), std::string);
                    ASSERT_EQ_PTR("2", rs->getItems().get(1), std::string);
                    ASSERT_EQ_PTR("3", rs->getItems().get(2), std::string);
                }

// checks if the max count works. So if more results are available than needed, the surplus results should not be read.
                TEST_F(RingbufferTest, readManyAsync_maxCount) {
                    client2Ringbuffer->add("1");
                    client2Ringbuffer->add("2");
                    client2Ringbuffer->add("3");
                    client2Ringbuffer->add("4");
                    client2Ringbuffer->add("5");
                    client2Ringbuffer->add("6");

                    auto f = clientRingbuffer->readManyAsync<void>(0, 3, 3, NULL);
                    auto rs = f.get();

                    ASSERT_EQ(3, rs->readCount());
                    DataArray <std::string> &items1 = rs->getItems();
                    ASSERT_EQ_PTR("1", items1.get(0), std::string);
                    ASSERT_EQ_PTR("2", items1.get(1), std::string);
                    ASSERT_EQ_PTR("3", items1.get(2), std::string);
                }

                TEST_F(RingbufferTest, readManyAsync_withFilter) {
                    client2Ringbuffer->add("good1");
                    client2Ringbuffer->add("bad1");
                    client2Ringbuffer->add("good2");
                    client2Ringbuffer->add("bad2");
                    client2Ringbuffer->add("good3");
                    client2Ringbuffer->add("bad3");

                    StartsWithStringFilter filter("good");
                    auto f = clientRingbuffer->readManyAsync<StartsWithStringFilter>(0, 3, 3, &filter);

                    auto rs = f.get();

                    ASSERT_EQ(5, rs->readCount());
                    DataArray <std::string> &items = rs->getItems();
                    ASSERT_EQ_PTR("good1", items.get(0), std::string);
                    ASSERT_EQ_PTR("good2", items.get(1), std::string);
                    ASSERT_EQ_PTR("good3", items.get(2), std::string);
                }
            }
        }
    }
}


//
// Created by sancar koyunlu on 9/13/13.



namespace hazelcast {
    namespace client {
        namespace test {
            class PolymorphicDataSerializableRingbufferTest : public ClientTestSupport {
            protected:
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

                class PolymorphicDataSerializableFactory : public serialization::DataSerializableFactory {
                public:
                    virtual std::unique_ptr<serialization::IdentifiedDataSerializable> create(int32_t typeId) {
                        switch (typeId) {
                            case 10:
                                return std::unique_ptr<serialization::IdentifiedDataSerializable>(new BaseDataSerializable);
                            case 11:
                                return std::unique_ptr<serialization::IdentifiedDataSerializable>(new Derived1DataSerializable);
                            case 12:
                                return std::unique_ptr<serialization::IdentifiedDataSerializable>(new Derived2DataSerializable);
                            default:
                                return std::unique_ptr<serialization::IdentifiedDataSerializable>();
                        }
                    }
                };

                virtual void TearDown() {
                }

                static void SetUpTestCase() {
                    instance = new HazelcastServer(*g_srvFactory);
                    ClientConfig clientConfig = getConfig();
                    SerializationConfig &serializationConfig = clientConfig.getSerializationConfig();
                    serializationConfig.addDataSerializableFactory(666,
                                                                   std::shared_ptr<serialization::DataSerializableFactory>(
                                                                           new PolymorphicDataSerializableFactory()));
                    client = new HazelcastClient(clientConfig);
                    rb = client->getRingbuffer<BaseDataSerializable>("rb-1");
                }

                static void TearDownTestCase() {
                    delete client;
                    delete instance;

                    client = NULL;
                    instance = NULL;
                }

                static HazelcastServer *instance;
                static HazelcastClient *client;
                static std::shared_ptr<Ringbuffer<BaseDataSerializable> > rb;
            };


            HazelcastServer *PolymorphicDataSerializableRingbufferTest::instance = NULL;
            HazelcastClient *PolymorphicDataSerializableRingbufferTest::client = NULL;
            std::shared_ptr<Ringbuffer<PolymorphicDataSerializableRingbufferTest::BaseDataSerializable> > PolymorphicDataSerializableRingbufferTest::rb;

            TEST_F(PolymorphicDataSerializableRingbufferTest, testPolymorhism) {
                BaseDataSerializable base;
                Derived1DataSerializable derived1;
                Derived2DataSerializable derived2;
                rb->add(base);
                rb->add(derived1);
                rb->add(derived2);

                int64_t sequence = rb->headSequence();
                std::unique_ptr<BaseDataSerializable> value = rb->readOne(sequence);
                ASSERT_NE((BaseDataSerializable *) NULL, value.get());
                ASSERT_EQ(base.getClassId(), value->getClassId());

                value = rb->readOne(sequence + 1);
                ASSERT_NE((BaseDataSerializable *) NULL, value.get());
                ASSERT_EQ(derived1.getClassId(), value->getClassId());

                value = rb->readOne(sequence + 2);
                ASSERT_NE((BaseDataSerializable *) NULL, value.get());
                ASSERT_EQ(derived2.getClassId(), value->getClassId());
            }
        }
    }
}




namespace hazelcast {
    namespace client {
        namespace test {
            std::string ClientTestSupportBase::getCAFilePath() {
                return "hazelcast/test/resources/cpp_client.crt";
            }

            hazelcast::client::ClientConfig ClientTestSupportBase::getConfig() {
                ClientConfig clientConfig;
                clientConfig.addAddress(Address(g_srvFactory->getServerAddress(), 5701));
                return clientConfig;
            }

            HazelcastClient ClientTestSupportBase::getNewClient() {
                return HazelcastClient(getConfig());
            }

            const std::string ClientTestSupportBase::getSslFilePath() {
                return "hazelcast/test/resources/hazelcast-ssl.xml";
            }

            std::string ClientTestSupportBase::randomMapName() {
                return randomString();
            }

            std::string ClientTestSupportBase::randomString() {
                // TODO: Change with secure uuid generator as in Java
                return hazelcast::util::UuidUtil::newUnsecureUuidString();
            }

            void ClientTestSupportBase::sleepSeconds(int32_t seconds) {
                hazelcast::util::sleep(seconds);
            }

            ClientTestSupportBase::ClientTestSupportBase() {
            }

            std::string ClientTestSupportBase::generateKeyOwnedBy(spi::ClientContext &context, const Member &member) {
                spi::ClientPartitionService &partitionService = context.getPartitionService();
                serialization::pimpl::SerializationService &serializationService = context.getSerializationService();
                while (true) {
                    std::string id = randomString();
                    int partitionId = partitionService.getPartitionId(serializationService.toData<std::string>(&id));
                    std::shared_ptr<impl::Partition> partition = partitionService.getPartition(partitionId);
                    if (*partition->getOwner() == member) {
                        return id;
                    }
                }
            }
        }
    }

    namespace util {
        StartedThread::StartedThread(const std::string &name, void (*func)(ThreadArgs &),
                                     void *arg0, void *arg1, void *arg2, void *arg3)
                : name(name), logger(new hazelcast::util::ILogger("StartedThread", "StartedThread", "testversion",
                                                                  client::config::LoggerConfig())) {
            init(func, arg0, arg1, arg2, arg3);
        }

        StartedThread::StartedThread(void (func)(ThreadArgs &),
                                     void *arg0,
                                     void *arg1,
                                     void *arg2,
                                     void *arg3)
                : name("hz.unnamed"),
                  logger(new hazelcast::util::ILogger("StartedThread", "StartedThread", "testversion",
                                                      client::config::LoggerConfig())) {
            init(func, arg0, arg1, arg2, arg3);
        }

        void StartedThread::init(void (func)(ThreadArgs &), void *arg0, void *arg1, void *arg2, void *arg3) {
            threadArgs.arg0 = arg0;
            threadArgs.arg1 = arg1;
            threadArgs.arg2 = arg2;
            threadArgs.arg3 = arg3;
            threadArgs.func = func;
            if (!logger->start()) {
                throw (client::exception::ExceptionBuilder<client::exception::IllegalStateException>(
                        "StartedThread::init") << "Could not start logger " << logger->getInstanceName()).build();
            }

            thread = std::thread([=]() { func(threadArgs); });
        }

        void StartedThread::run() {
            threadArgs.func(threadArgs);
        }

        const std::string StartedThread::getName() const {
            return name;
        }

        bool StartedThread::join() {
            if (!thread.joinable()) {
                return false;
            }
            thread.join();
            return true;
        }

        StartedThread::~StartedThread() {
            join();
        }

    }
}


#ifdef HZ_BUILD_WITH_SSL
#endif

namespace hazelcast {
    namespace client {
        namespace test {
            class ClientConnectionTest : public ClientTestSupport {
            protected:
#ifdef HZ_BUILD_WITH_SSL
                std::vector<hazelcast::client::internal::socket::SSLSocket::CipherInfo> getCiphers(ClientConfig &config) {
                    HazelcastClient client(config);
                    spi::ClientContext context(client);
                    std::vector<std::shared_ptr<connection::Connection> > conns = context.getConnectionManager().getActiveConnections();
                    EXPECT_GT(conns.size(), (size_t) 0);
                    std::shared_ptr<connection::Connection> aConnection = conns[0];
                    hazelcast::client::internal::socket::SSLSocket &socket = (hazelcast::client::internal::socket::SSLSocket &) aConnection->getSocket();
                    return socket.getCiphers();
                }

#endif
            };

            TEST_F(ClientConnectionTest, testTcpSocketTimeoutToOutsideNetwork) {
                HazelcastServer instance(*g_srvFactory);
                ClientConfig config;
                config.getNetworkConfig().setConnectionAttemptPeriod(1000).setConnectionTimeout(2000).addAddress(
                        Address("8.8.8.8", 5701));
                ASSERT_THROW(HazelcastClient client(config), exception::IllegalStateException);
            }

#ifdef HZ_BUILD_WITH_SSL
            TEST_F(ClientConnectionTest, testSslSocketTimeoutToOutsideNetwork) {
                HazelcastServerFactory sslFactory(g_srvFactory->getServerAddress(), getSslFilePath());
                HazelcastServer instance(sslFactory);
                ClientConfig config;
                config.getNetworkConfig().setConnectionAttemptPeriod(1000).setConnectionTimeout(2000).addAddress(
                        Address("8.8.8.8", 5701)).getSSLConfig().setEnabled(true).addVerifyFile(getCAFilePath());
                ASSERT_THROW(HazelcastClient client(config), exception::IllegalStateException);
            }

            TEST_F(ClientConnectionTest, testSSLWrongCAFilePath) {
                HazelcastServerFactory sslFactory(g_srvFactory->getServerAddress(), getSslFilePath());
                HazelcastServer instance(sslFactory);
                ClientConfig config = getConfig();
                config.getNetworkConfig().getSSLConfig().setEnabled(true).addVerifyFile("abc");
                ASSERT_THROW(HazelcastClient client(config), exception::IllegalStateException);
            }

            TEST_F(ClientConnectionTest, testExcludedCipher) {
                HazelcastServerFactory sslFactory(g_srvFactory->getServerAddress(), getSslFilePath());
                HazelcastServer instance(sslFactory);

                ClientConfig config = getConfig();
                config.getNetworkConfig().getSSLConfig().setEnabled(true).addVerifyFile(getCAFilePath()).setCipherList(
                        "HIGH");
                std::vector<hazelcast::client::internal::socket::SSLSocket::CipherInfo> supportedCiphers = getCiphers(
                        config);

                std::string unsupportedCipher = supportedCiphers[supportedCiphers.size() - 1].name;
                config = getConfig();
                config.getNetworkConfig().getSSLConfig().setEnabled(true).addVerifyFile(getCAFilePath()).
                        setCipherList(std::string("HIGH:!") + unsupportedCipher);

                std::vector<hazelcast::client::internal::socket::SSLSocket::CipherInfo> newCiphers = getCiphers(config);

                for (std::vector<hazelcast::client::internal::socket::SSLSocket::CipherInfo>::const_iterator it = newCiphers.begin();
                     it != newCiphers.end(); ++it) {
                    ASSERT_NE(unsupportedCipher, it->name) << ", expected ciphers list lis:" << supportedCiphers.size()
                                                           << ", actual size of new ciphers:" << newCiphers.size();
                }

                ASSERT_EQ(supportedCiphers.size() - 1, newCiphers.size());
            }

#endif
        }
    }
}






namespace hazelcast {
    namespace client {
        namespace test {
            class ClusterTest : public ClientTestSupportBase, public ::testing::TestWithParam<ClientConfig *> {
            public:
                ClusterTest() : sslFactory(g_srvFactory->getServerAddress(), getSslFilePath()) {}

            protected:
                class ClientAllStatesListener : public LifecycleListener {
                public:

                    ClientAllStatesListener(boost::latch *startingLatch,
                                            boost::latch *startedLatch = NULL,
                                            boost::latch *connectedLatch = NULL,
                                            boost::latch *disconnectedLatch = NULL,
                                            boost::latch *shuttingDownLatch = NULL,
                                            boost::latch *shutdownLatch = NULL)
                            : startingLatch(startingLatch), startedLatch(startedLatch), connectedLatch(connectedLatch),
                              disconnectedLatch(disconnectedLatch), shuttingDownLatch(shuttingDownLatch),
                              shutdownLatch(shutdownLatch) {}

                    virtual void stateChanged(const LifecycleEvent &lifecycleEvent) {
                        switch (lifecycleEvent.getState()) {
                            case LifecycleEvent::STARTING:
                                if (startingLatch) {
                                    startingLatch->count_down();
                                }
                                break;
                            case LifecycleEvent::STARTED:
                                if (startedLatch) {
                                    startedLatch->count_down();
                                }
                                break;
                            case LifecycleEvent::CLIENT_CONNECTED:
                                if (connectedLatch) {
                                    connectedLatch->count_down();
                                }
                                break;
                            case LifecycleEvent::CLIENT_DISCONNECTED:
                                if (disconnectedLatch) {
                                    disconnectedLatch->count_down();
                                }
                                break;
                            case LifecycleEvent::SHUTTING_DOWN:
                                if (shuttingDownLatch) {
                                    shuttingDownLatch->count_down();
                                }
                                break;
                            case LifecycleEvent::SHUTDOWN:
                                if (shutdownLatch) {
                                    shutdownLatch->count_down();
                                }
                                break;
                            default:
                                FAIL() << "No such state expected:" << lifecycleEvent.getState();
                        }
                    }

                private:
                    boost::latch *startingLatch;
                    boost::latch *startedLatch;
                    boost::latch *connectedLatch;
                    boost::latch *disconnectedLatch;
                    boost::latch *shuttingDownLatch;
                    boost::latch *shutdownLatch;
                };

                std::unique_ptr<HazelcastServer> startServer(ClientConfig &clientConfig) {
                    if (clientConfig.getNetworkConfig().getSSLConfig().isEnabled()) {
                        return std::unique_ptr<HazelcastServer>(new HazelcastServer(sslFactory));
                    } else {
                        return std::unique_ptr<HazelcastServer>(new HazelcastServer(*g_srvFactory));
                    }
                }

            private:
                HazelcastServerFactory sslFactory;
            };

            class SmartTcpClientConfig : public ClientConfig {
            };

            class SmartSSLClientConfig : public ClientConfig {
            public:
                SmartSSLClientConfig() {
                    this->getNetworkConfig().getSSLConfig().setEnabled(true).addVerifyFile(
                            ClientTestSupportBase::getCAFilePath());
                }
            };

            TEST_P(ClusterTest, testBehaviourWhenClusterNotFound) {
                ClientConfig &clientConfig = *const_cast<ParamType &>(GetParam());
                ASSERT_THROW(HazelcastClient client(clientConfig), exception::IllegalStateException);
            }

            TEST_P(ClusterTest, testDummyClientBehaviourWhenClusterNotFound) {
                ClientConfig &clientConfig = *const_cast<ParamType &>(GetParam());
                clientConfig.setSmart(false);
                ASSERT_THROW(HazelcastClient client(clientConfig), exception::IllegalStateException);
            }

            TEST_P(ClusterTest, testAllClientStates) {
                ClientConfig &clientConfig = *const_cast<ParamType &>(GetParam());

                std::unique_ptr<HazelcastServer> instance = startServer(clientConfig);

                clientConfig.setAttemptPeriod(1000);
                clientConfig.setConnectionAttemptLimit(1);
                boost::latch startingLatch(1);
                boost::latch startedLatch(1);
                boost::latch connectedLatch(1);
                boost::latch disconnectedLatch(1);
                boost::latch shuttingDownLatch(1);
                boost::latch shutdownLatch(1);
                ClientAllStatesListener listener(&startingLatch, &startedLatch, &connectedLatch, &disconnectedLatch,
                                                 &shuttingDownLatch, &shutdownLatch);
                clientConfig.addListener(&listener);

                HazelcastClient client(clientConfig);

                ASSERT_EQ(boost::cv_status::no_timeout, startingLatch.wait_for(boost::chrono::seconds(0)));
                ASSERT_EQ(boost::cv_status::no_timeout, startedLatch.wait_for(boost::chrono::seconds(0)));
                ASSERT_EQ(boost::cv_status::no_timeout, connectedLatch.wait_for(boost::chrono::seconds(0)));

                instance->shutdown();

                ASSERT_OPEN_EVENTUALLY(disconnectedLatch);
                ASSERT_OPEN_EVENTUALLY(shuttingDownLatch);
                ASSERT_OPEN_EVENTUALLY(shutdownLatch);
            }

            TEST_P(ClusterTest, testConnectionAttemptPeriod) {
                ClientConfig &clientConfig = *const_cast<ParamType &>(GetParam());
                clientConfig.getNetworkConfig().setConnectionAttemptPeriod(900).
                        setConnectionTimeout(2000).setConnectionAttemptLimit(2);
                clientConfig.getNetworkConfig().addAddress(Address("8.8.8.8", 8000));

                int64_t startTimeMillis = hazelcast::util::currentTimeMillis();
                try {
                    HazelcastClient client(clientConfig);
                } catch (exception::IllegalStateException &) {
// this is expected
                }
                ASSERT_GE(hazelcast::util::currentTimeMillis() - startTimeMillis, 2 * 900);
            }

            TEST_P(ClusterTest, testAllClientStatesWhenUserShutdown) {
                ClientConfig &clientConfig = *const_cast<ParamType &>(GetParam());
                std::unique_ptr<HazelcastServer> instance = startServer(clientConfig);

                boost::latch startingLatch(1);
                boost::latch startedLatch(1);
                boost::latch connectedLatch(1);
                boost::latch disconnectedLatch(1);
                boost::latch shuttingDownLatch(1);
                boost::latch shutdownLatch(1);
                ClientAllStatesListener listener(&startingLatch, &startedLatch, &connectedLatch, &disconnectedLatch,
                                                 &shuttingDownLatch, &shutdownLatch);
                clientConfig.addListener(&listener);

                HazelcastClient client(clientConfig);

                ASSERT_EQ(boost::cv_status::no_timeout, startingLatch.wait_for(boost::chrono::seconds(0)));
                ASSERT_EQ(boost::cv_status::no_timeout, startedLatch.wait_for(boost::chrono::seconds(0)));
                ASSERT_EQ(boost::cv_status::no_timeout, connectedLatch.wait_for(boost::chrono::seconds(0)));

                client.shutdown();

                ASSERT_OPEN_EVENTUALLY(shuttingDownLatch);
                ASSERT_OPEN_EVENTUALLY(shutdownLatch);
            }

#ifdef HZ_BUILD_WITH_SSL

            INSTANTIATE_TEST_SUITE_P(All,
                                     ClusterTest,
                                     ::testing::Values(new SmartTcpClientConfig(), new SmartSSLClientConfig()));
#else
            INSTANTIATE_TEST_SUITE_P(All,
                                     ClusterTest,
                                     ::testing::Values(new SmartTcpClientConfig()));                                                                                                                                    INSTANTIATE_TEST_SUITE_P(All,
                                    ClusterTest,
                                    ::testing::Values(new SmartTcpClientConfig()));
#endif
        }
    }
}





namespace hazelcast {
    namespace client {
        namespace test {
            class HeartbeatTest : public ClientTestSupport {
            };

            TEST_F(HeartbeatTest, testPing) {
                HazelcastServer instance(*g_srvFactory);
                ClientConfig config = getConfig();
                config.setProperty("hazelcast_client_heartbeat_interval", "1");

                HazelcastClient client(config);

                // sleep enough time so that the client ping is sent to the server
                hazelcast::util::sleep(3);
            }
        }
    }
}



namespace hazelcast {
    namespace client {
        namespace test {
            class SocketInterceptorTest : public ClientTestSupport {
            };

            class MySocketInterceptor : public SocketInterceptor {
            public:
                MySocketInterceptor(boost::latch &latch1) : interceptorLatch(latch1) {
                }

                void onConnect(const hazelcast::client::Socket &connectedSocket) {
                    ASSERT_EQ("127.0.0.1", connectedSocket.getAddress().getHost());
                    ASSERT_NE(0, connectedSocket.getAddress().getPort());
                    interceptorLatch.count_down();
                }

            private:
                boost::latch &interceptorLatch;
            };

#ifdef HZ_BUILD_WITH_SSL
            TEST_F(SocketInterceptorTest, interceptSSLBasic) {
                HazelcastServerFactory sslFactory(g_srvFactory->getServerAddress(), getSslFilePath());
                HazelcastServer instance(sslFactory);
                ClientConfig config = getConfig();
                boost::latch interceptorLatch(1);
                MySocketInterceptor interceptor(interceptorLatch);
                config.setSocketInterceptor(&interceptor);
                config::SSLConfig sslConfig;
                sslConfig.setEnabled(true).addVerifyFile(getCAFilePath());
                config.getNetworkConfig().setSSLConfig(sslConfig);
                HazelcastClient client(config);
                interceptorLatch.wait_for(boost::chrono::seconds(2));
            }

#endif

            TEST_F(SocketInterceptorTest, interceptBasic) {
                HazelcastServer instance(*g_srvFactory);
                ClientConfig config = getConfig();
                boost::latch interceptorLatch(1);
                MySocketInterceptor interceptor(interceptorLatch);
                config.setSocketInterceptor(&interceptor);
                HazelcastClient client(config);
                interceptorLatch.wait_for(boost::chrono::seconds(2));
            }
        }
    }
}




namespace hazelcast {
    namespace client {
        namespace test {
            class SocketOptionsTest : public ClientTestSupport {
            };

            TEST_F(SocketOptionsTest, testConfiguration) {
                HazelcastServer instance(*g_srvFactory);

                const int bufferSize = 2 * 1024;
                ClientConfig clientConfig;
                clientConfig.getNetworkConfig().getSocketOptions().setKeepAlive(false).setReuseAddress(
                        true).setTcpNoDelay(false).setLingerSeconds(5).setBufferSizeInBytes(bufferSize);

                HazelcastClient client(clientConfig);

                config::SocketOptions &socketOptions = client.getClientConfig().getNetworkConfig().getSocketOptions();
                ASSERT_FALSE(socketOptions.isKeepAlive());
                ASSERT_FALSE(socketOptions.isTcpNoDelay());
                ASSERT_EQ(5, socketOptions.getLingerSeconds());
                ASSERT_EQ(bufferSize, socketOptions.getBufferSizeInBytes());
            }
        }
    }
}




namespace hazelcast {
    namespace client {
        namespace test {
            class ClientAuthenticationTest : public ClientTestSupport {
            };

            TEST_F(ClientAuthenticationTest, testSetGroupConfig) {
                HazelcastServer instance(*g_srvFactory);
                ClientConfig config;
                config.setGroupConfig(GroupConfig("dev", "dev-pass"));

                HazelcastClient client(config);
            }

            TEST_F(ClientAuthenticationTest, testIncorrectGroupName) {
                HazelcastServer instance(*g_srvFactory);
                ClientConfig config;
                config.getGroupConfig().setName("invalid cluster");

                ASSERT_THROW((HazelcastClient(config)), exception::IllegalStateException);
            }
        }
    }
}



namespace hazelcast {
    namespace client {
        namespace test {
            class ClientEnpointTest : public ClientTestSupport {
            };

            TEST_F(ClientEnpointTest, testConnectedClientEnpoint) {
                HazelcastServer instance(*g_srvFactory);

                HazelcastClient client;
                const Client endpoint = client.getLocalEndpoint();
                spi::ClientContext context(client);
                ASSERT_EQ(context.getName(), endpoint.getName());

                std::shared_ptr<Address> endpointAddress = endpoint.getSocketAddress();
                ASSERT_NOTNULL(endpointAddress.get(), Address);
                connection::ClientConnectionManagerImpl &connectionManager = context.getConnectionManager();
                std::shared_ptr<connection::Connection> connection = connectionManager.getOwnerConnection();
                ASSERT_NOTNULL(connection.get(), connection::Connection);
                std::unique_ptr<Address> localAddress = connection->getLocalSocketAddress();
                ASSERT_NOTNULL(localAddress.get(), Address);
                ASSERT_EQ(*localAddress, *endpointAddress);

                std::shared_ptr<protocol::Principal> principal = connectionManager.getPrincipal();
                ASSERT_NOTNULL(principal.get(), protocol::Principal);
                ASSERT_NOTNULL(principal->getUuid(), std::string);
                ASSERT_EQ_PTR((*principal->getUuid()), endpoint.getUuid().get(), std::string);
            }
        }
    }
}






namespace hazelcast {
    namespace client {

        class HazelcastClient;

        namespace test {
            class MemberAttributeTest : public ClientTestSupport
            {};

            TEST_F(MemberAttributeTest, testInitialValues) {
                HazelcastServer instance(*g_srvFactory);
                ASSERT_TRUE(instance.setAttributes(0));
                HazelcastClient hazelcastClient(getNewClient());
                Cluster cluster = hazelcastClient.getCluster();
                std::vector<Member> members = cluster.getMembers();
                ASSERT_EQ(1U, members.size());
                Member &member = members[0];
                ASSERT_TRUE(member.lookupAttribute("intAttr"));
                ASSERT_EQ("211", *member.getAttribute("intAttr"));

                ASSERT_TRUE(member.lookupAttribute("boolAttr"));
                ASSERT_EQ("true", *member.getAttribute("boolAttr"));

                ASSERT_TRUE(member.lookupAttribute("byteAttr"));
                ASSERT_EQ("7", *member.getAttribute("byteAttr"));

                ASSERT_TRUE(member.lookupAttribute("doubleAttr"));
                ASSERT_EQ("2.0", *member.getAttribute("doubleAttr"));

                ASSERT_TRUE(member.lookupAttribute("floatAttr"));
                ASSERT_EQ("1.2", *member.getAttribute("floatAttr"));

                ASSERT_TRUE(member.lookupAttribute("shortAttr"));
                ASSERT_EQ("3", *member.getAttribute("shortAttr"));

                ASSERT_TRUE(member.lookupAttribute("strAttr"));
                ASSERT_EQ(std::string("strAttr"), *member.getAttribute("strAttr"));

                instance.shutdown();
            }

            class AttributeListener : public MembershipListener {
            public:
                AttributeListener(boost::latch &_attributeLatch)
                        : _attributeLatch(_attributeLatch) {

                }

                void memberAdded(const MembershipEvent &event) {
                }

                void memberRemoved(const MembershipEvent &event) {
                }

                void memberAttributeChanged(const MemberAttributeEvent &memberAttributeEvent) {
                    if (memberAttributeEvent.getOperationType() != MemberAttributeEvent::PUT) {
                        return;
                    }
                    const std::string &key = memberAttributeEvent.getKey();

                    if (key == "intAttr") {
                        const std::string &value = memberAttributeEvent.getValue();
                        if ("211" == value) {
                            _attributeLatch.count_down();
                        }
                    } else if (key == "boolAttr") {
                        const std::string &value = memberAttributeEvent.getValue();
                        if ("true" == value) {
                            _attributeLatch.count_down();
                        }
                    } else if (key == "byteAttr") {
                        const std::string &value = memberAttributeEvent.getValue();
                        if ("7" == value) {
                            _attributeLatch.count_down();
                        }
                    } else if (key == "doubleAttr") {
                        const std::string &value = memberAttributeEvent.getValue();
                        if ("2.0" == value) {
                            _attributeLatch.count_down();
                        }
                    } else if (key == "floatAttr") {
                        const std::string &value = memberAttributeEvent.getValue();
                        if ("1.2" == value) {
                            _attributeLatch.count_down();
                        }
                    } else if (key == "shortAttr") {
                        const std::string &value = memberAttributeEvent.getValue();
                        if ("3" == value) {
                            _attributeLatch.count_down();
                        }
                    } else if (key == "strAttr") {
                        const std::string &value = memberAttributeEvent.getValue();
                        if (std::string("strAttr") == value) {
                            _attributeLatch.count_down();
                        }
                    }
                }

            private:
                boost::latch &_attributeLatch;
            };

            TEST_F(MemberAttributeTest, testChangeWithListeners) {
                boost::latch attributeLatch(7);
                AttributeListener sampleListener(attributeLatch);

                ClientConfig clientConfig(getConfig());
                clientConfig.addListener(&sampleListener);

                HazelcastServer instance(*g_srvFactory);
                HazelcastClient hazelcastClient(clientConfig);

                HazelcastServer instance2(*g_srvFactory);
                ASSERT_TRUE(instance2.setAttributes(1));

                ASSERT_EQ(boost::cv_status::no_timeout, attributeLatch.wait_for(boost::chrono::seconds(30)));

                instance2.shutdown();

                instance.shutdown();
            }

        }
    }
}




namespace hazelcast {
    namespace client {
        namespace test {
            namespace crdt {
                namespace pncounter {
                    class BasicPnCounterAPITest : public ClientTestSupport {
                    public:
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

                    HazelcastServer *BasicPnCounterAPITest::instance = NULL;
                    HazelcastClient *BasicPnCounterAPITest::client = NULL;

                    TEST_F(BasicPnCounterAPITest, testGetStart) {
                        std::shared_ptr<client::crdt::pncounter::PNCounter> pnCounter = client->getPNCounter(
                                testing::UnitTest::GetInstance()->current_test_info()->name());
                                assertEquals(0, pnCounter->get());
                    }

                    TEST_F(BasicPnCounterAPITest, testGetAndAdd) {
                        std::shared_ptr<client::crdt::pncounter::PNCounter> pnCounter = client->getPNCounter(
                                testing::UnitTest::GetInstance()->current_test_info()->name());
                                assertEquals(0, pnCounter->getAndAdd(5));
                    }

                    TEST_F(BasicPnCounterAPITest, testAddAndGet) {
                        std::shared_ptr<client::crdt::pncounter::PNCounter> pnCounter = client->getPNCounter(
                                testing::UnitTest::GetInstance()->current_test_info()->name());
                                assertEquals(5, pnCounter->addAndGet(5));
                    }

                    TEST_F(BasicPnCounterAPITest, testGetAndAddExisting) {
                        std::shared_ptr<client::crdt::pncounter::PNCounter> pnCounter = client->getPNCounter(
                                testing::UnitTest::GetInstance()->current_test_info()->name());

                                assertEquals(0, pnCounter->getAndAdd(2));
                                assertEquals(2, pnCounter->getAndAdd(3));
                                assertEquals(5, pnCounter->get());
                    }

                    TEST_F(BasicPnCounterAPITest, testGetAndIncrement) {
                        std::shared_ptr<client::crdt::pncounter::PNCounter> pnCounter = client->getPNCounter(
                                testing::UnitTest::GetInstance()->current_test_info()->name());
                                assertEquals(0, pnCounter->getAndIncrement());
                                assertEquals(1, pnCounter->getAndIncrement());
                                assertEquals(2, pnCounter->get());
                    }

                    TEST_F(BasicPnCounterAPITest, testIncrementAndGet) {
                        std::shared_ptr<client::crdt::pncounter::PNCounter> pnCounter = client->getPNCounter(
                                testing::UnitTest::GetInstance()->current_test_info()->name());
                                assertEquals(1, pnCounter->incrementAndGet());
                                assertEquals(1, pnCounter->get());
                    }

                    TEST_F(BasicPnCounterAPITest, testGetAndDecrementFromDefault) {
                        std::shared_ptr<client::crdt::pncounter::PNCounter> pnCounter = client->getPNCounter(
                                testing::UnitTest::GetInstance()->current_test_info()->name());
                                assertEquals(0, pnCounter->getAndDecrement());
                                assertEquals(-1, pnCounter->get());
                    }

                    TEST_F(BasicPnCounterAPITest, testGetAndDecrement) {
                        std::shared_ptr<client::crdt::pncounter::PNCounter> pnCounter = client->getPNCounter(
                                testing::UnitTest::GetInstance()->current_test_info()->name());
                                assertEquals(1, pnCounter->incrementAndGet());
                                assertEquals(1, pnCounter->getAndDecrement());
                                assertEquals(0, pnCounter->get());
                    }

                    TEST_F(BasicPnCounterAPITest, testGetAndSubtract) {
                        std::shared_ptr<client::crdt::pncounter::PNCounter> pnCounter = client->getPNCounter(
                                testing::UnitTest::GetInstance()->current_test_info()->name());
                                assertEquals(0, pnCounter->getAndSubtract(2));
                                assertEquals(-2, pnCounter->get());
                    }

                    TEST_F(BasicPnCounterAPITest, testSubtractAndGet) {
                        std::shared_ptr<client::crdt::pncounter::PNCounter> pnCounter = client->getPNCounter(
                                testing::UnitTest::GetInstance()->current_test_info()->name());
                                assertEquals(-3, pnCounter->subtractAndGet(3));
                    }

                    TEST_F(BasicPnCounterAPITest, testReset) {
                        std::shared_ptr<client::crdt::pncounter::PNCounter> pnCounter = client->getPNCounter(
                                testing::UnitTest::GetInstance()->current_test_info()->name());
                        pnCounter->reset();
                    }
                }
            }

        }
    }
}



namespace hazelcast {
    namespace client {
        namespace test {
            namespace crdt {
                namespace pncounter {
                    /**
                     * Client implementation for testing behaviour of {@link ConsistencyLostException}
                     */
                    class ClientPNCounterConsistencyLostTest : public ClientTestSupport {
                    protected:
                        std::shared_ptr<Address> getCurrentTargetReplicaAddress(
                                const std::shared_ptr<client::crdt::pncounter::PNCounter> &pnCounter) {
                            return std::static_pointer_cast<proxy::ClientPNCounterProxy>(pnCounter)->getCurrentTargetReplicaAddress();
                        }

                        void terminateMember(const Address &address, HazelcastServer &server1, HazelcastServer &server2) {
                            auto member1 = server1.getMember();
                            if (address == Address(member1.host, member1.port)) {
                                server1.terminate();
                                return;
                            }

                            auto member2 = server2.getMember();
                            if (address == Address(member2.host, member2.port)) {
                                server2.terminate();
                                return;
                            }
                        }
                    };

                    TEST_F(ClientPNCounterConsistencyLostTest, consistencyLostExceptionIsThrownWhenTargetReplicaDisappears) {
                        HazelcastServerFactory factory(
                                "hazelcast/test/resources/hazelcast-pncounter-consistency-lost-test.xml");
                        HazelcastServer instance(factory);
                        HazelcastServer instance2(factory);

                        HazelcastClient client;

                        std::shared_ptr<client::crdt::pncounter::PNCounter> pnCounter = client.getPNCounter(
                                testing::UnitTest::GetInstance()->current_test_info()->name());

                        pnCounter->addAndGet(5);

                                assertEquals(5, pnCounter->get());

                        std::shared_ptr<Address> currentTarget = getCurrentTargetReplicaAddress(pnCounter);

                        terminateMember(*currentTarget, instance, instance2);

                        ASSERT_THROW(pnCounter->addAndGet(5), exception::ConsistencyLostException);
                    }

                    TEST_F(ClientPNCounterConsistencyLostTest, driverCanContinueSessionByCallingReset) {
                        HazelcastServerFactory factory(
                                "hazelcast/test/resources/hazelcast-pncounter-consistency-lost-test.xml");
                        HazelcastServer instance(factory);
                        HazelcastServer instance2(factory);

                        HazelcastClient client;

                        std::shared_ptr<client::crdt::pncounter::PNCounter> pnCounter = client.getPNCounter(
                                testing::UnitTest::GetInstance()->current_test_info()->name());

                        pnCounter->addAndGet(5);

                                assertEquals(5, pnCounter->get());

                        std::shared_ptr<Address> currentTarget = getCurrentTargetReplicaAddress(pnCounter);

                        terminateMember(*currentTarget, instance, instance2);

                        pnCounter->reset();

                        pnCounter->addAndGet(5);
                    }
                }
            }

        }
    }
}





namespace hazelcast {
    namespace client {
        namespace test {
            namespace crdt {
                namespace pncounter {
                    class PnCounterFunctionalityTest : public ClientTestSupport {
                    public:
                        static void SetUpTestCase() {
                            instance = new HazelcastServer(*g_srvFactory);
                            client = new HazelcastClient;
                        }

                        static void TearDownTestCase() {
                            delete client;
                            delete instance;

                            client = NULL;
                            instance = NULL;
                        }

                    protected:
                        static HazelcastServer *instance;
                        static HazelcastClient *client;
                    };

                    HazelcastServer *PnCounterFunctionalityTest::instance = NULL;
                    HazelcastClient *PnCounterFunctionalityTest::client = NULL;

                    TEST_F(PnCounterFunctionalityTest, testSimpleReplication) {
                        const char *name = testing::UnitTest::GetInstance()->current_test_info()->name();
                        std::shared_ptr<client::crdt::pncounter::PNCounter> counter1 = client->getPNCounter(name);
                        std::shared_ptr<client::crdt::pncounter::PNCounter> counter2 = client->getPNCounter(name);

                                assertEquals(5, counter1->addAndGet(5));

                        ASSERT_EQ_EVENTUALLY(5, counter1->get());
                        ASSERT_EQ_EVENTUALLY(5, counter2->get());
                    }

                    TEST_F(PnCounterFunctionalityTest, testParallelism) {
                        const char *name = testing::UnitTest::GetInstance()->current_test_info()->name();
                        std::shared_ptr<client::crdt::pncounter::PNCounter> counter1 = client->getPNCounter(name);
                        std::shared_ptr<client::crdt::pncounter::PNCounter> counter2 = client->getPNCounter(name);

                        int parallelism = 5;
                        int loopsPerThread = 100;
                        std::atomic<int64_t> finalValue(0);

                        std::vector<std::future<void>> futures;
                        for (int i = 0; i < parallelism; i++) {
                            futures.push_back(std::async([&]() {
                                for (int j = 0; j < loopsPerThread; j++) {
                                    counter1->addAndGet(5);
                                    finalValue += 5;
                                    counter2->addAndGet(-2);
                                    finalValue += -2;
                                }
                            }));
                        }

                        boost::wait_for_all(futures.begin(), futures.end());

                        int64_t finalExpectedValue = 3 * (int64_t) loopsPerThread * parallelism;
                        ASSERT_EQ_EVENTUALLY(finalExpectedValue, counter1->get());
                        ASSERT_EQ_EVENTUALLY(finalExpectedValue, counter2->get());
                    }
                }
            }

        }
    }
}



namespace hazelcast {
    namespace client {
        namespace test {
            namespace crdt {
                namespace pncounter {
                    class ClientPNCounterNoDataMemberTest : public ClientTestSupport {
                    };

                    TEST_F(ClientPNCounterNoDataMemberTest, noDataMemberExceptionIsThrown) {
                        HazelcastServerFactory factory("hazelcast/test/resources/hazelcast-lite-member.xml");
                        HazelcastServer instance(factory);

                        HazelcastClient client;

                        std::shared_ptr<client::crdt::pncounter::PNCounter> pnCounter = client.getPNCounter(
                                testing::UnitTest::GetInstance()->current_test_info()->name());

                        ASSERT_THROW(pnCounter->addAndGet(5), exception::NoDataMemberInClusterException);
                    }

                }
            }

        }
    }
}





namespace hazelcast {
    namespace client {
        namespace test {
            class SimpleListenerTest : public ClientTestSupportBase, public ::testing::TestWithParam<ClientConfig *> {
            public:
                SimpleListenerTest() {}

            protected:
                class MyEntryListener : public EntryListener<int, int> {
                public:
                    MyEntryListener(boost::latch &mapClearedLatch) : mapClearedLatch(mapClearedLatch) {}

                    virtual void entryAdded(const EntryEvent<int, int> &event) {
                    }

                    virtual void entryRemoved(const EntryEvent<int, int> &event) {
                    }

                    virtual void entryUpdated(const EntryEvent<int, int> &event) {
                    }

                    virtual void entryEvicted(const EntryEvent<int, int> &event) {
                    }

                    virtual void entryExpired(const EntryEvent<int, int> &event) {
                    }

                    virtual void entryMerged(const EntryEvent<int, int> &event) {
                    }

                    virtual void mapEvicted(const MapEvent &event) {
                    }

                    virtual void mapCleared(const MapEvent &event) {
                        assertEquals("testDeregisterListener", event.getName());
                        assertEquals(EntryEventType::CLEAR_ALL, event.getEventType());
                        std::string hostName = event.getMember().getAddress().getHost();
                        assertTrue(hostName == "127.0.0.1" || hostName == "localhost");
                        assertEquals(5701, event.getMember().getAddress().getPort());
                        assertEquals(1, event.getNumberOfEntriesAffected());
                        std::cout << "Map cleared event received:" << event << std::endl;
                        mapClearedLatch.count_down();
                    }

                private:
                    boost::latch &mapClearedLatch;
                };

                class SampleInitialListener : public InitialMembershipListener {
                public:
                    SampleInitialListener(boost::latch &_memberAdded, boost::latch &_attributeLatch,
                                          boost::latch &_memberRemoved)
                            : _memberAdded(_memberAdded), _attributeLatch(_attributeLatch),
                              _memberRemoved(_memberRemoved) {
                    }

                    void init(const InitialMembershipEvent &event) {
                        std::vector<Member> const &members = event.getMembers();
                        if (members.size() == 1) {
                            _memberAdded.count_down();
                        }
                    }

                    void memberAdded(const MembershipEvent &event) {
                        _memberAdded.count_down();
                    }

                    void memberRemoved(const MembershipEvent &event) {
                        _memberRemoved.count_down();
                    }


                    void memberAttributeChanged(const MemberAttributeEvent &memberAttributeEvent) {
                        _attributeLatch.count_down();
                    }

                private:
                    boost::latch &_memberAdded;
                    boost::latch &_attributeLatch;
                    boost::latch &_memberRemoved;
                };

                class SampleListenerInSimpleListenerTest : public MembershipListener {
                public:
                    SampleListenerInSimpleListenerTest(boost::latch &_memberAdded,
                                                       boost::latch &_attributeLatch,
                                                       boost::latch &_memberRemoved)
                            : _memberAdded(_memberAdded), _attributeLatch(_attributeLatch),
                              _memberRemoved(_memberRemoved) {
                    }

                    void memberAdded(const MembershipEvent &event) {
                        _memberAdded.count_down();
                    }

                    void memberRemoved(const MembershipEvent &event) {
                        _memberRemoved.count_down();
                    }

                    void memberAttributeChanged(const MemberAttributeEvent &memberAttributeEvent) {
                        memberAttributeEvent.getKey();
                        _attributeLatch.count_down();
                    }

                private:
                    boost::latch &_memberAdded;
                    boost::latch &_attributeLatch;
                    boost::latch &_memberRemoved;
                };
            };

            class NonSmartTcpClientConfig : public ClientConfig {
            public:
                NonSmartTcpClientConfig() {
                    getNetworkConfig().setSmartRouting(false);
                }
            };

            TEST_P(SimpleListenerTest, testSharedClusterListeners) {
                HazelcastServer instance(*g_srvFactory);
                HazelcastClient hazelcastClient(*const_cast<ParamType &>(GetParam()));
                Cluster cluster = hazelcastClient.getCluster();
                boost::latch memberAdded(1);
                boost::latch memberAddedInit(2);
                boost::latch memberRemoved(1);
                boost::latch memberRemovedInit(1);
                boost::latch attributeLatch(7);
                boost::latch attributeLatchInit(7);

                std::shared_ptr<MembershipListener> sampleInitialListener(
                        new SampleInitialListener(memberAddedInit, attributeLatchInit, memberRemovedInit));
                std::shared_ptr<MembershipListener> sampleListener(
                        new SampleListenerInSimpleListenerTest(memberAdded, attributeLatch, memberRemoved));

                std::string initialListenerRegistrationId = cluster.addMembershipListener(sampleInitialListener);
                std::string sampleListenerRegistrationId = cluster.addMembershipListener(sampleListener);

                HazelcastServer instance2(*g_srvFactory);

                ASSERT_EQ(boost::cv_status::no_timeout, memberAdded.wait_for(boost::chrono::seconds(30)));
                ASSERT_EQ(boost::cv_status::no_timeout, memberAddedInit.wait_for(boost::chrono::seconds(30)));

                ASSERT_TRUE(instance2.setAttributes(1));

                ASSERT_EQ(boost::cv_status::no_timeout, attributeLatchInit.wait_for(boost::chrono::seconds(30)));
                ASSERT_EQ(boost::cv_status::no_timeout, attributeLatch.wait_for(boost::chrono::seconds(30)));

                instance2.shutdown();

                ASSERT_EQ(boost::cv_status::no_timeout, memberRemoved.wait_for(boost::chrono::seconds(30)));
                ASSERT_EQ(boost::cv_status::no_timeout, memberRemovedInit.wait_for(boost::chrono::seconds(30)));

                instance.shutdown();

                ASSERT_TRUE(cluster.removeMembershipListener(initialListenerRegistrationId));
                ASSERT_TRUE(cluster.removeMembershipListener(sampleListenerRegistrationId));
            }

            TEST_P(SimpleListenerTest, testClusterListeners) {
                HazelcastServer instance(*g_srvFactory);
                HazelcastClient hazelcastClient(*const_cast<ParamType &>(GetParam()));
                Cluster cluster = hazelcastClient.getCluster();
                boost::latch memberAdded(1);
                boost::latch memberAddedInit(2);
                boost::latch memberRemoved(1);
                boost::latch memberRemovedInit(1);
                boost::latch attributeLatch(7);
                boost::latch attributeLatchInit(7);

                SampleInitialListener sampleInitialListener(memberAddedInit, attributeLatchInit, memberRemovedInit);
                SampleListenerInSimpleListenerTest sampleListener(memberAdded, attributeLatch, memberRemoved);

                cluster.addMembershipListener(&sampleInitialListener);
                cluster.addMembershipListener(&sampleListener);

                HazelcastServer instance2(*g_srvFactory);

                ASSERT_EQ(boost::cv_status::no_timeout, memberAdded.wait_for(boost::chrono::seconds(30)));
                ASSERT_EQ(boost::cv_status::no_timeout, memberAddedInit.wait_for(boost::chrono::seconds(30)));

                ASSERT_TRUE(instance2.setAttributes(1));

                ASSERT_OPEN_EVENTUALLY(attributeLatchInit);
                ASSERT_OPEN_EVENTUALLY(attributeLatch);

                instance2.shutdown();

                ASSERT_OPEN_EVENTUALLY(memberRemoved);
                ASSERT_OPEN_EVENTUALLY(memberRemovedInit);

                instance.shutdown();

                ASSERT_TRUE(cluster.removeMembershipListener(&sampleInitialListener));
                ASSERT_TRUE(cluster.removeMembershipListener(&sampleListener));
            }

            TEST_P(SimpleListenerTest, testClusterListenersFromConfig) {
                boost::latch memberAdded(1);
                boost::latch memberAddedInit(2);
                boost::latch memberRemoved(1);
                boost::latch memberRemovedInit(1);
                boost::latch attributeLatch(7);
                boost::latch attributeLatchInit(7);
                SampleInitialListener sampleInitialListener(memberAddedInit, attributeLatchInit, memberRemovedInit);
                SampleListenerInSimpleListenerTest sampleListener(memberAdded, attributeLatch, memberRemoved);

                ClientConfig &clientConfig = *const_cast<ParamType &>(GetParam());
                clientConfig.addListener(&sampleListener);
                clientConfig.addListener(&sampleInitialListener);

                HazelcastServer instance(*g_srvFactory);
                HazelcastClient hazelcastClient(clientConfig);

                HazelcastServer instance2(*g_srvFactory);

                ASSERT_OPEN_EVENTUALLY(memberAdded);
                ASSERT_OPEN_EVENTUALLY(memberAddedInit);

                ASSERT_TRUE(instance2.setAttributes(1));

                ASSERT_OPEN_EVENTUALLY(attributeLatchInit);
                ASSERT_OPEN_EVENTUALLY(attributeLatch);

                instance2.shutdown();

                ASSERT_OPEN_EVENTUALLY(memberRemoved);
                ASSERT_OPEN_EVENTUALLY(memberRemovedInit);

                instance.shutdown();
            }

            TEST_P(SimpleListenerTest, testDeregisterListener) {
                HazelcastServer instance(*g_srvFactory);
                ClientConfig &clientConfig = *const_cast<ParamType &>(GetParam());
                HazelcastClient hazelcastClient(clientConfig);

                IMap<int, int> map = hazelcastClient.getMap<int, int>("testDeregisterListener");

                ASSERT_FALSE(map.removeEntryListener("Unknown"));

                boost::latch mapClearedLatch(1);
                MyEntryListener listener(mapClearedLatch);
                std::string listenerRegistrationId = map.addEntryListener(listener, true);

                map.put(1, 1);

                map.clear();

                        assertOpenEventually(mapClearedLatch);

                        assertTrue(map.removeEntryListener(listenerRegistrationId));
            }

            INSTANTIATE_TEST_SUITE_P(All,
                                     SimpleListenerTest,
                                     ::testing::Values(new SmartTcpClientConfig(), new NonSmartTcpClientConfig()));
        }
    }
}







namespace hazelcast {
    namespace client {
        namespace test {
            class FlakeIdGeneratorApiTest : public ClientTestSupport {
            public:
                FlakeIdGeneratorApiTest() : flakeIdGenerator(
                        client->getFlakeIdGenerator(testing::UnitTest::GetInstance()->current_test_info()->name())) {
                }

                static void SetUpTestCase() {
                    instance = new HazelcastServer(*g_srvFactory);
                    ClientConfig clientConfig = getConfig();
                    std::shared_ptr<config::ClientFlakeIdGeneratorConfig> flakeIdConfig(
                            new config::ClientFlakeIdGeneratorConfig("test*"));
                    flakeIdConfig->setPrefetchCount(10).setPrefetchValidityMillis(20000);
                    clientConfig.addFlakeIdGeneratorConfig(flakeIdConfig);
                    client = new HazelcastClient(clientConfig);
                }

                static void TearDownTestCase() {
                    delete client;
                    delete instance;

                    client = NULL;
                    instance = NULL;
                }

            protected:
                static HazelcastServer *instance;
                static HazelcastClient *client;

                FlakeIdGenerator flakeIdGenerator;
            };

            HazelcastServer *FlakeIdGeneratorApiTest::instance = NULL;
            HazelcastClient *FlakeIdGeneratorApiTest::client = NULL;

            TEST_F (FlakeIdGeneratorApiTest, testStartingValue) {
                flakeIdGenerator.newId();
            }

            TEST_F (FlakeIdGeneratorApiTest, testInit) {
                int64_t currentId = flakeIdGenerator.newId();
                        assertTrue(flakeIdGenerator.init(currentId / 2));
                        assertFalse(flakeIdGenerator.init(currentId * 2));
            }

            TEST_F (FlakeIdGeneratorApiTest, testSmoke) {
                boost::latch startLatch(1);
                std::array<std::future<std::set<int64_t>>, 4> futures;
                constexpr size_t NUM_IDS_PER_THREAD = 100000;
                constexpr int NUM_THREADS = 4;

                for (int i = 0; i < NUM_THREADS; ++i) {
                    futures[i] = std::async([&]() {
                        std::set<int64_t> localIds;
                        startLatch.wait();
                        for (size_t j = 0; j < NUM_IDS_PER_THREAD; ++j) {
                            localIds.insert(flakeIdGenerator.newId());
                        }

                        return localIds;
                    });
                }

                startLatch.count_down();

                std::set<int64_t> allIds;
                for (auto &f : futures) {
                    auto ids = f.get();
                    allIds.insert(ids.begin(), ids.end());
                }

// if there were duplicate IDs generated, there will be less items in the set than expected
                ASSERT_EQ(4 * NUM_IDS_PER_THREAD, allIds.size());
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace test {
            class ClientTxnMapTest : public ClientTestSupport {
            public:
                ClientTxnMapTest();

                ~ClientTxnMapTest();

            protected:
                HazelcastServer instance;
                ClientConfig clientConfig;
                HazelcastClient client;
            };

            ClientTxnMapTest::ClientTxnMapTest() : instance(*g_srvFactory), client(getNewClient()) {
            }

            ClientTxnMapTest::~ClientTxnMapTest() {
            }

            TEST_F(ClientTxnMapTest, testPutGet) {
                std::string name = "testPutGet";

                TransactionContext context = client.newTransactionContext();
                context.beginTransaction();

                TransactionalMap<std::string, std::string> map = context.getMap<std::string, std::string>(name);

                ASSERT_EQ(map.put("key1", "value1").get(), (std::string *) NULL);
                ASSERT_EQ("value1", *(map.get("key1")));
                std::shared_ptr<std::string> val = client.getMap<std::string, std::string>(name).get("key1");
                ASSERT_EQ(val.get(), (std::string *) NULL);

                context.commitTransaction();

                ASSERT_EQ("value1", *(client.getMap<std::string, std::string>(name).get("key1")));
            }

            TEST_F(ClientTxnMapTest, testRemove) {
                std::string name = "testRemove";

                TransactionContext context = client.newTransactionContext();
                context.beginTransaction();

                TransactionalMap<std::string, std::string> map = context.getMap<std::string, std::string>(name);

                ASSERT_EQ(map.put("key1", "value1").get(), (std::string *) NULL);
                ASSERT_EQ("value1", *(map.get("key1")));
                std::shared_ptr<std::string> val = client.getMap<std::string, std::string>(name).get("key1");
                ASSERT_EQ(val.get(), (std::string *) NULL);

                ASSERT_EQ((std::string *) NULL, map.remove("key2").get());
                val = map.remove("key1");
                ASSERT_NE((std::string *) NULL, val.get());
                ASSERT_EQ("value1", *val);

                context.commitTransaction();

                IMap<std::string, std::string> regularMap = client.getMap<std::string, std::string>(name);
                ASSERT_TRUE(regularMap.isEmpty());
            }

            TEST_F(ClientTxnMapTest, testRemoveIfSame) {
                std::string name = "testRemoveIfSame";

                TransactionContext context = client.newTransactionContext();
                context.beginTransaction();

                TransactionalMap<std::string, std::string> map = context.getMap<std::string, std::string>(name);

                ASSERT_EQ(map.put("key1", "value1").get(), (std::string *) NULL);
                ASSERT_EQ("value1", *(map.get("key1")));
                std::shared_ptr<std::string> val = client.getMap<std::string, std::string>(name).get("key1");
                ASSERT_EQ(val.get(), (std::string *) NULL);

                ASSERT_EQ((std::string *) NULL, map.remove("key2").get());
                ASSERT_TRUE(map.remove("key1", "value1"));

                context.commitTransaction();

                IMap<std::string, std::string> regularMap = client.getMap<std::string, std::string>(name);
                ASSERT_TRUE(regularMap.isEmpty());
            }

            TEST_F(ClientTxnMapTest, testDeleteEntry) {
                std::string name = "testDeleteEntry";

                TransactionContext context = client.newTransactionContext();
                context.beginTransaction();

                TransactionalMap<std::string, std::string> map = context.getMap<std::string, std::string>(name);

                ASSERT_NO_THROW(map.deleteEntry("key1"));

                ASSERT_EQ(map.put("key1", "value1").get(), (std::string *) NULL);
                ASSERT_EQ("value1", *(map.get("key1")));
                std::shared_ptr<std::string> val = client.getMap<std::string, std::string>(name).get("key1");
                ASSERT_EQ(val.get(), (std::string *) NULL);

                ASSERT_NO_THROW(map.deleteEntry("key1"));
                val = map.get("key1");
                ASSERT_EQ((std::string *) NULL, val.get());

                context.commitTransaction();

                IMap<std::string, std::string> regularMap = client.getMap<std::string, std::string>(name);
                ASSERT_TRUE(regularMap.isEmpty());
            }

            TEST_F(ClientTxnMapTest, testReplace) {
                std::string name = "testReplace";

                TransactionContext context = client.newTransactionContext();
                context.beginTransaction();

                TransactionalMap<std::string, std::string> map = context.getMap<std::string, std::string>(name);

                ASSERT_EQ(map.put("key1", "value1").get(), (std::string *) NULL);
                ASSERT_EQ("value1", *(map.get("key1")));
                std::shared_ptr<std::string> val = client.getMap<std::string, std::string>(name).get("key1");
                ASSERT_EQ(val.get(), (std::string *) NULL);

                ASSERT_EQ("value1", *map.replace("key1", "myNewValue"));

                context.commitTransaction();

                ASSERT_EQ("myNewValue", *(client.getMap<std::string, std::string>(name).get("key1")));
            }

            TEST_F(ClientTxnMapTest, testSet) {
                std::string name = "testSet";

                TransactionContext context = client.newTransactionContext();
                context.beginTransaction();

                TransactionalMap<std::string, std::string> map = context.getMap<std::string, std::string>(name);

                ASSERT_NO_THROW(map.set("key1", "value1"));

                std::shared_ptr<std::string> val = map.get("key1");
                ASSERT_NE((std::string *) NULL, val.get());
                ASSERT_EQ("value1", *val);

                val = client.getMap<std::string, std::string>(name).get("key1");
                ASSERT_EQ(val.get(), (std::string *) NULL);

                ASSERT_NO_THROW(map.set("key1", "myNewValue"));

                val = map.get("key1");
                ASSERT_NE((std::string *) NULL, val.get());
                ASSERT_EQ("myNewValue", *val);

                context.commitTransaction();

                val = client.getMap<std::string, std::string>(name).get("key1");
                ASSERT_NE((std::string *) NULL, val.get());
                ASSERT_EQ("myNewValue", *val);
            }

            TEST_F(ClientTxnMapTest, testContains) {
                std::string name = "testContains";

                TransactionContext context = client.newTransactionContext();
                context.beginTransaction();

                TransactionalMap<std::string, std::string> map = context.getMap<std::string, std::string>(name);

                ASSERT_FALSE(map.containsKey("key1"));

                ASSERT_NO_THROW(map.set("key1", "value1"));

                std::shared_ptr<std::string> val = map.get("key1");
                ASSERT_NE((std::string *) NULL, val.get());
                ASSERT_EQ("value1", *val);

                ASSERT_TRUE(map.containsKey("key1"));

                context.commitTransaction();

                IMap<std::string, std::string> regularMap = client.getMap<std::string, std::string>(name);
                ASSERT_TRUE(regularMap.containsKey("key1"));
            }

            TEST_F(ClientTxnMapTest, testReplaceIfSame) {
                std::string name = "testReplaceIfSame";

                TransactionContext context = client.newTransactionContext();
                context.beginTransaction();

                TransactionalMap<std::string, std::string> map = context.getMap<std::string, std::string>(name);

                ASSERT_EQ(map.put("key1", "value1").get(), (std::string *) NULL);
                ASSERT_EQ("value1", *(map.get("key1")));
                std::shared_ptr<std::string> val = client.getMap<std::string, std::string>(name).get("key1");
                ASSERT_EQ(val.get(), (std::string *) NULL);

                ASSERT_FALSE(map.replace("key1", "valueNonExistent", "myNewValue"));
                ASSERT_TRUE(map.replace("key1", "value1", "myNewValue"));

                context.commitTransaction();

                ASSERT_EQ("myNewValue", *(client.getMap<std::string, std::string>(name).get("key1")));
            }

            TEST_F(ClientTxnMapTest, testPutIfSame) {
                std::string name = "testPutIfSame";

                TransactionContext context = client.newTransactionContext();
                context.beginTransaction();

                TransactionalMap<std::string, std::string> map = context.getMap<std::string, std::string>(name);

                std::shared_ptr<std::string> val = map.putIfAbsent("key1", "value1");
                ASSERT_EQ((std::string *) NULL, val.get());
                val = map.get("key1");
                ASSERT_NE((std::string *) NULL, val.get());
                ASSERT_EQ("value1", *val);
                val = client.getMap<std::string, std::string>(name).get("key1");
                ASSERT_EQ(val.get(), (std::string *) NULL);

                val = map.putIfAbsent("key1", "value1");
                ASSERT_NE((std::string *) NULL, val.get());
                ASSERT_EQ("value1", *val);

                context.commitTransaction();

                val = client.getMap<std::string, std::string>(name).get("key1");
                ASSERT_NE((std::string *) NULL, val.get());
                ASSERT_EQ("value1", *val);
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
//                        latch2.count_down();
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
//                        latch1.count_down();
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

            TEST_F(ClientTxnMapTest, testKeySetValues) {
                std::string name = "testKeySetValues";
                IMap<std::string, std::string> map = client.getMap<std::string, std::string>(name);
                map.put("key1", "value1");
                map.put("key2", "value2");

                TransactionContext context = client.newTransactionContext();
                context.beginTransaction();
                TransactionalMap<std::string, std::string> txMap = context.getMap<std::string, std::string>(name);
                ASSERT_EQ(txMap.put("key3", "value3").get(), (std::string *) NULL);


                ASSERT_EQ(3, (int) txMap.size());
                ASSERT_EQ(3, (int) txMap.keySet().size());
                ASSERT_EQ(3, (int) txMap.values().size());
                context.commitTransaction();

                ASSERT_EQ(3, (int) map.size());
                ASSERT_EQ(3, (int) map.keySet().size());
                ASSERT_EQ(3, (int) map.values().size());

            }

            TEST_F(ClientTxnMapTest, testKeySetAndValuesWithPredicates) {
                std::string name = "testKeysetAndValuesWithPredicates";
                IMap<Employee, Employee> map = client.getMap<Employee, Employee>(name);

                Employee emp1("abc-123-xvz", 34);
                Employee emp2("abc-123-xvz", 20);

                map.put(emp1, emp1);

                TransactionContext context = client.newTransactionContext();
                context.beginTransaction();

                TransactionalMap<Employee, Employee> txMap = context.getMap<Employee, Employee>(name);
                ASSERT_EQ(txMap.put(emp2, emp2).get(), (Employee *) NULL);

                ASSERT_EQ(2, (int) txMap.size());
                ASSERT_EQ(2, (int) txMap.keySet().size());
                query::SqlPredicate predicate("a = 10");
                ASSERT_EQ(0, (int) txMap.keySet(&predicate).size());
                ASSERT_EQ(0, (int) txMap.values(&predicate).size());
                query::SqlPredicate predicate2("a >= 10");
                ASSERT_EQ(2, (int) txMap.keySet(&predicate2).size());
                ASSERT_EQ(2, (int) txMap.values(&predicate2).size());

                context.commitTransaction();

                ASSERT_EQ(2, (int) map.size());
                ASSERT_EQ(2, (int) map.values().size());
            }

            TEST_F(ClientTxnMapTest, testIsEmpty) {
                std::string name = "testIsEmpty";

                TransactionContext context = client.newTransactionContext();
                context.beginTransaction();

                TransactionalMap<std::string, std::string> map = context.getMap<std::string, std::string>(name);

                ASSERT_TRUE(map.isEmpty());

                std::shared_ptr<std::string> oldValue = map.put("key1", "value1");
                ASSERT_NULL("old value should be null", oldValue.get(), std::string);

                ASSERT_FALSE(map.isEmpty());

                context.commitTransaction();

                IMap<std::string, std::string> regularMap = client.getMap<std::string, std::string>(name);
                ASSERT_FALSE(regularMap.isEmpty());
            }

        }
    }
}


//
// Created by sancar koyunlu on 9/18/13.




namespace hazelcast {
    namespace client {
        namespace test {
            class ClientTxnSetTest : public ClientTestSupport {
            public:
                ClientTxnSetTest();

                ~ClientTxnSetTest();

            protected:
                HazelcastServer instance;
                HazelcastClient client;
            };

            ClientTxnSetTest::ClientTxnSetTest() : instance(*g_srvFactory), client(getNewClient()) {
            }

            ClientTxnSetTest::~ClientTxnSetTest() {
            }

            TEST_F(ClientTxnSetTest, testAddRemove) {
                ISet<std::string> s = client.getSet<std::string>("testAddRemove");
                s.add("item1");

                TransactionContext context = client.newTransactionContext();
                context.beginTransaction();
                TransactionalSet<std::string> set = context.getSet<std::string>("testAddRemove");
                ASSERT_TRUE(set.add("item2"));
                ASSERT_EQ(2, set.size());
                ASSERT_EQ(1, s.size());
                ASSERT_FALSE(set.remove("item3"));
                ASSERT_TRUE(set.remove("item1"));

                context.commitTransaction();

                ASSERT_EQ(1, s.size());
            }
        }
    }
}


//
// Created by sancar koyunlu on 9/18/13.




namespace hazelcast {
    namespace client {
        namespace test {
            class ClientTxnTest : public ClientTestSupport {
            public:
                ClientTxnTest();

                ~ClientTxnTest();

            protected:
                HazelcastServerFactory & hazelcastInstanceFactory;
                std::unique_ptr<HazelcastServer> server;
                std::unique_ptr<HazelcastServer> second;
                std::unique_ptr<HazelcastClient> client;
                std::unique_ptr<LoadBalancer> loadBalancer;
            };

            class MyLoadBalancer : public impl::AbstractLoadBalancer {
            public:
                const Member next() {
                    std::vector<Member> members = getMembers();
                    size_t len = members.size();
                    if (len == 0) {
                        BOOST_THROW_EXCEPTION(exception::IOException("const Member& RoundRobinLB::next()",
                                                                     "No member in member list!!"));
                    }
                    for (size_t i = 0; i < len; i++) {
                        if (members[i].getAddress().getPort() == 5701) {
                            return members[i];
                        }
                    }
                    return members[0];
                }

            };

            class MyMembershipListener : public MembershipListener {
            public:
                MyMembershipListener(boost::latch &countDownLatch)
                        : countDownLatch(countDownLatch) {

                }

                void memberAdded(const MembershipEvent &membershipEvent) {

                }

                void memberRemoved(const MembershipEvent &membershipEvent) {
                    countDownLatch.count_down();
                }

                void memberAttributeChanged(const MemberAttributeEvent& memberAttributeEvent) {

                }

            private:
                boost::latch &countDownLatch;
            };

            ClientTxnTest::ClientTxnTest()
                    : hazelcastInstanceFactory(*g_srvFactory) {
                server.reset(new HazelcastServer(hazelcastInstanceFactory));
                ClientConfig clientConfig = getConfig();
                //always start the txn on first member
                loadBalancer.reset(new MyLoadBalancer());
                clientConfig.setLoadBalancer(loadBalancer.get());
                client.reset(new HazelcastClient(clientConfig));
                second.reset(new HazelcastServer(hazelcastInstanceFactory));
            }

            ClientTxnTest::~ClientTxnTest() {
                client->shutdown();
                server->shutdown();
                second->shutdown();
            }

            TEST_F(ClientTxnTest, testTxnConnectAfterClientShutdown) {
                client->shutdown();
                ASSERT_THROW(client->newTransactionContext(), exception::HazelcastClientNotActiveException);
            }

            TEST_F(ClientTxnTest, testTxnCommitAfterClusterShutdown) {
                TransactionContext context = client->newTransactionContext();
                context.beginTransaction();

                server->shutdown();
                second->shutdown();

                ASSERT_THROW(context.commitTransaction(), exception::TransactionException);
            }

            TEST_F(ClientTxnTest, testTxnCommit) {
                std::string queueName = randomString();
                TransactionContext context = client->newTransactionContext();
                context.beginTransaction();
                ASSERT_FALSE(context.getTxnId().empty());
                TransactionalQueue <std::string> queue = context.getQueue<std::string>(queueName);
                std::string value = randomString();
                queue.offer(value);

                context.commitTransaction();

                IQueue<std::string> q = client->getQueue<std::string>(queueName);
                std::shared_ptr<std::string> retrievedElement = q.poll();
                ASSERT_NOTNULL(retrievedElement.get(), std::string);
                ASSERT_EQ(value, *retrievedElement);
            }

            TEST_F(ClientTxnTest, testTxnCommitUniSocket) {
                ClientConfig clientConfig;
                clientConfig.getNetworkConfig().setSmartRouting(false);
                HazelcastClient uniSocketClient(clientConfig);

                std::string queueName = randomString();
                TransactionContext context = uniSocketClient.newTransactionContext();
                context.beginTransaction();
                ASSERT_FALSE(context.getTxnId().empty());
                TransactionalQueue <std::string> queue = context.getQueue<std::string>(queueName);
                std::string value = randomString();
                queue.offer(value);

                context.commitTransaction();

                IQueue<std::string> q = uniSocketClient.getQueue<std::string>(queueName);
                std::shared_ptr<std::string> retrievedElement = q.poll();
                ASSERT_NOTNULL(retrievedElement.get(), std::string);
                ASSERT_EQ(value, *retrievedElement);
            }

            TEST_F(ClientTxnTest, testTxnCommitWithOptions) {
                std::string queueName = randomString();
                TransactionOptions transactionOptions;
                transactionOptions.setTransactionType(TransactionType::TWO_PHASE);
                transactionOptions.setTimeout(60);
                transactionOptions.setDurability(2);
                TransactionContext context = client->newTransactionContext(transactionOptions);

                context.beginTransaction();
                ASSERT_FALSE(context.getTxnId().empty());
                TransactionalQueue <std::string> queue = context.getQueue<std::string>(queueName);
                std::string value = randomString();
                queue.offer(value);

                context.commitTransaction();

                IQueue<std::string> q = client->getQueue<std::string>(queueName);
                std::shared_ptr<std::string> retrievedElement = q.poll();
                ASSERT_NOTNULL(retrievedElement.get(), std::string);
                ASSERT_EQ(value, *retrievedElement);
            }

            TEST_F(ClientTxnTest, testTxnCommitAfterClientShutdown) {
                std::string queueName = randomString();
                TransactionContext context = client->newTransactionContext();
                context.beginTransaction();
                TransactionalQueue <std::string> queue = context.getQueue<std::string>(queueName);
                std::string value = randomString();
                queue.offer(value);

                client->shutdown();

                ASSERT_THROW(context.commitTransaction(), exception::TransactionException);
            }


            TEST_F(ClientTxnTest, testTxnRollback) {
                std::string queueName = randomString();
                TransactionContext context = client->newTransactionContext();
                boost::latch txnRollbackLatch(1);
                boost::latch memberRemovedLatch(1);
                MyMembershipListener myLifecycleListener(memberRemovedLatch);
                client->getCluster().addMembershipListener(&myLifecycleListener);

                try {
                    context.beginTransaction();
                    ASSERT_FALSE(context.getTxnId().empty());
                    TransactionalQueue <std::string> queue = context.getQueue<std::string>(queueName);
                    queue.offer(randomString());

                    server->shutdown();

                    context.commitTransaction();
                    FAIL();
                } catch (exception::TransactionException &) {
                    context.rollbackTransaction();
                    txnRollbackLatch.count_down();
                }

                ASSERT_OPEN_EVENTUALLY(txnRollbackLatch);
                ASSERT_OPEN_EVENTUALLY(memberRemovedLatch);

                IQueue<std::string> q = client->getQueue<std::string>(queueName);
                ASSERT_NULL("Poll result should be null since it is rolled back", q.poll().get(), std::string);
                ASSERT_EQ(0, q.size());
            }

            TEST_F(ClientTxnTest, testTxnRollbackOnServerCrash) {
                std::string queueName = randomString();
                TransactionContext context = client->newTransactionContext();
                boost::latch txnRollbackLatch(1);
                boost::latch memberRemovedLatch(1);

                context.beginTransaction();

                TransactionalQueue <std::string> queue = context.getQueue<std::string>(queueName);
                queue.offer("str");

                MyMembershipListener myLifecycleListener(memberRemovedLatch);
                client->getCluster().addMembershipListener(&myLifecycleListener);

                server->shutdown();

                ASSERT_THROW(context.commitTransaction(), exception::TransactionException);

                context.rollbackTransaction();
                txnRollbackLatch.count_down();

                ASSERT_OPEN_EVENTUALLY(txnRollbackLatch);
                ASSERT_OPEN_EVENTUALLY(memberRemovedLatch);

                IQueue<std::string> q = client->getQueue<std::string>(queueName);
                ASSERT_NULL("queue poll should return null", q.poll().get(), std::string);
                ASSERT_EQ(0, q.size());
            }
        }
    }
}

//
// Created by sancar koyunlu on 9/18/13.




namespace hazelcast {
    namespace client {
        class HazelcastClient;

        namespace test {
            class ClientTxnListTest : public ClientTestSupport {
            public:
                ClientTxnListTest();

                ~ClientTxnListTest();

            protected:
                HazelcastServer instance;
                ClientConfig clientConfig;
                HazelcastClient client;
            };

            ClientTxnListTest::ClientTxnListTest() : instance(*g_srvFactory), client(getNewClient()) {
            }

            ClientTxnListTest::~ClientTxnListTest() {
            }

            TEST_F(ClientTxnListTest, testAddRemove) {
                IList<std::string> l = client.getList<std::string>("testAddRemove");
                l.add("item1");

                TransactionContext context = client.newTransactionContext();
                context.beginTransaction();
                TransactionalList<std::string> list = context.getList<std::string>("testAddRemove");
                ASSERT_TRUE(list.add("item2"));
                ASSERT_EQ(2, list.size());
                ASSERT_EQ(1, l.size());
                ASSERT_FALSE(list.remove("item3"));
                ASSERT_TRUE(list.remove("item1"));

                context.commitTransaction();

                ASSERT_EQ(1, l.size());
            }
        }
    }
}


//
// Created by sancar koyunlu on 9/18/13.



namespace hazelcast {
    namespace client {
        namespace test {
            class ClientTxnMultiMapTest : public ClientTestSupport {
            public:
                ClientTxnMultiMapTest();

                ~ClientTxnMultiMapTest();

            protected:
                class PutGetRemoveTestTask : public hazelcast::util::Runnable {
                public:
                    PutGetRemoveTestTask(HazelcastClient &client, MultiMap<std::string, std::string> &mm,
                                         boost::latch &latch1) : client(client), mm(mm), latch1(latch1) {}

                    virtual void run() {
                        std::string key = hazelcast::util::IOUtil::to_string(hazelcast::util::getCurrentThreadId());
                        std::string key2 = key + "2";
                        client.getMultiMap<std::string, std::string>("testPutGetRemove").put(key, "value");
                        TransactionContext context = client.newTransactionContext();
                        context.beginTransaction();
                        TransactionalMultiMap<std::string, std::string> multiMap = context.getMultiMap<std::string, std::string>(
                                "testPutGetRemove");
                        ASSERT_FALSE(multiMap.put(key, "value"));
                        ASSERT_TRUE(multiMap.put(key, "value1"));
                        ASSERT_TRUE(multiMap.put(key, "value2"));
                        ASSERT_TRUE(multiMap.put(key2, "value21"));
                        ASSERT_TRUE(multiMap.put(key2, "value22"));
                        ASSERT_EQ(3, (int) multiMap.get(key).size());
                        ASSERT_EQ(3, (int) multiMap.valueCount(key));
                        std::vector<std::string> removedValues = multiMap.remove(key2);
                        ASSERT_EQ(2U, removedValues.size())  ;
                        ASSERT_TRUE((removedValues[0] == "value21" && removedValues[1] == "value22") ||
                                    (removedValues[1] == "value21" && removedValues[0] == "value22"));
                        context.commitTransaction();

                        ASSERT_EQ(3, (int) mm.get(key).size());

                        latch1.count_down();
                    }

                    virtual const std::string getName() const {
                        return "PutGetRemoveTestTask";
                    }

                private:
                    HazelcastClient &client;
                    MultiMap<std::string, std::string> &mm;
                    boost::latch &latch1;
                };

                HazelcastServer instance;
                HazelcastClient client;
            };

            ClientTxnMultiMapTest::ClientTxnMultiMapTest()
                    : instance(*g_srvFactory), client(getNewClient()) {
            }

            ClientTxnMultiMapTest::~ClientTxnMultiMapTest() {
            }

            TEST_F(ClientTxnMultiMapTest, testRemoveIfExists) {
                TransactionContext context = client.newTransactionContext();
                context.beginTransaction();
                TransactionalMultiMap<std::string, std::string> multiMap = context.getMultiMap<std::string, std::string>(
                        "testRemoveIfExists");
                std::string key("MyKey");
                ASSERT_TRUE(multiMap.put(key, "value"));
                ASSERT_TRUE(multiMap.put(key, "value1"));
                ASSERT_TRUE(multiMap.put(key, "value2"));
                ASSERT_EQ(3, (int) multiMap.get(key).size());

                ASSERT_FALSE(multiMap.remove(key, "NonExistentValue"));
                ASSERT_TRUE(multiMap.remove(key, "value1"));

                ASSERT_EQ(2, multiMap.size());
                ASSERT_EQ(2, (int) multiMap.valueCount(key));

                context.commitTransaction();

                MultiMap<std::string, std::string> mm = client.getMultiMap<std::string, std::string>(
                        "testRemoveIfExists");
                ASSERT_EQ(2, (int) mm.get(key).size());
            }

            TEST_F(ClientTxnMultiMapTest, testPutGetRemove) {
                MultiMap<std::string, std::string> mm = client.getMultiMap<std::string, std::string>(
                        "testPutGetRemove");
                constexpr int n = 10;
                boost::latch latch1(n);

                for (int i = 0; i < n; i++) {
                    std::thread(std::packaged_task<void()>([&]() {
                        PutGetRemoveTestTask(client, mm, latch1).run();
                    })).detach();
                }

                ASSERT_OPEN_EVENTUALLY(latch1);
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace test {
            class ClientTxnQueueTest : public ClientTestSupport {
            public:
                ClientTxnQueueTest();

                ~ClientTxnQueueTest();

            protected:
                HazelcastServer instance;
                HazelcastClient client;
            };

            ClientTxnQueueTest::ClientTxnQueueTest() : instance(*g_srvFactory), client(getNewClient()) {
            }

            ClientTxnQueueTest::~ClientTxnQueueTest() {
            }

            TEST_F(ClientTxnQueueTest, testTransactionalOfferPoll1) {
                std::string name = "defQueue";

                TransactionContext context = client.newTransactionContext();
                context.beginTransaction();
                TransactionalQueue<std::string> q = context.getQueue<std::string>(name);
                ASSERT_TRUE(q.offer("ali"));
                ASSERT_EQ(1, q.size());
                ASSERT_EQ("ali", *(q.poll()));
                ASSERT_EQ(0, q.size());
                context.commitTransaction();
                ASSERT_EQ(0, client.getQueue<std::string>(name).size());
            }

            TEST_F(ClientTxnQueueTest, testTransactionalOfferPollByteVector) {
                std::string name = "defQueue";

                TransactionContext context = client.newTransactionContext();
                context.beginTransaction();
                TransactionalQueue<std::vector<byte> > q = context.getQueue<std::vector<byte> >(name);
                std::vector<byte> value(3);
                ASSERT_TRUE(q.offer(value));
                ASSERT_EQ(1, q.size());
                ASSERT_EQ(value, *(q.poll()));
                ASSERT_EQ(0, q.size());
                context.commitTransaction();
                ASSERT_EQ(0, client.getQueue<std::vector<byte> >(name).size());
            }

            void testTransactionalOfferPoll2Thread(hazelcast::util::ThreadArgs &args) {
                boost::latch *latch1 = (boost::latch *) args.arg0;
                HazelcastClient *client = (HazelcastClient *) args.arg1;
                latch1->wait();
                client->getQueue<std::string>("defQueue0").offer("item0");
            }

            TEST_F(ClientTxnQueueTest, testTransactionalOfferPoll2) {
                boost::latch latch1(1);
                hazelcast::util::StartedThread t(testTransactionalOfferPoll2Thread, &latch1, &client);
                TransactionContext context = client.newTransactionContext();
                context.beginTransaction();
                TransactionalQueue<std::string> q0 = context.getQueue<std::string>("defQueue0");
                TransactionalQueue<std::string> q1 = context.getQueue<std::string>("defQueue1");
                std::shared_ptr<std::string> s;
                latch1.count_down();
                s = q0.poll(10 * 1000);
                ASSERT_EQ("item0", *s);
                ASSERT_TRUE(q1.offer(*s));

                ASSERT_NO_THROW(context.commitTransaction());

                ASSERT_EQ(0, client.getQueue<std::string>("defQueue0").size());
                ASSERT_EQ("item0", *(client.getQueue<std::string>("defQueue1").poll()));
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace test {
            namespace util {
                class DataInputTest : public ::testing::Test
                {};

                TEST_F(DataInputTest, testReadByte) {
                    std::vector<byte> bytes;
                    bytes.push_back(0x01);
                    bytes.push_back(0x12);
                    serialization::pimpl::DataInput dataInput(bytes);
                    ASSERT_EQ(0x01, dataInput.readByte());
                    ASSERT_EQ(0x12, dataInput.readByte());
                }

                TEST_F(DataInputTest, testReadBoolean) {
                    std::vector<byte> bytes;
                    bytes.push_back(0x00);
                    bytes.push_back(0x10);
                    serialization::pimpl::DataInput dataInput(bytes);
                    ASSERT_FALSE(dataInput.readBoolean());
                    ASSERT_TRUE(dataInput.readBoolean());
                }

                TEST_F(DataInputTest, testReadChar) {
                    std::vector<byte> bytes;
                    bytes.push_back('a');
                    bytes.push_back('b');
                    serialization::pimpl::DataInput dataInput(bytes);
                    ASSERT_EQ('b', dataInput.readChar());
                }

                TEST_F(DataInputTest, testReadShort) {
                    std::vector<byte> bytes;
                    bytes.push_back(0x12);
                    bytes.push_back(0x34);
                    bytes.push_back(0x56);
                    serialization::pimpl::DataInput dataInput(bytes);
                    ASSERT_EQ(0x1234, dataInput.readShort());
                }

                TEST_F(DataInputTest, testReadInteger) {
                    std::vector<byte> bytes;
                    bytes.push_back(0x12);
                    bytes.push_back(0x34);
                    bytes.push_back(0x56);
                    bytes.push_back(0x78);
                    bytes.push_back(0x90);
                    serialization::pimpl::DataInput dataInput(bytes);
                    ASSERT_EQ(INT32_C(0x12345678), dataInput.readInt());
                }

                TEST_F(DataInputTest, testReadLong) {
                    std::vector<byte> bytes;
                    bytes.push_back(0x12);
                    bytes.push_back(0x34);
                    bytes.push_back(0x56);
                    bytes.push_back(0x78);
                    bytes.push_back(0x90);
                    bytes.push_back(0x9A);
                    bytes.push_back(0x9B);
                    bytes.push_back(0x9C);
                    serialization::pimpl::DataInput dataInput(bytes);
                    ASSERT_EQ(INT64_C(0x12345678909A9B9C), dataInput.readLong());
                }

                TEST_F(DataInputTest, testReadUTF) {
                    std::vector<byte> bytes;
                    bytes.push_back(0x00);
                    bytes.push_back(0x00);
                    bytes.push_back(0x00);
                    bytes.push_back(0x04);
                    bytes.push_back('b');
                    bytes.push_back('d');
                    bytes.push_back('f');
                    bytes.push_back('h');
                    serialization::pimpl::DataInput dataInput(bytes);
                    std::unique_ptr<std::string> utf = dataInput.readUTF();
                    ASSERT_NE((std::string *) NULL, utf.get());
                    ASSERT_EQ("bdfh", *utf);
                }

                TEST_F(DataInputTest, testReadByteArray) {
                    std::vector<byte> bytes;
                    bytes.push_back(0x00);
                    bytes.push_back(0x00);
                    bytes.push_back(0x00);
                    bytes.push_back(0x02);
                    std::vector<byte> actualDataBytes;
                    actualDataBytes.push_back(0x12);
                    actualDataBytes.push_back(0x34);
                    bytes.insert(bytes.end(), actualDataBytes.begin(), actualDataBytes.end());
                    serialization::pimpl::DataInput dataInput(bytes);
                    std::unique_ptr<std::vector<byte> > readBytes = dataInput.readByteArray();
                    ASSERT_NE((std::vector<byte> *) NULL, readBytes.get());
                    ASSERT_EQ(actualDataBytes, *readBytes);
                }

                TEST_F(DataInputTest, testReadBooleanArray) {
                    std::vector<byte> bytes;
                    bytes.push_back(0x00);
                    bytes.push_back(0x00);
                    bytes.push_back(0x00);
                    bytes.push_back(0x02);
                    bytes.push_back(0x00);
                    bytes.push_back(0x01);
                    serialization::pimpl::DataInput dataInput(bytes);
                    std::unique_ptr<std::vector<bool> > booleanArray = dataInput.readBooleanArray();
                    ASSERT_NE((std::vector<bool> *) NULL, booleanArray.get());
                    ASSERT_EQ(2U, booleanArray->size());
                    ASSERT_FALSE((*booleanArray)[0]);
                    ASSERT_TRUE((*booleanArray)[1]);
                }

                TEST_F(DataInputTest, testReadCharArray) {
                    std::vector<byte> bytes;
                    bytes.push_back(0x00);
                    bytes.push_back(0x00);
                    bytes.push_back(0x00);
                    bytes.push_back(0x02);
                    bytes.push_back(0);
                    bytes.push_back('f');
                    bytes.push_back(0);
                    bytes.push_back('h');
                    serialization::pimpl::DataInput dataInput(bytes);
                    std::unique_ptr<std::vector<char> > charArray = dataInput.readCharArray();
                    ASSERT_NE((std::vector<char> *) NULL, charArray.get());
                    ASSERT_EQ(2U, charArray->size());
                    ASSERT_EQ('f', (*charArray)[0]);
                    ASSERT_EQ('h', (*charArray)[1]);
                }

                TEST_F(DataInputTest, testReadShortArray) {
                    std::vector<byte> bytes;
                    bytes.push_back(0x00);
                    bytes.push_back(0x00);
                    bytes.push_back(0x00);
                    bytes.push_back(0x02);
                    bytes.push_back(0x12);
                    bytes.push_back(0x34);
                    bytes.push_back(0x56);
                    bytes.push_back(0x78);
                    serialization::pimpl::DataInput dataInput(bytes);
                    std::unique_ptr<std::vector<int16_t> > array = dataInput.readShortArray();
                    ASSERT_NE((std::vector<int16_t> *) NULL, array.get());
                    ASSERT_EQ(2U, array->size());
                    ASSERT_EQ(0x1234, (*array)[0]);
                    ASSERT_EQ(0x5678, (*array)[1]);
                }

                TEST_F(DataInputTest, testReadIntegerArray) {
                    std::vector<byte> bytes;
                    bytes.push_back(0x00);
                    bytes.push_back(0x00);
                    bytes.push_back(0x00);
                    bytes.push_back(0x02);
                    bytes.push_back(0x12);
                    bytes.push_back(0x34);
                    bytes.push_back(0x56);
                    bytes.push_back(0x78);
                    bytes.push_back(0x1A);
                    bytes.push_back(0xBC);
                    bytes.push_back(0xDE);
                    bytes.push_back(0xEF);
                    serialization::pimpl::DataInput dataInput(bytes);
                    std::unique_ptr<std::vector<int32_t> > array = dataInput.readIntArray();
                    ASSERT_NE((std::vector<int32_t> *) NULL, array.get());
                    ASSERT_EQ(2U, array->size());
                    ASSERT_EQ(INT32_C(0x12345678), (*array)[0]);
                    ASSERT_EQ(INT32_C(0x1ABCDEEF), (*array)[1]);
                }

                TEST_F(DataInputTest, testReadLongArray) {
                    std::vector<byte> bytes;
                    bytes.push_back(0x00);
                    bytes.push_back(0x00);
                    bytes.push_back(0x00);
                    bytes.push_back(0x02);
                    bytes.push_back(0x12);
                    bytes.push_back(0x34);
                    bytes.push_back(0x56);
                    bytes.push_back(0x78);
                    bytes.push_back(0x9A);
                    bytes.push_back(0xBC);
                    bytes.push_back(0xDE);
                    bytes.push_back(0xEF);
                    bytes.push_back(0x11);
                    bytes.push_back(0xA2);
                    bytes.push_back(0xA3);
                    bytes.push_back(0xA4);
                    bytes.push_back(0xA5);
                    bytes.push_back(0xA6);
                    bytes.push_back(0xA7);
                    bytes.push_back(0xA8);
                    serialization::pimpl::DataInput dataInput(bytes);
                    std::unique_ptr<std::vector<int64_t> > array = dataInput.readLongArray();
                    ASSERT_NE((std::vector<int64_t> *) NULL, array.get());
                    ASSERT_EQ(2U, array->size());
                    ASSERT_EQ(INT64_C(0x123456789ABCDEEF), (*array)[0]);
                    ASSERT_EQ(INT64_C(0x11A2A3A4A5A6A7A8), (*array)[1]);
                }
            }
        }
    }
}



namespace hazelcast {
    namespace client {
        namespace test {
            class AddressUtilTest : public ::testing::Test {
            };

            TEST_F(AddressUtilTest, testParsingHostAndPort) {
                hazelcast::util::AddressHolder addressHolder = hazelcast::util::AddressUtil::getAddressHolder(
                        "[fe80::62c5:*:fe05:480a%en0]:8080");
                ASSERT_EQ("fe80::62c5:*:fe05:480a", addressHolder.getAddress());
                ASSERT_EQ(8080, addressHolder.getPort());
                ASSERT_EQ("en0", addressHolder.getScopeId());

                addressHolder = hazelcast::util::AddressUtil::getAddressHolder("[::ffff:192.0.2.128]:5700");
                ASSERT_EQ("::ffff:192.0.2.128", addressHolder.getAddress());
                ASSERT_EQ(5700, addressHolder.getPort());

                addressHolder = hazelcast::util::AddressUtil::getAddressHolder("192.168.1.1:5700");
                ASSERT_EQ("192.168.1.1", addressHolder.getAddress());
                ASSERT_EQ(5700, addressHolder.getPort());

                addressHolder = hazelcast::util::AddressUtil::getAddressHolder("hazelcast.com:80");
                ASSERT_EQ("hazelcast.com", addressHolder.getAddress());
                ASSERT_EQ(80, addressHolder.getPort());
            }

            TEST_F(AddressUtilTest, testGetByNameIpV4) {
                std::string addrString("127.0.0.1");
                boost::asio::ip::address address = hazelcast::util::AddressUtil::getByName(addrString);
                ASSERT_TRUE(address.is_v4());
                ASSERT_FALSE(address.is_v6());
                ASSERT_EQ(addrString, address.to_string());
            }
        }
    }
}




using namespace hazelcast::util;

namespace hazelcast {
    namespace client {
        namespace test {
            class RuntimeAvailableProcessorsTest : public ::testing::Test {
            protected:
                virtual void TearDown() {
                    RuntimeAvailableProcessors::resetOverride();
                }
            };

            TEST_F(RuntimeAvailableProcessorsTest, getAvailableProcessors_withoutOverride) {
                int availableProcessors = RuntimeAvailableProcessors::getNumberOfProcessors();
                ASSERT_EQ(availableProcessors, RuntimeAvailableProcessors::get());
            }

            TEST_F(RuntimeAvailableProcessorsTest, getAvailableProcessors_withOverride) {
                int customAvailableProcessors = 1234;
                RuntimeAvailableProcessors::override(customAvailableProcessors);
                try {
                    ASSERT_EQ(customAvailableProcessors, RuntimeAvailableProcessors::get());
                } catch (...) {
                    RuntimeAvailableProcessors::resetOverride();
                }
            }
        }
    }
}

//
// Created by İhsan Demir on 18/05/15.
//


namespace hazelcast {
    namespace client {
        namespace test {
            namespace util {
                class BitsTest : public ::testing::Test
                {};

                TEST_F(BitsTest, testLittleEndian) {
                    uint64_t ONE = 1;
                    uint64_t oneByteFactor = ONE << 8;
                    uint64_t twoBytesFactor = ONE << 16;
                    uint64_t threeBytesFactor = ONE << 24;
                    uint64_t fourBytesFactor = ONE << 32;
                    uint64_t fiveBytesFactor = ONE << 40;
                    uint64_t sixBytesFactor = ONE << 48;
                    uint64_t sevenBytesFactor = ONE << 56;

                    {
                        int16_t expected = 0x7A * 256 + 0xBC;
                        int16_t actual;
                        byte *resBytes = (byte *) &actual;
                        hazelcast::util::Bits::nativeToLittleEndian2(&expected, &actual);
                        ASSERT_EQ(0xBC, resBytes[0]);
                        ASSERT_EQ(0x7A, resBytes[1]);
                    }

                    {
                        int32_t expected = 0x1A * (int32_t) threeBytesFactor +
                                           0x9A * (int32_t) twoBytesFactor + 0xAA * (int32_t) oneByteFactor + 0xBA;
                        int32_t actual;
                        byte *resBytes = (byte *) &actual;
                        hazelcast::util::Bits::nativeToLittleEndian4(&expected, &actual);
                        ASSERT_EQ(0xBA, resBytes[0]);
                        ASSERT_EQ(0xAA, resBytes[1]);
                        ASSERT_EQ(0x9A, resBytes[2]);
                        ASSERT_EQ(0x1A, resBytes[3]);
                    }

                    {
                        int64_t expected =
                                0x1A * sevenBytesFactor +
                                0x2A * sixBytesFactor +
                                0x3A * fiveBytesFactor +
                                0x4A * fourBytesFactor +
                                0x5A * threeBytesFactor +
                                0x6A * twoBytesFactor +
                                0x7A * oneByteFactor +
                                0x8A;

                        int64_t actual;
                        byte *resBytes = (byte *) &actual;
                        hazelcast::util::Bits::nativeToLittleEndian8(&expected, &actual);
                        ASSERT_EQ(0x8A, resBytes[0]);
                        ASSERT_EQ(0x7A, resBytes[1]);
                        ASSERT_EQ(0x6A, resBytes[2]);
                        ASSERT_EQ(0x5A, resBytes[3]);
                        ASSERT_EQ(0x4A, resBytes[4]);
                        ASSERT_EQ(0x3A, resBytes[5]);
                        ASSERT_EQ(0x2A, resBytes[6]);
                        ASSERT_EQ(0x1A, resBytes[7]);
                    }

                    // Little to Native tests
                    {
                        byte source[2] = {0xAB, 0xBC};
                        uint16_t actual;
                        hazelcast::util::Bits::littleEndianToNative2(&source, &actual);
                        ASSERT_EQ(0xBC * oneByteFactor + 0xAB, actual);
                    }

                    {
                        byte source[4] = {0xAB, 0xBC, 0xDE, 0xA1};
                        uint32_t actual;
                        hazelcast::util::Bits::littleEndianToNative4(&source, &actual);
                        ASSERT_EQ(0xA1 * threeBytesFactor +
                                  0xDE * twoBytesFactor +
                                  0xBC * oneByteFactor +
                                  0xAB, actual);
                    }

                    {
                        byte source[8] = {0xAB, 0x9B, 0x8B, 0x7B, 0x6B, 0x5B, 0x4B, 0xA1};
                        uint64_t actual;
                        hazelcast::util::Bits::littleEndianToNative8(&source, &actual);
                        ASSERT_EQ(0xA1 * sevenBytesFactor +
                                  0x4B * sixBytesFactor +
                                  0x5B * fiveBytesFactor +
                                  0x6B * fourBytesFactor +
                                  0x7B * threeBytesFactor +
                                  0x8B * twoBytesFactor +
                                  0x9B * oneByteFactor +
                                  0xAB, actual);
                    }
                }
            }
        }
    }
}



namespace hazelcast {
    namespace client {
        namespace test {
            namespace util {
                class DataOutputTest : public ::testing::Test
                {};

                TEST_F(DataOutputTest, testWriteByte) {
                    std::vector<byte> bytes;
                    bytes.push_back(0x01);
                    bytes.push_back(0x12);
                    serialization::pimpl::DataOutput dataOutput;
                    dataOutput.writeByte((byte) 0x01);
                    dataOutput.writeByte(0x12);
                    ASSERT_EQ(bytes, *dataOutput.toByteArray());
                }

                TEST_F(DataOutputTest, testWriteBoolean) {
                    std::vector<byte> bytes;
                    bytes.push_back(0x00);
                    bytes.push_back(0x01);
                    serialization::pimpl::DataOutput dataOutput;
                    dataOutput.writeBoolean(false);
                    dataOutput.writeBoolean(true);
                    ASSERT_EQ(bytes, *dataOutput.toByteArray());
                }

                TEST_F(DataOutputTest, testWriteChar) {
                    std::vector<byte> bytes;
                    bytes.push_back(0);
                    bytes.push_back('b');
                    serialization::pimpl::DataOutput dataOutput;
                    dataOutput.writeChar('b');
                    ASSERT_EQ(bytes, *dataOutput.toByteArray());
                }

                TEST_F(DataOutputTest, testWriteShort) {
                    std::vector<byte> bytes;
                    bytes.push_back(0x12);
                    bytes.push_back(0x34);
                    serialization::pimpl::DataOutput dataOutput;
                    dataOutput.writeShort(0x1234);
                    ASSERT_EQ(bytes, *dataOutput.toByteArray());
                }

                TEST_F(DataOutputTest, testWriteInteger) {
                    std::vector<byte> bytes;
                    bytes.push_back(0x12);
                    bytes.push_back(0x34);
                    bytes.push_back(0x56);
                    bytes.push_back(0x78);
                    serialization::pimpl::DataOutput dataOutput;
                    dataOutput.writeInt(INT32_C(0x12345678));
                    ASSERT_EQ(bytes, *dataOutput.toByteArray());
                }

                TEST_F(DataOutputTest, testWriteLong) {
                    std::vector<byte> bytes;
                    bytes.push_back(0x12);
                    bytes.push_back(0x34);
                    bytes.push_back(0x56);
                    bytes.push_back(0x78);
                    bytes.push_back(0x90);
                    bytes.push_back(0x9A);
                    bytes.push_back(0x9B);
                    bytes.push_back(0x9C);
                    serialization::pimpl::DataOutput dataOutput;
                    dataOutput.writeLong(INT64_C(0x12345678909A9B9C));
                    ASSERT_EQ(bytes, *dataOutput.toByteArray());
                }

                TEST_F(DataOutputTest, testWriteUTF) {
                    std::vector<byte> bytes;
                    bytes.push_back(0x00);
                    bytes.push_back(0x00);
                    bytes.push_back(0x00);
                    bytes.push_back(0x04);
                    bytes.push_back('b');
                    bytes.push_back('d');
                    bytes.push_back('f');
                    bytes.push_back('h');
                    serialization::pimpl::DataOutput dataOutput;
                    std::string value("bdfh");
                    dataOutput.writeUTF(&value);
                    ASSERT_EQ(bytes, *dataOutput.toByteArray());
                }

                TEST_F(DataOutputTest, testWriteByteArray) {
                    std::vector<byte> bytes;
                    bytes.push_back(0x00);
                    bytes.push_back(0x00);
                    bytes.push_back(0x00);
                    bytes.push_back(0x02);
                    std::vector<byte> actualDataBytes;
                    actualDataBytes.push_back(0x12);
                    actualDataBytes.push_back(0x34);
                    bytes.insert(bytes.end(), actualDataBytes.begin(), actualDataBytes.end());
                    serialization::pimpl::DataOutput dataOutput;
                    dataOutput.writeByteArray(&actualDataBytes);
                    ASSERT_EQ(bytes, *dataOutput.toByteArray());
                }

                TEST_F(DataOutputTest, testWriteBooleanArray) {
                    std::vector<byte> bytes;
                    bytes.push_back(0x00);
                    bytes.push_back(0x00);
                    bytes.push_back(0x00);
                    bytes.push_back(0x02);
                    bytes.push_back(0x00);
                    bytes.push_back(0x01);
                    std::vector<bool> actualValues;
                    actualValues.push_back(false);
                    actualValues.push_back(true);
                    serialization::pimpl::DataOutput dataOutput;
                    dataOutput.writeBooleanArray(&actualValues);
                    ASSERT_EQ(bytes, *dataOutput.toByteArray());
                }

                TEST_F(DataOutputTest, testWriteCharArray) {
                    std::vector<byte> bytes;
                    bytes.push_back(0x00);
                    bytes.push_back(0x00);
                    bytes.push_back(0x00);
                    bytes.push_back(0x02);
                    std::vector<char> actualChars;
                    bytes.push_back(0);
                    bytes.push_back('f');
                    actualChars.push_back('f');
                    bytes.push_back(0);
                    bytes.push_back('h');
                    actualChars.push_back('h');
                    serialization::pimpl::DataOutput dataOutput;
                    dataOutput.writeCharArray(&actualChars);
                    ASSERT_EQ(bytes, *dataOutput.toByteArray());
                }

                TEST_F(DataOutputTest, testWriteShortArray) {
                    std::vector<byte> bytes;
                    bytes.push_back(0x00);
                    bytes.push_back(0x00);
                    bytes.push_back(0x00);
                    bytes.push_back(0x02);
                    std::vector<int16_t> actualValues;
                    bytes.push_back(0x12);
                    bytes.push_back(0x34);
                    actualValues.push_back(0x1234);
                    bytes.push_back(0x56);
                    bytes.push_back(0x78);
                    actualValues.push_back(0x5678);
                    serialization::pimpl::DataOutput dataOutput;
                    dataOutput.writeShortArray(&actualValues);
                    ASSERT_EQ(bytes, *dataOutput.toByteArray());
                }

                TEST_F(DataOutputTest, testWriteIntegerArray) {
                    std::vector<byte> bytes;
                    bytes.push_back(0x00);
                    bytes.push_back(0x00);
                    bytes.push_back(0x00);
                    bytes.push_back(0x02);
                    std::vector<int32_t> actualValues;
                    bytes.push_back(0x12);
                    bytes.push_back(0x34);
                    bytes.push_back(0x56);
                    bytes.push_back(0x78);
                    actualValues.push_back(INT32_C(0x12345678));
                    bytes.push_back(0x9A);
                    bytes.push_back(0xBC);
                    bytes.push_back(0xDE);
                    bytes.push_back(0xEF);
                    actualValues.push_back(INT32_C(0x9ABCDEEF));
                    serialization::pimpl::DataOutput dataOutput;
                    dataOutput.writeIntArray(&actualValues);
                    ASSERT_EQ(bytes, *dataOutput.toByteArray());
                }

                TEST_F(DataOutputTest, testWriteLongArray) {
                    std::vector<byte> bytes;
                    bytes.push_back(0x00);
                    bytes.push_back(0x00);
                    bytes.push_back(0x00);
                    bytes.push_back(0x02);
                    bytes.push_back(0x12);
                    bytes.push_back(0x34);
                    bytes.push_back(0x56);
                    bytes.push_back(0x78);
                    bytes.push_back(0x9A);
                    bytes.push_back(0xBC);
                    bytes.push_back(0xDE);
                    bytes.push_back(0xEF);
                    bytes.push_back(0xA1);
                    bytes.push_back(0xA2);
                    bytes.push_back(0xA3);
                    bytes.push_back(0xA4);
                    bytes.push_back(0xA5);
                    bytes.push_back(0xA6);
                    bytes.push_back(0xA7);
                    bytes.push_back(0xA8);
                    std::vector<int64_t> actualValues;
                    actualValues.push_back(INT64_C(0x123456789ABCDEEF));
                    actualValues.push_back(INT64_C(0xA1A2A3A4A5A6A7A8));
                    serialization::pimpl::DataOutput dataOutput;
                    dataOutput.writeLongArray(&actualValues);
                    ASSERT_EQ(bytes, *dataOutput.toByteArray());
                }
            }
        }
    }
}

#ifdef HZ_BUILD_WITH_SSL



namespace hazelcast {
    namespace client {
        namespace test {
            class HttpsClientTest : public ::testing::Test {
            };

            TEST_F(HttpsClientTest, testConnect) {
                SyncHttpsClient httpsClient("localhost", "non_existentURL/no_page");
                ASSERT_THROW(httpsClient.openConnection(), client::exception::IOException);
            }

            TEST_F(HttpsClientTest, testConnectToGithub) {
                SyncHttpsClient httpsClient("ec2.us-east-1.amazonaws.com",
                                            "/?Action=DescribeInstances&Version=2014-06-15&X-Amz-Algorithm=AWS4-HMAC-SHA256&X-Amz-Credential=AKIAIU5IAVNR6X75ARYQ%2F20170413%2Fus-east-1%2Fec2%2Faws4_request&X-Amz-Date=20170413T083821Z&X-Amz-Expires=30&X-Amz-Signature=dff261333170c81ecb21f3a0d5820147233197a32c&X-Amz-SignedHeaders=host");
                try {
                    httpsClient.openConnection();
                } catch (exception::IException &e) {
                    const std::string &msg = e.getMessage();
                    ASSERT_NE(std::string::npos, msg.find("status: 401"));
                }
                ASSERT_THROW(httpsClient.openConnection(), exception::IOException);
            }
        }
    }
}

#endif // HZ_BUILD_WITH_SSL

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

