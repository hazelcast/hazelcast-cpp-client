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
#include <vector>
#include <chrono>
#include <functional>
#include "hazelcast/client/LifecycleEvent.h"
#include "ringbuffer/StartsWithStringFilter.h"
#include "ClientTestSupportBase.h"
#include <hazelcast/client/ClientConfig.h>
#include <hazelcast/client/HazelcastClient.h>
#include <hazelcast/client/serialization/serialization.h>
#include <hazelcast/client/impl/Partition.h>
#include <gtest/gtest.h>
#include <thread>
#include <hazelcast/client/spi/ClientContext.h>
#include <hazelcast/client/connection/ClientConnectionManagerImpl.h>
#include <hazelcast/client/connection/Connection.h>
#include <memory>
#include <hazelcast/client/proxy/PNCounterImpl.h>
#include <hazelcast/client/serialization/pimpl/DataInput.h>
#include <hazelcast/util/AddressUtil.h>
#include <hazelcast/client/serialization/pimpl/DataOutput.h>
#include <hazelcast/util/AddressHelper.h>
#include <hazelcast/util/Util.h>
#include <TestHelperFunctions.h>
#include <ostream>
#include <hazelcast/util/ILogger.h>
#include <hazelcast/client/LifecycleListener.h>
#include "serialization/Serializables.h"
#include <unordered_set>
#include <cmath>
#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/thread/future.hpp>
#include <cassert>

#ifdef HZ_BUILD_WITH_SSL
#include <openssl/crypto.h>
#endif

