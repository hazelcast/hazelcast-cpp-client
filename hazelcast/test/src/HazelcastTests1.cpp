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
#include "hazelcast/client/LifecycleEvent.h"
#include "hazelcast/logger.h"
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
#include <hazelcast/client/LifecycleListener.h>
#include "serialization/Serializables.h"
#include <unordered_set>
#include <cmath>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/thread/future.hpp>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#ifdef HZ_BUILD_WITH_SSL
#include <openssl/crypto.h>
#endif

#include "hazelcast/client/exception/ProtocolExceptions.h"
#include "hazelcast/client/internal/socket/SSLSocket.h"
#include "hazelcast/client/MembershipListener.h"
#include "hazelcast/client/InitialMembershipEvent.h"
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
            extern std::shared_ptr<RemoteControllerClient> remoteController;

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

                static Response get_client_stats_from_server() {
                    const char *script = "client0=instance_0.getClientService().getConnectedClients()."
                                         "toArray()[0]\nresult=client0.getClientAttributes();";

                    Response response;
                    remoteController->executeOnController(response, g_srvFactory->get_cluster_id(), script, Lang::PYTHON);
                    return response;
                }

                static std::string unescape_special_characters(const std::string &value) {
                    std::string escapedValue = boost::replace_all_copy(value, "\\,", ",");
                    boost::replace_all(escapedValue, "\\=", "=");
                    boost::replace_all(escapedValue, "\\\\", "\\");
                    return escapedValue;
                }

                std::unordered_map<std::string, std::string>
                get_stats_from_response(const Response &stats_response) {
                    std::unordered_map<std::string, std::string> statsMap;
                    if (stats_response.success && !stats_response.result.empty()) {
                        std::vector<std::string> keyValuePairs;
                        boost::split(keyValuePairs, stats_response.result, boost::is_any_of(","));

                        for(const auto &pair : keyValuePairs) {
                            std::vector<std::string> keyValuePair;
                            auto input = unescape_special_characters(pair);
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

                std::unordered_map<std::string, std::string> get_stats() {
                    auto statsResponse = get_client_stats_from_server();

                    return get_stats_from_response(statsResponse);
                }

                bool verify_client_stats_from_server_is_not_empty() {
                    auto response = get_client_stats_from_server();
                    return response.success && !response.result.empty();
                }

                std::unique_ptr<HazelcastClient> create_hazelcast_client() {
                    ClientConfig clientConfig;
                    clientConfig.set_property(ClientProperties::STATISTICS_ENABLED, "true")
                            .set_property(ClientProperties::STATISTICS_PERIOD_SECONDS,
                                         std::to_string(STATS_PERIOD_SECONDS))
                                    // add IMap Near Cache config
                            .add_near_cache_config(config::NearCacheConfig(get_test_name()));

                    clientConfig.get_network_config().set_connection_attempt_limit(20);

                    return std::unique_ptr<HazelcastClient>(new HazelcastClient(clientConfig));
                }

                void wait_for_first_statistics_collection() {
                    ASSERT_TRUE_EVENTUALLY_WITH_TIMEOUT(verify_client_stats_from_server_is_not_empty(),
                                                        3 * STATS_PERIOD_SECONDS);
                }

                std::string get_client_local_address(HazelcastClient &client) {
                    spi::ClientContext clientContext(client);
                    connection::ClientConnectionManagerImpl &connectionManager = clientContext.get_connection_manager();
                    auto connection = connectionManager.get_random_connection();
                    auto localSocketAddress = connection->get_local_socket_address();
                    std::ostringstream localAddressString;
                    localAddressString << localSocketAddress->get_host() << ":" << localSocketAddress->get_port();
                    return localAddressString.str();
                }

                bool is_stats_updated(const std::string &last_statistics_collection_time) {
                    auto stats = get_stats();
                    if (stats["lastStatisticsCollectionTime"] != last_statistics_collection_time) {
                        return true;
                    }
                    return false;
                }

                void produce_some_stats(HazelcastClient &client) {
                    auto map = client.get_map(get_test_name());
                    produce_some_stats(map);
                }

                void produce_some_stats(std::shared_ptr<IMap> &map) {
                    map->put(5, 10).get();
                    auto nearCacheStatsImpl = std::static_pointer_cast<monitor::impl::NearCacheStatsImpl>(
                            map->get_local_map_stats().get_near_cache_stats());

                    auto invalidationRequests = nearCacheStatsImpl->get_invalidation_requests();
                    // When for invalidation to come from server for the put operation
                    ASSERT_EQ_EVENTUALLY(
                            invalidationRequests + 1, nearCacheStatsImpl->get_invalidation_requests());

                    ASSERT_EQ(10, (*map->get<int, int>(5).get()));
                    ASSERT_EQ(10, (*map->get<int, int>(5).get()));
                }

                std::string to_string(const std::unordered_map<std::string, std::string> &map) {
                    std::ostringstream out;
                    out << "Map {" << std::endl;
                    for(const auto &entry : map) {
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
                clientConfig.set_property(ClientProperties::STATISTICS_PERIOD_SECONDS, "1");

                HazelcastClient client(clientConfig);

                // sleep twice the collection period
                sleep_seconds(2);

                Response statsFromServer = get_client_stats_from_server();
                ASSERT_TRUE(statsFromServer.success);
                ASSERT_TRUE(statsFromServer.message.empty()) << "Statistics should be disabled by default.";
            }

            TEST_F(ClientStatisticsTest, testNoUpdateWhenDisabled) {
                ClientConfig clientConfig;
                clientConfig.set_property(ClientProperties::STATISTICS_ENABLED, "false").set_property(
                        ClientProperties::STATISTICS_PERIOD_SECONDS, "1");

                HazelcastClient client(clientConfig);

                ASSERT_TRUE_ALL_THE_TIME(get_stats().empty(), 2);
            }

            TEST_F(ClientStatisticsTest, testClientStatisticsDisabledWithWrongValue) {

                ClientConfig clientConfig;
                clientConfig.set_property(ClientProperties::STATISTICS_ENABLED, "trueee");

                HazelcastClient client(clientConfig);

                // sleep twice the collection period
                sleep_seconds(2);

                Response statsFromServer = get_client_stats_from_server();
                ASSERT_TRUE(statsFromServer.success);
                ASSERT_TRUE(statsFromServer.message.empty()) << "Statistics should not be enabled with wrong value.";
            }

            TEST_F(ClientStatisticsTest, testClientStatisticsContent) {
                ClientConfig clientConfig;
                std::string mapName = get_test_name();
                clientConfig.add_near_cache_config(config::NearCacheConfig(mapName));
                clientConfig.set_property(ClientProperties::STATISTICS_ENABLED, "true").set_property(
                        ClientProperties::STATISTICS_PERIOD_SECONDS, "1");

                HazelcastClient client(clientConfig);

                // initialize near cache
                client.get_map(mapName);

                // sleep twice the collection period
                sleep_seconds(2);

                Response statsFromServer;
                ASSERT_TRUE_EVENTUALLY((statsFromServer = get_client_stats_from_server()).success && !statsFromServer.result.empty());
                ASSERT_TRUE(statsFromServer.success);
                ASSERT_FALSE(statsFromServer.result.empty());

                std::string localAddress = get_client_local_address(client);

                ASSERT_NE(std::string::npos,
                          statsFromServer.result.find(std::string("clientName=") + client.get_name()));
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
                auto statsMap = get_stats();
                std::string previous_stat_time;
                if (!statsMap.empty()) {
                    previous_stat_time = statsMap["clusterConnectionTimestamp"];
                }

                std::unique_ptr<HazelcastClient> client = create_hazelcast_client();

                int64_t clientConnectionTime = util::current_time_millis();

                // wait enough time for statistics collection
                wait_for_first_statistics_collection();
                ASSERT_TRUE_EVENTUALLY(previous_stat_time != get_stats()["clusterConnectionTimestamp"]);

                Response statsResponse = get_client_stats_from_server();
                ASSERT_TRUE(statsResponse.success);
                auto &stats = statsResponse.result;
                ASSERT_TRUE(!stats.empty());

                statsMap = get_stats_from_response(statsResponse);

                ASSERT_EQ(1U, statsMap.count("clusterConnectionTimestamp"))
                                            << "clusterConnectionTimestamp stat should exist (" << stats << ")";
                int64_t connectionTimeStat;
                ASSERT_NO_THROW(
                        (connectionTimeStat = std::stoll(statsMap["clusterConnectionTimestamp"])))
                                            << "connectionTimeStat value is not in correct (" << stats << ")";

                ASSERT_EQ(1U, statsMap.count("clientAddress")) << "clientAddress stat should exist (" << stats << ")";
                std::string expectedClientAddress = get_client_local_address(*client);
                ASSERT_EQ(expectedClientAddress, statsMap["clientAddress"]);

                ASSERT_EQ(1U, statsMap.count("clientVersion")) << "clientVersion stat should exist (" << stats << ")";
                ASSERT_EQ(HAZELCAST_VERSION, statsMap["clientVersion"]);

                // time measured by us after client connection should be greater than the connection time reported by the statistics
                ASSERT_GE(clientConnectionTime, connectionTimeStat) << "connectionTimeStat was " << connectionTimeStat
                                                                    << ", clientConnectionTime was "
                                                                    << clientConnectionTime << "(" << stats << ")";

                std::string mapHitsKey = "nc." + get_test_name() + ".hits";
                ASSERT_EQ(0U, statsMap.count(mapHitsKey)) << mapHitsKey << " stat should not exist (" << stats << ")";

                ASSERT_EQ(1U, statsMap.count("lastStatisticsCollectionTime"))
                                            << "lastStatisticsCollectionTime stat should exist (" << stats << ")";
                std::string lastStatisticsCollectionTimeString = statsMap["lastStatisticsCollectionTime"];
                ASSERT_NO_THROW((boost::lexical_cast<int64_t>(lastStatisticsCollectionTimeString)))
                                            << "lastStatisticsCollectionTime value is not in correct (" << stats << ")";

                // this creates empty map statistics
                auto map = client->get_map(get_test_name());

                statsMap = get_stats();
                lastStatisticsCollectionTimeString = statsMap["lastStatisticsCollectionTime"];

                // wait enough time for statistics collection
                ASSERT_TRUE_EVENTUALLY(is_stats_updated(lastStatisticsCollectionTimeString));

                statsMap = get_stats();
                ASSERT_EQ(1U, statsMap.count(mapHitsKey))
                                            << mapHitsKey << " stat should exist (" << to_string(statsMap) << ")";
                ASSERT_EQ("0", statsMap[mapHitsKey]) << "Expected 0 map hits (" << to_string(statsMap) << ")";

                // produce map stat
                produce_some_stats(map);

                statsMap = get_stats();
                lastStatisticsCollectionTimeString = statsMap["lastStatisticsCollectionTime"];

                // wait enough time for statistics collection
                ASSERT_TRUE_EVENTUALLY(is_stats_updated(lastStatisticsCollectionTimeString));

                statsMap = get_stats();
                ASSERT_EQ(1U, statsMap.count(mapHitsKey))
                                            << mapHitsKey << " stat should exist (" << to_string(statsMap) << ")";
                ASSERT_EQ("1", statsMap[mapHitsKey]) << "Expected 1 map hits (" << to_string(statsMap) << ")";
            }

            TEST_F(ClientStatisticsTest, testStatisticsPeriod) {
                std::unique_ptr<HazelcastClient> client = create_hazelcast_client();

                // wait enough time for statistics collection
                wait_for_first_statistics_collection();

                std::unordered_map<std::string, std::string> initialStats = get_stats();

                // produce map stat
                produce_some_stats(*client);

                // wait enough time for statistics collection
                ASSERT_TRUE_EVENTUALLY(is_stats_updated(initialStats["lastStatisticsCollectionTime"]));

                ASSERT_NE(initialStats, get_stats()) << "initial statistics should not be the same as current stats";
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace test {
            namespace ringbuffer {
                StartsWithStringFilter::StartsWithStringFilter(const std::string &start_string) : start_string_(
                        start_string) {}
            }
        }

        namespace serialization {
            int32_t hz_serializer<test::ringbuffer::StartsWithStringFilter>::get_factory_id() {
                return 666;
            }

            int32_t hz_serializer<test::ringbuffer::StartsWithStringFilter>::get_class_id() {
                return 14;
            }

            void hz_serializer<test::ringbuffer::StartsWithStringFilter>::write_data(
                    const test::ringbuffer::StartsWithStringFilter &object, ObjectDataOutput &out) {
                out.write(object.start_string_);
            }

            test::ringbuffer::StartsWithStringFilter
            hz_serializer<test::ringbuffer::StartsWithStringFilter>::read_data(ObjectDataInput &in) {
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
                            items_.emplace_back(std::to_string(i));
                        }
                    }

                protected:
                    void SetUp() override {
                        std::string testName = get_test_name();
                        client_ringbuffer_ = client->get_ringbuffer(testName);
                        client2_ringbuffer_ = client2->get_ringbuffer(testName);
                    }

                    void TearDown() override {
                        if (client_ringbuffer_) {
                            client_ringbuffer_->destroy().get();
                        }
                        if (client2_ringbuffer_) {
                            client2_ringbuffer_->destroy().get();
                        }
                    }

                    static void SetUpTestCase() {
                        instance = new HazelcastServer(*g_srvFactory);
                        client = new HazelcastClient(get_config());
                        client2 = new HazelcastClient(get_config());
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
                    std::shared_ptr<Ringbuffer> client_ringbuffer_;
                    std::shared_ptr<Ringbuffer> client2_ringbuffer_;
                    std::vector<std::string> items_;

                    static constexpr int64_t CAPACITY = 10;
                };

                constexpr int64_t RingbufferTest::CAPACITY;
                
                HazelcastServer *RingbufferTest::instance = nullptr;
                HazelcastClient *RingbufferTest::client = nullptr;
                HazelcastClient *RingbufferTest::client2 = nullptr;

                TEST_F(RingbufferTest, testAPI) {
                    std::shared_ptr<Ringbuffer> rb = client->get_ringbuffer(get_test_name() + "2");
                    ASSERT_EQ(CAPACITY, rb->capacity().get());
                    ASSERT_EQ(0, rb->head_sequence().get());
                    ASSERT_EQ(-1, rb->tail_sequence().get());
                    ASSERT_EQ(0, rb->size().get());
                    ASSERT_EQ(CAPACITY, rb->remaining_capacity().get());
                    ASSERT_THROW(rb->read_one<Employee>(-1).get(), exception::IllegalArgumentException);
                    ASSERT_THROW(rb->read_one<Employee>(1).get(), exception::IllegalArgumentException);

                    Employee employee1("First", 10);
                    Employee employee2("Second", 20);

                    ASSERT_EQ(0, rb->add<Employee>(employee1).get());
                    ASSERT_EQ(CAPACITY, rb->capacity().get());
                    ASSERT_EQ(CAPACITY, rb->remaining_capacity().get());
                    ASSERT_EQ(0, rb->head_sequence().get());
                    ASSERT_EQ(0, rb->tail_sequence().get());
                    ASSERT_EQ(1, rb->size().get());
                    ASSERT_EQ(employee1, rb->read_one<Employee>(0).get().value());
                    ASSERT_THROW(rb->read_one<Employee>(2).get(), exception::IllegalArgumentException);

                    ASSERT_EQ(1, rb->add<Employee>(employee2).get());
                    ASSERT_EQ(CAPACITY, rb->capacity().get());
                    ASSERT_EQ(CAPACITY, rb->remaining_capacity().get());
                    ASSERT_EQ(0, rb->head_sequence().get());
                    ASSERT_EQ(1, rb->tail_sequence().get());
                    ASSERT_EQ(2, rb->size().get());
                    ASSERT_EQ(employee1, rb->read_one<Employee>(0).get().value());
                    ASSERT_EQ(employee2, rb->read_one<Employee>(1).get().value());
                    ASSERT_THROW(rb->read_one<Employee>(3).get(), exception::IllegalArgumentException);

                    // insert many employees to fill the ringbuffer capacity
                    for (int i = 0; i < CAPACITY - 2; ++i) {
                        Employee eleman("name", 10 * (i + 2));
                        ASSERT_EQ(i + 2, rb->add<Employee>(eleman).get());
                        ASSERT_EQ(CAPACITY, rb->capacity().get());
                        ASSERT_EQ(CAPACITY, rb->remaining_capacity().get());
                        ASSERT_EQ(0, rb->head_sequence().get());
                        ASSERT_EQ(i + 2, rb->tail_sequence().get());
                        ASSERT_EQ(i + 3, rb->size().get());
                        ASSERT_EQ(eleman, rb->read_one<Employee>(i+2).get().value());
                    }

                    // verify that the head element is overriden on the first add
                    Employee latestEmployee("latest employee", 100);
                    ASSERT_EQ(CAPACITY, rb->add<Employee>(latestEmployee).get());
                    ASSERT_EQ(CAPACITY, rb->capacity().get());
                    ASSERT_EQ(CAPACITY, rb->remaining_capacity().get());
                    ASSERT_EQ(1, rb->head_sequence().get());
                    ASSERT_EQ(CAPACITY, rb->tail_sequence().get());
                    ASSERT_EQ(CAPACITY, rb->size().get());
                    ASSERT_EQ(latestEmployee, rb->read_one<Employee>(CAPACITY).get().value());
                }

                TEST_F(RingbufferTest, readManyAsync_whenHitsStale_useHeadAsStartSequence) {
                    client2_ringbuffer_->add_all(items_, client::ringbuffer::overflow_policy::OVERWRITE);
                    auto f = client_ringbuffer_->read_many<std::string>(1, 1, 10);
                    auto rs = f.get();
                    ASSERT_EQ(10, rs.read_count());
                    ASSERT_EQ(std::string("1"), *rs.get_items()[0].get<std::string>());
                    ASSERT_EQ(std::string("10"), *rs.get_items()[9].get<std::string>());
                }

                TEST_F(RingbufferTest, readOne_whenHitsStale_shouldNotBeBlocked) {
                    std::shared_ptr<boost::latch> latch1 = std::make_shared<boost::latch>(1);
                    std::thread([=] () {
                        try {
                            client_ringbuffer_->read_one<std::string>(0).get();
                            latch1->count_down();
                        } catch (exception::StaleSequenceException &) {
                            latch1->count_down();
                        }
                    }).detach();
                    client2_ringbuffer_->add_all(items_, client::ringbuffer::overflow_policy::OVERWRITE);
                    ASSERT_OPEN_EVENTUALLY(*latch1);
                }

                TEST_F(RingbufferTest, headSequence) {
                    for (int k = 0; k < 2 * CAPACITY; k++) {
                        client2_ringbuffer_->add<std::string>("foo").get();
                    }

                    ASSERT_EQ(client2_ringbuffer_->head_sequence().get(), client_ringbuffer_->head_sequence().get());
                }

                TEST_F(RingbufferTest, tailSequence) {
                    for (int k = 0; k < 2 * CAPACITY; k++) {
                        client2_ringbuffer_->add<std::string>("foo").get();
                    }

                    ASSERT_EQ(client2_ringbuffer_->tail_sequence().get(), client_ringbuffer_->tail_sequence().get());
                }

                TEST_F(RingbufferTest, size) {
                    client2_ringbuffer_->add<std::string>("foo").get();

                    ASSERT_EQ(client2_ringbuffer_->tail_sequence().get(), client_ringbuffer_->tail_sequence().get());
                }

                TEST_F(RingbufferTest, capacity) {
                    ASSERT_EQ(client2_ringbuffer_->capacity().get(), client_ringbuffer_->capacity().get());
                }

                TEST_F(RingbufferTest, remainingCapacity) {
                    client2_ringbuffer_->add<std::string>("foo").get();

                    ASSERT_EQ(client2_ringbuffer_->remaining_capacity().get(), client_ringbuffer_->remaining_capacity().get());
                }

                TEST_F(RingbufferTest, add) {
                    client_ringbuffer_->add<std::string>("foo").get();
                    auto value = client2_ringbuffer_->read_one<std::string>(0).get();
                    ASSERT_TRUE(value.has_value());
                    ASSERT_EQ("foo", value.value());
                }

                TEST_F(RingbufferTest, add_all) {
                    std::vector<std::string> items;
                    items.push_back("foo");
                    items.push_back("bar");
                    auto result = client_ringbuffer_->add_all(items, client::ringbuffer::overflow_policy::OVERWRITE).get();

                    ASSERT_EQ(client2_ringbuffer_->tail_sequence().get(), result);
                    auto val0 = client2_ringbuffer_->read_one<std::string>(0).get();
                    auto val1 = client2_ringbuffer_->read_one<std::string>(1).get();
                    ASSERT_TRUE(val0);
                    ASSERT_TRUE(val1);
                    ASSERT_EQ(val0.value(), "foo");
                    ASSERT_EQ(val1.value(), "bar");
                    ASSERT_EQ(0, client2_ringbuffer_->head_sequence().get());
                    ASSERT_EQ(1, client2_ringbuffer_->tail_sequence().get());
                }

                TEST_F(RingbufferTest, readOne) {
                    client2_ringbuffer_->add<std::string>("foo").get();
                    auto value = client_ringbuffer_->read_one<std::string>(0).get();
                    ASSERT_TRUE(value.has_value());
                    ASSERT_EQ("foo", value.value());
                }

                TEST_F(RingbufferTest, readMany_noFilter) {
                    client2_ringbuffer_->add<std::string>("1");
                    client2_ringbuffer_->add<std::string>("2");
                    client2_ringbuffer_->add<std::string>("3");

                    auto rs = client_ringbuffer_->read_many(0, 3, 3).get();

                    ASSERT_EQ(3, rs.read_count());
                    auto &items = rs.get_items();
                    ASSERT_EQ("1", items[0].get<std::string>().value());
                    ASSERT_EQ("2", items[1].get<std::string>().value());
                    ASSERT_EQ("3", items[2].get<std::string>().value());
                }

                // checks if the max count works. So if more results are available than needed, the surplus results should not be read.
                TEST_F(RingbufferTest, readMany_maxCount) {
                    client2_ringbuffer_->add<std::string>("1").get();
                    client2_ringbuffer_->add<std::string>("2").get();
                    client2_ringbuffer_->add<std::string>("3").get();
                    client2_ringbuffer_->add<std::string>("4").get();
                    client2_ringbuffer_->add<std::string>("5").get();
                    client2_ringbuffer_->add<std::string>("6").get();

                    client::ringbuffer::ReadResultSet rs = client_ringbuffer_->read_many<std::string>(0, 3, 3).get();

                    ASSERT_EQ(3, rs.read_count());
                    auto &items1 = rs.get_items();
                    ASSERT_EQ("1", items1[0].get<std::string>().value());
                    ASSERT_EQ("2", items1[1].get<std::string>().value());
                    ASSERT_EQ("3", items1[2].get<std::string>().value());
                }

                TEST_F(RingbufferTest, readManyAsync_withFilter) {
                    client2_ringbuffer_->add<std::string>("good1").get();
                    client2_ringbuffer_->add<std::string>("bad1").get();
                    client2_ringbuffer_->add<std::string>("good2").get();
                    client2_ringbuffer_->add<std::string>("bad2").get();
                    client2_ringbuffer_->add<std::string>("good3").get();
                    client2_ringbuffer_->add<std::string>("bad3").get();

                    StartsWithStringFilter filter("good");
                    auto rs = client_ringbuffer_->read_many<StartsWithStringFilter>(0, 3, 3, &filter).get();

                    ASSERT_EQ(5, rs.read_count());
                    auto const &items = rs.get_items();
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
            std::string ClientTestSupportBase::get_ca_file_path() {
                return "hazelcast/test/resources/cpp_client.crt";
            }

            hazelcast::client::ClientConfig ClientTestSupportBase::get_config(bool ssl_enabled, bool smart) {
                ClientConfig clientConfig;
                clientConfig.get_network_config().add_address(Address(g_srvFactory->get_server_address(), 5701));
                if (ssl_enabled) {
                    clientConfig.set_cluster_name(get_ssl_cluster_name());
                    clientConfig.get_network_config().get_ssl_config().set_enabled(true).add_verify_file(get_ca_file_path());
                }
                clientConfig.get_network_config().set_smart_routing(smart);
                return clientConfig;
            }

            HazelcastClient ClientTestSupportBase::get_new_client() {
                return HazelcastClient(get_config());
            }

            const std::string ClientTestSupportBase::get_ssl_file_path() {
                return "hazelcast/test/resources/hazelcast-ssl.xml";
            }

            std::string ClientTestSupportBase::random_map_name() {
                return random_string();
            }

            std::string ClientTestSupportBase::random_string() {
                // performance is not important, hence we can use random_device for the tests
                std::random_device rand{};
                return boost::uuids::to_string(boost::uuids::basic_random_generator<std::random_device>{rand}());
            }

            void ClientTestSupportBase::sleep_seconds(int32_t seconds) {
                hazelcast::util::sleep(seconds);
            }

            ClientTestSupportBase::ClientTestSupportBase() = default;

            boost::uuids::uuid ClientTestSupportBase::generate_key_owned_by(spi::ClientContext &context, const Member &member) {
                spi::impl::ClientPartitionServiceImpl &partitionService = context.get_partition_service();
                serialization::pimpl::SerializationService &serializationService = context.get_serialization_service();
                while (true) {
                    auto id = context.random_uuid();
                    int partitionId = partitionService.get_partition_id(serializationService.to_data(id));
                    std::shared_ptr<impl::Partition> partition = partitionService.get_partition(partitionId);
                    auto owner = partition->get_owner();
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
                : name_(name)
                , logger_(std::make_shared<logger>("StartedThread", "StartedThread", 
                                                   logger::level::info, logger::default_handler)) {
            init(func, arg0, arg1, arg2, arg3);
        }

        StartedThread::StartedThread(void (func)(ThreadArgs &),
                                     void *arg0,
                                     void *arg1,
                                     void *arg2,
                                     void *arg3)
                : StartedThread("hz.unnamed", func, arg0, arg1, arg2, arg3) {
        }

        void StartedThread::init(void (func)(ThreadArgs &), void *arg0, void *arg1, void *arg2, void *arg3) {
            thread_args_.arg0 = arg0;
            thread_args_.arg1 = arg1;
            thread_args_.arg2 = arg2;
            thread_args_.arg3 = arg3;
            thread_args_.func = func;

            thread_ = std::thread([=]() { func(thread_args_); });
        }

        void StartedThread::run() {
            thread_args_.func(thread_args_);
        }

        const std::string StartedThread::get_name() const {
            return name_;
        }

        bool StartedThread::join() {
            if (!thread_.joinable()) {
                return false;
            }
            thread_.join();
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
                std::vector<hazelcast::client::internal::socket::SSLSocket::CipherInfo> get_ciphers(ClientConfig &config) {
                    HazelcastClient client(config);
                    spi::ClientContext context(client);
                    std::vector<std::shared_ptr<connection::Connection> > conns = context.get_connection_manager().get_active_connections();
                    EXPECT_GT(conns.size(), (size_t) 0);
                    std::shared_ptr<connection::Connection> aConnection = conns[0];
                    hazelcast::client::internal::socket::SSLSocket &socket = (hazelcast::client::internal::socket::SSLSocket &) aConnection->get_socket();
                    return socket.get_ciphers();
                }

#endif
            };

            TEST_F(ClientConnectionTest, testTcpSocketTimeoutToOutsideNetwork) {
                HazelcastServer instance(*g_srvFactory);
                ClientConfig config;
                config.get_network_config().set_connection_attempt_period(std::chrono::seconds(1)).set_connection_timeout(std::chrono::seconds (2)).add_address(
                        Address("8.8.8.8", 5701));
                ASSERT_THROW(HazelcastClient client(config), exception::IllegalStateException);
            }

#ifdef HZ_BUILD_WITH_SSL
            TEST_F(ClientConnectionTest, testSslSocketTimeoutToOutsideNetwork) {
                HazelcastServerFactory sslFactory(g_srvFactory->get_server_address(), get_ssl_file_path());
                HazelcastServer instance(sslFactory);
                ClientConfig config;
                config.set_cluster_name(get_ssl_cluster_name()).get_network_config().
                        set_connection_attempt_period(std::chrono::seconds(1)).set_connection_timeout(std::chrono::seconds(2)).add_address(
                        Address("8.8.8.8", 5701)).get_ssl_config().set_enabled(true).add_verify_file(get_ca_file_path());
                ASSERT_THROW(HazelcastClient client(config), exception::IllegalStateException);
            }

            TEST_F(ClientConnectionTest, testSSLWrongCAFilePath) {
                HazelcastServerFactory sslFactory(g_srvFactory->get_server_address(), get_ssl_file_path());
                HazelcastServer instance(sslFactory);
                ClientConfig config = get_config();
                config.set_cluster_name(get_ssl_cluster_name());
                config.get_network_config().get_ssl_config().set_enabled(true).add_verify_file("abc");
                ASSERT_THROW(HazelcastClient client(config), exception::IllegalStateException);
            }

            TEST_F(ClientConnectionTest, testExcludedCipher) {
                HazelcastServerFactory sslFactory(g_srvFactory->get_server_address(), get_ssl_file_path());
                HazelcastServer instance(sslFactory);

                ClientConfig config = get_config(true);
                config.get_network_config().get_ssl_config().set_cipher_list("HIGH");
                std::vector<hazelcast::client::internal::socket::SSLSocket::CipherInfo> supportedCiphers = get_ciphers(
                        config);

                std::string unsupportedCipher = supportedCiphers[supportedCiphers.size() - 1].name;
                config = get_config(true);
                config.get_network_config().get_ssl_config().set_cipher_list(std::string("HIGH:!") + unsupportedCipher);

                std::vector<hazelcast::client::internal::socket::SSLSocket::CipherInfo> newCiphers = get_ciphers(config);

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
                ClusterTest() : ssl_factory_(g_srvFactory->get_server_address(), get_ssl_file_path()) {}

            protected:
                LifecycleListener make_all_states_listener(boost::latch &starting,
                                                        boost::latch &started,
                                                        boost::latch &connected,
                                                        boost::latch &disconnected,
                                                        boost::latch &shutting_down,
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
                        .on_shutting_down([&shutting_down](){
                            shutting_down.count_down();
                        })
                        .on_shutdown([&shutdown](){
                            shutdown.count_down();
                        });
                }
                
                std::unique_ptr<HazelcastServer> start_server(ClientConfig &client_config) {
                    if (client_config.get_network_config().get_ssl_config().is_enabled()) {
                        return std::unique_ptr<HazelcastServer>(new HazelcastServer(ssl_factory_));
                    } else {
                        return std::unique_ptr<HazelcastServer>(new HazelcastServer(*g_srvFactory));
                    }
                }

            private:
                HazelcastServerFactory ssl_factory_;
            };

            TEST_P(ClusterTest, testBehaviourWhenClusterNotFound) {
                ASSERT_THROW(HazelcastClient client(GetParam()), exception::IllegalStateException);
            }

            TEST_P(ClusterTest, testDummyClientBehaviourWhenClusterNotFound) {
                auto clientConfig = GetParam();
                clientConfig.get_network_config().set_smart_routing(false);
                ASSERT_THROW(HazelcastClient client(clientConfig), exception::IllegalStateException);
            }

            TEST_P(ClusterTest, testAllClientStates) {
                auto clientConfig = GetParam();
                std::unique_ptr<HazelcastServer> instance = start_server(clientConfig);

                auto networkConfig = clientConfig.get_network_config();
                networkConfig.set_connection_attempt_period(std::chrono::seconds(1));
                networkConfig.set_connection_attempt_limit(1);
                boost::latch startingLatch(1);
                boost::latch startedLatch(1);
                boost::latch connectedLatch(1);
                boost::latch disconnectedLatch(1);
                boost::latch shuttingDownLatch(1);
                boost::latch shutdownLatch(1);
                auto listener = make_all_states_listener(startingLatch, startedLatch, connectedLatch, disconnectedLatch,
                                                      shuttingDownLatch, shutdownLatch);
                clientConfig.add_listener(std::move(listener));

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
                clientConfig.get_network_config().set_connection_attempt_period(std::chrono::milliseconds(900)).
                        set_connection_timeout(std::chrono::seconds(2)).set_connection_attempt_limit(2);
                clientConfig.get_network_config().add_address(Address("8.8.8.8", 8000));

                int64_t startTimeMillis = hazelcast::util::current_time_millis();
                try {
                    HazelcastClient client(clientConfig);
                } catch (exception::IllegalStateException &) {
                    // this is expected
                }
                ASSERT_GE(hazelcast::util::current_time_millis() - startTimeMillis, 2 * 900);
            }

            TEST_P(ClusterTest, testAllClientStatesWhenUserShutdown) {
                auto clientConfig = GetParam();
                std::unique_ptr<HazelcastServer> instance = start_server(clientConfig);

                boost::latch startingLatch(1);
                boost::latch startedLatch(1);
                boost::latch connectedLatch(1);
                boost::latch disconnectedLatch(1);
                boost::latch shuttingDownLatch(1);
                boost::latch shutdownLatch(1);
                auto listener = make_all_states_listener(startingLatch, startedLatch, connectedLatch, disconnectedLatch,
                                                      shuttingDownLatch, shutdownLatch);
                clientConfig.add_listener(std::move(listener));

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
                                     ::testing::Values(ClientTestSupportBase::get_config(),
                                                       ClientTestSupportBase::get_config(true)));
#else
            INSTANTIATE_TEST_SUITE_P(All, 
                                     ClusterTest, 
                                     ::testing::Values(ClientTestSupportBase::getConfig()));
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
                ClientConfig config = get_config();
                config.set_property("hazelcast_client_heartbeat_interval", "1");

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

            SocketInterceptor make_socket_interceptor(boost::latch &l) {
                return SocketInterceptor()
                    .on_connect([&l](const hazelcast::client::Socket &connected_sock) {
                        ASSERT_EQ("127.0.0.1", connected_sock.get_address().get_host());
                        ASSERT_NE(0, connected_sock.get_address().get_port());
                        l.count_down();
                    });
            }

#ifdef HZ_BUILD_WITH_SSL
            TEST_F(SocketInterceptorTest, interceptSSLBasic) {
                HazelcastServerFactory sslFactory(g_srvFactory->get_server_address(), get_ssl_file_path());
                HazelcastServer instance(sslFactory);
                ClientConfig config = get_config(true);
                boost::latch interceptorLatch(1);
                auto interceptor = make_socket_interceptor(interceptorLatch);
                config.set_socket_interceptor(std::move(interceptor));
                HazelcastClient client(config);
                interceptorLatch.wait_for(boost::chrono::seconds(2));
            }

#endif

            TEST_F(SocketInterceptorTest, interceptBasic) {
                HazelcastServer instance(*g_srvFactory);
                ClientConfig config = get_config();
                boost::latch interceptorLatch(1);
                auto interceptor = make_socket_interceptor(interceptorLatch);
                config.set_socket_interceptor(std::move(interceptor));
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
                clientConfig.get_network_config().get_socket_options().set_keep_alive(false).set_reuse_address(
                        true).set_tcp_no_delay(false).set_linger_seconds(5).set_buffer_size_in_bytes(bufferSize);

                HazelcastClient client(clientConfig);

                config::SocketOptions &socketOptions = client.get_client_config().get_network_config().get_socket_options();
                ASSERT_FALSE(socketOptions.is_keep_alive());
                ASSERT_FALSE(socketOptions.is_tcp_no_delay());
                ASSERT_EQ(5, socketOptions.get_linger_seconds());
                ASSERT_EQ(bufferSize, socketOptions.get_buffer_size_in_bytes());
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace test {
            class ClientAuthenticationTest : public ClientTestSupport {
            };

            TEST_F(ClientAuthenticationTest, testUserPasswordCredentials) {
                HazelcastServerFactory factory("hazelcast/test/resources/hazelcast-username-password.xml");
                HazelcastServer instance(factory);
                HazelcastClient client(ClientConfig().set_cluster_name("username-pass-dev").set_credentials(
                        std::make_shared<security::username_password_credentials>("test-user", "test-pass")));
            }

            TEST_F(ClientAuthenticationTest, testTokenCredentials) {
                HazelcastServerFactory factory("hazelcast/test/resources/hazelcast-token-credentials.xml");
                HazelcastServer instance(factory);
                std::vector<byte> my_token = {'S', 'G', 'F', '6', 'Z', 'W'};
                HazelcastClient client(ClientConfig().set_cluster_name("token-credentials-dev").set_credentials(
                        std::make_shared<security::token_credentials>(my_token)));
            }

            TEST_F(ClientAuthenticationTest, testIncorrectGroupName) {
                HazelcastServer instance(*g_srvFactory);
                ClientConfig config;
                config.set_cluster_name("invalid cluster");

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
                ASSERT_EQ_EVENTUALLY(1, client.get_cluster().get_members().size());
                const Client endpoint = client.get_local_endpoint();
                spi::ClientContext context(client);
                ASSERT_EQ(context.get_name(), endpoint.get_name());

                auto endpointAddress = endpoint.get_socket_address();
                ASSERT_TRUE(endpointAddress);
                connection::ClientConnectionManagerImpl &connectionManager = context.get_connection_manager();
                std::shared_ptr<connection::Connection> connection = connectionManager.get_random_connection();
                ASSERT_NOTNULL(connection.get(), connection::Connection);
                auto localAddress = connection->get_local_socket_address();
                ASSERT_TRUE(localAddress);
                ASSERT_EQ(*localAddress, *endpointAddress);
                ASSERT_EQ(connectionManager.get_client_uuid(), endpoint.get_uuid());
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
                HazelcastClient hazelcastClient(get_new_client());
                Cluster cluster = hazelcastClient.get_cluster();
                std::vector<Member> members = cluster.get_members();
                ASSERT_EQ(1U, members.size());
                Member &member = members[0];
                std::string attribute_name = "test-member-attribute-name";
                ASSERT_TRUE(member.lookup_attribute(attribute_name));
                ASSERT_EQ("test-member-attribute-value", *member.get_attribute(attribute_name));
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
                            client = new HazelcastClient(get_config());
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
                        std::shared_ptr<PNCounter> pnCounter = client->get_pn_counter(
                                testing::UnitTest::GetInstance()->current_test_info()->name());
                        ASSERT_EQ(0, pnCounter->get().get());
                    }

                    TEST_F(BasicPnCounterAPITest, testGetAndAdd) {
                        auto pnCounter = client->get_pn_counter(
                                testing::UnitTest::GetInstance()->current_test_info()->name());
                        ASSERT_EQ(0, pnCounter->get_and_add(5).get());
                    }

                    TEST_F(BasicPnCounterAPITest, testAddAndGet) {
                        auto pnCounter = client->get_pn_counter(
                                testing::UnitTest::GetInstance()->current_test_info()->name());
                        ASSERT_EQ(5, pnCounter->add_and_get(5).get());
                    }

                    TEST_F(BasicPnCounterAPITest, testGetAndAddExisting) {
                        auto pnCounter = client->get_pn_counter(
                                testing::UnitTest::GetInstance()->current_test_info()->name());

                        ASSERT_EQ(0, pnCounter->get_and_add(2).get());
                        ASSERT_EQ(2, pnCounter->get_and_add(3).get());
                        ASSERT_EQ(5, pnCounter->get().get());
                    }

                    TEST_F(BasicPnCounterAPITest, testGetAndIncrement) {
                        auto pnCounter = client->get_pn_counter(
                                testing::UnitTest::GetInstance()->current_test_info()->name());
                        ASSERT_EQ(0, pnCounter->get_and_increment().get());
                        ASSERT_EQ(1, pnCounter->get_and_increment().get());
                        ASSERT_EQ(2, pnCounter->get().get());
                    }

                    TEST_F(BasicPnCounterAPITest, testIncrementAndGet) {
                        auto pnCounter = client->get_pn_counter(
                                testing::UnitTest::GetInstance()->current_test_info()->name());
                        ASSERT_EQ(1, pnCounter->increment_and_get().get());
                        ASSERT_EQ(1, pnCounter->get().get());
                    }

                    TEST_F(BasicPnCounterAPITest, testGetAndDecrementFromDefault) {
                        auto pnCounter = client->get_pn_counter(
                                testing::UnitTest::GetInstance()->current_test_info()->name());
                        ASSERT_EQ(0, pnCounter->get_and_decrement().get());
                        ASSERT_EQ(-1, pnCounter->get().get());
                    }

                    TEST_F(BasicPnCounterAPITest, testGetAndDecrement) {
                        auto pnCounter = client->get_pn_counter(
                                testing::UnitTest::GetInstance()->current_test_info()->name());
                        ASSERT_EQ(1, pnCounter->increment_and_get().get());
                        ASSERT_EQ(1, pnCounter->get_and_decrement().get());
                        ASSERT_EQ(0, pnCounter->get().get());
                    }

                    TEST_F(BasicPnCounterAPITest, testGetAndSubtract) {
                        auto pnCounter = client->get_pn_counter(
                                testing::UnitTest::GetInstance()->current_test_info()->name());
                        ASSERT_EQ(0, pnCounter->get_and_subtract(2).get());
                        ASSERT_EQ(-2, pnCounter->get().get());
                    }

                    TEST_F(BasicPnCounterAPITest, testSubtractAndGet) {
                        auto pnCounter = client->get_pn_counter(
                                testing::UnitTest::GetInstance()->current_test_info()->name());
                        ASSERT_EQ(-3, pnCounter->subtract_and_get(3).get());
                    }

                    TEST_F(BasicPnCounterAPITest, testReset) {
                        auto pnCounter = client->get_pn_counter(
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
                        std::shared_ptr<PNCounter> counter1 = client->get_pn_counter(name);
                        std::shared_ptr<PNCounter> counter2 = client->get_pn_counter(name);

                        ASSERT_EQ(5, counter1->add_and_get(5).get());

                        ASSERT_EQ_EVENTUALLY(5, counter1->get().get());
                        ASSERT_EQ_EVENTUALLY(5, counter2->get().get());
                    }

                    TEST_F(PnCounterFunctionalityTest, testParallelism) {
                        const char *name = testing::UnitTest::GetInstance()->current_test_info()->name();
                        std::shared_ptr<PNCounter> counter1 = client->get_pn_counter(name);
                        std::shared_ptr<PNCounter> counter2 = client->get_pn_counter(name);

                        int parallelism = 5;
                        int loopsPerThread = 100;
                        std::atomic<int64_t> finalValue(0);

                        std::vector<std::future<void>> futures;
                        for (int i = 0; i < parallelism; i++) {
                            futures.push_back(std::async([&]() {
                                for (int j = 0; j < loopsPerThread; j++) {
                                    counter1->add_and_get(5).get();
                                    finalValue += 5;
                                    counter2->add_and_get(-2).get();
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

                        HazelcastClient client(ClientConfig().set_cluster_name("lite-dev"));

                        auto pnCounter = client.get_pn_counter(
                                testing::UnitTest::GetInstance()->current_test_info()->name());

                        ASSERT_THROW(pnCounter->add_and_get(5).get(), exception::NoDataMemberInClusterException);
                    }

                    /**
                     * Client implementation for testing behaviour of {@link ConsistencyLostException}
                     */
                    class ClientPNCounterConsistencyLostTest : public ClientTestSupport {
                    protected:
                        boost::shared_ptr<Member> get_current_target_replica_address(
                                const std::shared_ptr<PNCounter> &pn_counter) {
                            return pn_counter->get_current_target_replica_address();
                        }

                        void
                        terminate_member(const Member &address, HazelcastServer &server1, HazelcastServer &server2) {
                            auto member1 = server1.get_member();
                            if (boost::to_string(address.get_uuid()) == member1.uuid) {
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

                        HazelcastClient client(ClientConfig().set_cluster_name("consistency-lost-dev"));

                        auto pnCounter = client.get_pn_counter(
                                testing::UnitTest::GetInstance()->current_test_info()->name());

                        pnCounter->add_and_get(5).get();

                        ASSERT_EQ(5, pnCounter->get().get());

                        auto currentTarget = get_current_target_replica_address(pnCounter);

                        terminate_member(*currentTarget, instance, instance2);

                        ASSERT_THROW(pnCounter->add_and_get(5).get(), exception::ConsistencyLostException);
                    }

                    TEST_F(ClientPNCounterConsistencyLostTest, driverCanContinueSessionByCallingReset) {
                        HazelcastServerFactory factory(
                                "hazelcast/test/resources/hazelcast-pncounter-consistency-lost-test.xml");
                        HazelcastServer instance(factory);
                        HazelcastServer instance2(factory);

                        HazelcastClient client(ClientConfig().set_cluster_name("consistency-lost-dev"));

                        auto pnCounter = client.get_pn_counter(
                                testing::UnitTest::GetInstance()->current_test_info()->name());

                        pnCounter->add_and_get(5).get();

                        ASSERT_EQ(5, pnCounter->get().get());

                        auto currentTarget = get_current_target_replica_address(pnCounter);

                        terminate_member(*currentTarget, instance, instance2);

                        pnCounter->reset().get();

                        pnCounter->add_and_get(5).get();
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
                MembershipListener make_membership_listener(boost::latch &joined, boost::latch &left) {
                    return MembershipListener()
                        .on_joined([&joined](const MembershipEvent &) {
                            joined.count_down();
                        })
                        .on_left([&left](const MembershipEvent &) {
                            left.count_down();
                        });
                }

                MembershipListener make_initial_membership_listener(boost::latch &joined, boost::latch &left) {
                    return MembershipListener()
                        .on_init([&joined](const InitialMembershipEvent &event) {
                            auto &members = event.get_members();
                            if (members.size() == 1) {
                                joined.count_down();
                            }
                        })
                        .on_joined([&joined](const MembershipEvent &) {
                            joined.count_down();
                        })
                        .on_left([&left](const MembershipEvent &) {
                            left.count_down();
                        });
                }
            };

            TEST_P(SimpleListenerTest, testSharedClusterListeners) {
                HazelcastServer instance(*g_srvFactory);
                HazelcastClient hazelcastClient(GetParam());
                Cluster cluster = hazelcastClient.get_cluster();
                boost::latch memberAdded(1);
                boost::latch memberAddedInit(2);
                boost::latch memberRemoved(1);
                boost::latch memberRemovedInit(1);

                auto init_listener = make_initial_membership_listener(memberAddedInit, memberRemovedInit);
                auto listener = make_membership_listener(memberAdded, memberRemoved);

                auto initialListenerRegistrationId = cluster.add_membership_listener(std::move(init_listener));
                auto sampleListenerRegistrationId = cluster.add_membership_listener(std::move(listener));

                HazelcastServer instance2(*g_srvFactory);

                ASSERT_OPEN_EVENTUALLY(memberAdded);
                ASSERT_OPEN_EVENTUALLY(memberAddedInit);

                instance2.shutdown();

                ASSERT_OPEN_EVENTUALLY(memberRemoved);
                ASSERT_OPEN_EVENTUALLY(memberRemovedInit);

                instance.shutdown();

                ASSERT_TRUE(cluster.remove_membership_listener(initialListenerRegistrationId));
                ASSERT_TRUE(cluster.remove_membership_listener(sampleListenerRegistrationId));
            }

            TEST_P(SimpleListenerTest, testClusterListeners) {
                HazelcastServer instance(*g_srvFactory);
                HazelcastClient hazelcastClient(GetParam());
                Cluster cluster = hazelcastClient.get_cluster();
                boost::latch memberAdded(1);
                boost::latch memberAddedInit(2);
                boost::latch memberRemoved(1);
                boost::latch memberRemovedInit(1);

                auto init_listener = make_initial_membership_listener(memberAddedInit, memberRemovedInit);
                auto listener = make_membership_listener(memberAdded, memberRemoved);

                auto init_id = cluster.add_membership_listener(std::move(init_listener));
                auto id = cluster.add_membership_listener(std::move(listener));

                HazelcastServer instance2(*g_srvFactory);

                ASSERT_OPEN_EVENTUALLY(memberAdded);
                ASSERT_OPEN_EVENTUALLY(memberAddedInit);

                instance2.shutdown();

                ASSERT_OPEN_EVENTUALLY(memberRemoved);
                ASSERT_OPEN_EVENTUALLY(memberRemovedInit);

                instance.shutdown();

                ASSERT_TRUE(cluster.remove_membership_listener(init_id));
                ASSERT_TRUE(cluster.remove_membership_listener(id));
            }

            TEST_P(SimpleListenerTest, testClusterListenersFromConfig) {
                boost::latch memberAdded(1);
                boost::latch memberAddedInit(2);
                boost::latch memberRemoved(1);
                boost::latch memberRemovedInit(1);
                auto init_listener = make_initial_membership_listener(memberAddedInit, memberRemovedInit);
                auto listener = make_membership_listener(memberAdded, memberRemoved);

                ClientConfig clientConfig = GetParam();
                clientConfig.add_listener(std::move(init_listener));
                clientConfig.add_listener(std::move(listener));

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

                auto map = hazelcastClient.get_map("testDeregisterListener");

                ASSERT_FALSE(map->remove_entry_listener(spi::ClientContext(hazelcastClient).random_uuid()).get());

                boost::latch map_clearedLatch(1);

                EntryListener listener;

                listener.on_map_cleared([&map_clearedLatch](MapEvent &&event) {
                    ASSERT_EQ("testDeregisterListener", event.get_name());
                    ASSERT_EQ(EntryEvent::type::CLEAR_ALL, event.get_event_type());
                    const std::string &hostName = event.get_member().get_address().get_host();
                    ASSERT_TRUE(hostName == "127.0.0.1" || hostName == "localhost");
                    ASSERT_EQ(5701, event.get_member().get_address().get_port());
                    ASSERT_EQ(1, event.get_number_of_entries_affected());
                    std::cout << "Map cleared event received:" << event << std::endl;
                    map_clearedLatch.count_down();
                });

                auto listenerRegistrationId = map->add_entry_listener(std::move(listener), true).get();
                map->put(1, 1).get();
                map->clear().get();
                ASSERT_OPEN_EVENTUALLY(map_clearedLatch);
                ASSERT_TRUE(map->remove_entry_listener(listenerRegistrationId).get());
            }

            TEST_P(SimpleListenerTest, testEmptyListener) {
                HazelcastServer instance(*g_srvFactory);
                HazelcastClient hazelcastClient(GetParam());

                auto map = hazelcastClient.get_map("testEmptyListener");

                // empty listener with no handlers
                EntryListener listener;

                auto listenerRegistrationId = map->add_entry_listener(std::move(listener), true).get();

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

                ASSERT_TRUE(map->remove_entry_listener(listenerRegistrationId).get());
            }

            INSTANTIATE_TEST_SUITE_P(All,
                                     SimpleListenerTest,
                                     ::testing::Values(ClientTestSupportBase::get_config(),
                                                       ClientTestSupportBase::get_config(false, false)));
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
                    flake_id_generator_ = client->get_flake_id_generator(testing::UnitTest::GetInstance()->current_test_info()->name());
                }

                static void SetUpTestCase() {
                    instance = new HazelcastServer(*g_srvFactory);
                    ClientConfig clientConfig = get_config();
                    config::ClientFlakeIdGeneratorConfig flakeIdConfig("test*");
                    flakeIdConfig.set_prefetch_count(10).set_prefetch_validity_duration(std::chrono::seconds(20));
                    clientConfig.add_flake_id_generator_config(flakeIdConfig);
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

                std::shared_ptr<FlakeIdGenerator> flake_id_generator_;
            };

            HazelcastServer *FlakeIdGeneratorApiTest::instance = nullptr;
            HazelcastClient *FlakeIdGeneratorApiTest::client = nullptr;

            TEST_F (FlakeIdGeneratorApiTest, testStartingValue) {
                ASSERT_NO_THROW(flake_id_generator_->new_id().get());
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
                            localIds.insert(flake_id_generator_->new_id().get());
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
                HazelcastServer instance_;
                ClientConfig client_config_;
                HazelcastClient client_;
            };

            ClientTxnMapTest::ClientTxnMapTest() : instance_(*g_srvFactory), client_(get_new_client()) {
            }

            ClientTxnMapTest::~ClientTxnMapTest() = default;

            TEST_F(ClientTxnMapTest, testPutGet) {
                std::string name = "testPutGet";

                TransactionContext context = client_.new_transaction_context();
                context.begin_transaction().get();

                auto map = context.get_map(name);

                ASSERT_FALSE((map->put<std::string, std::string>("key1", "value1").get().has_value()));
                ASSERT_EQ("value1", (map->get<std::string, std::string>("key1").get().value()));
                auto val = client_.get_map(name)->get<std::string, std::string>("key1").get();
                ASSERT_FALSE(val.has_value());

                context.commit_transaction().get();

                ASSERT_EQ("value1", (client_.get_map(name)->get<std::string, std::string>("key1").get().value()));
            }

            TEST_F(ClientTxnMapTest, testRemove) {
                std::string name = "testRemove";

                TransactionContext context = client_.new_transaction_context();
                context.begin_transaction().get();

                auto map = context.get_map(name);

                ASSERT_FALSE((map->put<std::string, std::string>("key1", "value1").get().has_value()));
                ASSERT_EQ("value1", (map->get<std::string, std::string>("key1").get().value()));
                auto val = client_.get_map(name)->get<std::string, std::string>("key1").get();
                ASSERT_FALSE(val.has_value());

                ASSERT_FALSE((map->remove<std::string, std::string>("key2").get().has_value()));
                val = map->remove<std::string, std::string>("key1").get();
                ASSERT_TRUE(val.has_value());
                ASSERT_EQ("value1", val.value());

                context.commit_transaction().get();

                auto regularMap = client_.get_map(name);
                ASSERT_TRUE(regularMap->is_empty().get());
            }

            TEST_F(ClientTxnMapTest, testRemoveIfSame) {
                std::string name = "testRemoveIfSame";

                TransactionContext context = client_.new_transaction_context();
                context.begin_transaction().get();

                auto map = context.get_map(name);

                ASSERT_FALSE((map->put<std::string, std::string>("key1", "value1").get().has_value()));
                ASSERT_EQ("value1", (map->get<std::string, std::string>("key1").get().value()));
                ASSERT_EQ("value1", (map->get<std::string, std::string>("key1").get().value()));
                auto val = client_.get_map(name)->get<std::string, std::string>("key1").get();
                ASSERT_FALSE(val.has_value());

                ASSERT_FALSE((map->remove<std::string, std::string>("key2").get().has_value()));;
                ASSERT_TRUE(map->remove("key1", "value1").get());

                context.commit_transaction().get();

                auto regularMap = client_.get_map(name);
                ASSERT_TRUE(regularMap->is_empty().get());
            }

            TEST_F(ClientTxnMapTest, testDeleteEntry) {
                std::string name = "testDeleteEntry";

                TransactionContext context = client_.new_transaction_context();
                context.begin_transaction().get();

                auto map = context.get_map(name);

                ASSERT_NO_THROW(map->delete_entry("key1").get());

                ASSERT_FALSE((map->put<std::string, std::string>("key1", "value1").get().has_value()));
                ASSERT_EQ("value1", (map->get<std::string, std::string>("key1").get().value()));
                auto val = client_.get_map(name)->get<std::string, std::string>("key1").get();
                ASSERT_FALSE(val.has_value());

                ASSERT_NO_THROW(map->delete_entry("key1").get());
                val = map->get<std::string, std::string>("key1").get();
                ASSERT_FALSE(val.has_value());

                context.commit_transaction().get();

                auto regularMap = client_.get_map(name);
                ASSERT_TRUE(regularMap->is_empty().get());
            }

            TEST_F(ClientTxnMapTest, testReplace) {
                std::string name = "testReplace";

                TransactionContext context = client_.new_transaction_context();
                context.begin_transaction().get();

                auto map = context.get_map(name);

                ASSERT_FALSE((map->put<std::string, std::string>("key1", "value1").get().has_value()));
                ASSERT_EQ("value1", (map->get<std::string, std::string>("key1").get().value()));
                auto val = client_.get_map(name)->get<std::string, std::string>("key1").get();
                ASSERT_FALSE(val.has_value());

                ASSERT_EQ("value1", (map->replace<std::string, std::string>("key1", "myNewValue").get().value()));

                context.commit_transaction().get();

                ASSERT_EQ("myNewValue", (client_.get_map(name)->get<std::string, std::string>("key1").get().value()));
            }

            TEST_F(ClientTxnMapTest, testSet) {
                std::string name = "testSet";

                TransactionContext context = client_.new_transaction_context();
                context.begin_transaction().get();

                auto map = context.get_map(name);

                ASSERT_NO_THROW(map->set("key1", "value1").get());

                auto val = map->get<std::string, std::string>("key1").get();
                ASSERT_TRUE(val.has_value());
                ASSERT_EQ("value1", val.value());

                val = client_.get_map(name)->get<std::string, std::string>("key1").get();
                ASSERT_FALSE(val.has_value());

                ASSERT_NO_THROW(map->set("key1", "myNewValue").get());

                val = map->get<std::string, std::string>("key1").get();
                ASSERT_TRUE(val.has_value());
                ASSERT_EQ("myNewValue", val.value());

                context.commit_transaction().get();

                val = client_.get_map(name)->get<std::string, std::string>("key1").get();
                ASSERT_TRUE(val.has_value());
                ASSERT_EQ("myNewValue", val.value());
            }

            TEST_F(ClientTxnMapTest, testContains) {
                std::string name = "testContains";

                TransactionContext context = client_.new_transaction_context();
                context.begin_transaction().get();

                auto map = context.get_map(name);

                ASSERT_FALSE(map->contains_key("key1").get());

                ASSERT_NO_THROW(map->set("key1", "value1").get());

                auto val = map->get<std::string, std::string>("key1").get();
                ASSERT_TRUE(val.has_value());
                ASSERT_EQ("value1", val.value());

                ASSERT_TRUE(map->contains_key("key1").get());

                context.commit_transaction().get();

                auto regularMap = client_.get_map(name);
                ASSERT_TRUE(regularMap->contains_key("key1").get());
            }

            TEST_F(ClientTxnMapTest, testReplaceIfSame) {
                std::string name = "testReplaceIfSame";

                TransactionContext context = client_.new_transaction_context();
                context.begin_transaction().get();

                auto map = context.get_map(name);

                ASSERT_FALSE((map->put<std::string, std::string>("key1", "value1").get().has_value()));
                ASSERT_EQ("value1", (map->get<std::string, std::string>("key1").get().value()));
                auto val = client_.get_map(name)->get<std::string, std::string>("key1").get();
                ASSERT_FALSE(val.has_value());

                ASSERT_FALSE(map->replace("key1", "valueNonExistent", "myNewValue").get());
                ASSERT_TRUE(map->replace("key1", "value1", "myNewValue").get());

                context.commit_transaction().get();

                ASSERT_EQ("myNewValue", (client_.get_map(name)->get<std::string, std::string>("key1").get().value()));
            }

            TEST_F(ClientTxnMapTest, testPutIfSame) {
                std::string name = "testPutIfSame";

                TransactionContext context = client_.new_transaction_context();
                context.begin_transaction().get();

                auto map = context.get_map(name);

                auto val = map->put_if_absent<std::string, std::string>("key1", "value1").get();
                ASSERT_FALSE(val.has_value());
                val = map->get<std::string, std::string>("key1").get();
                ASSERT_TRUE(val.has_value());
                ASSERT_EQ("value1", val.value());
                val = client_.get_map(name)->get<std::string, std::string>("key1").get();
                ASSERT_FALSE(val.has_value());

                val = map->put_if_absent<std::string, std::string>("key1", "value1").get();
                ASSERT_TRUE(val.has_value());
                ASSERT_EQ("value1", val.value());

                context.commit_transaction().get();

                val = client_.get_map(name)->get<std::string, std::string>("key1").get();
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
//                        pass.set(map->try_put("var", 1, 0, TimeUnit.SECONDS) == false);
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
//            ASSERT_TRUE(map->try_put("var", 1, 0, TimeUnit.SECONDS));
//        }

            TEST_F(ClientTxnMapTest, testKeySetValues) {
                std::string name = "testKeySetValues";
                auto map = client_.get_map(name);
                map->put<std::string, std::string>("key1", "value1").get();
                map->put<std::string, std::string>("key2", "value2").get();

                TransactionContext context = client_.new_transaction_context();
                context.begin_transaction().get();
                auto txMap = context.get_map(name);
                ASSERT_FALSE((txMap->put<std::string, std::string>("key3", "value3").get().has_value()));


                ASSERT_EQ(3, (int) txMap->size().get());
                ASSERT_EQ(3, (int) txMap->key_set<std::string>().get().size());
                ASSERT_EQ(3, (int) txMap->values<std::string>().get().size());
                context.commit_transaction().get();

                ASSERT_EQ(3, (int) map->size().get());
                ASSERT_EQ(3, (int) map->key_set<std::string>().get().size());
                ASSERT_EQ(3, (int) map->values<std::string>().get().size());

            }

            TEST_F(ClientTxnMapTest, testKeySetAndValuesWithPredicates) {
                std::string name = "testKeysetAndValuesWithPredicates";
                auto map = client_.get_map(name);

                Employee emp1("abc-123-xvz", 34);
                Employee emp2("abc-123-xvz", 20);

                map->put<Employee, Employee>(emp1, emp1).get();

                TransactionContext context = client_.new_transaction_context();
                context.begin_transaction().get();

                auto txMap = context.get_map(name);
                ASSERT_FALSE((txMap->put<Employee, Employee>(emp2, emp2).get().has_value()));

                ASSERT_EQ(2, (int) txMap->size().get());
                ASSERT_EQ(2, (int) txMap->key_set<Employee>().get().size());
                query::SqlPredicate predicate(client_, "a = 10");
                ASSERT_EQ(0, (int) txMap->key_set<Employee>(predicate).get().size());
                ASSERT_EQ(0, (int) txMap->values<Employee>(predicate).get().size());
                query::SqlPredicate predicate2(client_, "a >= 10");
                ASSERT_EQ(2, (int) txMap->key_set<Employee>(predicate2).get().size());
                ASSERT_EQ(2, (int) txMap->values<Employee>(predicate2).get().size());

                context.commit_transaction().get();

                ASSERT_EQ(2, (int) map->size().get());
                ASSERT_EQ(2, (int) map->values<Employee>().get().size());
            }

            TEST_F(ClientTxnMapTest, testIsEmpty) {
                std::string name = "testIsEmpty";

                TransactionContext context = client_.new_transaction_context();
                context.begin_transaction().get();

                auto map = context.get_map(name);

                ASSERT_TRUE(map->is_empty().get());

                auto oldValue = map->put<std::string, std::string>("key1", "value1").get();
                ASSERT_FALSE(oldValue.has_value()) << "old value should not exist";

                ASSERT_FALSE(map->is_empty().get());

                context.commit_transaction().get();

                auto regularMap = client_.get_map(name);
                ASSERT_FALSE(regularMap->is_empty().get());
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
                HazelcastServer instance_;
                HazelcastClient client_;
            };

            ClientTxnSetTest::ClientTxnSetTest() : instance_(*g_srvFactory), client_(get_new_client()) {
            }

            ClientTxnSetTest::~ClientTxnSetTest() = default;

            TEST_F(ClientTxnSetTest, testAddRemove) {
                auto s = client_.get_set("testAddRemove");
                s->add<std::string>("item1").get();

                TransactionContext context = client_.new_transaction_context();
                context.begin_transaction().get();
                auto set = context.get_set("testAddRemove");
                ASSERT_TRUE(set->add<std::string>("item2").get());
                ASSERT_EQ(2, set->size().get());
                ASSERT_EQ(1, s->size().get());
                ASSERT_FALSE(set->remove("item3").get());
                ASSERT_TRUE(set->remove("item1").get());

                context.commit_transaction().get();

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
                HazelcastServerFactory & hazelcast_instance_factory_;
                std::unique_ptr<HazelcastServer> server_;
                std::unique_ptr<HazelcastServer> second_;
                std::unique_ptr<HazelcastClient> client_;
                std::unique_ptr<LoadBalancer> load_balancer_;
            };

            class MyLoadBalancer : public impl::AbstractLoadBalancer {
            public:
                boost::optional<Member> next() override {
                    std::vector<Member> members = get_members();
                    size_t len = members.size();
                    if (len == 0) {
                        return boost::none;
                    }
                    for (size_t i = 0; i < len; i++) {
                        if (members[i].get_address().get_port() == 5701) {
                            return members[i];
                        }
                    }
                    return members[0];
                }

            };

            MembershipListener make_member_removed_listener(boost::latch &l) {
                return MembershipListener()
                    .on_left([&l](const MembershipEvent &){
                        l.count_down();
                    });
            }

            ClientTxnTest::ClientTxnTest()
                    : hazelcast_instance_factory_(*g_srvFactory) {
                server_.reset(new HazelcastServer(hazelcast_instance_factory_));
                ClientConfig clientConfig = get_config();
                //always start the txn on first member
                load_balancer_.reset(new MyLoadBalancer());
                clientConfig.set_load_balancer(load_balancer_.get());
                client_.reset(new HazelcastClient(clientConfig));
                second_.reset(new HazelcastServer(hazelcast_instance_factory_));
            }

            ClientTxnTest::~ClientTxnTest() {
                client_->shutdown();
                server_->shutdown();
                second_->shutdown();
            }

            TEST_F(ClientTxnTest, testTxnConnectAfterClientShutdown) {
                client_->shutdown();
                ASSERT_THROW(client_->new_transaction_context(), exception::HazelcastClientNotActiveException);
            }

            TEST_F(ClientTxnTest, testTxnCommitAfterClusterShutdown) {
                TransactionContext context = client_->new_transaction_context();
                context.begin_transaction().get();

                server_->shutdown();
                second_->shutdown();

                ASSERT_THROW(context.commit_transaction().get(), exception::TransactionException);
            }

            TEST_F(ClientTxnTest, testTxnCommit) {
                std::string queueName = random_string();
                TransactionContext context = client_->new_transaction_context();
                context.begin_transaction().get();
                ASSERT_FALSE(context.get_txn_id().is_nil());
                auto queue = context.get_queue(queueName);
                std::string value = random_string();
                queue->offer(value).get();

                context.commit_transaction().get();

                auto q = client_->get_queue(queueName);
                auto  retrievedElement = q->poll<std::string>().get();
                ASSERT_TRUE(retrievedElement.has_value());
                ASSERT_EQ(value, retrievedElement.value());
            }

            TEST_F(ClientTxnTest, testTxnCommitUniSocket) {
                ClientConfig clientConfig;
                clientConfig.get_network_config().set_smart_routing(false);
                HazelcastClient uniSocketClient(clientConfig);

                std::string queueName = random_string();
                TransactionContext context = uniSocketClient.new_transaction_context();
                context.begin_transaction().get();
                ASSERT_FALSE(context.get_txn_id().is_nil());
                auto queue = context.get_queue(queueName);
                std::string value = random_string();
                queue->offer(value).get();

                context.commit_transaction().get();

                auto q = uniSocketClient.get_queue(queueName);
                auto  retrievedElement = q->poll<std::string>().get();
                ASSERT_TRUE(retrievedElement.has_value());
                ASSERT_EQ(value, retrievedElement.value());
            }

            TEST_F(ClientTxnTest, testTxnCommitWithOptions) {
                std::string queueName = random_string();
                TransactionOptions transactionOptions;
                transactionOptions.set_transaction_type(TransactionOptions::transaction_type::TWO_PHASE);
                transactionOptions.set_timeout(std::chrono::seconds(60));
                transactionOptions.set_durability(2);
                TransactionContext context = client_->new_transaction_context(transactionOptions);

                context.begin_transaction().get();
                ASSERT_FALSE(context.get_txn_id().is_nil());
                auto queue = context.get_queue(queueName);
                std::string value = random_string();
                queue->offer(value).get();

                context.commit_transaction().get();

                auto q = client_->get_queue(queueName);
                auto  retrievedElement = q->poll<std::string>().get();
                ASSERT_TRUE(retrievedElement.has_value());
                ASSERT_EQ(value, retrievedElement.value());
            }

            TEST_F(ClientTxnTest, testTxnCommitAfterClientShutdown) {
                std::string queueName = random_string();
                TransactionContext context = client_->new_transaction_context();
                context.begin_transaction().get();
                auto queue = context.get_queue(queueName);
                std::string value = random_string();
                queue->offer(value).get();

                client_->shutdown();

                ASSERT_THROW(context.commit_transaction().get(), exception::TransactionException);
            }


            TEST_F(ClientTxnTest, testTxnRollback) {
                std::string queueName = random_string();
                TransactionContext context = client_->new_transaction_context();
                boost::latch txnRollbackLatch(1);
                boost::latch memberRemovedLatch(1);
                auto listener = make_member_removed_listener(memberRemovedLatch);
                client_->get_cluster().add_membership_listener(std::move(listener));

                try {
                    context.begin_transaction().get();
                    ASSERT_FALSE(context.get_txn_id().is_nil());
                    auto queue = context.get_queue(queueName);
                    queue->offer(random_string()).get();

                    server_->shutdown();

                    context.commit_transaction().get();
                    FAIL();
                } catch (exception::TransactionException &) {
                    context.rollback_transaction().get();
                    txnRollbackLatch.count_down();
                }

                ASSERT_OPEN_EVENTUALLY(txnRollbackLatch);
                ASSERT_OPEN_EVENTUALLY(memberRemovedLatch);

                auto q = client_->get_queue(queueName);
                ASSERT_FALSE(q->poll<std::string>().get().has_value())
                                            << "Poll result should be null since it is rolled back";
                ASSERT_EQ(0, q->size().get());
            }

            TEST_F(ClientTxnTest, testTxnRollbackOnServerCrash) {
                std::string queueName = random_string();
                TransactionContext context = client_->new_transaction_context();
                boost::latch txnRollbackLatch(1);
                boost::latch memberRemovedLatch(1);

                context.begin_transaction().get();

                auto queue = context.get_queue(queueName);
                queue->offer("str").get();

                auto listener = make_member_removed_listener(memberRemovedLatch);
                client_->get_cluster().add_membership_listener(std::move(listener));

                server_->shutdown();

                ASSERT_THROW(context.commit_transaction().get(), exception::TransactionException);

                context.rollback_transaction().get();
                txnRollbackLatch.count_down();

                ASSERT_OPEN_EVENTUALLY(txnRollbackLatch);
                ASSERT_OPEN_EVENTUALLY(memberRemovedLatch);

                auto q = client_->get_queue(queueName);
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
                HazelcastServer instance_;
                ClientConfig client_config_;
                HazelcastClient client_;
            };

            ClientTxnListTest::ClientTxnListTest() : instance_(*g_srvFactory), client_(get_new_client()) {}

            ClientTxnListTest::~ClientTxnListTest() = default;

            TEST_F(ClientTxnListTest, testAddRemove) {
                auto l = client_.get_list("testAddRemove");
                l->add<std::string>("item1").get();

                TransactionContext context = client_.new_transaction_context();
                context.begin_transaction().get();
                auto list = context.get_list("testAddRemove");
                ASSERT_TRUE(list->add<std::string>("item2").get());
                ASSERT_EQ(2, list->size().get());
                ASSERT_EQ(1, l->size().get());
                ASSERT_FALSE(list->remove("item3").get());
                ASSERT_TRUE(list->remove("item1").get());

                context.commit_transaction().get();

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
                HazelcastServer instance_;
                HazelcastClient client_;
            };

            ClientTxnMultiMapTest::ClientTxnMultiMapTest()
                    : instance_(*g_srvFactory), client_(get_new_client()) {}

            ClientTxnMultiMapTest::~ClientTxnMultiMapTest() = default;

            TEST_F(ClientTxnMultiMapTest, testRemoveIfExists) {
                TransactionContext context = client_.new_transaction_context();
                context.begin_transaction().get();
                auto multiMap = context.get_multi_map("testRemoveIfExists");
                std::string key("MyKey");
                ASSERT_TRUE(multiMap->put(key, "value").get());
                ASSERT_TRUE(multiMap->put(key, "value1").get());
                ASSERT_TRUE(multiMap->put(key, "value2").get());
                ASSERT_EQ(3, (int) (multiMap->get<std::string, std::string>(key).get().size()));

                ASSERT_FALSE(multiMap->remove(key, "NonExistentValue").get());
                ASSERT_TRUE(multiMap->remove(key, "value1").get());

                ASSERT_EQ(2, multiMap->size().get());
                ASSERT_EQ(2, (int) (multiMap->value_count<std::string>(key).get()));

                context.commit_transaction().get();

                auto mm = client_.get_multi_map("testRemoveIfExists");
                ASSERT_EQ(2, (int) (mm->get<std::string, std::string>(key).get().size()));
            }

            TEST_F(ClientTxnMultiMapTest, testPutGetRemove) {
                auto mm = client_.get_multi_map("testPutGetRemove");
                constexpr int n = 10;

                std::array<boost::future<void>, n> futures;
                for (int i = 0; i < n; i++) {
                    futures[i] = boost::async(std::packaged_task<void()>([&]() {
                        std::string key = std::to_string(hazelcast::util::get_current_thread_id());
                        std::string key2 = key + "2";
                        client_.get_multi_map("testPutGetRemove")->put(key, "value").get();
                        TransactionContext context = client_.new_transaction_context();
                        context.begin_transaction().get();
                        auto multiMap = context.get_multi_map("testPutGetRemove");
                        ASSERT_FALSE(multiMap->put(key, "value").get());
                        ASSERT_TRUE(multiMap->put(key, "value1").get());
                        ASSERT_TRUE(multiMap->put(key, "value2").get());
                        ASSERT_TRUE(multiMap->put(key2, "value21").get());
                        ASSERT_TRUE(multiMap->put(key2, "value22").get());
                        ASSERT_EQ(3, (int) (multiMap->get<std::string, std::string>(key).get().size()));
                        ASSERT_EQ(3, (int) (multiMap->value_count<std::string>(key).get()));
                        auto removedValues = multiMap->remove<std::string, std::string>(key2).get();
                        ASSERT_EQ(2U, removedValues.size());
                        ASSERT_TRUE((removedValues[0] == "value21" && removedValues[1] == "value22") ||
                                    (removedValues[1] == "value21" && removedValues[0] == "value22"));
                        context.commit_transaction().get();

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
                HazelcastServer instance_;
                HazelcastClient client_;
            };

            ClientTxnQueueTest::ClientTxnQueueTest() : instance_(*g_srvFactory), client_(get_new_client()) {}

            ClientTxnQueueTest::~ClientTxnQueueTest() = default;

            TEST_F(ClientTxnQueueTest, testTransactionalOfferPoll1) {
                std::string name = "defQueue";

                TransactionContext context = client_.new_transaction_context();
                context.begin_transaction().get();
                auto q = context.get_queue(name);
                ASSERT_TRUE(q->offer("ali").get());
                ASSERT_EQ(1, q->size().get());
                ASSERT_EQ("ali", q->poll<std::string>().get().value());
                ASSERT_EQ(0, q->size().get());
                context.commit_transaction().get();
                ASSERT_EQ(0, client_.get_queue(name)->size().get());
            }

            TEST_F(ClientTxnQueueTest, testTransactionalOfferPollByteVector) {
                std::string name = "defQueue";

                TransactionContext context = client_.new_transaction_context();
                context.begin_transaction().get();
                auto q = context.get_queue(name);
                std::vector<byte> value(3);
                ASSERT_TRUE(q->offer(value).get());
                ASSERT_EQ(1, q->size().get());
                ASSERT_EQ(value, q->poll<std::vector<byte>>().get().value());
                ASSERT_EQ(0, q->size().get());
                context.commit_transaction().get();
                ASSERT_EQ(0, client_.get_queue(name)->size().get());
            }

            void test_transactional_offer_poll2_thread(hazelcast::util::ThreadArgs &args) {
                boost::latch *latch1 = (boost::latch *) args.arg0;
                HazelcastClient *client = (HazelcastClient *) args.arg1;
                latch1->wait();
                client->get_queue("defQueue0")->offer("item0").get();
            }

            TEST_F(ClientTxnQueueTest, testTransactionalOfferPoll2) {
                boost::latch latch1(1);
                hazelcast::util::StartedThread t(test_transactional_offer_poll2_thread, &latch1, &client_);
                TransactionContext context = client_.new_transaction_context();
                context.begin_transaction().get();
                auto q0 = context.get_queue("defQueue0");
                auto q1 = context.get_queue("defQueue1");
                boost::optional<std::string> s;
                latch1.count_down();
                s = q0->poll<std::string>(std::chrono::seconds(10)).get();
                ASSERT_EQ("item0", s.value());
                ASSERT_TRUE(q1->offer(s.value()).get());

                ASSERT_NO_THROW(context.commit_transaction().get());

                ASSERT_EQ(0, client_.get_queue("defQueue0")->size().get());
                ASSERT_EQ("item0", client_.get_queue("defQueue1")->poll<std::string>().get().value());
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
                hazelcast::util::AddressHolder addressHolder = hazelcast::util::AddressUtil::get_address_holder(
                        "[fe80::62c5:*:fe05:480a%en0]:8080");
                ASSERT_EQ("fe80::62c5:*:fe05:480a", addressHolder.get_address());
                ASSERT_EQ(8080, addressHolder.get_port());
                ASSERT_EQ("en0", addressHolder.get_scope_id());

                addressHolder = hazelcast::util::AddressUtil::get_address_holder("[::ffff:192.0.2.128]:5700");
                ASSERT_EQ("::ffff:192.0.2.128", addressHolder.get_address());
                ASSERT_EQ(5700, addressHolder.get_port());

                addressHolder = hazelcast::util::AddressUtil::get_address_holder("192.168.1.1:5700");
                ASSERT_EQ("192.168.1.1", addressHolder.get_address());
                ASSERT_EQ(5700, addressHolder.get_port());

                addressHolder = hazelcast::util::AddressUtil::get_address_holder("hazelcast.com:80");
                ASSERT_EQ("hazelcast.com", addressHolder.get_address());
                ASSERT_EQ(80, addressHolder.get_port());
            }

            TEST_F(AddressUtilTest, testGetByNameIpV4) {
                std::string addrString("127.0.0.1");
                boost::asio::ip::address address = hazelcast::util::AddressUtil::get_by_name(addrString);
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
                        hazelcast::util::Bits::native_to_little_endian2(&expected, &actual);
                        ASSERT_EQ(0xBC, resBytes[0]);
                        ASSERT_EQ(0x7A, resBytes[1]);
                    }

                    {
                        int32_t expected = 0x1A * (int32_t) threeBytesFactor +
                                           0x9A * (int32_t) twoBytesFactor + 0xAA * (int32_t) oneByteFactor + 0xBA;
                        int32_t actual;
                        byte *resBytes = (byte *) &actual;
                        hazelcast::util::Bits::native_to_little_endian4(&expected, &actual);
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
                        hazelcast::util::Bits::native_to_little_endian8(&expected, &actual);
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
                        hazelcast::util::Bits::little_endian_to_native2(&source, &actual);
                        ASSERT_EQ(0xBC * oneByteFactor + 0xAB, actual);
                    }

                    {
                        byte source[4] = {0xAB, 0xBC, 0xDE, 0xA1};
                        uint32_t actual;
                        hazelcast::util::Bits::little_endian_to_native4(&source, &actual);
                        ASSERT_EQ(0xA1 * threeBytesFactor +
                                  0xDE * twoBytesFactor +
                                  0xBC * oneByteFactor +
                                  0xAB, actual);
                    }

                    {
                        byte source[8] = {0xAB, 0x9B, 0x8B, 0x7B, 0x6B, 0x5B, 0x4B, 0xA1};
                        uint64_t actual;
                        hazelcast::util::Bits::little_endian_to_native8(&source, &actual);
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
                    ASSERT_EQ(bytes, dataOutput.to_byte_array());
                }

                TEST_F(DataOutputTest, testWriteBoolean) {
                    std::vector<byte> bytes{0x00, 0x01};
                    serialization::pimpl::DataOutput dataOutput;
                    dataOutput.write<bool>(false);
                    dataOutput.write<bool>(true);
                    ASSERT_EQ(bytes, dataOutput.to_byte_array());
                }

                TEST_F(DataOutputTest, testWriteChar) {
                    std::vector<byte> bytes;
                    bytes.push_back(0);
                    bytes.push_back('b');
                    serialization::pimpl::DataOutput dataOutput;
                    dataOutput.write<char>('b');
                    ASSERT_EQ(bytes, dataOutput.to_byte_array());
                }

                TEST_F(DataOutputTest, testWriteShort) {
                    std::vector<byte> bytes{0x12, 0x34};
                    serialization::pimpl::DataOutput dataOutput;
                    dataOutput.write<int16_t>(0x1234);
                    ASSERT_EQ(bytes, dataOutput.to_byte_array());
                }

                TEST_F(DataOutputTest, testWriteInteger) {
                    std::vector<byte> bytes{0x12, 0x34, 0x56, 0x78};
                    serialization::pimpl::DataOutput dataOutput;
                    dataOutput.write<int32_t>(INT32_C(0x12345678));
                    ASSERT_EQ(bytes, dataOutput.to_byte_array());
                }

                TEST_F(DataOutputTest, testWriteLong) {
                    std::vector<byte> bytes{0x12, 0x34, 0x56, 0x78, 0x90, 0x9A, 0x9B, 0x9C};
                    serialization::pimpl::DataOutput dataOutput;
                    dataOutput.write<int64_t>(INT64_C(0x12345678909A9B9C));
                    ASSERT_EQ(bytes, dataOutput.to_byte_array());
                }

                TEST_F(DataOutputTest, testWriteUTF) {
                    std::vector<byte> bytes{0x00, 0x00, 0x00, 0x04, 'b', 'd', 'f', 'h'};
                    serialization::pimpl::DataOutput dataOutput;
                    std::string value("bdfh");
                    dataOutput.write<std::string>(&value);
                    ASSERT_EQ(bytes, dataOutput.to_byte_array());
                }

                TEST_F(DataOutputTest, testWriteByteArray) {
                    std::vector<byte> bytes{0x00, 0x00, 0x00, 0x02};
                    std::vector<byte> actualDataBytes{0x12, 0x34};
                    bytes.insert(bytes.end(), actualDataBytes.begin(), actualDataBytes.end());
                    serialization::pimpl::DataOutput dataOutput;
                    dataOutput.write(&actualDataBytes);
                    ASSERT_EQ(bytes, dataOutput.to_byte_array());
                }

                TEST_F(DataOutputTest, testWriteBooleanArray) {
                    std::vector<byte> bytes{0x00, 0x00, 0x00, 0x02, 0x00, 0x01};
                    std::vector<bool> actualValues;
                    actualValues.push_back(false);
                    actualValues.push_back(true);
                    serialization::pimpl::DataOutput dataOutput;
                    dataOutput.write<std::vector<bool>>(&actualValues);
                    ASSERT_EQ(bytes, dataOutput.to_byte_array());
                }

                TEST_F(DataOutputTest, testWriteCharArray) {
                    std::vector<byte> bytes{0x00, 0x00, 0x00, 0x02, 0, 'f', 0, 'h'};
                    std::vector<char> actualChars{'f', 'h'};
                    serialization::pimpl::DataOutput dataOutput;
                    dataOutput.write<std::vector<char>>(actualChars);
                    ASSERT_EQ(bytes, dataOutput.to_byte_array());
                }

                TEST_F(DataOutputTest, testWriteShortArray) {
                    std::vector<byte> bytes{0x00, 0x00, 0x00, 0x02, 0x12, 0x34, 0x56, 0x78};
                    std::vector<int16_t> actualValues{0x1234, 0x5678};
                    serialization::pimpl::DataOutput dataOutput;
                    dataOutput.write<std::vector<int16_t>>(&actualValues);
                    ASSERT_EQ(bytes, dataOutput.to_byte_array());
                }

                TEST_F(DataOutputTest, testWriteIntegerArray) {
                    std::vector<byte> bytes{0x00, 0x00, 0x00, 0x02, 0x12, 0x34, 0x56, 0x78, 0x1A, 0xBC, 0xDE, 0xEF};
                    std::vector<int32_t> actualValues{INT32_C(0x12345678), INT32_C(0x1ABCDEEF)};
                    serialization::pimpl::DataOutput dataOutput;
                    dataOutput.write<std::vector<int32_t>>(&actualValues);
                    ASSERT_EQ(bytes, dataOutput.to_byte_array());
                }

                TEST_F(DataOutputTest, testWriteLongArray) {
                    std::vector<byte> bytes = {0x00, 0x00, 0x00, 0x02, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xEF,
                                               0x01, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8};
                    std::vector<int64_t> actualValues{INT64_C(0x123456789ABCDEEF), INT64_C(0x01A2A3A4A5A6A7A8)};
                    serialization::pimpl::DataOutput dataOutput;
                    dataOutput.write<std::vector<int64_t>>(&actualValues);
                    ASSERT_EQ(bytes, dataOutput.to_byte_array());
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
                ASSERT_THROW(httpsClient.open_connection(), client::exception::IOException);
            }

            TEST_F(HttpsClientTest, testConnectToGithub) {
                SyncHttpsClient httpsClient("ec2.us-east-1.amazonaws.com",
                                            "/?Action=DescribeInstances&Version=2014-06-15&X-Amz-Algorithm=AWS4-HMAC-SHA256&X-Amz-Credential=AKIAIU5IAVNR6X75ARYQ%2F20170413%2Fus-east-1%2Fec2%2Faws4_request&X-Amz-Date=20170413T083821Z&X-Amz-Expires=30&X-Amz-Signature=dff261333170c81ecb21f3a0d5820147233197a32c&X-Amz-SignedHeaders=host");
                try {
                    httpsClient.open_connection();
                } catch (exception::IException &e) {
                    const std::string &msg = e.get_message();
                    ASSERT_NE(std::string::npos, msg.find("status: 401"));
                }
                ASSERT_THROW(httpsClient.open_connection(), exception::IOException);
            }
        }
    }
}

#endif // HZ_BUILD_WITH_SSL

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
