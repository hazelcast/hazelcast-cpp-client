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
#include "ClientTestSupportBase.h"
#include <hazelcast/client/client_config.h>
#include <hazelcast/client/hazelcast_client.h>
#include <hazelcast/client/serialization/serialization.h>
#include <hazelcast/client/impl/Partition.h>
#include <gtest/gtest.h>
#include <hazelcast/client/connection/ClientConnectionManagerImpl.h>
#include <hazelcast/client/connection/Connection.h>
#include <hazelcast/client/serialization/pimpl/data_input.h>
#include <hazelcast/util/AddressHelper.h>
#include <hazelcast/util/Util.h>
#include <TestHelperFunctions.h>
#include <ostream>
#include <ctime>
#include <errno.h>
#include <hazelcast/client/lifecycle_listener.h>
#include "hazelcast/client/lifecycle_event.h"
#include "serialization/Serializables.h"
#include <hazelcast/client/serialization_config.h>
#include <hazelcast/client/hazelcast_json_value.h>
#include <hazelcast/client/internal/nearcache/impl/NearCacheRecordStore.h>
#include <hazelcast/client/internal/nearcache/impl/store/NearCacheDataRecordStore.h>
#include <hazelcast/client/internal/nearcache/impl/store/NearCacheObjectRecordStore.h>
#include <cmath>
#include <iostream>
#include <string>
#include <atomic>
#include <stdlib.h>
#include <fstream>
#include <boost/asio.hpp>
#include <boost/thread/barrier.hpp>

#ifdef HZ_BUILD_WITH_SSL
#include <openssl/crypto.h>
#endif

#include "hazelcast/client/exception/protocol_exceptions.h"
#include "hazelcast/client/internal/socket/SSLSocket.h"
#include "hazelcast/client/initial_membership_event.h"
#include "hazelcast/client/socket_interceptor.h"
#include "hazelcast/client/imap.h"
#include "hazelcast/util/Bits.h"
#include "hazelcast/util/SyncHttpsClient.h"
#include "hazelcast/util/BlockingConcurrentQueue.h"
#include "hazelcast/util/ConcurrentQueue.h"
#include "hazelcast/util/concurrent/locks/LockSupport.h"
#include "hazelcast/client/pipelining.h"
#include "hazelcast/util/MurmurHash3.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4996) //for unsafe getenv
#endif

namespace hazelcast {
    namespace client {
        namespace test {
            class AddressHelperTest : public ClientTestSupport {
            };

            TEST_F(AddressHelperTest, testGetPossibleSocketAddresses) {
                std::string addr("10.2.3.1");
                std::vector<address> addresses = util::AddressHelper::get_socket_addresses(addr, get_logger());
                ASSERT_EQ(3U, addresses.size());
                std::unordered_set<address> socketAddresses;
                socketAddresses.insert(addresses.begin(), addresses.end());
                ASSERT_NE(socketAddresses.end(), socketAddresses.find(address(addr, 5701)));
                ASSERT_NE(socketAddresses.end(), socketAddresses.find(address(addr, 5702)));
                ASSERT_NE(socketAddresses.end(), socketAddresses.find(address(addr, 5703)));
            }