#include "hazelcast/client/exception/ProtocolExceptions.h"
#include "hazelcast/client/internal/socket/SSLSocket.h"
#include "hazelcast/client/MembershipListener.h"
#include "hazelcast/client/InitialMembershipEvent.h"
#include "hazelcast/client/InitialMembershipListener.h"
#include "hazelcast/client/SocketInterceptor.h"
#include "hazelcast/client/Socket.h"
#include "hazelcast/client/Cluster.h"
#include "hazelcast/client/IMap.h"
#include "hazelcast/util/Bits.h"
#include "hazelcast/util/SyncHttpsClient.h"
#include "hazelcast/util/MurmurHash3.h"
#include "hazelcast/client/ITopic.h"
#include "hazelcast/client/MultiMap.h"
#include "hazelcast/client/EntryEvent.h"
#include "hazelcast/client/ReliableTopic.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
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

                    instance = nullptr;
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

                std::unordered_map<std::string, std::string>
                getStatsFromResponse(const Response &statsResponse) {
                    std::unordered_map<std::string, std::string> statsMap;
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

                std::unordered_map<std::string, std::string> getStats() {
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
                                         std::to_string<int>(STATS_PERIOD_SECONDS))
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
                    std::unordered_map<string, string> stats = getStats();
                    if (stats["lastStatisticsCollectionTime"] != lastStatisticsCollectionTime) {
                        return true;
                    }
                    return false;
                }

                void produceSomeStats(HazelcastClient &client) {
                    auto map = client.getMap(getTestName());
                    produceSomeStats(map);
                }

                void produceSomeStats(IMap<int, int> &map) {
                    map->put<std::string, std::string>(5, 10);
                    ASSERT_EQ(10, *map->get<std::string, std::string>(5));
                    ASSERT_EQ(10, *map->get<std::string, std::string>(5));
                }

                std::string toString(const std::unordered_map<std::string, std::string> &map) {
                    std::ostringstream out;
                    typedef std::unordered_map<std::string, std::string> StringMap;
                    out << "Map {" << std::endl;
                    for (const StringMap::value_type &entry : map) {
                        out << "\t\t(" << entry.first << " , " << entry.second << ")" << std::endl;
                    }
                    out << "}" << std::endl;

                    return out.str();
                }

                static HazelcastServer *instance;
            };

            HazelcastServer *ClientStatisticsTest::instance = nullptr;

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
                client.getMap(mapName);

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

                std::unordered_map<std::string, std::string> statsMap = getStatsFromResponse(statsResponse);

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
                auto map = client->getMap(getTestName());

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

                std::unordered_map<std::string, std::string> initialStats = getStats();

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
            }
        }

        namespace serialization {
            int32_t hz_serializer<test::ringbuffer::StartsWithStringFilter>::getFactoryId() {
                return 666;
            }

            int32_t hz_serializer<test::ringbuffer::StartsWithStringFilter>::getClassId() {
                return 14;
            }

            void hz_serializer<test::ringbuffer::StartsWithStringFilter>::writeData(
                    const test::ringbuffer::StartsWithStringFilter &object, ObjectDataOutput &out) {
                out.write(object.startString);
            }

            test::ringbuffer::StartsWithStringFilter
            hz_serializer<test::ringbuffer::StartsWithStringFilter>::readData(ObjectDataInput &in) {
                return test::ringbuffer::StartsWithStringFilter(in.read<std::string>());
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
                            items.emplace_back(std::to_string(i));
                        }
                    }

                protected:
                    void SetUp() override {
                        std::string testName = getTestName();
                        clientRingbuffer = client->getRingbuffer(testName);
                        client2Ringbuffer = client2->getRingbuffer(testName);
                    }

                    void TearDown() override {
                        if (clientRingbuffer) {
                            clientRingbuffer->destroy().get();
                        }
                        if (client2Ringbuffer) {
                            client2Ringbuffer->destroy().get();
                        }
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

                        client = nullptr;
                        client2 = nullptr;
                        instance = nullptr;
                    }

                    static HazelcastServer *instance;
                    static HazelcastClient *client;
                    static HazelcastClient *client2;
                    std::shared_ptr<Ringbuffer> clientRingbuffer;
                    std::shared_ptr<Ringbuffer> client2Ringbuffer;
                    std::vector<std::string> items;

                    static constexpr int64_t CAPACITY = 10;
                };

                constexpr int64_t RingbufferTest::CAPACITY;
                
                HazelcastServer *RingbufferTest::instance = nullptr;
                HazelcastClient *RingbufferTest::client = nullptr;
                HazelcastClient *RingbufferTest::client2 = nullptr;

                TEST_F(RingbufferTest, testAPI) {
                    std::shared_ptr<Ringbuffer> rb = client->getRingbuffer(getTestName() + "2");
                    ASSERT_EQ(CAPACITY, rb->capacity().get());
                    ASSERT_EQ(0, rb->headSequence().get());
                    ASSERT_EQ(-1, rb->tailSequence().get());
                    ASSERT_EQ(0, rb->size().get());
                    ASSERT_EQ(CAPACITY, rb->remainingCapacity().get());
                    ASSERT_THROW(rb->readOne<Employee>(-1).get(), exception::IllegalArgumentException);
                    ASSERT_THROW(rb->readOne<Employee>(1).get(), exception::IllegalArgumentException);

                    Employee employee1("First", 10);
                    Employee employee2("Second", 20);

                    ASSERT_EQ(0, rb->add<Employee>(employee1).get());
                    ASSERT_EQ(CAPACITY, rb->capacity().get());
                    ASSERT_EQ(CAPACITY, rb->remainingCapacity().get());
                    ASSERT_EQ(0, rb->headSequence().get());
                    ASSERT_EQ(0, rb->tailSequence().get());
                    ASSERT_EQ(1, rb->size().get());
                    ASSERT_EQ(employee1, rb->readOne<Employee>(0).get().value());
                    ASSERT_THROW(rb->readOne<Employee>(2).get(), exception::IllegalArgumentException);

                    ASSERT_EQ(1, rb->add<Employee>(employee2).get());
                    ASSERT_EQ(CAPACITY, rb->capacity().get());
                    ASSERT_EQ(CAPACITY, rb->remainingCapacity().get());
                    ASSERT_EQ(0, rb->headSequence().get());
                    ASSERT_EQ(1, rb->tailSequence().get());
                    ASSERT_EQ(2, rb->size().get());
                    ASSERT_EQ(employee1, rb->readOne<Employee>(0).get().value());
                    ASSERT_EQ(employee2, rb->readOne<Employee>(1).get().value());
                    ASSERT_THROW(rb->readOne<Employee>(3).get(), exception::IllegalArgumentException);

                    // insert many employees to fill the ringbuffer capacity
                    for (int i = 0; i < CAPACITY - 2; ++i) {
                        Employee eleman("name", 10 * (i + 2));
                        ASSERT_EQ(i + 2, rb->add<Employee>(eleman).get());
                        ASSERT_EQ(CAPACITY, rb->capacity().get());
                        ASSERT_EQ(CAPACITY, rb->remainingCapacity().get());
                        ASSERT_EQ(0, rb->headSequence().get());
                        ASSERT_EQ(i + 2, rb->tailSequence().get());
                        ASSERT_EQ(i + 3, rb->size().get());
                        ASSERT_EQ(eleman, rb->readOne<Employee>(i+2).get().value());
                    }

                    // verify that the head element is overriden on the first add
                    Employee latestEmployee("latest employee", 100);
                    ASSERT_EQ(CAPACITY, rb->add<Employee>(latestEmployee).get());
                    ASSERT_EQ(CAPACITY, rb->capacity().get());
                    ASSERT_EQ(CAPACITY, rb->remainingCapacity().get());
                    ASSERT_EQ(1, rb->headSequence().get());
                    ASSERT_EQ(CAPACITY, rb->tailSequence().get());
                    ASSERT_EQ(CAPACITY, rb->size().get());
                    ASSERT_EQ(latestEmployee, rb->readOne<Employee>(CAPACITY).get().value());
                }

                TEST_F(RingbufferTest, readManyAsync_whenHitsStale_useHeadAsStartSequence) {
                    client2Ringbuffer->addAll(items, client::ringbuffer::OverflowPolicy::OVERWRITE);
                    auto f = clientRingbuffer->readMany<std::string>(1, 1, 10);
                    auto rs = f.get();
                    ASSERT_EQ(10, rs.readCount());
                    ASSERT_EQ(std::string("1"), *rs.getItems()[0].get<std::string>());
                    ASSERT_EQ(std::string("10"), *rs.getItems()[9].get<std::string>());
                }

                TEST_F(RingbufferTest, readOne_whenHitsStale_shouldNotBeBlocked) {
                    std::shared_ptr<boost::latch> latch1 = std::make_shared<boost::latch>(1);
                    std::thread([=] () {
                        try {
                            clientRingbuffer->readOne<std::string>(0).get();
                            latch1->count_down();
                        } catch (exception::StaleSequenceException &) {
                            latch1->count_down();
                        }
                    }).detach();
                    client2Ringbuffer->addAll(items, client::ringbuffer::OverflowPolicy::OVERWRITE);
                    ASSERT_OPEN_EVENTUALLY(*latch1);
                }

                TEST_F(RingbufferTest, headSequence) {
                    for (int k = 0; k < 2 * CAPACITY; k++) {
                        client2Ringbuffer->add<std::string>("foo").get();
                    }

                    ASSERT_EQ(client2Ringbuffer->headSequence().get(), clientRingbuffer->headSequence().get());
                }

                TEST_F(RingbufferTest, tailSequence) {
                    for (int k = 0; k < 2 * CAPACITY; k++) {
                        client2Ringbuffer->add<std::string>("foo").get();
                    }

                    ASSERT_EQ(client2Ringbuffer->tailSequence().get(), clientRingbuffer->tailSequence().get());
                }

                TEST_F(RingbufferTest, size) {
                    client2Ringbuffer->add<std::string>("foo").get();

                    ASSERT_EQ(client2Ringbuffer->tailSequence().get(), clientRingbuffer->tailSequence().get());
                }

                TEST_F(RingbufferTest, capacity) {
                    ASSERT_EQ(client2Ringbuffer->capacity().get(), clientRingbuffer->capacity().get());
                }

                TEST_F(RingbufferTest, remainingCapacity) {
                    client2Ringbuffer->add<std::string>("foo").get();

                    ASSERT_EQ(client2Ringbuffer->remainingCapacity().get(), clientRingbuffer->remainingCapacity().get());
                }

                TEST_F(RingbufferTest, add) {
                    clientRingbuffer->add<std::string>("foo").get();
                    auto value = client2Ringbuffer->readOne<std::string>(0).get();
                    ASSERT_TRUE(value.has_value());
                    ASSERT_EQ("foo", value.value());
                }

                TEST_F(RingbufferTest, addAll) {
                    std::vector<std::string> items;
                    items.push_back("foo");
                    items.push_back("bar");
                    auto result = clientRingbuffer->addAll(items, client::ringbuffer::OverflowPolicy::OVERWRITE).get();

                    ASSERT_EQ(client2Ringbuffer->tailSequence().get(), result);
                    auto val0 = client2Ringbuffer->readOne<std::string>(0).get();
                    auto val1 = client2Ringbuffer->readOne<std::string>(1).get();
                    ASSERT_TRUE(val0);
                    ASSERT_TRUE(val1);
                    ASSERT_EQ(val0.value(), "foo");
                    ASSERT_EQ(val1.value(), "bar");
                    ASSERT_EQ(0, client2Ringbuffer->headSequence().get());
                    ASSERT_EQ(1, client2Ringbuffer->tailSequence().get());
                }

                TEST_F(RingbufferTest, readOne) {
                    client2Ringbuffer->add<std::string>("foo").get();
                    auto value = clientRingbuffer->readOne<std::string>(0).get();
                    ASSERT_TRUE(value.has_value());
                    ASSERT_EQ("foo", value.value());
                }

                TEST_F(RingbufferTest, readMany_noFilter) {
                    client2Ringbuffer->add<std::string>("1");
                    client2Ringbuffer->add<std::string>("2");
                    client2Ringbuffer->add<std::string>("3");

                    auto rs = clientRingbuffer->readMany(0, 3, 3).get();

                    ASSERT_EQ(3, rs.readCount());
                    auto &items = rs.getItems();
                    ASSERT_EQ("1", items[0].get<std::string>().value());
                    ASSERT_EQ("2", items[1].get<std::string>().value());
                    ASSERT_EQ("3", items[2].get<std::string>().value());
                }

                // checks if the max count works. So if more results are available than needed, the surplus results should not be read.
                TEST_F(RingbufferTest, readMany_maxCount) {
                    client2Ringbuffer->add<std::string>("1").get();
                    client2Ringbuffer->add<std::string>("2").get();
                    client2Ringbuffer->add<std::string>("3").get();
                    client2Ringbuffer->add<std::string>("4").get();
                    client2Ringbuffer->add<std::string>("5").get();
                    client2Ringbuffer->add<std::string>("6").get();

                    client::ringbuffer::ReadResultSet rs = clientRingbuffer->readMany<std::string>(0, 3, 3).get();

                    ASSERT_EQ(3, rs.readCount());
                    auto &items1 = rs.getItems();
                    ASSERT_EQ("1", items1[0].get<std::string>().value());
                    ASSERT_EQ("2", items1[1].get<std::string>().value());
                    ASSERT_EQ("3", items1[2].get<std::string>().value());
                }

                TEST_F(RingbufferTest, readManyAsync_withFilter) {
                    client2Ringbuffer->add<std::string>("good1").get();
                    client2Ringbuffer->add<std::string>("bad1").get();
                    client2Ringbuffer->add<std::string>("good2").get();
                    client2Ringbuffer->add<std::string>("bad2").get();
                    client2Ringbuffer->add<std::string>("good3").get();
                    client2Ringbuffer->add<std::string>("bad3").get();

                    StartsWithStringFilter filter("good");
                    auto rs = clientRingbuffer->readMany<StartsWithStringFilter>(0, 3, 3, &filter).get();

                    ASSERT_EQ(5, rs.readCount());
                    auto const &items = rs.getItems();
                    ASSERT_EQ("good1", items[0].get<std::string>().value());
                    ASSERT_EQ("good2", items[1].get<std::string>().value());
                    ASSERT_EQ("good3", items[2].get<std::string>().value());
                }
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

            hazelcast::client::ClientConfig ClientTestSupportBase::getConfig(bool ssl_enabled, bool smart) {
                ClientConfig clientConfig;
                clientConfig.getNetworkConfig().addAddress(Address(g_srvFactory->getServerAddress(), 5701));
                if (ssl_enabled) {
                    clientConfig.setClusterName(get_ssl_cluster_name());
                    clientConfig.getNetworkConfig().getSSLConfig().setEnabled(true).addVerifyFile(getCAFilePath());
                }
                clientConfig.getNetworkConfig().setSmartRouting(smart);
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
                // performance is not important, hence we can use random_device for the tests
                std::random_device rand{};
                return boost::uuids::to_string(boost::uuids::basic_random_generator<std::random_device>{rand}());
            }

            void ClientTestSupportBase::sleepSeconds(int32_t seconds) {
                hazelcast::util::sleep(seconds);
            }

            ClientTestSupportBase::ClientTestSupportBase() = default;

            boost::uuids::uuid ClientTestSupportBase::generateKeyOwnedBy(spi::ClientContext &context, const Member &member) {
                spi::impl::ClientPartitionServiceImpl &partitionService = context.getPartitionService();
                serialization::pimpl::SerializationService &serializationService = context.getSerializationService();
                while (true) {
                    auto id = context.random_uuid();
                    int partitionId = partitionService.getPartitionId(serializationService.toData(id));
                    std::shared_ptr<impl::Partition> partition = partitionService.getPartition(partitionId);
                    auto owner = partition->getOwner();
                    if (owner && *owner == member) {
                        return id;
                    }
                }
            }

            const std::string ClientTestSupportBase::get_ssl_cluster_name() {
                return "ssl-dev";
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
                config.setClusterName(get_ssl_cluster_name()).getNetworkConfig().
                        setConnectionAttemptPeriod(1000).setConnectionTimeout(2000).addAddress(
                        Address("8.8.8.8", 5701)).getSSLConfig().setEnabled(true).addVerifyFile(getCAFilePath());
                ASSERT_THROW(HazelcastClient client(config), exception::IllegalStateException);
            }

            TEST_F(ClientConnectionTest, testSSLWrongCAFilePath) {
                HazelcastServerFactory sslFactory(g_srvFactory->getServerAddress(), getSslFilePath());
                HazelcastServer instance(sslFactory);
                ClientConfig config = getConfig();
                config.setClusterName(get_ssl_cluster_name());
                config.getNetworkConfig().getSSLConfig().setEnabled(true).addVerifyFile("abc");
                ASSERT_THROW(HazelcastClient client(config), exception::IllegalStateException);
            }

            TEST_F(ClientConnectionTest, testExcludedCipher) {
                HazelcastServerFactory sslFactory(g_srvFactory->getServerAddress(), getSslFilePath());
                HazelcastServer instance(sslFactory);

                ClientConfig config = getConfig(true);
                config.getNetworkConfig().getSSLConfig().setCipherList("HIGH");
                std::vector<hazelcast::client::internal::socket::SSLSocket::CipherInfo> supportedCiphers = getCiphers(
                        config);

                std::string unsupportedCipher = supportedCiphers[supportedCiphers.size() - 1].name;
                config = getConfig(true);
                config.getNetworkConfig().getSSLConfig().setCipherList(std::string("HIGH:!") + unsupportedCipher);

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
            class ClusterTest : public ClientTestSupportBase, public ::testing::TestWithParam<ClientConfig> {
            public:
                ClusterTest() : sslFactory(g_srvFactory->getServerAddress(), getSslFilePath()) {}

            protected:
                LifecycleListener makeAllStatesListener(boost::latch &starting,
                                                        boost::latch &started,
                                                        boost::latch &connected,
                                                        boost::latch &disconnected,
                                                        boost::latch &shuttingDown,
                                                        boost::latch &shutdown) {
                    return LifecycleListener()
                        .on_starting([&starting](){
                            starting.count_down();
                        })
                        .on_started([&started](){
                            started.count_down();
                        })
                        .on_connected([&connected](){
                            connected.count_down();
                        })
                        .on_disconnected([&disconnected](){
                            disconnected.count_down();
                        })
                        .on_shutting_down([&shuttingDown](){
                            shuttingDown.count_down();
                        })
                        .on_shutdown([&shutdown](){
                            shutdown.count_down();
                        });
                }
                
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

            TEST_P(ClusterTest, testBehaviourWhenClusterNotFound) {
                ASSERT_THROW(HazelcastClient client(GetParam()), exception::IllegalStateException);
            }

            TEST_P(ClusterTest, testDummyClientBehaviourWhenClusterNotFound) {
                auto clientConfig = GetParam();
                clientConfig.getNetworkConfig().setSmartRouting(false);
                ASSERT_THROW(HazelcastClient client(clientConfig), exception::IllegalStateException);
            }

            TEST_P(ClusterTest, testAllClientStates) {
                auto clientConfig = GetParam();
                std::unique_ptr<HazelcastServer> instance = startServer(clientConfig);

                auto networkConfig = clientConfig.getNetworkConfig();
                networkConfig.setConnectionAttemptPeriod(1000);
                networkConfig.setConnectionAttemptLimit(1);
                boost::latch startingLatch(1);
                boost::latch startedLatch(1);
                boost::latch connectedLatch(1);
                boost::latch disconnectedLatch(1);
                boost::latch shuttingDownLatch(1);
                boost::latch shutdownLatch(1);
                auto listener = makeAllStatesListener(startingLatch, startedLatch, connectedLatch, disconnectedLatch,
                                                      shuttingDownLatch, shutdownLatch);
                clientConfig.addListener(std::move(listener));

                HazelcastClient client(clientConfig);

                ASSERT_OPEN_EVENTUALLY(startingLatch);
                ASSERT_OPEN_EVENTUALLY(startedLatch);
                ASSERT_OPEN_EVENTUALLY(connectedLatch);

                instance->shutdown();

                ASSERT_OPEN_EVENTUALLY(disconnectedLatch);
                ASSERT_OPEN_EVENTUALLY(shuttingDownLatch);
                ASSERT_OPEN_EVENTUALLY(shutdownLatch);
            }

            TEST_P(ClusterTest, testConnectionAttemptPeriod) {
                ClientConfig clientConfig = GetParam();
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
                auto clientConfig = GetParam();
                std::unique_ptr<HazelcastServer> instance = startServer(clientConfig);

                boost::latch startingLatch(1);
                boost::latch startedLatch(1);
                boost::latch connectedLatch(1);
                boost::latch disconnectedLatch(1);
                boost::latch shuttingDownLatch(1);
                boost::latch shutdownLatch(1);
                auto listener = makeAllStatesListener(startingLatch, startedLatch, connectedLatch, disconnectedLatch,
                                                      shuttingDownLatch, shutdownLatch);
                clientConfig.addListener(std::move(listener));

                HazelcastClient client(clientConfig);

                ASSERT_OPEN_EVENTUALLY(startingLatch);
                ASSERT_OPEN_EVENTUALLY(startedLatch);
                ASSERT_OPEN_EVENTUALLY(connectedLatch);

                client.shutdown();

                ASSERT_OPEN_EVENTUALLY(shuttingDownLatch);
                ASSERT_OPEN_EVENTUALLY(shutdownLatch);
            }

#ifdef HZ_BUILD_WITH_SSL
            INSTANTIATE_TEST_SUITE_P(All,
                                     ClusterTest,
                                     ::testing::Values(ClientTestSupportBase::getConfig(),
                                                       ClientTestSupportBase::getConfig(true)));
#else
            INSTANTIATE_TEST_SUITE_P(All, ClusterTest,ClientTestSupportBase::getConfig());
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
                std::this_thread::sleep_for(std::chrono::seconds(3));
            }
        }
    }
}



