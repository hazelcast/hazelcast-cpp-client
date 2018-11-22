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
/**
 * This has to be the first include, so that Python.h is the first include. Otherwise, compilation warning such as
 * "_POSIX_C_SOURCE" redefined occurs.
 */
#include "HazelcastServerFactory.h"

#include "HazelcastServer.h"
#include "ClientTestSupport.h"

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>

#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/client/connection/ClientConnectionManagerImpl.h"

namespace hazelcast {
    namespace client {
        namespace test {
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

                HazelcastServerFactory::Response getClientStatsFromServer() {
                    const char *script = "client0=instance_0.getClientService().getConnectedClients()."
                                         "toArray()[0]\nresult=client0.getClientStatistics();";

                    return g_srvFactory->executeOnController(script, HazelcastServerFactory::PYTHON);
                }

                std::string unescapeSpecialCharacters(const std::string &value) {
                    std::string escapedValue = boost::replace_all_copy(value, "\\,", ",");
                    boost::replace_all(escapedValue, "\\=", "=");
                    boost::replace_all(escapedValue, "\\\\", "\\");
                    return escapedValue;
                }

                std::map<std::string, std::string>
                getStatsFromResponse(const HazelcastServerFactory::Response &statsResponse) {
                    std::map<std::string, std::string> statsMap;
                    if (statsResponse.success && !statsResponse.result.empty()) {
                        std::vector<string> keyValuePairs;
                        boost::split(keyValuePairs, statsResponse.result, boost::is_any_of(","));

                        BOOST_FOREACH(const std::string &pair, keyValuePairs) {
                                        std::vector<string> keyValuePair;
                                        string input = unescapeSpecialCharacters(pair);
                                        boost::split(keyValuePair, input, boost::is_any_of("="));

                                        if (keyValuePair.size() > 1) {
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

                std::auto_ptr<HazelcastClient> createHazelcastClient() {
                    ClientConfig clientConfig;
                    clientConfig.setProperty(ClientProperties::STATISTICS_ENABLED, "true")
                            .setProperty(ClientProperties::STATISTICS_PERIOD_SECONDS,
                                         util::IOUtil::to_string<int>(STATS_PERIOD_SECONDS))
                                    // add IMap Near Cache config
                            .addNearCacheConfig(boost::shared_ptr<config::NearCacheConfig<int, int> >(
                                    new config::NearCacheConfig<int, int>(getTestName())));

                    clientConfig.getNetworkConfig().setConnectionAttemptLimit(20);

                    return std::auto_ptr<HazelcastClient>(new HazelcastClient(clientConfig));
                }

                void waitForFirstStatisticsCollection() {
                    ASSERT_TRUE_EVENTUALLY_WITH_TIMEOUT(verifyClientStatsFromServerIsNotEmpty(),
                                                        3 * STATS_PERIOD_SECONDS);
                }

                std::string getClientLocalAddress(HazelcastClient &client) {
                    spi::ClientContext clientContext(client);
                    connection::ClientConnectionManagerImpl &connectionManager = clientContext.getConnectionManager();
                    boost::shared_ptr<connection::Connection> ownerConnection = connectionManager.getOwnerConnection();
                    std::auto_ptr<Address> localSocketAddress = ownerConnection->getLocalSocketAddress();
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
                    BOOST_FOREACH(const StringMap::value_type &entry, map) {
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
                        boost::shared_ptr<config::NearCacheConfig<int, int> >(new config::NearCacheConfig<int, int>(
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
                std::auto_ptr<HazelcastClient> client = createHazelcastClient();

                int64_t clientConnectionTime = util::currentTimeMillis();

                // wait enough time for statistics collection
                waitForFirstStatisticsCollection();

                HazelcastServerFactory::Response statsResponse = getClientStatsFromServer();
                ASSERT_TRUE(statsResponse.success);
                string &stats = statsResponse.result;
                ASSERT_TRUE(!stats.empty());

                std::map<std::string, std::string> statsMap = getStatsFromResponse(statsResponse);

                ASSERT_EQ(1U, statsMap.count("clusterConnectionTimestamp"))
                                            << "clusterConnectionTimestamp stat should exist (" << stats << ")";
                int64_t connectionTimeStat;
                ASSERT_NO_THROW(
                        (connectionTimeStat = boost::lexical_cast<int64_t>(statsMap["clusterConnectionTimestamp"])))
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
                int64_t lastCollectionTime;
                std::string lastStatisticsCollectionTimeString = statsMap["lastStatisticsCollectionTime"];
                ASSERT_NO_THROW((lastCollectionTime = boost::lexical_cast<int64_t>(lastStatisticsCollectionTimeString)))
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
                std::auto_ptr<HazelcastClient> client = createHazelcastClient();

                // wait enough time for statistics collection
                waitForFirstStatisticsCollection();

                std::map<std::string, std::string> initialStats = getStats();

                // produce map stat
                produceSomeStats(*client);

                // wait enough time for statistics collection
                ASSERT_TRUE_EVENTUALLY(isStatsUpdated(initialStats["lastStatisticsCollectionTime"]));

                ASSERT_NE(initialStats, getStats()) << "initial statistics should not be the same as current stats";
            }

        }
    }
}