            TEST_F(AddressHelperTest, testAddressHolder) {
                util::AddressHolder holder("127.0.0.1", "en0", 8000);
                ASSERT_EQ("127.0.0.1", holder.get_address());
                ASSERT_EQ(8000, holder.get_port());
                ASSERT_EQ("en0", holder.get_scope_id());
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace test {
            namespace util {
                class ExceptionTest : public ::testing::Test {
                protected:
                };

                TEST_F(ExceptionTest, testExceptionDetail) {
                    std::string details("A lot of details");
                    exception::target_disconnected targetDisconnectedException("testExceptionCause",
                                                                                       "test message", details);


                    ASSERT_EQ(details, targetDisconnectedException.get_details());
                }

                TEST_F(ExceptionTest, testExceptionStreaming) {
                    std::string source("testException");
                    std::string originalMessage("original message");
                    exception::io e(source, originalMessage);

                    ASSERT_EQ(source, e.get_source());
                    ASSERT_EQ(originalMessage, e.get_message());

                    std::string extendedMessage(" this is an extension message");
                    int messageNumber = 1;
                    exception::io ioException = (exception::exception_builder<exception::io>(source)
                            << originalMessage << extendedMessage << messageNumber).build();

                    ASSERT_EQ(source, ioException.get_source());
                    ASSERT_EQ(
                            originalMessage + extendedMessage + std::to_string(messageNumber),
                            ioException.get_message());
                }
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace test {
            namespace util {
                class BlockingConcurrentQueueTest : public ::testing::Test
                {
                protected:
                    static void push_delayed(hazelcast::util::ThreadArgs &args) {
                        hazelcast::util::BlockingConcurrentQueue<int> *q = (hazelcast::util::BlockingConcurrentQueue<int> *)args.arg0;
                        int *val = (int *)args.arg1;
                        unsigned long *milliseconds = (unsigned long *)args.arg2;
                        hazelcast::util::sleepmillis(*milliseconds);
                        q->push(*val);
                    }
                    static void pop(hazelcast::util::ThreadArgs &args) {
                        hazelcast::util::BlockingConcurrentQueue<int> *q = (hazelcast::util::BlockingConcurrentQueue<int> *)args.arg0;
                        std::atomic<int> *val = (std::atomic<int> *)args.arg1;
                        val->store(q->pop());
                    }

                };

                TEST_F(BlockingConcurrentQueueTest, testPushDelyaed) {
                    size_t capacity = 3;

                    hazelcast::util::BlockingConcurrentQueue<int> q(capacity);

                    for (size_t i = 0; i < capacity; ++i) {
                        ASSERT_NO_THROW(q.push(i));
                    }

                    for (size_t i = 0; i < capacity; ++i) {
                        ASSERT_EQ((int) i, q.pop());
                    }

                    int val = 7;

                    unsigned long sleepTime = 3000U;
                    hazelcast::util::StartedThread t(push_delayed, &q, &val, &sleepTime);

                    int64_t start = hazelcast::util::current_time_millis();
                    ASSERT_EQ(val, q.pop());
                    int64_t diff = hazelcast::util::current_time_millis() - start;
                    ASSERT_TRUE(diff > 1000);
                }

                TEST_F(BlockingConcurrentQueueTest, testPop) {
                    size_t capacity = 3;

                    hazelcast::util::BlockingConcurrentQueue<int> q(capacity);

                    for (size_t i = 0; i < capacity; ++i) {
                        ASSERT_NO_THROW(q.push(i));
                    }

                    for (size_t i = 0; i < capacity; ++i) {
                        ASSERT_EQ((int) i, q.pop());
                    }

                    std::atomic<int> val(-1);
                    int testValue = 7;
                    unsigned long sleepTime = 3000U;
                    hazelcast::util::StartedThread t(pop, &q, &val, &sleepTime);

                    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
                    ASSERT_NO_THROW(q.push(testValue));
                    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                    ASSERT_EQ(testValue, (int) val);
                }

                TEST_F(BlockingConcurrentQueueTest, testInterrupt) {
                    size_t capacity = 3;
                    hazelcast::util::BlockingConcurrentQueue<int> q(capacity);

                    std::atomic<bool> finished{false};
                    std::thread t([&] () {
                        while (!finished) {
                            std::this_thread::sleep_for(std::chrono::milliseconds(100));
                            q.interrupt();
                        }
                    });
                    // Note that this test is time sensitive, this thread shoulc be waiting at blocking pop when the
                    // other thread executes the interrup call.
                    ASSERT_THROW(q.pop(), client::exception::interrupted);
                    finished = true;
                    t.join();
                }
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace test {
            namespace util {
                class ConcurentQueueTest : public ClientTestSupport
                {
                protected:
                    class ConcurrentQueueTask {
                    public:
                        ConcurrentQueueTask(hazelcast::util::ConcurrentQueue<int> &q,
                                            boost::latch &start_latch,
                                            boost::latch &start_remove_latch, int removal_value) : q_(q),
                                                                                                start_latch_(
                                                                                                        start_latch),
                                                                                                start_remove_latch_(
                                                                                                        start_remove_latch),
                                                                                                removal_value_(
                                                                                                        removal_value) {}

                        virtual void run() {
                            int numItems = 1000;

                            std::vector<int> values((size_t) numItems);

                            start_latch_.count_down();

                            ASSERT_OPEN_EVENTUALLY(start_latch_);

                            // insert items
                            for (int i = 0; i < numItems; ++i) {
                                values[i] = i;
                                q_.offer(&values[i]);
                            }

                            q_.offer(&removal_value_);
                            start_remove_latch_.count_down();

                            // poll items
                            for (int i = 0; i < numItems; ++i) {
                                values[i] = i;
                                ASSERT_NE((int *) nullptr, q_.poll());
                            }
                        }

                        virtual const std::string get_name() const {
                            return "ConcurrentQueueTask";
                        }

                    private:
                        hazelcast::util::ConcurrentQueue<int> &q_;
                        boost::latch &start_latch_;
                        boost::latch &start_remove_latch_;
                        int removal_value_;
                    };
                };

                TEST_F(ConcurentQueueTest, testSingleThread) {
                    hazelcast::util::ConcurrentQueue<int> q;

                    ASSERT_EQ((int *) nullptr, q.poll());

                    int val1, val2;

                    q.offer(&val1);

                    ASSERT_EQ(&val1, q.poll());

                    ASSERT_EQ((int *) nullptr, q.poll());

                    q.offer(&val1);
                    q.offer(&val2);
                    q.offer(&val2);
                    q.offer(&val1);

                    ASSERT_EQ(2, q.remove_all(&val2));
                    ASSERT_EQ(0, q.remove_all(&val2));

                    ASSERT_EQ(&val1, q.poll());
                    ASSERT_EQ(&val1, q.poll());

                    ASSERT_EQ((int *) nullptr, q.poll());
                }

                TEST_F(ConcurentQueueTest, testMultiThread) {
                    constexpr int numThreads = 40;

                    boost::latch startLatch(numThreads);

                    boost::latch startRemoveLatch(numThreads);

                    hazelcast::util::ConcurrentQueue<int> q;

                    int removalValue = 10;

                    std::array<std::future<void>, numThreads> allFutures;
                    for (int i = 0; i < numThreads; i++) {
                        allFutures[i] = std::async([&]() {
                            ConcurrentQueueTask(q, startLatch, startRemoveLatch, removalValue).run();
                        });
                    }

                    // wait for the remove start
                    ASSERT_OPEN_EVENTUALLY(startRemoveLatch);

                    int numRemoved = q.remove_all(&removalValue);

                    int numRemaining = numThreads - numRemoved;

                    for (int j = 0; j < numRemaining; ++j) {
                        ASSERT_NE((int *) nullptr, q.poll());
                    }
                    ASSERT_EQ(0, q.remove_all(&removalValue));

                }
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace test {
            class ClientUtilTest : public ClientTestSupport {
            protected:
            };

            TEST_F (ClientUtilTest, testStrError) {
#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
                                                                                                                                        int error = WSAEINPROGRESS;
                std::string expectedErrorString("A blocking operation is currently executing.\r\n");
#else
                int error = EINPROGRESS;
                std::string expectedErrorString("Operation now in progress");
#endif

                char msg[100];
                const std::string prefix = "testStrError prefix message";
                ASSERT_EQ(0, hazelcast::util::strerror_s(error, msg, 100, prefix.c_str()));
                ASSERT_STREQ((prefix + " " + expectedErrorString).c_str(), msg);

                ASSERT_EQ(0, hazelcast::util::strerror_s(error, msg, prefix.length() + 1, prefix.c_str()));
                ASSERT_STREQ(prefix.c_str(), msg);

                ASSERT_EQ(0, hazelcast::util::strerror_s(error, msg, 100));
                ASSERT_STREQ(expectedErrorString.c_str(), msg);
            }

            TEST_F (ClientUtilTest, testAvailableCoreCount) {
                ASSERT_GT(hazelcast::util::get_available_core_count(), 0);
            }

            TEST_F (ClientUtilTest, testStringUtilTimeToString) {
                std::string timeString = hazelcast::util::StringUtil::time_to_string(
                        std::chrono::steady_clock::now());
                //expected format is "%Y-%m-%d %H:%M:%S.%f" it will be something like 2018-03-20 15:36:07.280
                ASSERT_EQ((size_t) 23, timeString.length());
                ASSERT_EQ(timeString[0], '2');
                ASSERT_EQ(timeString[1], '0');
                ASSERT_EQ(timeString[4], '-');
                ASSERT_EQ(timeString[7], '-');
            }

            TEST_F (ClientUtilTest, testStringUtilTimeToStringFriendly) {
                ASSERT_EQ("never", hazelcast::util::StringUtil::time_to_string(std::chrono::steady_clock::time_point()));
            }

            TEST_F (ClientUtilTest, testLockSupport) {
                int64_t parkDurationNanos = 100;
                int64_t start = hazelcast::util::current_time_nanos();
                hazelcast::util::concurrent::locks::LockSupport::park_nanos(parkDurationNanos);
                int64_t end = hazelcast::util::current_time_nanos();
                int64_t actualDuration = end - start;
                ASSERT_GE(actualDuration, parkDurationNanos);
            }
        }
    }
}

namespace hazelcast {
    namespace client {

        class hazelcast_client;

        namespace test {
            class ClientConfigTest : public ClientTestSupport
            {};
            
            TEST_F(ClientConfigTest, testGetAddresses) {
                client_config clientConfig;
                address address("localhost", 5555);
                clientConfig.get_network_config().add_address(address);

                auto addresses = clientConfig.get_network_config().get_addresses();
                ASSERT_EQ(1U, addresses.size());
                ASSERT_EQ(address, *addresses.begin());
            }

            TEST_F(ClientConfigTest, testAddresseses) {
                client_config clientConfig;
                std::vector<address> addresses{address("localhost", 5555), address("localhost", 6666)};
                std::sort(addresses.begin(), addresses.end());
                clientConfig.get_network_config().add_addresses(addresses);

                auto configuredAddresses = clientConfig.get_network_config().get_addresses();
                ASSERT_EQ(2U, addresses.size());
                std::vector<address> configuredAddressVector(configuredAddresses.begin(), configuredAddresses.end());
                std::sort(configuredAddressVector.begin(), configuredAddressVector.end());
                ASSERT_EQ(addresses, configuredAddressVector);
            }

            TEST_F(ClientConfigTest, testSetCredentials) {
                client_config clientConfig;
                std::string name("myGroup");
                std::string password("myPass");
                clientConfig.set_credentials(std::make_shared<security::username_password_credentials>(name, password));
                auto credentials = clientConfig.get_credentials();
                ASSERT_EQ(security::credentials::credential_type::username_password, credentials->type());
                auto user_pass_credential = std::static_pointer_cast<security::username_password_credentials>(
                        credentials);
                ASSERT_EQ(name, user_pass_credential->name());
                ASSERT_EQ(password, user_pass_credential->password());
            }

            TEST_F(ClientConfigTest, testSetNullLogHandlerThrows) {
                client_config config;
                auto logger_config = config.get_logger_config();

                ASSERT_THROW(config.get_logger_config().handler(nullptr), exception::illegal_argument);
            }

            TEST_F(ClientConfigTest, testSetGetLogHandler) {
                client_config config;
                auto logger_config = config.get_logger_config();

                bool called = false;

                auto h = [&called](const std::string &,
                                   const std::string &,
                                   logger::level,
                                   const std::string &) {
                    called = true;
                };

                logger_config.handler(h);
                logger_config.handler()("", "", logger::level::fine, "");

                ASSERT_TRUE(called);
            }

            TEST_F(ClientConfigTest, testDefaultLogLevel) {
                client_config config;
                auto logger_config = config.get_logger_config();

                ASSERT_EQ(logger::level::info, logger_config.level());
            }

            TEST_F(ClientConfigTest, testSetGetLogLevel) {
                client_config config;
                auto logger_config = config.get_logger_config();

                logger_config.level(logger::level::fine);

                ASSERT_EQ(logger::level::fine, logger_config.level());
            }

            TEST_F(ClientConfigTest, test_set_instance_name) {
                HazelcastServer instance(*g_srvFactory);
                auto test_name = get_test_name();
                hazelcast_client client(std::move(client_config().set_instance_name(test_name)));
                ASSERT_EQ(test_name, client.get_name());
            }

            class connection_retry_config_test : public ClientTestSupport
            {};

            TEST_F(connection_retry_config_test, large_jitter) {
                ASSERT_THROW(client_config().get_connection_strategy_config().get_retry_config().set_jitter(1.01), exception::illegal_argument);
            }

            TEST_F(connection_retry_config_test, max_backoff_duration_boundaries) {
                auto retry_config = client_config().get_connection_strategy_config().get_retry_config();
                ASSERT_THROW(retry_config.set_max_backoff_duration(std::chrono::milliseconds(-1)), exception::illegal_argument);
                ASSERT_NO_THROW(retry_config.set_max_backoff_duration(std::chrono::milliseconds(0)));
                ASSERT_NO_THROW(retry_config.set_max_backoff_duration(std::chrono::seconds(100)));
            }

            TEST_F(connection_retry_config_test, initial_backoff_duration_boundaries) {
                auto retry_config = client_config().get_connection_strategy_config().get_retry_config();
                ASSERT_THROW(retry_config.set_initial_backoff_duration(std::chrono::milliseconds(-1)), exception::illegal_argument);
                ASSERT_NO_THROW(retry_config.set_initial_backoff_duration(std::chrono::milliseconds(0)));
                ASSERT_NO_THROW(retry_config.set_initial_backoff_duration(std::chrono::seconds(100)));
            }

            TEST_F(connection_retry_config_test, cluster_connect_timeout_boundaries) {
                auto retry_config = client_config().get_connection_strategy_config().get_retry_config();
                ASSERT_THROW(retry_config.set_cluster_connect_timeout(std::chrono::milliseconds(-1)), exception::illegal_argument);
                ASSERT_NO_THROW(retry_config.set_cluster_connect_timeout(std::chrono::milliseconds(0)));
                ASSERT_NO_THROW(retry_config.set_cluster_connect_timeout(std::chrono::seconds(100)));
            }

            TEST_F(connection_retry_config_test, multiplier_boundaries) {
                auto retry_config = client_config().get_connection_strategy_config().get_retry_config();
                ASSERT_THROW(retry_config.set_multiplier(0.99), exception::illegal_argument);
                ASSERT_THROW(retry_config.set_multiplier(-1), exception::illegal_argument);
                ASSERT_NO_THROW(retry_config.set_multiplier(1));
                ASSERT_NO_THROW(retry_config.set_multiplier(2));
            }

            TEST_F(connection_retry_config_test, jitter_boundaries) {
                auto retry_config = client_config().get_connection_strategy_config().get_retry_config();
                ASSERT_THROW(retry_config.set_jitter(1.01), exception::illegal_argument);
                ASSERT_THROW(retry_config.set_jitter(-0.01), exception::illegal_argument);
                ASSERT_NO_THROW(retry_config.set_jitter(1));
                ASSERT_NO_THROW(retry_config.set_jitter(0));
                ASSERT_NO_THROW(retry_config.set_jitter(0.5));
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace test {
            namespace connectionstrategy {

                class ConfiguredBehaviourTest : public ClientTestSupport {
                public:
                    ConfiguredBehaviourTest() {
                        client_config_.get_connection_strategy_config().get_retry_config().set_cluster_connect_timeout(
                                std::chrono::seconds(2));
                    }

                protected:
                    client_config client_config_;
                };

                TEST_F(ConfiguredBehaviourTest, testAsyncStartTrueNoCluster) {
                    client_config_.get_connection_strategy_config().set_async_start(true);
                    hazelcast_client client(std::move(client_config_));

                    ASSERT_THROW((client.get_map(random_map_name()).get()),
                                 exception::hazelcast_client_offline);

                    client.shutdown();
                }

                TEST_F(ConfiguredBehaviourTest, testAsyncStartTrueNoCluster_thenShutdown) {
                    client_config_.get_connection_strategy_config().set_async_start(true);
                    hazelcast_client client(std::move(client_config_));
                    client.shutdown();
                    ASSERT_THROW((client.get_map(random_map_name()).get()), exception::hazelcast_client_not_active);

                    client.shutdown();
                }

                TEST_F(ConfiguredBehaviourTest, testAsyncStartTrue) {
                    boost::latch connectedLatch(1);

                    // trying 8.8.8.8 address will delay the initial connection since no such server exist
                    client_config_.get_network_config().add_address(address("8.8.8.8", 5701))
                            .add_address(address("127.0.0.1", 5701));
                    client_config_.get_connection_strategy_config().get_retry_config().set_cluster_connect_timeout(
                            (std::chrono::milliseconds::max)());
                    client_config_.set_property("hazelcast.client.shuffle.member.list", "false");
                    client_config_.add_listener(
                            lifecycle_listener()
                                    .on_connected([&connectedLatch]() {
                                        connectedLatch.try_count_down();
                                    })
                    );
                    client_config_.get_connection_strategy_config().set_async_start(true);

                    hazelcast_client client(std::move(client_config_));

                    ASSERT_TRUE(client.get_lifecycle_service().is_running());

                    HazelcastServer server(*g_srvFactory);

                    ASSERT_OPEN_EVENTUALLY(connectedLatch);

                    auto map = client.get_map(random_map_name()).get();
                    map->size().get();

                    client.shutdown();
                }

                TEST_F(ConfiguredBehaviourTest, testReconnectModeOFFSingleMember) {
                    HazelcastServer hazelcastInstance(*g_srvFactory);

                    client_config_.get_connection_strategy_config().set_reconnect_mode(
                            config::client_connection_strategy_config::OFF);
                    hazelcast_client client(std::move(client_config_));
                    boost::latch shutdownLatch(1);
                    client.add_lifecycle_listener(
                        lifecycle_listener()
                            .on_shutdown([&shutdownLatch](){
                                shutdownLatch.try_count_down();
                            })
                    );

                    // no exception at this point
                    auto map = client.get_map(random_map_name()).get();
                    map->put(1, 5).get();

                    hazelcastInstance.shutdown();
                    ASSERT_OPEN_EVENTUALLY(shutdownLatch);

                    ASSERT_THROW(map->put(1, 5).get(), exception::hazelcast_client_not_active);

                    client.shutdown();
                }

                TEST_F(ConfiguredBehaviourTest, testReconnectModeOFFTwoMembers) {
                    HazelcastServer server1(*g_srvFactory);
                    HazelcastServer server2(*g_srvFactory);

                    client_config_.get_connection_strategy_config().set_reconnect_mode(
                            config::client_connection_strategy_config::OFF);
                    hazelcast_client client(std::move(client_config_));
                    boost::latch shutdownLatch(1);
                    client.add_lifecycle_listener(
                        lifecycle_listener()
                            .on_shutdown([&shutdownLatch](){
                                shutdownLatch.try_count_down();
                            })
                    );

                    // no exception at this point
                    auto map = client.get_map(random_map_name()).get();
                    map->put(1, 5).get();

                    server1.shutdown();
                    server2.shutdown();
                    ASSERT_OPEN_EVENTUALLY(shutdownLatch);

                    ASSERT_THROW(map->put(1, 5).get(), exception::hazelcast_client_not_active);

                    client.shutdown();
                }

                TEST_F(ConfiguredBehaviourTest, testReconnectModeASYNCSingleMemberInitiallyOffline) {
                    HazelcastServer hazelcastInstance(*g_srvFactory);

                    client_config_.get_connection_strategy_config().set_reconnect_mode(
                            config::client_connection_strategy_config::OFF);
                    hazelcast_client client(std::move(client_config_));
                    boost::latch shutdownLatch(1);
                    client.add_lifecycle_listener(
                        lifecycle_listener()
                            .on_shutdown([&shutdownLatch](){
                                shutdownLatch.try_count_down();
                            })
                    );

// no exception at this point
                    auto map = client.get_map(random_map_name()).get();
                    map->put(1, 5).get();

                    hazelcastInstance.shutdown();
                    ASSERT_OPEN_EVENTUALLY(shutdownLatch);

                    ASSERT_THROW(map->put(1, 5).get(), exception::hazelcast_client_not_active);

                    client.shutdown();
                }

                TEST_F(ConfiguredBehaviourTest, testReconnectModeASYNCSingleMember) {
                    HazelcastServer hazelcastInstance(*g_srvFactory);

                    boost::latch connectedLatch(1);

                    client_config_.add_listener(
                        lifecycle_listener()
                            .on_connected([&connectedLatch](){
                                connectedLatch.try_count_down();
                            })
                    );
                    client_config_.get_connection_strategy_config().set_reconnect_mode(
                            config::client_connection_strategy_config::ASYNC);
                    hazelcast_client client(std::move(client_config_));
                    ASSERT_TRUE(client.get_lifecycle_service().is_running());
                    ASSERT_OPEN_EVENTUALLY(connectedLatch);

                    auto map = client.get_map(random_map_name()).get();
                    map->size().get();

                    client.shutdown();
                }

                TEST_F(ConfiguredBehaviourTest, testReconnectModeASYNCSingleMemberStartLate) {
                    HazelcastServer hazelcastInstance(*g_srvFactory);

                    boost::latch initialConnectionLatch(1);
                    boost::latch reconnectedLatch(1);

                    client_config_.get_connection_strategy_config().get_retry_config().set_cluster_connect_timeout(
                            (std::chrono::milliseconds::max)());
                    client_config_.add_listener(
                            lifecycle_listener()
                                    .on_connected([&initialConnectionLatch]() {
                                        initialConnectionLatch.try_count_down();
                                    })
                    );
                    client_config_.get_connection_strategy_config().set_reconnect_mode(
                            config::client_connection_strategy_config::ASYNC);
                    hazelcast_client client(std::move(client_config_));

                    ASSERT_OPEN_EVENTUALLY(initialConnectionLatch);

                    hazelcastInstance.shutdown();

                    client.add_lifecycle_listener(
                        lifecycle_listener()
                            .on_connected([&reconnectedLatch](){
                                reconnectedLatch.try_count_down();
                            })
                    );

                    HazelcastServer hazelcastInstance2(*g_srvFactory);

                    ASSERT_TRUE(client.get_lifecycle_service().is_running());
                    ASSERT_OPEN_EVENTUALLY(reconnectedLatch);

                    auto map = client.get_map(random_map_name()).get();
                    map->size().get();

                    client.shutdown();
                }

                TEST_F(ConfiguredBehaviourTest, testReconnectModeASYNCTwoMembers) {
                    HazelcastServer server1(*g_srvFactory);
                    HazelcastServer server2(*g_srvFactory);

                    boost::latch connectedLatch(1), disconnectedLatch(1), reconnectedLatch(1);

                    client_config_.get_connection_strategy_config().get_retry_config().set_cluster_connect_timeout(
                            std::chrono::seconds(10));
                    client_config_.add_listener(
                            lifecycle_listener()
                                    .on_connected([&connectedLatch]() {
                                        connectedLatch.try_count_down();
                                    })
                    );
                    client_config_.get_connection_strategy_config().set_reconnect_mode(
                            config::client_connection_strategy_config::ASYNC);
                    hazelcast_client client(std::move(client_config_));

                    ASSERT_TRUE(client.get_lifecycle_service().is_running());

                    ASSERT_OPEN_EVENTUALLY(connectedLatch);

                    auto map = client.get_map(random_map_name()).get();
                    map->put(1, 5).get();

                    client.add_lifecycle_listener(lifecycle_listener().on_disconnected([&disconnectedLatch](){
                                disconnectedLatch.try_count_down();
                            })
                    );

                    client.add_lifecycle_listener(lifecycle_listener().on_connected([&reconnectedLatch](){
                                reconnectedLatch.try_count_down();
                            })
                    );

                    server1.shutdown();
                    server2.shutdown();

                    ASSERT_OPEN_EVENTUALLY(disconnectedLatch);

                    HazelcastServer server3(*g_srvFactory);

                    ASSERT_OPEN_EVENTUALLY(reconnectedLatch);

                    map->get<int, int>(1).get();

                    client.shutdown();
                }
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace test {
            class PipeliningTest : public ClientTestSupport {
            public:
                static void SetUpTestCase() {
                    instance = new HazelcastServer(*g_srvFactory);
                    client = new hazelcast_client(client_config());

                    map = client->get_map(MAP_NAME).get();
                    expected = new std::vector<int>;
                    for (int k = 0; k < MAP_SIZE; ++k) {
                        int item = rand();
                        expected->emplace_back(item);
                        map->put(k, item).get();
                    }
                }

                static void TearDownTestCase() {
                    delete instance;
                    instance = nullptr;
                    delete client;
                    client = nullptr;
                    delete expected;
                    expected = nullptr;
                }

            protected:
                static void test_pipelining(const std::shared_ptr<pipelining<int> > &pipelining) {
                    for (int k = 0; k < MAP_SIZE; k++) {
                        pipelining->add(map->get<int, int>(k));
                    }

                    auto results = pipelining->results();
                    ASSERT_EQ(expected->size(), results.size());
                    for (int k = 0; k < MAP_SIZE; ++k) {
                        ASSERT_TRUE(results[k].has_value());
                        ASSERT_EQ((*expected)[k], results[k].value());
                    }
                }

                static HazelcastServer *instance;
                static hazelcast_client *client;
                static const char *MAP_NAME;
                static std::shared_ptr<imap> map;
                static std::vector<int> *expected;
                static const int MAP_SIZE = 10000;
            };

            HazelcastServer *PipeliningTest::instance = nullptr;
            std::shared_ptr<imap> PipeliningTest::map;
            hazelcast_client *PipeliningTest::client = nullptr;
            const char *PipeliningTest::MAP_NAME = "PipeliningTestMap";
            std::vector<int> *PipeliningTest::expected = nullptr;

            TEST_F(PipeliningTest, testConstructor_whenNegativeDepth) {
                ASSERT_THROW(pipelining<std::string>::create(0), exception::illegal_argument);
                ASSERT_THROW(pipelining<std::string>::create(-1), exception::illegal_argument);
            }

            TEST_F(PipeliningTest, testPipeliningFunctionalityDepthOne) {
                test_pipelining(pipelining<int>::create(1));
            }

            TEST_F(PipeliningTest, testPipeliningFunctionalityDepth100) {
                test_pipelining(pipelining<int>::create(100));
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace test {
            class PortableVersionTest : public ::testing::Test {
            public:
                class Child {
                public:
                    Child() = default;

                    Child(std::string name) : name_(name) {}

                    const std::string &get_name() const {
                        return name_;
                    }

                    friend bool operator==(const Child &lhs, const Child &rhs) {
                        return lhs.name_ == rhs.name_;
                    }

                private:
                    std::string name_;
                };

                class Parent {
                public:
                    friend bool operator==(const Parent &lhs, const Parent &rhs) {
                        return lhs.child_ == rhs.child_;
                    }

                    Parent() = default;

                    Parent(Child child) : child_(child) {}

                    const Child &get_child() const {
                        return child_;
                    }

                private:
                    Child child_;
                };
            };

            // Test for issue https://github.com/hazelcast/hazelcast/issues/12733
            TEST_F(PortableVersionTest, test_nestedPortable_versionedSerializer) {
                serialization_config serializationConfig;
                serialization::pimpl::SerializationService ss1(serializationConfig);

                serialization_config serializationConfig2;
                serializationConfig2.set_portable_version(6);
                serialization::pimpl::SerializationService ss2(serializationConfig2);

                //make sure ss2 cached class definition of Child
                ss2.to_data<Child>(new Child("sancar"));

                //serialized parent from ss1
                Parent parent(Child("sancar"));
                serialization::pimpl::data data = ss1.to_data<Parent>(&parent);

                // cached class definition of Child and the class definition from data coming from ss1 should be compatible
                ASSERT_EQ(parent, *ss2.to_object<Parent>(data));
            }
        }
        namespace serialization {
            template<>
            struct hz_serializer<test::PortableVersionTest::Child> : public portable_serializer {
                static int32_t get_factory_id() {
                    return 1;
                }

                static int get_class_id() {
                    return 2;
                }

                static void write_portable(const test::PortableVersionTest::Child &object,
                                          portable_writer &writer) {
                    writer.write("name", object.get_name());
                }

                static test::PortableVersionTest::Child read_portable(portable_reader &reader) {
                    return test::PortableVersionTest::Child(reader.read<std::string>("name"));
                }
            };

            template<>
            struct hz_serializer<test::PortableVersionTest::Parent> : public portable_serializer {
                static int32_t get_factory_id() {
                    return 1;
                }

                static int32_t get_class_id() {
                    return 1;
                }

                static void write_portable(const test::PortableVersionTest::Parent &object, portable_writer &writer) {
                    writer.write_portable<test::PortableVersionTest::Child>("child", &object.get_child());
                }

                static test::PortableVersionTest::Parent read_portable(portable_reader &reader) {
                    return test::PortableVersionTest::Parent(
                            reader.read_portable<test::PortableVersionTest::Child>("child").value());
                }
            };

        }
    }
}

namespace hazelcast {
    namespace client {
        namespace test {
            class PartitionAwareTest : public ClientTestSupport {
            public:
                class SimplePartitionAwareObject : public partition_aware<int> {
                public:
                    SimplePartitionAwareObject() : test_key_(5) {}

                    const int *get_partition_key() const override {
                        return &test_key_;
                    }

                    int get_test_key() const {
                        return test_key_;
                    }
                private:
                    int test_key_;
                };
            };

            TEST_F(PartitionAwareTest, testSimplePartitionAwareObjectSerialisation) {
                serialization_config serializationConfig;
                serialization::pimpl::SerializationService serializationService(serializationConfig);

                SimplePartitionAwareObject obj;
                serialization::pimpl::data data = serializationService.to_data<SimplePartitionAwareObject>(&obj);
                ASSERT_TRUE(data.has_partition_hash());

                int testKey = obj.get_test_key();
                serialization::pimpl::data expected_data = serializationService.to_data<int>(&testKey);

                ASSERT_EQ(expected_data.get_partition_hash(), data.get_partition_hash());
            }

            TEST_F(PartitionAwareTest, testNonPartitionAwareObjectSerialisation) {
                serialization_config serializationConfig;
                serialization::pimpl::SerializationService serializationService(serializationConfig);

                int obj = 7;
                serialization::pimpl::data data = serializationService.to_data<int>(&obj);
                ASSERT_FALSE(data.has_partition_hash());
            }
        }

        namespace serialization {
            template<>
            struct hz_serializer<test::PartitionAwareTest::SimplePartitionAwareObject> : public identified_data_serializer {
                static int32_t get_factory_id() {
                    return 1;
                }

                static int32_t get_class_id() {
                    return 2;
                }

                static void write_data(const test::PartitionAwareTest::SimplePartitionAwareObject &object, object_data_output &out) {}

                static test::PartitionAwareTest::SimplePartitionAwareObject read_data(object_data_input &in) {
                    return test::PartitionAwareTest::SimplePartitionAwareObject();
                }
            };
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace test {
            class JsonValueSerializationTest : public ClientTestSupport {
            public:
                JsonValueSerializationTest() : serialization_service_(config_) {}

            protected:
                serialization::pimpl::SerializationService serialization_service_;
                serialization_config config_;
            };

            TEST_F(JsonValueSerializationTest, testSerializeDeserializeJsonValue) {
                hazelcast_json_value jsonValue("{ \"key\": \"value\" }");
                serialization::pimpl::data json_data = serialization_service_.to_data<hazelcast_json_value>(&jsonValue);
                auto jsonDeserialized = serialization_service_.to_object<hazelcast_json_value>(json_data);
                ASSERT_TRUE(jsonDeserialized.has_value());
                ASSERT_EQ(jsonValue, jsonDeserialized.value());
            }

            class ClientSerializationTest : public ::testing::Test {
            protected:
                TestInnerPortable create_inner_portable() {
                    std::vector<byte> bb = {0, 1, 2};
                    std::vector<char> cc = {'c', 'h', 'a', 'r'};
                    std::vector<bool> ba = {false, true, true, false};
                    std::vector<int16_t> ss = {3, 4, 5};
                    std::vector<int32_t> ii = {9, 8, 7, 6};
                    std::vector<int64_t> ll = {0, 1, 5, 7, 9, 11};
                    std::vector<float> ff = {0.6543f, -3.56f, 45.67f};
                    std::vector<double> dd = {456.456, 789.789, 321.321};
                    TestNamedPortable portableArray[5];
                    std::vector<std::string> stringVector{"イロハニホヘト", "チリヌルヲ", "ワカヨタレソ"};
                    std::vector<TestNamedPortable> nn;
                    for (int i = 0; i < 5; i++) {
                        nn.emplace_back(TestNamedPortable{"named-portable-" + std::to_string(i), i});
                    }
                    return TestInnerPortable{bb, ba, cc, ss, ii, ll, ff, dd, stringVector, nn};
                }

                struct NonSerializableObject {
                    std::string s;

                    friend bool operator==(const NonSerializableObject &lhs, const NonSerializableObject &rhs) {
                        return lhs.s == rhs.s;
                    }
                };

                class DummyGlobalSerializer : public serialization::global_serializer {
                public:
                    void write(const boost::any &object, serialization::object_data_output &out) override {
                        auto const &obj = boost::any_cast<NonSerializableObject>(object);
                        out.write<std::string>(obj.s);
                    }

                    boost::any read(serialization::object_data_input &in) override {
                        return boost::any(NonSerializableObject{in.read<std::string>()});
                    }
                };

                template<typename T>
                T to_data_and_back_to_object(serialization::pimpl::SerializationService &ss, T &value) {
                    serialization::pimpl::data data = ss.to_data<T>(value);
                    return *(ss.to_object<T>(data));
                }

                static const unsigned int LARGE_ARRAY_SIZE;
            };

            const unsigned int ClientSerializationTest::LARGE_ARRAY_SIZE =
                    1 * 1024 * 1024;   // 1 MB. Previously it was 10 MB but then the
            // test fails when using Windows 32-bit DLL
            // library with std::bad_alloc with 10 MB

            TEST_F(ClientSerializationTest, testCustomSerialization) {
                serialization_config serializationConfig;
                serializationConfig.set_portable_version(1);
                serialization::pimpl::SerializationService serializationService(serializationConfig);

                TestCustomXSerializable a{131321};
                serialization::pimpl::data data = serializationService.to_data<TestCustomXSerializable>(&a);
                auto a2 = serializationService.to_object<TestCustomXSerializable>(data);
                ASSERT_TRUE(a2);
                ASSERT_EQ(a, a2.value());

                TestCustomPerson b{"TestCustomPerson"};
                serialization::pimpl::data data1 = serializationService.to_data<TestCustomPerson>(&b);
                auto b2 = serializationService.to_object<TestCustomPerson>(data1);
                ASSERT_TRUE(b2);
                ASSERT_EQ(b, *b2);
            }


            TEST_F(ClientSerializationTest, testRawData) {
                serialization_config serializationConfig;
                serializationConfig.set_portable_version(1);
                serialization::pimpl::SerializationService serializationService(serializationConfig);
                char charA[] = "test chars";
                std::vector<char> chars(charA, charA + 10);
                std::vector<byte> bytes;
                bytes.resize(5, 0);
                TestDataSerializable ds{123, 's'};
                TestNamedPortable np{"named portable", 34567};
                TestRawDataPortable p{123213, chars, np, 22, "Testing raw portable", ds};

                serialization::pimpl::data data = serializationService.to_data<TestRawDataPortable>(&p);
                auto x = serializationService.to_object<TestRawDataPortable>(data);
                ASSERT_TRUE(x);
                ASSERT_EQ(p, *x);
            }

            TEST_F(ClientSerializationTest, testIdentifiedDataSerializable) {
                serialization_config serializationConfig;
                serializationConfig.set_portable_version(1);
                serialization::pimpl::SerializationService serializationService(serializationConfig);
                serialization::pimpl::data data;
                TestDataSerializable np{4, 'k'};
                data = serializationService.to_data<TestDataSerializable>(&np);

                auto tnp1 = serializationService.to_object<TestDataSerializable>(data);
                ASSERT_TRUE(tnp1);
                ASSERT_EQ(np, *tnp1);
                int x = 4;
                data = serializationService.to_data<int>(&x);
                auto ptr = serializationService.to_object<int>(data);
                ASSERT_TRUE(*ptr);
                int y = *ptr;
                ASSERT_EQ(x, y);
            }

            TEST_F(ClientSerializationTest, testRawDataWithoutRegistering) {
                serialization_config serializationConfig;
                serializationConfig.set_portable_version(1);
                serialization::pimpl::SerializationService serializationService(serializationConfig);
                char charA[] = "test chars";
                std::vector<char> chars(charA, charA + 10);
                std::vector<byte> bytes;
                bytes.resize(5, 0);
                TestNamedPortable np{"named portable", 34567};
                TestDataSerializable ds{123, 's'};
                TestRawDataPortable p{123213, chars, np, 22, "Testing raw portable", ds};

                serialization::pimpl::data data = serializationService.to_data<TestRawDataPortable>(&p);
                auto x = serializationService.to_object<TestRawDataPortable>(data);
                ASSERT_TRUE(x);
                ASSERT_EQ(p, *x);
            }

            TEST_F(ClientSerializationTest, testInvalidWrite) {
                serialization_config serializationConfig;
                serializationConfig.set_portable_version(1);
                serialization::pimpl::SerializationService serializationService(serializationConfig);
                TestInvalidWritePortable p{2131, 123, "q4edfd"};
                ASSERT_THROW(serializationService.to_data<TestInvalidWritePortable>(&p),
                             exception::hazelcast_serialization);
            }

            TEST_F(ClientSerializationTest, testInvalidRead) {
                serialization_config serializationConfig;
                serializationConfig.set_portable_version(1);
                serialization::pimpl::SerializationService serializationService(serializationConfig);
                TestInvalidReadPortable p{2131, 123, "q4edfd"};
                serialization::pimpl::data data = serializationService.to_data<TestInvalidReadPortable>(&p);
                ASSERT_THROW(serializationService.to_object<TestInvalidReadPortable>(data),
                             exception::hazelcast_serialization);
            }

            TEST_F(ClientSerializationTest, testDifferentVersions) {
                serialization_config serializationConfig;
                serializationConfig.set_portable_version(1);
                serialization::pimpl::SerializationService serializationService(serializationConfig);

                serialization_config serializationConfig2;
                serializationConfig2.set_portable_version(2);
                serialization::pimpl::SerializationService serializationService2(serializationConfig2);

                serialization::pimpl::data data = serializationService.to_data<TestNamedPortable>(
                        TestNamedPortable{"portable-v1", 111});

                serialization::pimpl::data data2 = serializationService2.to_data<TestNamedPortableV2>(
                        TestNamedPortableV2{"portable-v2", 123});

                auto t2 = serializationService2.to_object<TestNamedPortableV2>(data);
                ASSERT_TRUE(t2);
                ASSERT_EQ(std::string("portable-v1"), t2->name);
                ASSERT_EQ(111, t2->k);
                ASSERT_EQ(0, t2->v);

                auto t1 = serializationService.to_object<TestNamedPortable>(data2);
                ASSERT_TRUE(t1);
                ASSERT_EQ(std::string("portable-v2"), t1->name);
                ASSERT_EQ(123, t1->k);

            }

            TEST_F(ClientSerializationTest, testBasicFunctionality) {
                serialization_config serializationConfig;
                serializationConfig.set_portable_version(1);
                serialization::pimpl::SerializationService serializationService(serializationConfig);
                serialization::pimpl::data data;

                int x = 3;
                data = serializationService.to_data<int>(&x);

                auto returnedInt = serializationService.to_object<int>(data);
                ASSERT_EQ(x, *returnedInt);

                int16_t f = 2;
                data = serializationService.to_data<int16_t>(&f);

                auto temp = serializationService.to_object<int16_t>(data);
                ASSERT_EQ(f, *temp);

                TestNamedPortable np{"name", 5};
                data = serializationService.to_data<TestNamedPortable>(&np);
                auto tnp1 = serializationService.to_object<TestNamedPortable>(data);
                auto tnp2 = serializationService.to_object<TestNamedPortable>(data);
                ASSERT_TRUE(tnp1);
                ASSERT_EQ(np, *tnp1);
                ASSERT_TRUE(tnp2);
                ASSERT_EQ(np, *tnp2);
                TestInnerPortable inner = create_inner_portable();
                data = serializationService.to_data<TestInnerPortable>(inner);
                auto tip1 = serializationService.to_object<TestInnerPortable>(data);
                auto tip2 = serializationService.to_object<TestInnerPortable>(data);
                ASSERT_TRUE(tip1);
                ASSERT_EQ(inner, *tip1);
                ASSERT_TRUE(tip2);
                ASSERT_EQ(inner, *tip2);

                TestMainPortable main{(byte) 113, false, 'x', -500, 56789, -50992225, 900.5678f, -897543.3678909,
                                      "this is main portable object created for testing!", inner};
                data = serializationService.to_data<TestMainPortable>(&main);

                auto tmp1 = serializationService.to_object<TestMainPortable>(data);
                auto tmp2 = serializationService.to_object<TestMainPortable>(data);
                ASSERT_TRUE(tmp1);
                ASSERT_TRUE(tmp2);
                ASSERT_EQ(main, *tmp1);
                ASSERT_EQ(main, *tmp2);
            }

            TEST_F(ClientSerializationTest, testStringLiterals) {
                auto literal = R"delimeter(My example string literal)delimeter";
                serialization_config config;
                serialization::pimpl::SerializationService serializationService(config);
                auto data = serializationService.to_data(literal);
                auto obj = serializationService.to_object<decltype(literal)>(data);
                ASSERT_TRUE(obj);
                ASSERT_EQ(obj.value(), literal);
            }

            TEST_F(ClientSerializationTest, testBasicFunctionalityWithLargeData) {
                serialization_config serializationConfig;
                serializationConfig.set_portable_version(1);
                serialization::pimpl::SerializationService serializationService(serializationConfig);
                serialization::pimpl::data data;

                std::vector<byte> bb(LARGE_ARRAY_SIZE);
                std::vector<bool> ba(LARGE_ARRAY_SIZE);
                std::vector<char> cc(LARGE_ARRAY_SIZE);
                std::vector<int16_t> ss(LARGE_ARRAY_SIZE);
                std::vector<int32_t> ii(LARGE_ARRAY_SIZE);
                std::vector<int64_t> ll(LARGE_ARRAY_SIZE);
                std::vector<float> ff(LARGE_ARRAY_SIZE);
                std::vector<double> dd(LARGE_ARRAY_SIZE);
                std::vector<std::string> stringVector{"イロハニホヘト", "チリヌルヲ", "ワカヨタレソ"};

                TestNamedPortable portableArray[5];
                std::vector<TestNamedPortable> nn;
                for (int i = 0; i < 5; i++) {
                    nn.emplace_back(TestNamedPortable{"named-portable-" + std::to_string(i), i});
                }
                TestInnerPortable inner{bb, ba, cc, ss, ii, ll, ff, dd, stringVector, nn};
                data = serializationService.to_data<TestInnerPortable>(inner);
                auto tip1 = serializationService.to_object<TestInnerPortable>(data);
                auto tip2 = serializationService.to_object<TestInnerPortable>(data);
                ASSERT_TRUE(tip1);
                ASSERT_EQ(inner, *tip1);
                ASSERT_TRUE(tip2);
                ASSERT_EQ(inner, *tip2);
            }

            TEST_F(ClientSerializationTest, testBasicFunctionalityWithDifferentVersions) {
                serialization_config serializationConfig;
                serializationConfig.set_portable_version(1);
                serialization::pimpl::SerializationService serializationService(serializationConfig);

                serialization_config serializationConfig2;
                serializationConfig2.set_portable_version(2);
                serialization::pimpl::SerializationService serializationService2(serializationConfig2);
                serialization::pimpl::data data;

                int32_t x = 3;
                data = serializationService.to_data<int32_t>(&x);
                auto returnedInt = serializationService.to_object<int32_t>(data);
                ASSERT_EQ(x, *returnedInt);

                int16_t f = 2;
                data = serializationService.to_data<int16_t>(&f);

                auto temp = serializationService.to_object<int16_t>(data);
                ASSERT_EQ(f, *temp);

                TestNamedPortable np{"name", 5};
                data = serializationService.to_data<TestNamedPortable>(&np);
                auto tnp1 = serializationService.to_object<TestNamedPortable>(data);
                auto tnp2 = serializationService2.to_object<TestNamedPortable>(data);
                ASSERT_TRUE(tnp1);
                ASSERT_TRUE(tnp2);
                ASSERT_EQ(np, *tnp1);
                ASSERT_EQ(np, *tnp2);

                TestInnerPortable inner = create_inner_portable();
                data = serializationService.to_data<TestInnerPortable>(&inner);

                auto tip1 = serializationService.to_object<TestInnerPortable>(data);
                auto tip2 = serializationService2.to_object<TestInnerPortable>(data);
                ASSERT_TRUE(tip1);
                ASSERT_TRUE(tip2);
                ASSERT_EQ(inner, *tip1);
                ASSERT_EQ(inner, *tip2);

                TestMainPortable main{(byte) 113, true, 'x', -500, 56789, -50992225, 900.5678f, -897543.3678909,
                                      "this is main portable object created for testing!", inner};
                data = serializationService.to_data<TestMainPortable>(&main);

                auto tmp1 = serializationService.to_object<TestMainPortable>(data);
                auto tmp2 = serializationService2.to_object<TestMainPortable>(data);
                ASSERT_TRUE(tmp1);
                ASSERT_TRUE(tmp2);
                ASSERT_EQ(main, *tmp1);
                ASSERT_EQ(main, *tmp2);
            }

            TEST_F(ClientSerializationTest, testDataHash) {
                serialization_config serializationConfig;
                serialization::pimpl::SerializationService serializationService(serializationConfig);
                std::string serializable = "key1";
                serialization::pimpl::data data = serializationService.to_data<std::string>(&serializable);
                serialization::pimpl::data data2 = serializationService.to_data<std::string>(&serializable);
                ASSERT_EQ(data.get_partition_hash(), data2.get_partition_hash());

            }

            TEST_F(ClientSerializationTest, testPrimitives) {
                serialization_config serializationConfig;
                serialization::pimpl::SerializationService serializationService(serializationConfig);
                byte by = 2;
                bool boolean = true;
                char c = 'c';
                int16_t s = 4;
                int32_t i = 2000;
                int64_t l = 321324141;
                float f = 3.14f;
                double d = 3.14334;
                std::string str = "Hello world";
                std::string utfStr = "イロハニホヘト チリヌルヲ ワカヨタレソ ツネナラム";

                ASSERT_EQ(by, to_data_and_back_to_object(serializationService, by));
                ASSERT_EQ(boolean, to_data_and_back_to_object(serializationService, boolean));
                ASSERT_EQ(c, to_data_and_back_to_object(serializationService, c));
                ASSERT_EQ(s, to_data_and_back_to_object(serializationService, s));
                ASSERT_EQ(i, to_data_and_back_to_object(serializationService, i));
                ASSERT_EQ(l, to_data_and_back_to_object(serializationService, l));
                ASSERT_EQ(f, to_data_and_back_to_object(serializationService, f));
                ASSERT_EQ(d, to_data_and_back_to_object(serializationService, d));
                ASSERT_EQ(str, to_data_and_back_to_object(serializationService, str));
                ASSERT_EQ(utfStr, to_data_and_back_to_object(serializationService, utfStr));
            }

            TEST_F(ClientSerializationTest, testPrimitiveArrays) {
                serialization_config serializationConfig;
                serialization::pimpl::SerializationService serializationService(serializationConfig);

                char charArray[] = {'c', 'h', 'a', 'r'};
                std::vector<char> cc(charArray, charArray + 4);
                bool boolArray[] = {true, false, false, true};
                byte byteArray[] = {0, 1, 2};
                std::vector<byte> bb(byteArray, byteArray + 3);
                std::vector<bool> ba(boolArray, boolArray + 4);
                int16_t shortArray[] = {3, 4, 5};
                std::vector<int16_t> ss(shortArray, shortArray + 3);
                int32_t integerArray[] = {9, 8, 7, 6};
                std::vector<int32_t> ii(integerArray, integerArray + 4);
                int64_t longArray[] = {0, 1, 5, 7, 9, 11};
                std::vector<int64_t> ll(longArray, longArray + 6);
                float floatArray[] = {0.6543f, -3.56f, 45.67f};
                std::vector<float> ff(floatArray, floatArray + 3);
                double doubleArray[] = {456.456, 789.789, 321.321};
                std::vector<double> dd(doubleArray, doubleArray + 3);
                const std::string stringArray[] = {"ali", "veli", "イロハニホヘト チリヌルヲ ワカヨタレソ ツネナラム"};
                std::vector<std::string> stringVector;
                for (int i = 0; i < 3; ++i) {
                    stringVector.push_back(stringArray[i]);
                }

                ASSERT_EQ(cc, to_data_and_back_to_object<std::vector<char> >(serializationService, cc));
                ASSERT_EQ(ba, to_data_and_back_to_object<std::vector<bool> >(serializationService, ba));
                ASSERT_EQ(bb, to_data_and_back_to_object<std::vector<byte> >(serializationService, bb));
                ASSERT_EQ(ss, to_data_and_back_to_object<std::vector<int16_t> >(serializationService, ss));
                ASSERT_EQ(ii, to_data_and_back_to_object<std::vector<int32_t> >(serializationService, ii));
                ASSERT_EQ(ll, to_data_and_back_to_object<std::vector<int64_t> >(serializationService, ll));
                ASSERT_EQ(ff, to_data_and_back_to_object<std::vector<float> >(serializationService, ff));
                ASSERT_EQ(dd, to_data_and_back_to_object<std::vector<double> >(serializationService, dd));
                ASSERT_EQ(stringVector, to_data_and_back_to_object<std::vector<std::string> >(serializationService,
                                                                                         stringVector));
            }

            TEST_F(ClientSerializationTest, testWriteObjectWithPortable) {
                serialization_config serializationConfig;
                serialization::pimpl::SerializationService ss(serializationConfig);

                ObjectCarryingPortable<TestNamedPortable> objectCarryingPortable{TestNamedPortable{"name", 2}};
                serialization::pimpl::data data = ss.to_data < ObjectCarryingPortable < TestNamedPortable > > (
                        &objectCarryingPortable);
                auto ptr = ss.to_object < ObjectCarryingPortable < TestNamedPortable > > (data);
                ASSERT_EQ(objectCarryingPortable, *ptr);
            }

            TEST_F(ClientSerializationTest, testWriteObjectWithIdentifiedDataSerializable) {
                serialization_config serializationConfig;
                serialization::pimpl::SerializationService ss(serializationConfig);

                ObjectCarryingPortable <TestDataSerializable> objectCarryingPortable{TestDataSerializable{2, 'c'}};
                serialization::pimpl::data data = ss.to_data < ObjectCarryingPortable < TestDataSerializable > > (
                        &objectCarryingPortable);
                auto ptr = ss.to_object < ObjectCarryingPortable < TestDataSerializable > > (data);
                ASSERT_EQ(objectCarryingPortable, *ptr);
            }

            TEST_F(ClientSerializationTest, testWriteObjectWithCustomXSerializable) {
                serialization_config serializationConfig;
                serialization::pimpl::SerializationService ss(serializationConfig);
                ObjectCarryingPortable <TestCustomXSerializable> objectCarryingPortable{TestCustomXSerializable{131321}};
                serialization::pimpl::data data = ss.to_data < ObjectCarryingPortable < TestCustomXSerializable > > (
                        &objectCarryingPortable);
                auto ptr = ss.to_object < ObjectCarryingPortable < TestCustomXSerializable > > (data);
                ASSERT_EQ(objectCarryingPortable, *ptr);
            }

            TEST_F(ClientSerializationTest, testWriteObjectWithCustomPersonSerializable) {
                serialization_config serializationConfig;
                serialization::pimpl::SerializationService ss(serializationConfig);

                ObjectCarryingPortable <TestCustomPerson> objectCarryingPortable{TestCustomPerson{"TestCustomPerson"}};
                serialization::pimpl::data data = ss.to_data < ObjectCarryingPortable < TestCustomPerson > > (
                        &objectCarryingPortable);
                auto ptr = ss.to_object < ObjectCarryingPortable < TestCustomPerson > > (data);
                ASSERT_EQ(objectCarryingPortable, *ptr);
            }


            TEST_F(ClientSerializationTest, testNullData) {
                serialization::pimpl::data data;
                serialization_config serializationConfig;
                serialization::pimpl::SerializationService ss(serializationConfig);
                auto ptr = ss.to_object<int32_t>(data);
                ASSERT_FALSE(ptr.has_value());
            }

            TEST_F(ClientSerializationTest, testMorphingPortableV1ToV2Conversion) {
                serialization_config serializationConfig;
                serializationConfig.set_portable_version(1);
                serialization::pimpl::SerializationService serializationService(serializationConfig);

                TestNamedPortable p{"portable-v1", 123};
                serialization::pimpl::data data = serializationService.to_data<TestNamedPortable>(p);

                auto p2 = serializationService.to_object<TestNamedPortableV2>(data);
                ASSERT_TRUE(p2);
                ASSERT_EQ(std::string("portable-v1"), p2->name);
                ASSERT_EQ(123, p2->k);
                ASSERT_EQ(0, p2->v);
            }

            TEST_F(ClientSerializationTest, testMorphingPortableV2ToV1Conversion) {
                serialization_config serializationConfig;
                serializationConfig.set_portable_version(1);
                serialization::pimpl::SerializationService serializationService(serializationConfig);

                TestNamedPortableV2 p2{"portable-v2", 123, 9999};
                serialization::pimpl::data data = serializationService.to_data<TestNamedPortableV2>(p2);

                auto p = serializationService.to_object<TestNamedPortable>(data);
                ASSERT_TRUE(p);
                ASSERT_EQ(std::string("portable-v2"), p->name);
                ASSERT_EQ(123, p->k);
            }

            TEST_F(ClientSerializationTest, testMorphingPortableV1ToV3Conversion) {
                serialization_config serializationConfig;
                serializationConfig.set_portable_version(1);
                serialization::pimpl::SerializationService serializationService(serializationConfig);

                TestNamedPortable p{"portable-v1", 123};
                serialization::pimpl::data data = serializationService.to_data<TestNamedPortable>(p);

                ASSERT_THROW(serializationService.to_object<TestNamedPortableV3>(data), exception::hazelcast_serialization);
            }

            TEST_F(ClientSerializationTest, testMorphingPortableWithDifferentTypes_differentVersions_V2ToV1) {
                serialization_config serializationConfig;
                serializationConfig.set_portable_version(1);
                serialization::pimpl::SerializationService serializationService(serializationConfig);

                serialization_config serializationConfig2;
                serializationConfig.set_portable_version(5);
                serialization::pimpl::SerializationService serializationService2(serializationConfig2);

                TestNamedPortableV2 p2{"portable-v2", 123, 7};
                serialization::pimpl::data data2 = serializationService2.to_data<TestNamedPortableV2>(p2);

                auto t1 = serializationService.to_object<TestNamedPortable>(data2);
                ASSERT_TRUE(t1.has_value());
                ASSERT_EQ(std::string("portable-v2"), t1->name);
                ASSERT_EQ(123, t1->k);
            }

            TEST_F(ClientSerializationTest, object_data_input_output) {
                serialization_config serializationConfig;
                serializationConfig.set_portable_version(1);
                serialization::pimpl::SerializationService serializationService(serializationConfig);

                serialization::object_data_output out(boost::endian::order::big);

                byte by = 2;
                bool boolean = true;
                char c = 'c';
                int16_t s = 4;
                int32_t i = 2000;
                int64_t l = 321324141;
                float f = 3.14f;
                double d = 3.14334;
                std::string str = "Hello world";
                std::string utfStr = "イロハニホヘト チリヌルヲ ワカヨタレソ ツネナラム";

                std::vector<byte> byteVec = {50, 100, 150, 200};
                std::vector<char> cc = {'c', 'h', 'a', 'r'};
                std::vector<bool> ba = {true, false, false, true};
                std::vector<int16_t> ss = {3, 4, 5};
                std::vector<int32_t> ii = {9, 8, 7, 6};
                std::vector<int64_t> ll = {0, 1, 5, 7, 9, 11};
                std::vector<float> ff = {0.6543f, -3.56f, 45.67f};
                std::vector<double> dd = {456.456, 789.789, 321.321};
                std::vector<std::string> stringVector = {"ali", "veli", "イロハニホヘト チリヌルヲ ワカヨタレソ ツネナラム"};

                out.write<byte>(by);
                out.write(c);
                out.write(boolean);
                out.write<int16_t>(s);
                out.write<int32_t>(i);
                out.write<int64_t>(l);
                out.write<float>(f);
                out.write<double>(d);
                out.write(str);
                out.write(utfStr);

                out.write(&byteVec);
                out.write(&cc);
                out.write(&ba);
                out.write(&ss);
                out.write(&ii);
                out.write(&ff);
                out.write(&dd);
                out.write(&stringVector);

                out.write_object<byte>(&by);
                out.write_object<char>(c);
                out.write_object<bool>(&boolean);
                out.write_object<int16_t>(&s);
                out.write_object<int32_t>(&i);
                out.write_object<float>(&f);
                out.write_object<double>(&d);
                out.write_object<std::string>(&str);
                out.write_object<std::string>(&utfStr);
                out.write<int32_t>(5);
                out.write<std::string>(nullptr);
                out.write<std::vector<std::string>>(nullptr);

                serialization::object_data_input in(boost::endian::order::big, out.to_byte_array(), 0,
                                                    serializationService.get_portable_serializer(),
                                                    serializationService.get_data_serializer(),
                                                    serializationConfig.get_global_serializer());

                ASSERT_EQ(by, in.read<byte>());
                ASSERT_EQ(c, in.read<char>());
                ASSERT_EQ(boolean, in.read<bool>());
                ASSERT_EQ(s, in.read<int16_t>());
                ASSERT_EQ(i, in.read<int32_t>());
                ASSERT_EQ(l, in.read<int64_t>());
                ASSERT_FLOAT_EQ(f, in.read<float>());
                ASSERT_DOUBLE_EQ(d, in.read<double>());
                ASSERT_EQ(str, in.read<std::string>());
                ASSERT_EQ(utfStr, in.read<std::string>());

                ASSERT_EQ(byteVec, *in.read<std::vector<byte>>());
                ASSERT_EQ(cc, *in.read<std::vector<char>>());
                ASSERT_EQ(ba, *in.read<std::vector<bool>>());
                ASSERT_EQ(ss, *in.read<std::vector<int16_t>>());
                ASSERT_EQ(ii, *in.read<std::vector<int32_t>>());
                ASSERT_EQ(ff, *in.read<std::vector<float>>());
                ASSERT_EQ(dd, *in.read<std::vector<double>>());
                auto strArrRead = in.read<std::vector<std::string>>();
                ASSERT_TRUE(strArrRead.has_value());
                ASSERT_EQ(stringVector.size(), strArrRead->size());
                for (size_t j = 0; j < stringVector.size(); ++j) {
                    ASSERT_EQ((*strArrRead)[j], stringVector[j]);
                }

                ASSERT_EQ(by, *in.read_object<byte>());
                ASSERT_EQ(c, *in.read_object<char>());
                ASSERT_EQ(boolean, *in.read_object<bool>());
                ASSERT_EQ(s, *in.read_object<int16_t>());
                ASSERT_EQ(i, *in.read_object<int32_t>());
                ASSERT_EQ(f, *in.read_object<float>());
                ASSERT_EQ(d, *in.read_object<double>());
                ASSERT_EQ(str, *in.read_object<std::string>());
                ASSERT_EQ(utfStr, *in.read_object<std::string>());
                ASSERT_EQ(4, in.skip_bytes(4));
                ASSERT_FALSE(in.read<boost::optional<std::string>>().has_value()) << "Expected null string";
                ASSERT_FALSE(in.read<std::vector<std::string>>().has_value()) << "Expected null string array";
            }

            TEST_F(ClientSerializationTest, testExtendedAscii) {
                std::string utfStr = "Num\xc3\xa9ro";

                serialization_config serializationConfig;
                serialization::pimpl::SerializationService serializationService(serializationConfig);

                serialization::pimpl::data data = serializationService.to_data<std::string>(&utfStr);
                auto deserializedString = serializationService.to_object<std::string>(data);
                ASSERT_TRUE(deserializedString.has_value());
                ASSERT_EQ(utfStr, deserializedString.value());
            }

            TEST_F(ClientSerializationTest, testGlobalSerializer) {
                serialization_config serializationConfig;
                serializationConfig.set_global_serializer(std::make_shared<DummyGlobalSerializer>());
                serialization::pimpl::SerializationService serializationService(serializationConfig);

                NonSerializableObject obj{"My class with no serializer"};

                serialization::pimpl::data data = serializationService.to_data(obj);

                auto deserializedValue = serializationService.to_object<NonSerializableObject>(data);
                ASSERT_TRUE(deserializedValue.has_value());
                ASSERT_EQ(obj, deserializedValue.value());
            }

            extern std::shared_ptr<RemoteControllerClient> remoteController;

            class serialization_with_server : public ClientTestSupport, 
                    public ::testing::WithParamInterface<boost::endian::order> {
            protected:
                void SetUp() override {
                    if (!server_) {
                        little_endian_server_factory_ = new HazelcastServerFactory(
                                "hazelcast/test/resources/hazelcast-serialization-little-endian.xml");
                    }

                    server_.reset(new HazelcastServer(
                            *(boost::endian::order::little == GetParam() ? little_endian_server_factory_
                                                                         : g_srvFactory)));

                    auto config = get_config();
                    config.set_cluster_name(boost::endian::order::little == GetParam() ? "little-endian-cluster" : "dev");
                    config.get_serialization_config().set_byte_order(GetParam());

                    client_.reset(new hazelcast_client(std::move(config)));
                    map_ = client_->get_map("serialization_with_server_map").get();
                }

                static void TearDownTestCase() {
                    delete little_endian_server_factory_;
                }

            protected:
                static Response get_value_from_server() {
                    const char *script = "function foo() {\n"
                                             "var map = instance_0.getMap(\"serialization_with_server_map\");\n"
                                             "var res = map.get(\"key\");\n"
                                             "if (res.getClass().isArray()) {\n"
                                                "return Java.from(res);\n"
                                             "} else {\n"
                                                "return res;\n"
                                             "}\n"
                                         "}\n"
                                         "result = \"\"+foo();";

                    Response response;
                    HazelcastServerFactory *factory =
                            boost::endian::order::little == GetParam() ? little_endian_server_factory_ : g_srvFactory;
                    remoteController->executeOnController(response, factory->get_cluster_id(), script, Lang::JAVASCRIPT);
                    return response;
                }

                bool set_on_server(const std::string &object) {
                    auto script = (boost::format("var map = instance_0.getMap(\"serialization_with_server_map\");\n"
                                   "map.set(\"key\", %1%);\n"
                    ) %object).str();

                    Response response;
                    remoteController->executeOnController(response, server_->cluster_id(), script, Lang::JAVASCRIPT);
                    return response.success;
                }

                static HazelcastServerFactory *little_endian_server_factory_;
                std::unique_ptr<HazelcastServer> server_;
                std::unique_ptr<hazelcast_client> client_;
                std::shared_ptr<imap> map_;
            };

            HazelcastServerFactory *serialization_with_server::little_endian_server_factory_ = nullptr;

            TEST_P(serialization_with_server, test_bool) {
                bool value = true;
                map_->set("key", value).get();
                ASSERT_EQ(value, (*map_->get<std::string, bool>("key").get()));
                auto response = get_value_from_server();
                ASSERT_TRUE(response.success);
                ASSERT_EQ("true", response.result);
            }

            TEST_P(serialization_with_server, test_byte) {
                byte value = INT8_MAX;
                map_->set("key", value).get();
                ASSERT_EQ(value, (*map_->get<std::string, byte>("key").get()));
                auto response = get_value_from_server();
                ASSERT_TRUE(response.success);
                ASSERT_EQ("127", response.result);
            }

            TEST_P(serialization_with_server, test_int16_t) {
                int16_t value = INT16_MIN;
                map_->set("key", value).get();
                ASSERT_EQ(value, (*map_->get<std::string, int16_t>("key").get()));
                auto response = get_value_from_server();
                ASSERT_TRUE(response.success);
                ASSERT_EQ(std::to_string(value), response.result);
            }

            TEST_P(serialization_with_server, test_int32_t) {
                int32_t value = INT32_MAX;
                map_->set("key", value).get();
                ASSERT_EQ(value, (*map_->get<std::string, int32_t>("key").get()));
                auto response = get_value_from_server();
                ASSERT_TRUE(response.success);
                ASSERT_EQ(std::to_string(value), response.result);
            }

            TEST_P(serialization_with_server, test_int64_t) {
                int64_t value = INT64_MIN;
                map_->set("key", value).get();
                ASSERT_EQ(value, (*map_->get<std::string, int64_t>("key").get()));
                auto response = get_value_from_server();
                ASSERT_TRUE(response.success);
                ASSERT_EQ(std::to_string(value), response.result);
            }

            TEST_P(serialization_with_server, test_double) {
                double value = 123.6;
                map_->set("key", value).get();
                ASSERT_EQ(value, (*map_->get<std::string, double>("key").get()));
                auto response = get_value_from_server();
                ASSERT_TRUE(response.success);
                ASSERT_EQ("123.6", response.result);
            }

            TEST_P(serialization_with_server, test_string) {
                std::string value = "string_value";
                map_->set("key", value).get();
                ASSERT_EQ(value, (*map_->get<std::string, std::string>("key").get()));
                auto response = get_value_from_server();
                ASSERT_TRUE(response.success);
                ASSERT_EQ(value, response.result);
            }

            TEST_P(serialization_with_server, test_utf_string) {
                std::string value = u8"Iñtërnâtiônàlizætiøn";
                map_->set("key", value).get();
                ASSERT_EQ(value, (*map_->get<std::string, std::string>("key").get()));
                auto response = get_value_from_server();
                ASSERT_TRUE(response.success);
                ASSERT_EQ(value, response.result);
            }

            TEST_P(serialization_with_server, test_emoji) {
                std::string value = u8"1⚐中\U0001f4a6 \U0001F62D \U0001F45F";
                map_->set("key", value).get();
                ASSERT_EQ(value, (*map_->get<std::string, std::string>("key").get()));
                auto response = get_value_from_server();
                ASSERT_TRUE(response.success);
                ASSERT_EQ(value, response.result);
            }

            TEST_P(serialization_with_server, test_utf_chars) {
                std::string value = u8"\u0040 test \u01DF \u06A0 \u12E0 \u1D30";
                map_->set("key", value).get();
                ASSERT_EQ(value, (*map_->get<std::string, std::string>("key").get()));
                auto response = get_value_from_server();
                ASSERT_TRUE(response.success);
                ASSERT_EQ(value, response.result);
            }

            TEST_P(serialization_with_server, test_uuid) {
                auto value = spi::ClientContext(*client_).random_uuid();
                map_->set("key", value).get();
                ASSERT_EQ(value, (*map_->get<std::string, boost::uuids::uuid>("key").get()));
                auto response = get_value_from_server();
                ASSERT_TRUE(response.success);
                ASSERT_EQ(boost::uuids::to_string(value), response.result);
            }

            TEST_P(serialization_with_server, test_json_value) {
                hazelcast_json_value value("{\"a\": 3}");
                map_->set("key", value).get();
                ASSERT_EQ(value, (*map_->get<std::string, hazelcast_json_value>("key").get()));
                auto response = get_value_from_server();
                ASSERT_TRUE(response.success);
                ASSERT_EQ(value.to_string(), response.result);
            }

            TEST_P(serialization_with_server, test_list) {
                std::vector<std::string> value({"1", "2", "3"});
                map_->set("key", value).get();
                ASSERT_EQ(value, (*map_->get<std::string, std::vector<std::string>>("key").get()));
                auto response = get_value_from_server();
                ASSERT_TRUE(response.success);
                ASSERT_EQ("1,2,3", response.result);
            }

            TEST_P(serialization_with_server, test_bool_from_server) {
                ASSERT_TRUE(set_on_server("true"));
                ASSERT_TRUE((*map_->get<std::string, bool>("key").get()));
            }

            TEST_P(serialization_with_server, test_byte_from_server) {
                ASSERT_TRUE(set_on_server("new java.lang.Byte(-23)"));
                ASSERT_EQ((byte)-23, (*map_->get<std::string, byte>("key").get()));
            }

            TEST_P(serialization_with_server, test_char_from_server) {
                ASSERT_TRUE(set_on_server("new java.lang.Character('x')"));
                ASSERT_EQ('x', (*map_->get<std::string, char>("key").get()));
            }

            TEST_P(serialization_with_server, test_int16_t_from_server) {
                ASSERT_TRUE(set_on_server("new java.lang.Short(23)"));
                ASSERT_EQ(23, (*map_->get<std::string, int16_t>("key").get()));
            }

            TEST_P(serialization_with_server, test_int32_t_from_server) {
                ASSERT_TRUE(set_on_server("new java.lang.Integer(1 << 30)"));
                ASSERT_EQ(1 << 30, (*map_->get<std::string, int32_t>("key").get()));
            }

            TEST_P(serialization_with_server, test_int64_t_from_server) {
                ASSERT_TRUE(set_on_server(std::string("new java.lang.Long(") + std::to_string(1LL << 63) + ")"));
                ASSERT_EQ(1LL << 63, (*map_->get<std::string, int64_t>("key").get()));
            }

            TEST_P(serialization_with_server, test_float_from_server) {
                ASSERT_TRUE(set_on_server("new java.lang.Float(32.0)"));
                ASSERT_EQ(32.0, (*map_->get<std::string, float>("key").get()));
            }

            TEST_P(serialization_with_server, test_double_from_server) {
                ASSERT_TRUE(set_on_server("new java.lang.Double(-12332.0)"));
                ASSERT_EQ(-12332.0, (*map_->get<std::string, double>("key").get()));
            }

            TEST_P(serialization_with_server, test_string_from_server) {
                ASSERT_TRUE(set_on_server(u8"\"1⚐中💦2😭‍🙆😔5\""));
                ASSERT_EQ(u8"1⚐中💦2😭‍🙆😔5", (*map_->get<std::string, std::string>("key").get()));
            }

            TEST_P(serialization_with_server, test_uuid_from_server) {
                ASSERT_TRUE(set_on_server("new java.util.UUID(0, 1)"));
                boost::uuids::uuid id = boost::uuids::nil_uuid();
                id.data[15] = 1;
                ASSERT_EQ(id, (*map_->get<std::string, boost::uuids::uuid>("key").get()));
            }

            TEST_P(serialization_with_server, test_json_from_server) {
                ASSERT_TRUE(set_on_server("new com.hazelcast.core.HazelcastJsonValue(\"{\\\"a\\\": 3}\")"));
                ASSERT_EQ(hazelcast_json_value("{\"a\": 3}"), (*map_->get<std::string, hazelcast_json_value>("key").get()));
            }

            TEST_P(serialization_with_server, test_bool_array_from_server) {
                ASSERT_TRUE(set_on_server("Java.to([true, false], \"boolean[]\")"));
                ASSERT_EQ(std::vector<bool>({true, false}), (*map_->get<std::string, std::vector<bool>>("key").get()));
            }

            TEST_P(serialization_with_server, test_byte_array_from_server) {
                ASSERT_TRUE(set_on_server("Java.to([3, 123], \"byte[]\")"));
                ASSERT_EQ(std::vector<byte>({3, 123}), (*map_->get<std::string, std::vector<byte>>("key").get()));
            }

            TEST_P(serialization_with_server, test_char_array_from_server) {
                ASSERT_TRUE(set_on_server("Java.to(['x', 'y'], \"char[]\")"));
                ASSERT_EQ(std::vector<char>({'x', 'y'}), (*map_->get<std::string, std::vector<char>>("key").get()));
            }

            TEST_P(serialization_with_server, test_int16_array_from_server) {
                ASSERT_TRUE(set_on_server("Java.to([1323, -1232], \"short[]\")"));
                ASSERT_EQ(std::vector<int16_t>({1323, -1232}), (*map_->get<std::string, std::vector<int16_t>>("key").get()));
            }

            TEST_P(serialization_with_server, test_int32_array_from_server) {
                ASSERT_TRUE(set_on_server("Java.to([2147483647, -2147483648], \"int[]\")"));
                ASSERT_EQ(std::vector<int32_t>({INT32_MAX, INT32_MIN}), (*map_->get<std::string, std::vector<int32_t>>("key").get()));
            }

            TEST_P(serialization_with_server, test_int64_array_from_server) {
                ASSERT_TRUE(set_on_server("Java.to([1152921504606846976, -1152921504606846976], \"long[]\")"));
                ASSERT_EQ(std::vector<int64_t>({1152921504606846976LL, -1152921504606846976LL}), (*map_->get<std::string, std::vector<int64_t>>("key").get()));
            }

            TEST_P(serialization_with_server, test_float_array_from_server) {
                ASSERT_TRUE(set_on_server("Java.to([3123.0, -123.0], \"float[]\")"));
                ASSERT_EQ(std::vector<float>({3123.0, -123.0}), (*map_->get<std::string, std::vector<float>>("key").get()));
            }

            TEST_P(serialization_with_server, test_double_array_from_server) {
                ASSERT_TRUE(set_on_server("Java.to([3123.0, -123.0], \"double[]\")"));
                ASSERT_EQ(std::vector<double>({3123.0, -123.0}), (*map_->get<std::string, std::vector<double>>("key").get()));
            }

            TEST_P(serialization_with_server, test_string_array_from_server) {
                ASSERT_TRUE(set_on_server(u8"Java.to([\"hey\", \"1⚐中💦2😭‍🙆😔5\"], \"java.lang.String[]\")"));
                ASSERT_EQ(std::vector<std::string>({u8"hey", u8"1⚐中💦2😭‍🙆😔5"}), (*map_->get<std::string, std::vector<std::string>>("key").get()));
            }

            INSTANTIATE_TEST_SUITE_P(serialization_with_server_endian, serialization_with_server,
                                     ::testing::Values(boost::endian::order::little, boost::endian::order::big));
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace test {
            namespace internal {
                namespace nearcache {
                    class NearCacheRecordStoreTest
                            : public ClientTestSupport, public ::testing::WithParamInterface<config::in_memory_format> {
                    public:
                        NearCacheRecordStoreTest() {
                            ss_ = std::unique_ptr<serialization::pimpl::SerializationService>(
                                    new serialization::pimpl::SerializationService(serialization_config_));
                        }

                    protected:
                        static const int DEFAULT_RECORD_COUNT;
                        static const char *DEFAULT_NEAR_CACHE_NAME;

                        void put_and_get_record(config::in_memory_format in_memory_format) {
                            auto nearCacheConfig = create_near_cache_config(
                                    DEFAULT_NEAR_CACHE_NAME, in_memory_format);
                            auto nearCacheRecordStore = create_near_cache_record_store(nearCacheConfig, in_memory_format);

                            for (int i = 0; i < DEFAULT_RECORD_COUNT; i++) {
                                nearCacheRecordStore->put(get_shared_key(i), get_shared_value(i));
                            }

                            ASSERT_EQ(DEFAULT_RECORD_COUNT, nearCacheRecordStore->size());

                            for (int i = 0; i < DEFAULT_RECORD_COUNT; i++) {
                                auto value = nearCacheRecordStore->get(get_shared_key(i));
                                ASSERT_TRUE(value);
                                ASSERT_EQ(*get_shared_value(i), *value);
                            }
                        }

                        void put_and_remove_record(config::in_memory_format in_memory_format) {
                            auto nearCacheConfig = create_near_cache_config(
                                    DEFAULT_NEAR_CACHE_NAME, in_memory_format);
                            auto nearCacheRecordStore = create_near_cache_record_store(nearCacheConfig, in_memory_format);

                            for (int i = 0; i < DEFAULT_RECORD_COUNT; i++) {
                                std::shared_ptr<serialization::pimpl::data> key = get_shared_key(i);
                                nearCacheRecordStore->put(key, get_shared_value(i));

                                // ensure that they are stored
                                ASSERT_TRUE(nearCacheRecordStore->get(key));
                            }

                            ASSERT_EQ(DEFAULT_RECORD_COUNT, nearCacheRecordStore->size());

                            for (int i = 0; i < DEFAULT_RECORD_COUNT; i++) {
                                std::shared_ptr<serialization::pimpl::data> key = get_shared_key(i);
                                ASSERT_TRUE(nearCacheRecordStore->invalidate(key));
                                ASSERT_FALSE(nearCacheRecordStore->get(key)) << "Should not exist";
                            }

                            ASSERT_EQ(0, nearCacheRecordStore->size());
                        }

                        void clear_records_or_destroy_store(config::in_memory_format in_memory_format, bool destroy) {
                            auto nearCacheConfig = create_near_cache_config(
                                    DEFAULT_NEAR_CACHE_NAME, in_memory_format);
                            auto nearCacheRecordStore = create_near_cache_record_store(nearCacheConfig, in_memory_format);

                            for (int i = 0; i < DEFAULT_RECORD_COUNT; i++) {
                                std::shared_ptr<serialization::pimpl::data> key = get_shared_key(i);
                                nearCacheRecordStore->put(key, get_shared_value(i));

                                // ensure that they are stored
                                ASSERT_TRUE(nearCacheRecordStore->get(key));
                            }

                            if (destroy) {
                                nearCacheRecordStore->destroy();
                            } else {
                                nearCacheRecordStore->clear();
                            }

                            ASSERT_EQ(0, nearCacheRecordStore->size());
                        }

                        void stats_calculated(config::in_memory_format in_memory_format) {
                            int64_t creationStartTime = hazelcast::util::current_time_millis();
                            auto nearCacheConfig = create_near_cache_config(
                                    DEFAULT_NEAR_CACHE_NAME, in_memory_format);
                            auto nearCacheRecordStore = create_near_cache_record_store(nearCacheConfig, in_memory_format);
                            int64_t creationEndTime = hazelcast::util::current_time_millis();

                            int64_t expectedEntryCount = 0;
                            int64_t expectedHits = 0;
                            int64_t expectedMisses = 0;

                            for (int i = 0; i < DEFAULT_RECORD_COUNT; i++) {
                                nearCacheRecordStore->put(get_shared_key(i), get_shared_value(i));

                                expectedEntryCount++;
                            }

                            for (int i = 0; i < DEFAULT_RECORD_COUNT; i++) {
                                int selectedKey = i * 3;
                                if (nearCacheRecordStore->get(get_shared_key(selectedKey)) != nullptr) {
                                    expectedHits++;
                                } else {
                                    expectedMisses++;
                                }
                            }

                            auto nearCacheStats = nearCacheRecordStore->get_near_cache_stats();

                            int64_t memoryCostWhenFull = nearCacheStats->get_owned_entry_memory_cost();
                            ASSERT_TRUE(nearCacheStats->get_creation_time() >= creationStartTime);
                            ASSERT_TRUE(nearCacheStats->get_creation_time() <= creationEndTime);
                            ASSERT_EQ(expectedHits, nearCacheStats->get_hits());
                            ASSERT_EQ(expectedMisses, nearCacheStats->get_misses());
                            ASSERT_EQ(expectedEntryCount, nearCacheStats->get_owned_entry_count());
                            switch (in_memory_format) {
                                case config::BINARY:
                                    ASSERT_TRUE(memoryCostWhenFull > 0);
                                    break;
                                case config::OBJECT:
                                    ASSERT_EQ(0, memoryCostWhenFull);
                            }

                            for (int i = 0; i < DEFAULT_RECORD_COUNT; i++) {
                                int selectedKey = i * 3;
                                if (nearCacheRecordStore->invalidate(get_shared_key(selectedKey))) {
                                    expectedEntryCount--;
                                }
                            }

                            ASSERT_EQ(expectedEntryCount, nearCacheStats->get_owned_entry_count());
                            switch (in_memory_format) {
                                case config::BINARY:
                                    ASSERT_TRUE(nearCacheStats->get_owned_entry_memory_cost() > 0);
                                    ASSERT_TRUE(nearCacheStats->get_owned_entry_memory_cost() < memoryCostWhenFull);
                                    break;
                                case config::OBJECT:
                                    ASSERT_EQ(0, nearCacheStats->get_owned_entry_memory_cost());
                                    break;
                            }

                            nearCacheRecordStore->clear();

                            switch (in_memory_format) {
                                case config::BINARY:
                                case config::OBJECT:
                                    ASSERT_EQ(0, nearCacheStats->get_owned_entry_memory_cost());
                                    break;
                            }
                        }

                        void ttl_evaluated(config::in_memory_format in_memory_format) {
                            int ttlSeconds = 3;

                            auto nearCacheConfig = create_near_cache_config(
                                    DEFAULT_NEAR_CACHE_NAME, in_memory_format);
                            nearCacheConfig.set_time_to_live_seconds(ttlSeconds);

                            auto nearCacheRecordStore = create_near_cache_record_store(nearCacheConfig, in_memory_format);

                            for (int i = 0; i < DEFAULT_RECORD_COUNT; i++) {
                                nearCacheRecordStore->put(get_shared_key(i), get_shared_value(i));
                            }

                            for (int i = 0; i < DEFAULT_RECORD_COUNT; i++) {
                                ASSERT_TRUE(nearCacheRecordStore->get(get_shared_key(i)));
                            }

                            hazelcast::util::sleep(ttlSeconds + 1);

                            for (int i = 0; i < DEFAULT_RECORD_COUNT; i++) {
                                ASSERT_FALSE(nearCacheRecordStore->get(get_shared_key(i)));
                            }
                        }

                        void max_idle_time_evaluated_successfully(config::in_memory_format in_memory_format) {
                            int maxIdleSeconds = 3;

                            auto nearCacheConfig = create_near_cache_config(
                                    DEFAULT_NEAR_CACHE_NAME, in_memory_format);
                            nearCacheConfig.set_max_idle_seconds(maxIdleSeconds);

                            auto nearCacheRecordStore = create_near_cache_record_store(nearCacheConfig, in_memory_format);

                            for (int i = 0; i < DEFAULT_RECORD_COUNT; i++) {
                                nearCacheRecordStore->put(get_shared_key(i), get_shared_value(i));
                            }

                            for (int i = 0; i < DEFAULT_RECORD_COUNT; i++) {
                                ASSERT_TRUE(nearCacheRecordStore->get(get_shared_key(i)));
                            }

                            hazelcast::util::sleep(maxIdleSeconds + 1);

                            for (int i = 0; i < DEFAULT_RECORD_COUNT; i++) {
                                ASSERT_FALSE(nearCacheRecordStore->get(get_shared_key(i)));
                            }
                        }

                        void expired_records_cleaned_up_successfully(config::in_memory_format in_memory_format,
                                                                 bool use_idle_time) {
                            int cleanUpThresholdSeconds = 3;

                            auto nearCacheConfig = create_near_cache_config(
                                    DEFAULT_NEAR_CACHE_NAME, in_memory_format);
                            if (use_idle_time) {
                                nearCacheConfig.set_max_idle_seconds(cleanUpThresholdSeconds);
                            } else {
                                nearCacheConfig.set_time_to_live_seconds(cleanUpThresholdSeconds);
                            }

                            auto nearCacheRecordStore = create_near_cache_record_store(nearCacheConfig, in_memory_format);

                            for (int i = 0; i < DEFAULT_RECORD_COUNT; i++) {
                                nearCacheRecordStore->put(get_shared_key(i), get_shared_value(i));
                            }

                            hazelcast::util::sleep(cleanUpThresholdSeconds + 1);

                            nearCacheRecordStore->do_expiration();

                            ASSERT_EQ(0, nearCacheRecordStore->size());

                            auto nearCacheStats = nearCacheRecordStore->get_near_cache_stats();
                            ASSERT_EQ(0, nearCacheStats->get_owned_entry_count());
                            ASSERT_EQ(0, nearCacheStats->get_owned_entry_memory_cost());
                        }

                        void create_near_cache_with_max_size_policy(config::in_memory_format in_memory_format,
                                                              config::eviction_config::max_size_policy max_size_policy,
                                                              int32_t size) {
                            auto nearCacheConfig = create_near_cache_config(
                                    DEFAULT_NEAR_CACHE_NAME, in_memory_format);
                            config::eviction_config evictionConfig;
                            evictionConfig.set_maximum_size_policy(max_size_policy);
                            evictionConfig.set_size(size);
                            nearCacheConfig.set_eviction_config(evictionConfig);

                            auto nearCacheRecordStore = create_near_cache_record_store(nearCacheConfig, in_memory_format);
                        }

                        void do_eviction_with_entry_count_max_size_policy(config::in_memory_format in_memory_format,
                                                                   config::eviction_policy eviction_policy) {
                            int32_t maxSize = DEFAULT_RECORD_COUNT / 2;

                            auto nearCacheConfig = create_near_cache_config(
                                    DEFAULT_NEAR_CACHE_NAME, in_memory_format);

                            config::eviction_config evictionConfig;
                            evictionConfig.set_maximum_size_policy(config::eviction_config::ENTRY_COUNT);
                            evictionConfig.set_size(maxSize);
                            evictionConfig.set_eviction_policy(eviction_policy);
                            nearCacheConfig.set_eviction_config(evictionConfig);

                            auto nearCacheRecordStore = create_near_cache_record_store(nearCacheConfig, in_memory_format);

                            for (int i = 0; i < DEFAULT_RECORD_COUNT; i++) {
                                nearCacheRecordStore->put(get_shared_key(i), get_shared_value(i));
                                nearCacheRecordStore->do_eviction_if_required();
                                ASSERT_TRUE(maxSize >= nearCacheRecordStore->size());
                            }
                        }

                        std::unique_ptr<hazelcast::client::internal::nearcache::impl::NearCacheRecordStore<serialization::pimpl::data, serialization::pimpl::data> > create_near_cache_record_store(
                                config::near_cache_config &near_cache_config,
                                config::in_memory_format in_memory_format) {
                            std::unique_ptr<hazelcast::client::internal::nearcache::impl::NearCacheRecordStore<serialization::pimpl::data, serialization::pimpl::data> > recordStore;
                            switch (in_memory_format) {
                                case config::BINARY:
                                    recordStore = std::unique_ptr<hazelcast::client::internal::nearcache::impl::NearCacheRecordStore<serialization::pimpl::data, serialization::pimpl::data> >(
                                            new hazelcast::client::internal::nearcache::impl::store::NearCacheDataRecordStore<serialization::pimpl::data, serialization::pimpl::data, serialization::pimpl::data>(
                                                    DEFAULT_NEAR_CACHE_NAME, near_cache_config, *ss_));
                                    break;
                                case config::OBJECT:
                                    recordStore = std::unique_ptr<hazelcast::client::internal::nearcache::impl::NearCacheRecordStore<serialization::pimpl::data, serialization::pimpl::data> >(
                                            new hazelcast::client::internal::nearcache::impl::store::NearCacheObjectRecordStore<serialization::pimpl::data, serialization::pimpl::data, serialization::pimpl::data>(
                                                    DEFAULT_NEAR_CACHE_NAME,
                                                    near_cache_config, *ss_));
                                    break;
                                default:
                                    std::ostringstream out;
                                    out << "Unsupported in-memory format: " << in_memory_format;
                                    BOOST_THROW_EXCEPTION(
                                            exception::illegal_argument("NearCacheRecordStoreTest", out.str()));
                            }
                            recordStore->initialize();

                            return recordStore;
                        }

                        config::near_cache_config
                        create_near_cache_config(const char *name,
                                              config::in_memory_format in_memory_format) {
                            config::near_cache_config config;
                            config.set_name(name).set_in_memory_format(in_memory_format);
                            return config;
                        }

                        std::shared_ptr<serialization::pimpl::data> get_shared_value(int value) const {
                            char buf[30];
                            hazelcast::util::hz_snprintf(buf, 30, "Record-%ld", value);
                            return ss_->to_shared_data(new std::string(buf));
                        }

                        std::shared_ptr<serialization::pimpl::data> get_shared_key(int value) {
                            return ss_->to_shared_data<int>(&value);
                        }

                        std::unique_ptr<serialization::pimpl::SerializationService> ss_;
                        serialization_config serialization_config_;
                    };

                    const int NearCacheRecordStoreTest::DEFAULT_RECORD_COUNT = 100;
                    const char *NearCacheRecordStoreTest::DEFAULT_NEAR_CACHE_NAME = "TestNearCache";

                    TEST_P(NearCacheRecordStoreTest, putAndGetRecord) {
                        put_and_get_record(GetParam());
                    }

                    TEST_P(NearCacheRecordStoreTest, putAndRemoveRecord) {
                        put_and_remove_record(GetParam());
                    }

                    TEST_P(NearCacheRecordStoreTest, clearRecords) {
                        clear_records_or_destroy_store(GetParam(), false);
                    }

                    TEST_P(NearCacheRecordStoreTest, destroyStore) {
                        clear_records_or_destroy_store(GetParam(), true);
                    }

                    TEST_P(NearCacheRecordStoreTest, statsCalculated) {
                        stats_calculated(GetParam());
                    }

                    TEST_P(NearCacheRecordStoreTest, ttlEvaluated) {
                        ttl_evaluated(GetParam());
                    }

                    TEST_P(NearCacheRecordStoreTest, maxIdleTimeEvaluatedSuccessfully) {
                        max_idle_time_evaluated_successfully(GetParam());
                    }

                    TEST_P(NearCacheRecordStoreTest, expiredRecordsCleanedUpSuccessfullyBecauseOfTTL) {
                        expired_records_cleaned_up_successfully(GetParam(), false);
                    }

                    TEST_P(NearCacheRecordStoreTest, expiredRecordsCleanedUpSuccessfullyBecauseOfIdleTime) {
                        expired_records_cleaned_up_successfully(GetParam(), true);
                    }

                    TEST_P(NearCacheRecordStoreTest, canCreateWithEntryCountMaxSizePolicy) {
                        create_near_cache_with_max_size_policy(GetParam(),
                                                         config::eviction_config::ENTRY_COUNT,
                                                         1000);
                    }

                    TEST_P(NearCacheRecordStoreTest,
                           evictionTriggeredAndHandledSuccessfullyWithEntryCountMaxSizePolicyAndLRUEvictionPolicy) {
                        do_eviction_with_entry_count_max_size_policy(GetParam(), config::LRU);
                    }

                    TEST_P(NearCacheRecordStoreTest,
                           evictionTriggeredAndHandledSuccessfullyWithEntryCountMaxSizePolicyAndLFUEvictionPolicy) {
                        do_eviction_with_entry_count_max_size_policy(GetParam(), config::LFU);
                    }

                    TEST_P(NearCacheRecordStoreTest,
                           evictionTriggeredAndHandledSuccessfullyWithEntryCountMaxSizePolicyAndRandomEvictionPolicy) {
                        do_eviction_with_entry_count_max_size_policy(GetParam(), config::RANDOM);
                    }

                    INSTANTIATE_TEST_SUITE_P(BasicStoreTest, NearCacheRecordStoreTest,
                                             ::testing::Values(config::BINARY, config::OBJECT));

                }
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