namespace hazelcast {
    namespace client {
        namespace test {
            class SocketInterceptorTest : public ClientTestSupport {
            };

            SocketInterceptor makeSocketInterceptor(boost::latch &l) {
                return SocketInterceptor()
                    .on_connect([&l](const hazelcast::client::Socket &connected_sock) {
                        ASSERT_EQ("127.0.0.1", connected_sock.getAddress().getHost());
                        ASSERT_NE(0, connected_sock.getAddress().getPort());
                        l.count_down();
                    });
            }

#ifdef HZ_BUILD_WITH_SSL
            TEST_F(SocketInterceptorTest, interceptSSLBasic) {
                HazelcastServerFactory sslFactory(g_srvFactory->getServerAddress(), getSslFilePath());
                HazelcastServer instance(sslFactory);
                ClientConfig config = getConfig(true);
                boost::latch interceptorLatch(1);
                auto interceptor = makeSocketInterceptor(interceptorLatch);
                config.setSocketInterceptor(std::move(interceptor));
                HazelcastClient client(config);
                interceptorLatch.wait_for(boost::chrono::seconds(2));
            }

#endif

            TEST_F(SocketInterceptorTest, interceptBasic) {
                HazelcastServer instance(*g_srvFactory);
                ClientConfig config = getConfig();
                boost::latch interceptorLatch(1);
                auto interceptor = makeSocketInterceptor(interceptorLatch);
                config.setSocketInterceptor(std::move(interceptor));
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
                HazelcastServerFactory factory("hazelcast/test/resources/hazelcast-username-password.xml");
                HazelcastServer instance(factory);
                HazelcastClient client(ClientConfig().setClusterName("username-pass-dev").setGroupConfig(
                        GroupConfig("dev", "dev-pass")));
            }

            TEST_F(ClientAuthenticationTest, testIncorrectGroupName) {
                HazelcastServer instance(*g_srvFactory);
                ClientConfig config;
                config.setClusterName("invalid cluster");

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
                ASSERT_EQ_EVENTUALLY(1, client.getCluster().getMembers().size());
                const Client endpoint = client.getLocalEndpoint();
                spi::ClientContext context(client);
                ASSERT_EQ(context.getName(), endpoint.getName());

                auto endpointAddress = endpoint.getSocketAddress();
                ASSERT_TRUE(endpointAddress);
                connection::ClientConnectionManagerImpl &connectionManager = context.getConnectionManager();
                std::shared_ptr<connection::Connection> connection = connectionManager.get_random_connection();
                ASSERT_NOTNULL(connection.get(), connection::Connection);
                auto localAddress = connection->getLocalSocketAddress();
                ASSERT_TRUE(localAddress);
                ASSERT_EQ(*localAddress, *endpointAddress);
                ASSERT_EQ(connectionManager.getClientUuid(), endpoint.getUuid());
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace test {
            class MemberAttributeTest : public ClientTestSupport
            {};

            TEST_F(MemberAttributeTest, testInitialValues) {
                HazelcastServer instance(*g_srvFactory);
                HazelcastClient hazelcastClient(getNewClient());
                Cluster cluster = hazelcastClient.getCluster();
                std::vector<Member> members = cluster.getMembers();
                ASSERT_EQ(1U, members.size());
                Member &member = members[0];
                std::string attribute_name = "test-member-attribute-name";
                ASSERT_TRUE(member.lookupAttribute(attribute_name));
                ASSERT_EQ("test-member-attribute-value", *member.getAttribute(attribute_name));
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

                            client = nullptr;
                            instance = nullptr;
                        }

                        static HazelcastServer *instance;
                        static HazelcastClient *client;
                    };

                    HazelcastServer *BasicPnCounterAPITest::instance = nullptr;
                    HazelcastClient *BasicPnCounterAPITest::client = nullptr;

                    TEST_F(BasicPnCounterAPITest, testGetStart) {
                        std::shared_ptr<PNCounter> pnCounter = client->getPNCounter(
                                testing::UnitTest::GetInstance()->current_test_info()->name());
                        ASSERT_EQ(0, pnCounter->get().get());
                    }

                    TEST_F(BasicPnCounterAPITest, testGetAndAdd) {
                        auto pnCounter = client->getPNCounter(
                                testing::UnitTest::GetInstance()->current_test_info()->name());
                        ASSERT_EQ(0, pnCounter->getAndAdd(5).get());
                    }

                    TEST_F(BasicPnCounterAPITest, testAddAndGet) {
                        auto pnCounter = client->getPNCounter(
                                testing::UnitTest::GetInstance()->current_test_info()->name());
                        ASSERT_EQ(5, pnCounter->addAndGet(5).get());
                    }

                    TEST_F(BasicPnCounterAPITest, testGetAndAddExisting) {
                        auto pnCounter = client->getPNCounter(
                                testing::UnitTest::GetInstance()->current_test_info()->name());

                        ASSERT_EQ(0, pnCounter->getAndAdd(2).get());
                        ASSERT_EQ(2, pnCounter->getAndAdd(3).get());
                        ASSERT_EQ(5, pnCounter->get().get());
                    }

                    TEST_F(BasicPnCounterAPITest, testGetAndIncrement) {
                        auto pnCounter = client->getPNCounter(
                                testing::UnitTest::GetInstance()->current_test_info()->name());
                        ASSERT_EQ(0, pnCounter->getAndIncrement().get());
                        ASSERT_EQ(1, pnCounter->getAndIncrement().get());
                        ASSERT_EQ(2, pnCounter->get().get());
                    }

                    TEST_F(BasicPnCounterAPITest, testIncrementAndGet) {
                        auto pnCounter = client->getPNCounter(
                                testing::UnitTest::GetInstance()->current_test_info()->name());
                        ASSERT_EQ(1, pnCounter->incrementAndGet().get());
                        ASSERT_EQ(1, pnCounter->get().get());
                    }

                    TEST_F(BasicPnCounterAPITest, testGetAndDecrementFromDefault) {
                        auto pnCounter = client->getPNCounter(
                                testing::UnitTest::GetInstance()->current_test_info()->name());
                        ASSERT_EQ(0, pnCounter->getAndDecrement().get());
                        ASSERT_EQ(-1, pnCounter->get().get());
                    }

                    TEST_F(BasicPnCounterAPITest, testGetAndDecrement) {
                        auto pnCounter = client->getPNCounter(
                                testing::UnitTest::GetInstance()->current_test_info()->name());
                        ASSERT_EQ(1, pnCounter->incrementAndGet().get());
                        ASSERT_EQ(1, pnCounter->getAndDecrement().get());
                        ASSERT_EQ(0, pnCounter->get().get());
                    }

                    TEST_F(BasicPnCounterAPITest, testGetAndSubtract) {
                        auto pnCounter = client->getPNCounter(
                                testing::UnitTest::GetInstance()->current_test_info()->name());
                        ASSERT_EQ(0, pnCounter->getAndSubtract(2).get());
                        ASSERT_EQ(-2, pnCounter->get().get());
                    }

                    TEST_F(BasicPnCounterAPITest, testSubtractAndGet) {
                        auto pnCounter = client->getPNCounter(
                                testing::UnitTest::GetInstance()->current_test_info()->name());
                        ASSERT_EQ(-3, pnCounter->subtractAndGet(3).get());
                    }

                    TEST_F(BasicPnCounterAPITest, testReset) {
                        auto pnCounter = client->getPNCounter(
                                testing::UnitTest::GetInstance()->current_test_info()->name());
                        pnCounter->reset().get();
                    }

                    class PnCounterFunctionalityTest : public ClientTestSupport {
                    public:
                        static void SetUpTestCase() {
                            instance = new HazelcastServer(*g_srvFactory);
                            client = new HazelcastClient;
                        }

                        static void TearDownTestCase() {
                            delete client;
                            delete instance;

                            client = nullptr;
                            instance = nullptr;
                        }

                    protected:
                        static HazelcastServer *instance;
                        static HazelcastClient *client;
                    };

                    HazelcastServer *PnCounterFunctionalityTest::instance = nullptr;
                    HazelcastClient *PnCounterFunctionalityTest::client = nullptr;

                    TEST_F(PnCounterFunctionalityTest, testSimpleReplication) {
                        const char *name = testing::UnitTest::GetInstance()->current_test_info()->name();
                        std::shared_ptr<PNCounter> counter1 = client->getPNCounter(name);
                        std::shared_ptr<PNCounter> counter2 = client->getPNCounter(name);

                        ASSERT_EQ(5, counter1->addAndGet(5).get());

                        ASSERT_EQ_EVENTUALLY(5, counter1->get().get());
                        ASSERT_EQ_EVENTUALLY(5, counter2->get().get());
                    }

                    TEST_F(PnCounterFunctionalityTest, testParallelism) {
                        const char *name = testing::UnitTest::GetInstance()->current_test_info()->name();
                        std::shared_ptr<PNCounter> counter1 = client->getPNCounter(name);
                        std::shared_ptr<PNCounter> counter2 = client->getPNCounter(name);

                        int parallelism = 5;
                        int loopsPerThread = 100;
                        std::atomic<int64_t> finalValue(0);

                        std::vector<std::future<void>> futures;
                        for (int i = 0; i < parallelism; i++) {
                            futures.push_back(std::async([&]() {
                                for (int j = 0; j < loopsPerThread; j++) {
                                    counter1->addAndGet(5).get();
                                    finalValue += 5;
                                    counter2->addAndGet(-2).get();
                                    finalValue += -2;
                                }
                            }));
                        }

                        boost::wait_for_all(futures.begin(), futures.end());

                        int64_t finalExpectedValue = 3 * (int64_t) loopsPerThread * parallelism;
                        ASSERT_EQ_EVENTUALLY(finalExpectedValue, counter1->get().get());
                        ASSERT_EQ_EVENTUALLY(finalExpectedValue, counter2->get().get());
                    }

                    class ClientPNCounterNoDataMemberTest : public ClientTestSupport {
                    };

                    TEST_F(ClientPNCounterNoDataMemberTest, noDataMemberExceptionIsThrown) {
                        HazelcastServerFactory factory("hazelcast/test/resources/hazelcast-lite-member.xml");
                        HazelcastServer instance(factory);

                        HazelcastClient client(ClientConfig().setClusterName("lite-dev"));

                        auto pnCounter = client.getPNCounter(
                                testing::UnitTest::GetInstance()->current_test_info()->name());

                        ASSERT_THROW(pnCounter->addAndGet(5).get(), exception::NoDataMemberInClusterException);
                    }

                    /**
                     * Client implementation for testing behaviour of {@link ConsistencyLostException}
                     */
                    class ClientPNCounterConsistencyLostTest : public ClientTestSupport {
                    protected:
                        boost::shared_ptr<Member> getCurrentTargetReplicaAddress(
                                const std::shared_ptr<PNCounter> &pnCounter) {
                            return pnCounter->getCurrentTargetReplicaAddress();
                        }

                        void
                        terminateMember(const Member &address, HazelcastServer &server1, HazelcastServer &server2) {
                            auto member1 = server1.getMember();
                            if (boost::to_string(address.getUuid()) == member1.uuid) {
                                server1.terminate();
                                return;
                            }

                            server2.terminate();
                        }
                    };

                    TEST_F(ClientPNCounterConsistencyLostTest,
                           consistencyLostExceptionIsThrownWhenTargetReplicaDisappears) {
                        HazelcastServerFactory factory(
                                "hazelcast/test/resources/hazelcast-pncounter-consistency-lost-test.xml");
                        HazelcastServer instance(factory);
                        HazelcastServer instance2(factory);

                        HazelcastClient client(ClientConfig().setClusterName("consistency-lost-dev"));

                        auto pnCounter = client.getPNCounter(
                                testing::UnitTest::GetInstance()->current_test_info()->name());

                        pnCounter->addAndGet(5).get();

                        ASSERT_EQ(5, pnCounter->get().get());

                        auto currentTarget = getCurrentTargetReplicaAddress(pnCounter);

                        terminateMember(*currentTarget, instance, instance2);

                        ASSERT_THROW(pnCounter->addAndGet(5).get(), exception::ConsistencyLostException);
                    }

                    TEST_F(ClientPNCounterConsistencyLostTest, driverCanContinueSessionByCallingReset) {
                        HazelcastServerFactory factory(
                                "hazelcast/test/resources/hazelcast-pncounter-consistency-lost-test.xml");
                        HazelcastServer instance(factory);
                        HazelcastServer instance2(factory);

                        HazelcastClient client(ClientConfig().setClusterName("consistency-lost-dev"));

                        auto pnCounter = client.getPNCounter(
                                testing::UnitTest::GetInstance()->current_test_info()->name());

                        pnCounter->addAndGet(5).get();

                        ASSERT_EQ(5, pnCounter->get().get());

                        auto currentTarget = getCurrentTargetReplicaAddress(pnCounter);

                        terminateMember(*currentTarget, instance, instance2);

                        pnCounter->reset().get();

                        pnCounter->addAndGet(5).get();
                    }
                }
            }

        }
    }
}

namespace hazelcast {
    namespace client {
        namespace test {
            class SimpleListenerTest : public ClientTestSupportBase, public ::testing::TestWithParam<ClientConfig> {
            public:
                SimpleListenerTest() = default;

            protected:
                class SampleInitialListener : public InitialMembershipListener {
                public:
                    SampleInitialListener(boost::latch &_memberAdded, boost::latch &_memberRemoved)
                            : _memberAdded(_memberAdded), _memberRemoved(_memberRemoved) {
                    }

                    void init(InitialMembershipEvent event) override {
                        auto &members = event.getMembers();
                        if (members.size() == 1) {
                            _memberAdded.count_down();
                        }
                    }

                    void memberAdded(const MembershipEvent &event) override {
                        _memberAdded.count_down();
                    }

                    void memberRemoved(const MembershipEvent &event) override {
                        _memberRemoved.count_down();
                    }

                private:
                    boost::latch &_memberAdded;
                    boost::latch &_memberRemoved;
                };

                class SampleListenerInSimpleListenerTest : public MembershipListener {
                public:
                    SampleListenerInSimpleListenerTest(boost::latch &_memberAdded,
                                                       boost::latch &_memberRemoved)
                            : _memberAdded(_memberAdded), _memberRemoved(_memberRemoved) {
                    }

                    void memberAdded(const MembershipEvent &event) override {
                        _memberAdded.count_down();
                    }

                    void memberRemoved(const MembershipEvent &event) override {
                        _memberRemoved.count_down();
                    }

                private:
                    boost::latch &_memberAdded;
                    boost::latch &_memberRemoved;
                };
            };

            TEST_P(SimpleListenerTest, testSharedClusterListeners) {
                HazelcastServer instance(*g_srvFactory);
                HazelcastClient hazelcastClient(GetParam());
                Cluster cluster = hazelcastClient.getCluster();
                boost::latch memberAdded(1);
                boost::latch memberAddedInit(2);
                boost::latch memberRemoved(1);
                boost::latch memberRemovedInit(1);

                std::shared_ptr<MembershipListener> sampleInitialListener(
                        new SampleInitialListener(memberAddedInit, memberRemovedInit));
                std::shared_ptr<MembershipListener> sampleListener(
                        new SampleListenerInSimpleListenerTest(memberAdded, memberRemoved));

                auto initialListenerRegistrationId = cluster.addMembershipListener(sampleInitialListener);
                auto sampleListenerRegistrationId = cluster.addMembershipListener(sampleListener);

                HazelcastServer instance2(*g_srvFactory);

                ASSERT_OPEN_EVENTUALLY(memberAdded);
                ASSERT_OPEN_EVENTUALLY(memberAddedInit);

                instance2.shutdown();

                ASSERT_OPEN_EVENTUALLY(memberRemoved);
                ASSERT_OPEN_EVENTUALLY(memberRemovedInit);

                instance.shutdown();

                ASSERT_TRUE(cluster.removeMembershipListener(initialListenerRegistrationId));
                ASSERT_TRUE(cluster.removeMembershipListener(sampleListenerRegistrationId));
            }

            TEST_P(SimpleListenerTest, testClusterListeners) {
                HazelcastServer instance(*g_srvFactory);
                HazelcastClient hazelcastClient(GetParam());
                Cluster cluster = hazelcastClient.getCluster();
                boost::latch memberAdded(1);
                boost::latch memberAddedInit(2);
                boost::latch memberRemoved(1);
                boost::latch memberRemovedInit(1);

                SampleInitialListener sampleInitialListener(memberAddedInit, memberRemovedInit);
                SampleListenerInSimpleListenerTest sampleListener(memberAdded, memberRemoved);

                cluster.addMembershipListener(&sampleInitialListener);
                cluster.addMembershipListener(&sampleListener);

                HazelcastServer instance2(*g_srvFactory);

                ASSERT_OPEN_EVENTUALLY(memberAdded);
                ASSERT_OPEN_EVENTUALLY(memberAddedInit);

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
                SampleInitialListener sampleInitialListener(memberAddedInit, memberRemovedInit);
                SampleListenerInSimpleListenerTest sampleListener(memberAdded, memberRemoved);

                ClientConfig clientConfig = GetParam();
                clientConfig.addListener(&sampleListener);
                clientConfig.addListener(&sampleInitialListener);

                HazelcastServer instance(*g_srvFactory);
                HazelcastClient hazelcastClient(clientConfig);

                HazelcastServer instance2(*g_srvFactory);

                ASSERT_OPEN_EVENTUALLY(memberAdded);
                ASSERT_OPEN_EVENTUALLY(memberAddedInit);

                instance2.shutdown();

                ASSERT_OPEN_EVENTUALLY(memberRemoved);
                ASSERT_OPEN_EVENTUALLY(memberRemovedInit);

                instance.shutdown();
            }

            TEST_P(SimpleListenerTest, testDeregisterListener) {
                HazelcastServer instance(*g_srvFactory);
                ClientConfig clientConfig = GetParam();
                HazelcastClient hazelcastClient(clientConfig);

                auto map = hazelcastClient.getMap("testDeregisterListener");

                ASSERT_FALSE(map->removeEntryListener(spi::ClientContext(hazelcastClient).random_uuid()).get());

                boost::latch map_clearedLatch(1);

                EntryListener listener;

                listener.on_map_cleared([&map_clearedLatch](MapEvent &&event) {
                    ASSERT_EQ("testDeregisterListener", event.getName());
                    ASSERT_EQ(EntryEvent::type::CLEAR_ALL, event.getEventType());
                    const std::string &hostName = event.getMember().getAddress().getHost();
                    ASSERT_TRUE(hostName == "127.0.0.1" || hostName == "localhost");
                    ASSERT_EQ(5701, event.getMember().getAddress().getPort());
                    ASSERT_EQ(1, event.getNumberOfEntriesAffected());
                    std::cout << "Map cleared event received:" << event << std::endl;
                    map_clearedLatch.count_down();
                });

                auto listenerRegistrationId = map->addEntryListener(std::move(listener), true).get();
                map->put(1, 1).get();
                map->clear().get();
                ASSERT_OPEN_EVENTUALLY(map_clearedLatch);
                ASSERT_TRUE(map->removeEntryListener(listenerRegistrationId).get());
            }

            TEST_P(SimpleListenerTest, testEmptyListener) {
                HazelcastServer instance(*g_srvFactory);
                HazelcastClient hazelcastClient(GetParam());

                auto map = hazelcastClient.getMap("testEmptyListener");

                // empty listener with no handlers
                EntryListener listener;

                auto listenerRegistrationId = map->addEntryListener(std::move(listener), true).get();

                // entry added
                ASSERT_EQ(boost::none, map->put(1, 1).get());
                // entry updated
                ASSERT_EQ(1, map->put(1, 2).get());
                // entry removed
                ASSERT_EQ(2, (map->remove<int, int>(1).get()));
                // map cleared
                map->clear().get();

                // wait to ensure events are triggered
                std::this_thread::sleep_for(std::chrono::milliseconds(500));

                ASSERT_TRUE(map->removeEntryListener(listenerRegistrationId).get());
            }

            INSTANTIATE_TEST_SUITE_P(All,
                                     SimpleListenerTest,
                                     ::testing::Values(ClientTestSupportBase::getConfig(),
                                                       ClientTestSupportBase::getConfig(false, false)));
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace test {
            class FlakeIdGeneratorApiTest : public ClientTestSupport {
            protected:
                virtual void SetUp() {
                    ASSERT_TRUE(client);
                    flakeIdGenerator = client->getFlakeIdGenerator(testing::UnitTest::GetInstance()->current_test_info()->name());
                }

                static void SetUpTestCase() {
                    instance = new HazelcastServer(*g_srvFactory);
                    ClientConfig clientConfig = getConfig();
                    std::shared_ptr<config::ClientFlakeIdGeneratorConfig> flakeIdConfig(
                            new config::ClientFlakeIdGeneratorConfig("test*"));
                    flakeIdConfig->setPrefetchCount(10).setPrefetchValidityDuration(std::chrono::seconds(20));
                    clientConfig.addFlakeIdGeneratorConfig(flakeIdConfig);
                    client = new HazelcastClient(clientConfig);
                }

                static void TearDownTestCase() {
                    delete client;
                    delete instance;

                    client = nullptr;
                    instance = nullptr;
                }

            protected:
                static HazelcastServer *instance;
                static HazelcastClient *client;

                std::shared_ptr<FlakeIdGenerator> flakeIdGenerator;
            };

            HazelcastServer *FlakeIdGeneratorApiTest::instance = nullptr;
            HazelcastClient *FlakeIdGeneratorApiTest::client = nullptr;

            TEST_F (FlakeIdGeneratorApiTest, testStartingValue) {
                ASSERT_NO_THROW(flakeIdGenerator->newId().get());
            }

            TEST_F (FlakeIdGeneratorApiTest, testSmoke) {
                boost::latch startLatch(1);
                std::array<std::future<std::unordered_set<int64_t>>, 4> futures;
                constexpr size_t NUM_IDS_PER_THREAD = 100000;
                constexpr int NUM_THREADS = 4;

                for (int i = 0; i < NUM_THREADS; ++i) {
                    futures[i] = std::async([&]() {
                        std::unordered_set<int64_t> localIds;
                        startLatch.wait();
                        for (size_t j = 0; j < NUM_IDS_PER_THREAD; ++j) {
                            localIds.insert(flakeIdGenerator->newId().get());
                        }

                        return localIds;
                    });
                }

                startLatch.count_down();

                std::unordered_set<int64_t> allIds;
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

                ~ClientTxnMapTest() override;

            protected:
                HazelcastServer instance;
                ClientConfig clientConfig;
                HazelcastClient client;
            };

            ClientTxnMapTest::ClientTxnMapTest() : instance(*g_srvFactory), client(getNewClient()) {
            }

            ClientTxnMapTest::~ClientTxnMapTest() = default;

            TEST_F(ClientTxnMapTest, testPutGet) {
                std::string name = "testPutGet";

                TransactionContext context = client.newTransactionContext();
                context.beginTransaction().get();

                auto map = context.getMap(name);

                ASSERT_FALSE((map->put<std::string, std::string>("key1", "value1").get().has_value()));
                ASSERT_EQ("value1", (map->get<std::string, std::string>("key1").get().value()));
                auto val = client.getMap(name)->get<std::string, std::string>("key1").get();
                ASSERT_FALSE(val.has_value());

                context.commitTransaction().get();

                ASSERT_EQ("value1", (client.getMap(name)->get<std::string, std::string>("key1").get().value()));
            }

            TEST_F(ClientTxnMapTest, testRemove) {
                std::string name = "testRemove";

                TransactionContext context = client.newTransactionContext();
                context.beginTransaction().get();

                auto map = context.getMap(name);

                ASSERT_FALSE((map->put<std::string, std::string>("key1", "value1").get().has_value()));
                ASSERT_EQ("value1", (map->get<std::string, std::string>("key1").get().value()));
                auto val = client.getMap(name)->get<std::string, std::string>("key1").get();
                ASSERT_FALSE(val.has_value());

                ASSERT_FALSE((map->remove<std::string, std::string>("key2").get().has_value()));
                val = map->remove<std::string, std::string>("key1").get();
                ASSERT_TRUE(val.has_value());
                ASSERT_EQ("value1", val.value());

                context.commitTransaction().get();

                auto regularMap = client.getMap(name);
                ASSERT_TRUE(regularMap->isEmpty().get());
            }

            TEST_F(ClientTxnMapTest, testRemoveIfSame) {
                std::string name = "testRemoveIfSame";

                TransactionContext context = client.newTransactionContext();
                context.beginTransaction().get();

                auto map = context.getMap(name);

                ASSERT_FALSE((map->put<std::string, std::string>("key1", "value1").get().has_value()));
                ASSERT_EQ("value1", (map->get<std::string, std::string>("key1").get().value()));
                ASSERT_EQ("value1", (map->get<std::string, std::string>("key1").get().value()));
                auto val = client.getMap(name)->get<std::string, std::string>("key1").get();
                ASSERT_FALSE(val.has_value());

                ASSERT_FALSE((map->remove<std::string, std::string>("key2").get().has_value()));;
                ASSERT_TRUE(map->remove("key1", "value1").get());

                context.commitTransaction().get();

                auto regularMap = client.getMap(name);
                ASSERT_TRUE(regularMap->isEmpty().get());
            }

            TEST_F(ClientTxnMapTest, testDeleteEntry) {
                std::string name = "testDeleteEntry";

                TransactionContext context = client.newTransactionContext();
                context.beginTransaction().get();

                auto map = context.getMap(name);

                ASSERT_NO_THROW(map->deleteEntry("key1").get());

                ASSERT_FALSE((map->put<std::string, std::string>("key1", "value1").get().has_value()));
                ASSERT_EQ("value1", (map->get<std::string, std::string>("key1").get().value()));
                auto val = client.getMap(name)->get<std::string, std::string>("key1").get();
                ASSERT_FALSE(val.has_value());

                ASSERT_NO_THROW(map->deleteEntry("key1").get());
                val = map->get<std::string, std::string>("key1").get();
                ASSERT_FALSE(val.has_value());

                context.commitTransaction().get();

                auto regularMap = client.getMap(name);
                ASSERT_TRUE(regularMap->isEmpty().get());
            }

            TEST_F(ClientTxnMapTest, testReplace) {
                std::string name = "testReplace";

                TransactionContext context = client.newTransactionContext();
                context.beginTransaction().get();

                auto map = context.getMap(name);

                ASSERT_FALSE((map->put<std::string, std::string>("key1", "value1").get().has_value()));
                ASSERT_EQ("value1", (map->get<std::string, std::string>("key1").get().value()));
                auto val = client.getMap(name)->get<std::string, std::string>("key1").get();
                ASSERT_FALSE(val.has_value());

                ASSERT_EQ("value1", (map->replace<std::string, std::string>("key1", "myNewValue").get().value()));

                context.commitTransaction().get();

                ASSERT_EQ("myNewValue", (client.getMap(name)->get<std::string, std::string>("key1").get().value()));
            }

            TEST_F(ClientTxnMapTest, testSet) {
                std::string name = "testSet";

                TransactionContext context = client.newTransactionContext();
                context.beginTransaction().get();

                auto map = context.getMap(name);

                ASSERT_NO_THROW(map->set("key1", "value1").get());

                auto val = map->get<std::string, std::string>("key1").get();
                ASSERT_TRUE(val.has_value());
                ASSERT_EQ("value1", val.value());

                val = client.getMap(name)->get<std::string, std::string>("key1").get();
                ASSERT_FALSE(val.has_value());

                ASSERT_NO_THROW(map->set("key1", "myNewValue").get());

                val = map->get<std::string, std::string>("key1").get();
                ASSERT_TRUE(val.has_value());
                ASSERT_EQ("myNewValue", val.value());

                context.commitTransaction().get();

                val = client.getMap(name)->get<std::string, std::string>("key1").get();
                ASSERT_TRUE(val.has_value());
                ASSERT_EQ("myNewValue", val.value());
            }

            TEST_F(ClientTxnMapTest, testContains) {
                std::string name = "testContains";

                TransactionContext context = client.newTransactionContext();
                context.beginTransaction().get();

                auto map = context.getMap(name);

                ASSERT_FALSE(map->containsKey("key1").get());

                ASSERT_NO_THROW(map->set("key1", "value1").get());

                auto val = map->get<std::string, std::string>("key1").get();
                ASSERT_TRUE(val.has_value());
                ASSERT_EQ("value1", val.value());

                ASSERT_TRUE(map->containsKey("key1").get());

                context.commitTransaction().get();

                auto regularMap = client.getMap(name);
                ASSERT_TRUE(regularMap->containsKey("key1").get());
            }

            TEST_F(ClientTxnMapTest, testReplaceIfSame) {
                std::string name = "testReplaceIfSame";

                TransactionContext context = client.newTransactionContext();
                context.beginTransaction().get();

                auto map = context.getMap(name);

                ASSERT_FALSE((map->put<std::string, std::string>("key1", "value1").get().has_value()));
                ASSERT_EQ("value1", (map->get<std::string, std::string>("key1").get().value()));
                auto val = client.getMap(name)->get<std::string, std::string>("key1").get();
                ASSERT_FALSE(val.has_value());

                ASSERT_FALSE(map->replace("key1", "valueNonExistent", "myNewValue").get());
                ASSERT_TRUE(map->replace("key1", "value1", "myNewValue").get());

                context.commitTransaction().get();

                ASSERT_EQ("myNewValue", (client.getMap(name)->get<std::string, std::string>("key1").get().value()));
            }

            TEST_F(ClientTxnMapTest, testPutIfSame) {
                std::string name = "testPutIfSame";

                TransactionContext context = client.newTransactionContext();
                context.beginTransaction().get();

                auto map = context.getMap(name);

                auto val = map->putIfAbsent<std::string, std::string>("key1", "value1").get();
                ASSERT_FALSE(val.has_value());
                val = map->get<std::string, std::string>("key1").get();
                ASSERT_TRUE(val.has_value());
                ASSERT_EQ("value1", val.value());
                val = client.getMap(name)->get<std::string, std::string>("key1").get();
                ASSERT_FALSE(val.has_value());

                val = map->putIfAbsent<std::string, std::string>("key1", "value1").get();
                ASSERT_TRUE(val.has_value());
                ASSERT_EQ("value1", val.value());

                context.commitTransaction().get();

                val = client.getMap(name)->get<std::string, std::string>("key1").get();
                ASSERT_TRUE(val.has_value());
                ASSERT_EQ("value1", val.value());
            }

//            @Test MTODO
//            public void testGetForUpdate() throws TransactionException {
//            final auto map = hz.getMap("testTxnGetForUpdate");
//            final CountDownLatch latch1 = new CountDownLatch(1);
//            final CountDownLatch latch2 = new CountDownLatch(1);
//            map->put<std::string, std::string>("var", 0);
//            final AtomicBoolean pass = new AtomicBoolean(true);
//
//
//            Runnable incrementor = new Runnable() {
//                public void run() {
//                    try {
//                        latch1.await(100, TimeUnit.SECONDS);
//                        pass.set(map->tryPut("var", 1, 0, TimeUnit.SECONDS) == false);
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
//                        txMap->getForUpdate("var");
//                        latch1.count_down();
//                        latch2.await(100, TimeUnit.SECONDS);
//                    } catch (Exception e) {
//                    }
//                    return true;
//                }
//            });
//            ASSERT_TRUE(b);
//            ASSERT_TRUE(pass.get());
//            ASSERT_TRUE(map->tryPut("var", 1, 0, TimeUnit.SECONDS));
//        }

            TEST_F(ClientTxnMapTest, testKeySetValues) {
                std::string name = "testKeySetValues";
                auto map = client.getMap(name);
                map->put<std::string, std::string>("key1", "value1").get();
                map->put<std::string, std::string>("key2", "value2").get();

                TransactionContext context = client.newTransactionContext();
                context.beginTransaction().get();
                auto txMap = context.getMap(name);
                ASSERT_FALSE((txMap->put<std::string, std::string>("key3", "value3").get().has_value()));


                ASSERT_EQ(3, (int) txMap->size().get());
                ASSERT_EQ(3, (int) txMap->keySet<std::string>().get().size());
                ASSERT_EQ(3, (int) txMap->values<std::string>().get().size());
                context.commitTransaction().get();

                ASSERT_EQ(3, (int) map->size().get());
                ASSERT_EQ(3, (int) map->keySet<std::string>().get().size());
                ASSERT_EQ(3, (int) map->values<std::string>().get().size());

            }

            TEST_F(ClientTxnMapTest, testKeySetAndValuesWithPredicates) {
                std::string name = "testKeysetAndValuesWithPredicates";
                auto map = client.getMap(name);

                Employee emp1("abc-123-xvz", 34);
                Employee emp2("abc-123-xvz", 20);

                map->put<Employee, Employee>(emp1, emp1).get();

                TransactionContext context = client.newTransactionContext();
                context.beginTransaction().get();

                auto txMap = context.getMap(name);
                ASSERT_FALSE((txMap->put<Employee, Employee>(emp2, emp2).get().has_value()));

                ASSERT_EQ(2, (int) txMap->size().get());
                ASSERT_EQ(2, (int) txMap->keySet<Employee>().get().size());
                query::SqlPredicate predicate(client, "a = 10");
                ASSERT_EQ(0, (int) txMap->keySet<Employee>(predicate).get().size());
                ASSERT_EQ(0, (int) txMap->values<Employee>(predicate).get().size());
                query::SqlPredicate predicate2(client, "a >= 10");
                ASSERT_EQ(2, (int) txMap->keySet<Employee>(predicate2).get().size());
                ASSERT_EQ(2, (int) txMap->values<Employee>(predicate2).get().size());

                context.commitTransaction().get();

                ASSERT_EQ(2, (int) map->size().get());
                ASSERT_EQ(2, (int) map->values<Employee>().get().size());
            }

            TEST_F(ClientTxnMapTest, testIsEmpty) {
                std::string name = "testIsEmpty";

                TransactionContext context = client.newTransactionContext();
                context.beginTransaction().get();

                auto map = context.getMap(name);

                ASSERT_TRUE(map->isEmpty().get());

                auto oldValue = map->put<std::string, std::string>("key1", "value1").get();
                ASSERT_FALSE(oldValue.has_value()) << "old value should not exist";

                ASSERT_FALSE(map->isEmpty().get());

                context.commitTransaction().get();

                auto regularMap = client.getMap(name);
                ASSERT_FALSE(regularMap->isEmpty().get());
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace test {
            class ClientTxnSetTest : public ClientTestSupport {
            public:
                ClientTxnSetTest();

                ~ClientTxnSetTest() override;

            protected:
                HazelcastServer instance;
                HazelcastClient client;
            };

            ClientTxnSetTest::ClientTxnSetTest() : instance(*g_srvFactory), client(getNewClient()) {
            }

            ClientTxnSetTest::~ClientTxnSetTest() = default;

            TEST_F(ClientTxnSetTest, testAddRemove) {
                auto s = client.getSet("testAddRemove");
                s->add<std::string>("item1").get();

                TransactionContext context = client.newTransactionContext();
                context.beginTransaction().get();
                auto set = context.getSet("testAddRemove");
                ASSERT_TRUE(set->add<std::string>("item2").get());
                ASSERT_EQ(2, set->size().get());
                ASSERT_EQ(1, s->size().get());
                ASSERT_FALSE(set->remove("item3").get());
                ASSERT_TRUE(set->remove("item1").get());

                context.commitTransaction().get();

                ASSERT_EQ(1, s->size().get());
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace test {
            class ClientTxnTest : public ClientTestSupport {
            public:
                ClientTxnTest();

                ~ClientTxnTest() override;

            protected:
                HazelcastServerFactory & hazelcastInstanceFactory;
                std::unique_ptr<HazelcastServer> server;
                std::unique_ptr<HazelcastServer> second;
                std::unique_ptr<HazelcastClient> client;
                std::unique_ptr<LoadBalancer> loadBalancer;
            };

            class MyLoadBalancer : public impl::AbstractLoadBalancer {
            public:
                boost::optional<Member> next() override {
                    std::vector<Member> members = getMembers();
                    size_t len = members.size();
                    if (len == 0) {
                        return boost::none;
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
                        : countDownLatch(countDownLatch) {}

                void memberAdded(const MembershipEvent &membershipEvent) override {}

                void memberRemoved(const MembershipEvent &membershipEvent) override {
                    countDownLatch.count_down();
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
                context.beginTransaction().get();

                server->shutdown();
                second->shutdown();

                ASSERT_THROW(context.commitTransaction().get(), exception::TransactionException);
            }

            TEST_F(ClientTxnTest, testTxnCommit) {
                std::string queueName = randomString();
                TransactionContext context = client->newTransactionContext();
                context.beginTransaction().get();
                ASSERT_FALSE(context.getTxnId().is_nil());
                auto queue = context.getQueue(queueName);
                std::string value = randomString();
                queue->offer(value).get();

                context.commitTransaction().get();

                auto q = client->getQueue(queueName);
                auto  retrievedElement = q->poll<std::string>().get();
                ASSERT_TRUE(retrievedElement.has_value());
                ASSERT_EQ(value, retrievedElement.value());
            }

            TEST_F(ClientTxnTest, testTxnCommitUniSocket) {
                ClientConfig clientConfig;
                clientConfig.getNetworkConfig().setSmartRouting(false);
                HazelcastClient uniSocketClient(clientConfig);

                std::string queueName = randomString();
                TransactionContext context = uniSocketClient.newTransactionContext();
                context.beginTransaction().get();
                ASSERT_FALSE(context.getTxnId().is_nil());
                auto queue = context.getQueue(queueName);
                std::string value = randomString();
                queue->offer(value).get();

                context.commitTransaction().get();

                auto q = uniSocketClient.getQueue(queueName);
                auto  retrievedElement = q->poll<std::string>().get();
                ASSERT_TRUE(retrievedElement.has_value());
                ASSERT_EQ(value, retrievedElement.value());
            }

            TEST_F(ClientTxnTest, testTxnCommitWithOptions) {
                std::string queueName = randomString();
                TransactionOptions transactionOptions;
                transactionOptions.setTransactionType(TransactionOptions::TransactionType::TWO_PHASE);
                transactionOptions.setTimeout(std::chrono::seconds(60));
                transactionOptions.setDurability(2);
                TransactionContext context = client->newTransactionContext(transactionOptions);

                context.beginTransaction().get();
                ASSERT_FALSE(context.getTxnId().is_nil());
                auto queue = context.getQueue(queueName);
                std::string value = randomString();
                queue->offer(value).get();

                context.commitTransaction().get();

                auto q = client->getQueue(queueName);
                auto  retrievedElement = q->poll<std::string>().get();
                ASSERT_TRUE(retrievedElement.has_value());
                ASSERT_EQ(value, retrievedElement.value());
            }

            TEST_F(ClientTxnTest, testTxnCommitAfterClientShutdown) {
                std::string queueName = randomString();
                TransactionContext context = client->newTransactionContext();
                context.beginTransaction().get();
                auto queue = context.getQueue(queueName);
                std::string value = randomString();
                queue->offer(value).get();

                client->shutdown();

                ASSERT_THROW(context.commitTransaction().get(), exception::TransactionException);
            }


            TEST_F(ClientTxnTest, testTxnRollback) {
                std::string queueName = randomString();
                TransactionContext context = client->newTransactionContext();
                boost::latch txnRollbackLatch(1);
                boost::latch memberRemovedLatch(1);
                MyMembershipListener myLifecycleListener(memberRemovedLatch);
                client->getCluster().addMembershipListener(&myLifecycleListener);

                try {
                    context.beginTransaction().get();
                    ASSERT_FALSE(context.getTxnId().is_nil());
                    auto queue = context.getQueue(queueName);
                    queue->offer(randomString()).get();

                    server->shutdown();

                    context.commitTransaction().get();
                    FAIL();
                } catch (exception::TransactionException &) {
                    context.rollbackTransaction().get();
                    txnRollbackLatch.count_down();
                }

                ASSERT_OPEN_EVENTUALLY(txnRollbackLatch);
                ASSERT_OPEN_EVENTUALLY(memberRemovedLatch);

                auto q = client->getQueue(queueName);
                ASSERT_FALSE(q->poll<std::string>().get().has_value())
                                            << "Poll result should be null since it is rolled back";
                ASSERT_EQ(0, q->size().get());
            }

            TEST_F(ClientTxnTest, testTxnRollbackOnServerCrash) {
                std::string queueName = randomString();
                TransactionContext context = client->newTransactionContext();
                boost::latch txnRollbackLatch(1);
                boost::latch memberRemovedLatch(1);

                context.beginTransaction().get();

                auto queue = context.getQueue(queueName);
                queue->offer("str").get();

                MyMembershipListener myLifecycleListener(memberRemovedLatch);
                client->getCluster().addMembershipListener(&myLifecycleListener);

                server->shutdown();

                ASSERT_THROW(context.commitTransaction().get(), exception::TransactionException);

                context.rollbackTransaction().get();
                txnRollbackLatch.count_down();

                ASSERT_OPEN_EVENTUALLY(txnRollbackLatch);
                ASSERT_OPEN_EVENTUALLY(memberRemovedLatch);

                auto q = client->getQueue(queueName);
                ASSERT_FALSE(q->poll<std::string>().get().has_value()) << "queue poll should return null";
                ASSERT_EQ(0, q->size().get());
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        class HazelcastClient;

        namespace test {
            class ClientTxnListTest : public ClientTestSupport {
            public:
                ClientTxnListTest();
                ~ClientTxnListTest() override;
            protected:
                HazelcastServer instance;
                ClientConfig clientConfig;
                HazelcastClient client;
            };

            ClientTxnListTest::ClientTxnListTest() : instance(*g_srvFactory), client(getNewClient()) {}

            ClientTxnListTest::~ClientTxnListTest() = default;

            TEST_F(ClientTxnListTest, testAddRemove) {
                auto l = client.getList("testAddRemove");
                l->add<std::string>("item1").get();

                TransactionContext context = client.newTransactionContext();
                context.beginTransaction().get();
                auto list = context.getList("testAddRemove");
                ASSERT_TRUE(list->add<std::string>("item2").get());
                ASSERT_EQ(2, list->size().get());
                ASSERT_EQ(1, l->size().get());
                ASSERT_FALSE(list->remove("item3").get());
                ASSERT_TRUE(list->remove("item1").get());

                context.commitTransaction().get();

                ASSERT_EQ(1, l->size().get());
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace test {
            class ClientTxnMultiMapTest : public ClientTestSupport {
            public:
                ClientTxnMultiMapTest();
                ~ClientTxnMultiMapTest() override;
            protected:
                HazelcastServer instance;
                HazelcastClient client;
            };

            ClientTxnMultiMapTest::ClientTxnMultiMapTest()
                    : instance(*g_srvFactory), client(getNewClient()) {}

            ClientTxnMultiMapTest::~ClientTxnMultiMapTest() = default;

            TEST_F(ClientTxnMultiMapTest, testRemoveIfExists) {
                TransactionContext context = client.newTransactionContext();
                context.beginTransaction().get();
                auto multiMap = context.getMultiMap("testRemoveIfExists");
                std::string key("MyKey");
                ASSERT_TRUE(multiMap->put(key, "value").get());
                ASSERT_TRUE(multiMap->put(key, "value1").get());
                ASSERT_TRUE(multiMap->put(key, "value2").get());
                ASSERT_EQ(3, (int) (multiMap->get<std::string, std::string>(key).get().size()));

                ASSERT_FALSE(multiMap->remove(key, "NonExistentValue").get());
                ASSERT_TRUE(multiMap->remove(key, "value1").get());

                ASSERT_EQ(2, multiMap->size().get());
                ASSERT_EQ(2, (int) (multiMap->valueCount<std::string>(key).get()));

                context.commitTransaction().get();

                auto mm = client.getMultiMap("testRemoveIfExists");
                ASSERT_EQ(2, (int) (mm->get<std::string, std::string>(key).get().size()));
            }

            TEST_F(ClientTxnMultiMapTest, testPutGetRemove) {
                auto mm = client.getMultiMap("testPutGetRemove");
                constexpr int n = 10;

                std::array<boost::future<void>, n> futures;
                for (int i = 0; i < n; i++) {
                    futures[i] = boost::async(std::packaged_task<void()>([&]() {
                        std::string key = std::to_string(hazelcast::util::getCurrentThreadId());
                        std::string key2 = key + "2";
                        client.getMultiMap("testPutGetRemove")->put(key, "value").get();
                        TransactionContext context = client.newTransactionContext();
                        context.beginTransaction().get();
                        auto multiMap = context.getMultiMap("testPutGetRemove");
                        ASSERT_FALSE(multiMap->put(key, "value").get());
                        ASSERT_TRUE(multiMap->put(key, "value1").get());
                        ASSERT_TRUE(multiMap->put(key, "value2").get());
                        ASSERT_TRUE(multiMap->put(key2, "value21").get());
                        ASSERT_TRUE(multiMap->put(key2, "value22").get());
                        ASSERT_EQ(3, (int) (multiMap->get<std::string, std::string>(key).get().size()));
                        ASSERT_EQ(3, (int) (multiMap->valueCount<std::string>(key).get()));
                        auto removedValues = multiMap->remove<std::string, std::string>(key2).get();
                        ASSERT_EQ(2U, removedValues.size());
                        ASSERT_TRUE((removedValues[0] == "value21" && removedValues[1] == "value22") ||
                                    (removedValues[1] == "value21" && removedValues[0] == "value22"));
                        context.commitTransaction().get();

                        ASSERT_EQ(3, (int) (mm->get<std::string, std::string>(key).get().size()));
                    }));
                }

                boost::wait_for_all(futures.begin(), futures.end());
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
                ~ClientTxnQueueTest() override;
            protected:
                HazelcastServer instance;
                HazelcastClient client;
            };

            ClientTxnQueueTest::ClientTxnQueueTest() : instance(*g_srvFactory), client(getNewClient()) {}

            ClientTxnQueueTest::~ClientTxnQueueTest() = default;

            TEST_F(ClientTxnQueueTest, testTransactionalOfferPoll1) {
                std::string name = "defQueue";

                TransactionContext context = client.newTransactionContext();
                context.beginTransaction().get();
                auto q = context.getQueue(name);
                ASSERT_TRUE(q->offer("ali").get());
                ASSERT_EQ(1, q->size().get());
                ASSERT_EQ("ali", q->poll<std::string>().get().value());
                ASSERT_EQ(0, q->size().get());
                context.commitTransaction().get();
                ASSERT_EQ(0, client.getQueue(name)->size().get());
            }

            TEST_F(ClientTxnQueueTest, testTransactionalOfferPollByteVector) {
                std::string name = "defQueue";

                TransactionContext context = client.newTransactionContext();
                context.beginTransaction().get();
                auto q = context.getQueue(name);
                std::vector<byte> value(3);
                ASSERT_TRUE(q->offer(value).get());
                ASSERT_EQ(1, q->size().get());
                ASSERT_EQ(value, q->poll<std::vector<byte>>().get().value());
                ASSERT_EQ(0, q->size().get());
                context.commitTransaction().get();
                ASSERT_EQ(0, client.getQueue(name)->size().get());
            }

            void testTransactionalOfferPoll2Thread(hazelcast::util::ThreadArgs &args) {
                boost::latch *latch1 = (boost::latch *) args.arg0;
                HazelcastClient *client = (HazelcastClient *) args.arg1;
                latch1->wait();
                client->getQueue("defQueue0")->offer("item0").get();
            }

            TEST_F(ClientTxnQueueTest, testTransactionalOfferPoll2) {
                boost::latch latch1(1);
                hazelcast::util::StartedThread t(testTransactionalOfferPoll2Thread, &latch1, &client);
                TransactionContext context = client.newTransactionContext();
                context.beginTransaction().get();
                auto q0 = context.getQueue("defQueue0");
                auto q1 = context.getQueue("defQueue1");
                boost::optional<std::string> s;
                latch1.count_down();
                s = q0->poll<std::string>(std::chrono::seconds(10)).get();
                ASSERT_EQ("item0", s.value());
                ASSERT_TRUE(q1->offer(s.value()).get());

                ASSERT_NO_THROW(context.commitTransaction().get());

                ASSERT_EQ(0, client.getQueue("defQueue0")->size().get());
                ASSERT_EQ("item0", client.getQueue("defQueue1")->poll<std::string>().get().value());
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
                    std::vector<byte> bytes{0x01, 0x12};
                    serialization::pimpl::DataInput<std::vector<byte>> dataInput(bytes);
                    ASSERT_EQ(0x01, dataInput.read<byte>());
                    ASSERT_EQ(0x12, dataInput.read<byte>());
                }

                TEST_F(DataInputTest, testReadBoolean) {
                    std::vector<byte> bytes{0x00, 0x10};
                    serialization::pimpl::DataInput<std::vector<byte>> dataInput(bytes);
                    ASSERT_FALSE(dataInput.read<bool>());
                    ASSERT_TRUE(dataInput.read<bool>());
                }

                TEST_F(DataInputTest, testReadChar) {
                    std::vector<byte> bytes{'a', 'b'};
                    serialization::pimpl::DataInput<std::vector<byte>> dataInput(bytes);
                    ASSERT_EQ('b', dataInput.read<char>());
                }

                TEST_F(DataInputTest, testReadShort) {
                    std::vector<byte> bytes{0x12, 0x34, 0x56};
                    serialization::pimpl::DataInput<std::vector<byte>> dataInput(bytes);
                    ASSERT_EQ(0x1234, dataInput.read<int16_t>());
                }

                TEST_F(DataInputTest, testReadInteger) {
                    std::vector<byte> bytes{0x12, 0x34, 0x56, 0x78, 0x90};
                    serialization::pimpl::DataInput<std::vector<byte>> dataInput(bytes);
                    ASSERT_EQ(INT32_C(0x12345678), dataInput.read<int32_t>());
                }

                TEST_F(DataInputTest, testReadLong) {
                    std::vector<byte> bytes{0x12, 0x34, 0x56, 0x78, 0x90, 0x9A, 0x9B, 0x9C};
                    serialization::pimpl::DataInput<std::vector<byte>> dataInput(bytes);
                    ASSERT_EQ(INT64_C(0x12345678909A9B9C), dataInput.read<int64_t>());
                }

                TEST_F(DataInputTest, testReadUTF) {
                    std::vector<byte> bytes{0x00, 0x00, 0x00, 0x04, 'b', 'd', 'f', 'h'};
                    serialization::pimpl::DataInput<std::vector<byte>> dataInput(bytes);
                    ASSERT_EQ("bdfh", dataInput.read<std::string>());
                }

                TEST_F(DataInputTest, testReadByteArray) {
                    std::vector<byte> bytes{0x00, 0x00, 0x00, 0x02};
                    std::vector<byte> actualDataBytes{0x12, 0x34};
                    bytes.insert(bytes.end(), actualDataBytes.begin(), actualDataBytes.end());
                    serialization::pimpl::DataInput<std::vector<byte>> dataInput(bytes);
                    auto readBytes = dataInput.read<std::vector<byte>>();
                    ASSERT_TRUE(readBytes.has_value());
                    ASSERT_EQ(actualDataBytes, *readBytes);
                }

                TEST_F(DataInputTest, testReadBooleanArray) {
                    std::vector<byte> bytes{0x00, 0x00, 0x00, 0x02, 0x00, 0x01};
                    serialization::pimpl::DataInput<std::vector<byte>> dataInput(bytes);
                    auto booleanArray = dataInput.read<std::vector<bool>>();
                    ASSERT_TRUE(booleanArray);
                    ASSERT_EQ(2U, booleanArray->size());
                    ASSERT_FALSE((*booleanArray)[0]);
                    ASSERT_TRUE((*booleanArray)[1]);
                }

                TEST_F(DataInputTest, testReadCharArray) {
                    std::vector<byte> bytes{0x00, 0x00, 0x00, 0x02, 0x00, 'f', 0x00, 'h'};
                    serialization::pimpl::DataInput<std::vector<byte>> dataInput(bytes);
                    auto charArray = dataInput.read<std::vector<char>>();
                    ASSERT_TRUE(charArray);
                    ASSERT_EQ(2U, charArray->size());
                    ASSERT_EQ('f', (*charArray)[0]);
                    ASSERT_EQ('h', (*charArray)[1]);
                }

                TEST_F(DataInputTest, testReadShortArray) {
                    std::vector<byte> bytes{0x00, 0x00, 0x00, 0x02, 0x12, 0x34, 0x56, 0x78};
                    serialization::pimpl::DataInput<std::vector<byte>> dataInput(bytes);
                    auto array = dataInput.read<std::vector<int16_t>>();
                    ASSERT_TRUE(array);
                    ASSERT_EQ(2U, array->size());
                    ASSERT_EQ(0x1234, (*array)[0]);
                    ASSERT_EQ(0x5678, (*array)[1]);
                }

                TEST_F(DataInputTest, testReadIntegerArray) {
                    std::vector<byte> bytes{0x00, 0x00, 0x00, 0x02, 0x12, 0x34, 0x56, 0x78, 0x1A, 0xBC, 0xDE, 0xEF};
                    serialization::pimpl::DataInput<std::vector<byte>> dataInput(bytes);
                    auto array = dataInput.read<std::vector<int32_t>>();
                    ASSERT_TRUE(array.has_value());
                    ASSERT_EQ(2U, array->size());
                    ASSERT_EQ(INT32_C(0x12345678), (*array)[0]);
                    ASSERT_EQ(INT32_C(0x1ABCDEEF), (*array)[1]);
                }

                TEST_F(DataInputTest, testReadLongArray) {
                    std::vector<byte> bytes{0x00, 0x00, 0x00, 0x02, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xEF,
                                            0x11, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8};
                    serialization::pimpl::DataInput<std::vector<byte>> dataInput(bytes);
                    auto array = dataInput.read<std::vector<int64_t>>();
                    ASSERT_TRUE(array.has_value());
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
                    std::vector<byte> bytes{0x01, 0x12};
                    serialization::pimpl::DataOutput dataOutput;
                    dataOutput.write<byte>((byte) 0x01);
                    dataOutput.write<byte>(0x12);
                    ASSERT_EQ(bytes, dataOutput.toByteArray());
                }

                TEST_F(DataOutputTest, testWriteBoolean) {
                    std::vector<byte> bytes{0x00, 0x01};
                    serialization::pimpl::DataOutput dataOutput;
                    dataOutput.write<bool>(false);
                    dataOutput.write<bool>(true);
                    ASSERT_EQ(bytes, dataOutput.toByteArray());
                }

                TEST_F(DataOutputTest, testWriteChar) {
                    std::vector<byte> bytes;
                    bytes.push_back(0);
                    bytes.push_back('b');
                    serialization::pimpl::DataOutput dataOutput;
                    dataOutput.write<char>('b');
                    ASSERT_EQ(bytes, dataOutput.toByteArray());
                }

                TEST_F(DataOutputTest, testWriteShort) {
                    std::vector<byte> bytes{0x12, 0x34};
                    serialization::pimpl::DataOutput dataOutput;
                    dataOutput.write<int16_t>(0x1234);
                    ASSERT_EQ(bytes, dataOutput.toByteArray());
                }

                TEST_F(DataOutputTest, testWriteInteger) {
                    std::vector<byte> bytes{0x12, 0x34, 0x56, 0x78};
                    serialization::pimpl::DataOutput dataOutput;
                    dataOutput.write<int32_t>(INT32_C(0x12345678));
                    ASSERT_EQ(bytes, dataOutput.toByteArray());
                }

                TEST_F(DataOutputTest, testWriteLong) {
                    std::vector<byte> bytes{0x12, 0x34, 0x56, 0x78, 0x90, 0x9A, 0x9B, 0x9C};
                    serialization::pimpl::DataOutput dataOutput;
                    dataOutput.write<int64_t>(INT64_C(0x12345678909A9B9C));
                    ASSERT_EQ(bytes, dataOutput.toByteArray());
                }

                TEST_F(DataOutputTest, testWriteUTF) {
                    std::vector<byte> bytes{0x00, 0x00, 0x00, 0x04, 'b', 'd', 'f', 'h'};
                    serialization::pimpl::DataOutput dataOutput;
                    std::string value("bdfh");
                    dataOutput.write<std::string>(&value);
                    ASSERT_EQ(bytes, dataOutput.toByteArray());
                }

                TEST_F(DataOutputTest, testWriteByteArray) {
                    std::vector<byte> bytes{0x00, 0x00, 0x00, 0x02};
                    std::vector<byte> actualDataBytes{0x12, 0x34};
                    bytes.insert(bytes.end(), actualDataBytes.begin(), actualDataBytes.end());
                    serialization::pimpl::DataOutput dataOutput;
                    dataOutput.write(&actualDataBytes);
                    ASSERT_EQ(bytes, dataOutput.toByteArray());
                }

                TEST_F(DataOutputTest, testWriteBooleanArray) {
                    std::vector<byte> bytes{0x00, 0x00, 0x00, 0x02, 0x00, 0x01};
                    std::vector<bool> actualValues;
                    actualValues.push_back(false);
                    actualValues.push_back(true);
                    serialization::pimpl::DataOutput dataOutput;
                    dataOutput.write<std::vector<bool>>(&actualValues);
                    ASSERT_EQ(bytes, dataOutput.toByteArray());
                }

                TEST_F(DataOutputTest, testWriteCharArray) {
                    std::vector<byte> bytes{0x00, 0x00, 0x00, 0x02, 0, 'f', 0, 'h'};
                    std::vector<char> actualChars{'f', 'h'};
                    serialization::pimpl::DataOutput dataOutput;
                    dataOutput.write<std::vector<char>>(actualChars);
                    ASSERT_EQ(bytes, dataOutput.toByteArray());
                }

                TEST_F(DataOutputTest, testWriteShortArray) {
                    std::vector<byte> bytes{0x00, 0x00, 0x00, 0x02, 0x12, 0x34, 0x56, 0x78};
                    std::vector<int16_t> actualValues{0x1234, 0x5678};
                    serialization::pimpl::DataOutput dataOutput;
                    dataOutput.write<std::vector<int16_t>>(&actualValues);
                    ASSERT_EQ(bytes, dataOutput.toByteArray());
                }

                TEST_F(DataOutputTest, testWriteIntegerArray) {
                    std::vector<byte> bytes{0x00, 0x00, 0x00, 0x02, 0x12, 0x34, 0x56, 0x78, 0x1A, 0xBC, 0xDE, 0xEF};
                    std::vector<int32_t> actualValues{INT32_C(0x12345678), INT32_C(0x1ABCDEEF)};
                    serialization::pimpl::DataOutput dataOutput;
                    dataOutput.write<std::vector<int32_t>>(&actualValues);
                    ASSERT_EQ(bytes, dataOutput.toByteArray());
                }

                TEST_F(DataOutputTest, testWriteLongArray) {
                    std::vector<byte> bytes = {0x00, 0x00, 0x00, 0x02, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xEF,
                                               0x01, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8};
                    std::vector<int64_t> actualValues{INT64_C(0x123456789ABCDEEF), INT64_C(0x01A2A3A4A5A6A7A8)};
                    serialization::pimpl::DataOutput dataOutput;
                    dataOutput.write<std::vector<int64_t>>(&actualValues);
                    ASSERT_EQ(bytes, dataOutput.toByteArray());
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
