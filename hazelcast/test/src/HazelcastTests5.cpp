/*
 * Copyright (c) 2008-2021, Hazelcast, Inc. All Rights Reserved.
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
#include <cmath>
#include <fstream>
#include <memory>
#include <ostream>
#include <regex>
#include <string>
#include <vector>

#include <boost/asio.hpp>

#include <gtest/gtest.h>

#ifdef HZ_BUILD_WITH_SSL
#include <openssl/crypto.h>
#endif

#include <hazelcast/client/client_config.h>
#include <hazelcast/client/client_properties.h>
#include <hazelcast/client/connection/ClientConnectionManagerImpl.h>
#include <hazelcast/client/entry_event.h>
#include <hazelcast/client/entry_listener.h>
#include <hazelcast/client/exception/protocol_exceptions.h>
#include <hazelcast/client/hazelcast_client.h>
#include <hazelcast/client/hazelcast_json_value.h>
#include <hazelcast/client/imap.h>
#include <hazelcast/client/impl/Partition.h>
#include <hazelcast/client/initial_membership_event.h>
#include <hazelcast/client/internal/nearcache/impl/store/NearCacheObjectRecordStore.h>
#include <hazelcast/client/internal/socket/SSLSocket.h>
#include <hazelcast/client/multi_map.h>
#include <hazelcast/client/pipelining.h>
#include <hazelcast/client/proxy/PNCounterImpl.h>
#include <hazelcast/client/query/predicates.h>
#include <hazelcast/client/serialization/serialization.h>
#include <hazelcast/client/spi/impl/sequence/CallIdSequenceWithBackpressure.h>
#include <hazelcast/client/spi/impl/sequence/CallIdSequenceWithoutBackpressure.h>
#include <hazelcast/client/spi/impl/sequence/FailFastCallIdSequence.h>
#include <hazelcast/util/AddressHelper.h>
#include <hazelcast/util/MurmurHash3.h>
#include <hazelcast/util/Util.h>

#include "ClientTestSupport.h"
#include "ClientTestSupportBase.h"
#include "HazelcastServer.h"
#include "HazelcastServerFactory.h"
#include "serialization/Serializables.h"
#include "TestHelperFunctions.h"


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4996) //for unsafe getenv
#endif

namespace hazelcast {
    namespace client {

        namespace test {
            class CallIdSequenceWithoutBackpressureTest : public ClientTestSupport {
            protected:
                spi::impl::sequence::CallIdSequenceWithoutBackpressure sequence_;

                void next(bool is_urgent) {
                    int64_t oldSequence = sequence_.get_last_call_id();
                    int64_t result = next_call_id(sequence_, is_urgent);
                    ASSERT_EQ(oldSequence + 1, result);
                    ASSERT_EQ(oldSequence + 1, sequence_.get_last_call_id());
                }

                int64_t next_call_id(spi::impl::sequence::CallIdSequence &seq, bool is_urgent) {
                    return is_urgent ? seq.force_next() : seq.next();
                }
            };

            TEST_F(CallIdSequenceWithoutBackpressureTest, testInit) {
                ASSERT_EQ(0, sequence_.get_last_call_id());
                ASSERT_EQ(INT32_MAX, sequence_.get_max_concurrent_invocations());
            }

            TEST_F(CallIdSequenceWithoutBackpressureTest, testNext) {
                // regular operation
                next(false);
                next(true);
            }

            TEST_F(CallIdSequenceWithoutBackpressureTest, whenNextRepeated_thenKeepSucceeding) {
                for (int64_t k = 1; k < 10000; k++) {
                    ASSERT_EQ(k, next_call_id(sequence_, false));
                }
            }

            TEST_F(CallIdSequenceWithoutBackpressureTest, complete) {
                next_call_id(sequence_, false);
                int64_t oldSequence = sequence_.get_last_call_id();
                sequence_.complete();
                ASSERT_EQ(oldSequence, sequence_.get_last_call_id());
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
                                                      boost::latch &next_called_latch) : sequence_(
                            sequence),
                                                                                       next_called_latch_(
                                                                                               next_called_latch) {}

                    virtual const std::string get_name() const {
                        return "ThreeSecondDelayCompleteOperation";
                    }

                    virtual void run() {
                        sequence_.next();
                        next_called_latch_.count_down();
                        sleep_seconds(3);
                        sequence_.complete();
                    }

                private:
                    spi::impl::sequence::CallIdSequenceWithBackpressure &sequence_;
                    boost::latch &next_called_latch_;
                };

                int64_t next_call_id(spi::impl::sequence::CallIdSequence &seq, bool is_urgent) {
                    return is_urgent ? seq.force_next() : seq.next();
                }
            };

            TEST_F(CallIdSequenceWithBackpressureTest, testInit) {
                spi::impl::sequence::CallIdSequenceWithBackpressure sequence(100, 60000);
                ASSERT_EQ(0, sequence.get_last_call_id());
                ASSERT_EQ(100, sequence.get_max_concurrent_invocations());
            }

            TEST_F(CallIdSequenceWithBackpressureTest, whenNext_thenSequenceIncrements) {
                spi::impl::sequence::CallIdSequenceWithBackpressure sequence(100, 60000);
                int64_t oldSequence = sequence.get_last_call_id();
                int64_t result = sequence.next();
                ASSERT_EQ(oldSequence + 1, result);
                ASSERT_EQ(oldSequence + 1, sequence.get_last_call_id());

                oldSequence = sequence.get_last_call_id();
                result = sequence.force_next();
                ASSERT_EQ(oldSequence + 1, result);
                ASSERT_EQ(oldSequence + 1, sequence.get_last_call_id());
            }

            TEST_F(CallIdSequenceWithBackpressureTest, next_whenNoCapacity_thenBlockTillCapacity) {
                spi::impl::sequence::CallIdSequenceWithBackpressure sequence(1, 60000);
                int64_t oldLastCallId = sequence.get_last_call_id();

                boost::latch nextCalledLatch(1);

                auto f = std::async(std::packaged_task<void()>(
                        [&]() { ThreeSecondDelayCompleteOperation(sequence, nextCalledLatch).run(); }));

                ASSERT_OPEN_EVENTUALLY(nextCalledLatch);

                int64_t result = sequence.next();
                ASSERT_EQ(oldLastCallId + 2, result);
                ASSERT_EQ(oldLastCallId + 2, sequence.get_last_call_id());
                f.get();
            }

            TEST_F(CallIdSequenceWithBackpressureTest, next_whenNoCapacity_thenBlockTillTimeout) {
                spi::impl::sequence::CallIdSequenceWithBackpressure sequence(1, 2000);
// first invocation consumes the available call ID
                next_call_id(sequence, false);

                int64_t oldLastCallId = sequence.get_last_call_id();
                ASSERT_THROW(sequence.next(), exception::hazelcast_overload);

                ASSERT_EQ(oldLastCallId, sequence.get_last_call_id());
            }

            TEST_F(CallIdSequenceWithBackpressureTest, when_overCapacityButPriorityItem_then_noBackpressure) {
                spi::impl::sequence::CallIdSequenceWithBackpressure sequence(1, 60000);

// occupy the single call ID slot
                next_call_id(sequence, true);

                int64_t oldLastCallId = sequence.get_last_call_id();

                int64_t result = next_call_id(sequence, true);
                ASSERT_EQ(oldLastCallId + 1, result);
                ASSERT_EQ(oldLastCallId + 1, sequence.get_last_call_id());
            }

            TEST_F(CallIdSequenceWithBackpressureTest, whenComplete_thenTailIncrements) {
                spi::impl::sequence::CallIdSequenceWithBackpressure sequence(1, 60000);

                next_call_id(sequence, false);

                int64_t oldSequence = sequence.get_last_call_id();
                int64_t oldTail = sequence.get_tail();
                sequence.complete();

                ASSERT_EQ(oldSequence, sequence.get_last_call_id());
                ASSERT_EQ(oldTail + 1, sequence.get_tail());
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
                ASSERT_EQ(0, sequence.get_last_call_id());
                ASSERT_EQ(100, sequence.get_max_concurrent_invocations());
            }

            TEST_F(FailFastCallIdSequenceTest, whenNext_thenSequenceIncrements) {
                spi::impl::sequence::FailFastCallIdSequence sequence(100);
                int64_t oldSequence = sequence.get_last_call_id();
                int64_t result = sequence.next();
                ASSERT_EQ(oldSequence + 1, result);
                ASSERT_EQ(oldSequence + 1, sequence.get_last_call_id());
            }

            TEST_F(FailFastCallIdSequenceTest, next_whenNoCapacity_thenThrowException) {
                spi::impl::sequence::FailFastCallIdSequence sequence(1);

                // take the only slot available
                sequence.next();

                // this next is going to fail with an exception
                ASSERT_THROW(sequence.next(), exception::hazelcast_overload);
            }

            TEST_F(FailFastCallIdSequenceTest, when_overCapacityButPriorityItem_then_noException) {
                spi::impl::sequence::FailFastCallIdSequence sequence(1);

// take the only slot available
                ASSERT_EQ(1, sequence.next());

                ASSERT_EQ(2, sequence.force_next());
            }

            TEST_F(FailFastCallIdSequenceTest, whenComplete_thenTailIncrements) {
                spi::impl::sequence::FailFastCallIdSequence sequence(100);
                sequence.next();

                int64_t oldSequence = sequence.get_last_call_id();
                int64_t oldTail = sequence.get_tail();
                sequence.complete();

                ASSERT_EQ(oldSequence, sequence.get_last_call_id());
                ASSERT_EQ(oldTail + 1, sequence.get_tail());
            }

        }
    }
}

namespace hazelcast {
    namespace client {
        namespace test {
            extern HazelcastServerFactory *g_srvFactory;
            extern std::shared_ptr<RemoteControllerClient> remoteController;

            HazelcastServerFactory::HazelcastServerFactory(const std::string &server_xml_config_file_path)
                    : HazelcastServerFactory::HazelcastServerFactory(g_srvFactory->get_server_address(),
                                                                     server_xml_config_file_path) {
            }

            HazelcastServerFactory::HazelcastServerFactory(const std::string &server_address,
                                                           const std::string &server_xml_config_file_path)
                    : logger_(std::make_shared<logger>("HazelcastServerFactory", "HazelcastServerFactory",
                                                       logger::level::info, logger::default_handler)),
                      server_address_(server_address) {
                std::string xmlConfig = read_from_xml_file(server_xml_config_file_path);

                remote::Cluster cluster;
                remoteController->createClusterKeepClusterName(cluster, HAZELCAST_VERSION, xmlConfig);

                this->cluster_id_ = cluster.id;
            }

            HazelcastServerFactory::~HazelcastServerFactory() {
                remoteController->shutdownCluster(cluster_id_);
            }

            remote::Member HazelcastServerFactory::start_server() {
                remote::Member member;
                remoteController->startMember(member, cluster_id_);
                return member;
            }

            bool HazelcastServerFactory::shutdown_server(const remote::Member &member) {
                return remoteController->shutdownMember(cluster_id_, member.uuid);
            }

            bool HazelcastServerFactory::terminate_server(const remote::Member &member) {
                return remoteController->terminateMember(cluster_id_, member.uuid);
            }

            const std::string &HazelcastServerFactory::get_server_address() {
                return server_address_;
            }

            std::string HazelcastServerFactory::read_from_xml_file(const std::string &xml_file_path) {
                std::ifstream xmlFile(xml_file_path.c_str());
                if (!xmlFile) {
                    std::ostringstream out;
                    out << "Failed to read from xml file to at " << xml_file_path;
                    BOOST_THROW_EXCEPTION(
                            exception::illegal_state("HazelcastServerFactory::readFromXmlFile", out.str()));
                }

                std::ostringstream buffer;

                buffer << xmlFile.rdbuf();

                xmlFile.close();

                return buffer.str();
            }

            const std::string &HazelcastServerFactory::get_cluster_id() const {
                return cluster_id_;
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

                    int get_value() const {
                        return value_;
                    }

                private:
                    int value_;
                };

                class WriteReadIntGlobalSerializer : public serialization::global_serializer {
                public:
                    void write(const boost::any &object, serialization::object_data_output &out) override {
                        auto const &obj = boost::any_cast<UnknownObject>(object);
                        out.write<int32_t>(obj.get_value());
                    }

                    boost::any read(serialization::object_data_input &in) override {
                        return boost::any(UnknownObject(in.read<int32_t>()));
                    }
                };
            protected:

                static void SetUpTestCase() {
                    instance = new HazelcastServer(*g_srvFactory);
                    client_config clientConfig = get_config();
                    clientConfig.get_serialization_config().set_global_serializer(
                            std::make_shared<WriteReadIntGlobalSerializer>());
                    client = new hazelcast_client{new_client(std::move(clientConfig)).get()};
                    unknown_object_map = client->get_map("UnknownObject").get();
                }

                static void TearDownTestCase() {
                    delete client;
                    delete instance;

                    unknown_object_map = nullptr;
                    client = nullptr;
                    instance = nullptr;
                }

                static HazelcastServer *instance;
                static hazelcast_client *client;
                static std::shared_ptr<imap> unknown_object_map;
            };

            HazelcastServer *MapGlobalSerializerTest::instance = nullptr;
            hazelcast_client *MapGlobalSerializerTest::client = nullptr;
            std::shared_ptr<imap> MapGlobalSerializerTest::unknown_object_map;

            TEST_F(MapGlobalSerializerTest, testPutGetUnserializableObject) {
                MapGlobalSerializerTest::UnknownObject myObject(8);
                unknown_object_map->put<int, MapGlobalSerializerTest::UnknownObject>(2, myObject).get();

                boost::optional<MapGlobalSerializerTest::UnknownObject> data =
                        unknown_object_map->get<int, MapGlobalSerializerTest::UnknownObject>(2).get();
                ASSERT_TRUE(data.has_value());
                ASSERT_EQ(8, data.value().get_value());
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
                    int_map->clear().get();
                }

                static void SetUpTestCase() {
                    instance = new HazelcastServer(*g_srvFactory);
                    instance2 = new HazelcastServer(*g_srvFactory);
                    client = new hazelcast_client{new_client(get_config()).get()};
                    int_map = client->get_map("IntMap").get();
                }

                static void TearDownTestCase() {
                    delete client;
                    delete instance2;
                    delete instance;

                    int_map = nullptr;
                    client = nullptr;
                    instance2 = nullptr;
                    instance = nullptr;
                }

                static HazelcastServer *instance;
                static HazelcastServer *instance2;
                static hazelcast_client *client;
                static std::shared_ptr<imap> int_map;
            };

            HazelcastServer *ClientExpirationListenerTest::instance = nullptr;
            HazelcastServer *ClientExpirationListenerTest::instance2 = nullptr;
            hazelcast_client *ClientExpirationListenerTest::client = nullptr;
            std::shared_ptr<imap> ClientExpirationListenerTest::int_map = nullptr;

            TEST_F(ClientExpirationListenerTest, notified_afterExpirationOfEntries) {
                int numberOfPutOperations = 10000;
                boost::latch expirationEventArrivalCount(numberOfPutOperations);

                entry_listener expirationListener;

                expirationListener.
                    on_expired([&expirationEventArrivalCount](entry_event &&) {
                        expirationEventArrivalCount.count_down();
                    });

                auto registrationId = int_map->add_entry_listener(std::move(expirationListener), true).get();

                for (int i = 0; i < numberOfPutOperations; i++) {
                    int_map->put<int, int>(i, i, std::chrono::milliseconds(100)).get();
                }

                // wait expiration of entries.
                std::this_thread::sleep_for(std::chrono::seconds(1));

                // trigger immediate fire of expiration events by touching them.
                for (int i = 0; i < numberOfPutOperations; ++i) {
                    int_map->get<int, int>(i).get();
                }

                ASSERT_OPEN_EVENTUALLY(expirationEventArrivalCount);
                ASSERT_TRUE(int_map->remove_entry_listener(registrationId).get());
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
                    client_config clientConfig(get_config());
                    clientConfig.set_property(client_properties::PROP_HEARTBEAT_TIMEOUT, "20");
                    client = new hazelcast_client{new_client(std::move(clientConfig)).get()};
                    map = client->get_map("map").get();
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
                static hazelcast_client *client;
                static std::shared_ptr<imap> map;
            };

            HazelcastServer *ClientListenerFunctionObjectsTest::instance = nullptr;
            hazelcast_client *ClientListenerFunctionObjectsTest::client = nullptr;
            std::shared_ptr<imap> ClientListenerFunctionObjectsTest::map = nullptr;

            TEST_F(ClientListenerFunctionObjectsTest, lambda) {
                boost::latch called {1};

                auto id = map->add_entry_listener(
                        entry_listener().
                        on_added([&called](entry_event &&e) {
                            ASSERT_EQ(1, e.get_key().get<int>().get());
                            ASSERT_EQ(2, e.get_value().get<int>().get());
                            called.count_down();
                        })
                , true).get();

                map->put(1, 2).get();

                ASSERT_OPEN_EVENTUALLY(called);

                map->remove_entry_listener(id).get();
            }

            TEST_F(ClientListenerFunctionObjectsTest, functor) {
                struct OnEntryAdded {
                    boost::latch& latch;

                    void operator()(entry_event &&e) {
                        ASSERT_EQ(1, e.get_key().get<int>().get());
                        ASSERT_EQ(2, e.get_value().get<int>().get());
                        latch.count_down();
                    }
                };

                boost::latch called {1};
                OnEntryAdded handler {called};

                auto id = map->add_entry_listener(
                        entry_listener().on_added(std::move(handler))
                , true).get();

                map->put(1, 2).get();

                ASSERT_OPEN_EVENTUALLY(called);

                map->remove_entry_listener(id).get();
            }

            TEST_F(ClientListenerFunctionObjectsTest, staticFunction) {
                static boost::latch called {1};

                struct OnEntryAdded {
                    static void handler(entry_event &&e) {
                        ASSERT_EQ(1, e.get_key().get<int>().get());
                        ASSERT_EQ(2, e.get_value().get<int>().get());
                        called.count_down();
                    }
                };

                auto id = map->add_entry_listener(
                        entry_listener().on_added(&OnEntryAdded::handler)
                , true).get();

                map->put(1, 2).get();

                ASSERT_OPEN_EVENTUALLY(called);

                map->remove_entry_listener(id).get();
            }

            TEST_F(ClientListenerFunctionObjectsTest, bind) {
                boost::latch called {1};

                struct MyListener {
                    boost::latch &latch;

                    void added(entry_event &&e) {
                        ASSERT_EQ(1, e.get_key().get<int>().get());
                        ASSERT_EQ(2, e.get_value().get<int>().get());
                        latch.count_down();
                    }
                };

                MyListener listener {called};
                auto handler = std::bind(&MyListener::added, &listener, std::placeholders::_1);

                auto id = map->add_entry_listener(
                        entry_listener().on_added(std::move(handler))
                , true).get();

                map->put(1, 2).get();

                ASSERT_OPEN_EVENTUALLY(called);

                map->remove_entry_listener(id).get();
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace test {
            class PartitionAwareInt : public partition_aware<int> {
            public:
                PartitionAwareInt() : partition_key_(0), actual_key_(0) {}

                PartitionAwareInt(int partition_key, int actual_key)
                        : partition_key_(partition_key), actual_key_(actual_key) {}

                const int *get_partition_key() const override {
                    return &partition_key_;
                }

                int get_actual_key() const {
                    return actual_key_;
                }
            private:
                int partition_key_;
                int actual_key_;
            };

            class ClientMapTest : public ClientTestSupport,
                                  public ::testing::WithParamInterface<std::function<client_config()>> {
            public:
                static constexpr const char *intMapName = "IntMap";
                static constexpr const char *employeesMapName = "EmployeesMap";
                static constexpr const char *imapName = "clientMapTest";
                static constexpr const char *ONE_SECOND_MAP_NAME = "OneSecondTtlMap";

                static client_config create_map_client_config() {
                    client_config config;
                    config.get_network_config().add_address(address(g_srvFactory->get_server_address(), 5701));
                    return config;
                }

                static client_config create_near_cached_map_client_config() {
                    client_config config = create_map_client_config();

                    config.add_near_cache_config(config::near_cache_config(intMapName));
                    config.add_near_cache_config(config::near_cache_config(employeesMapName));
                    config.add_near_cache_config(config::near_cache_config(imapName));
                    config.add_near_cache_config(config::near_cache_config(ONE_SECOND_MAP_NAME));

                    return config;
                }

                static client_config create_near_cached_object_map_client_config() {
                    client_config config = create_map_client_config();

                    config.add_near_cache_config(config::near_cache_config(intMapName, config::OBJECT));
                    config.add_near_cache_config(config::near_cache_config(employeesMapName, config::OBJECT));
                    config.add_near_cache_config(config::near_cache_config(imapName, config::OBJECT));
                    config.add_near_cache_config(config::near_cache_config(ONE_SECOND_MAP_NAME, config::OBJECT));

                    return config;
                }

                ClientMapTest() : client_(new_client(GetParam()()).get()),
                                  imap_(client_.get_map(imapName).get()),
                                  int_map_(client_.get_map(intMapName).get()),
                                  employees_(client_.get_map(employeesMapName).get()) {
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
                    EntryMultiplier(int multiplier) : multiplier_(multiplier) {}

                    int get_multiplier() const {
                        return multiplier_;
                    }
                private:
                    int multiplier_;
                };
            protected:
                void SetUp() override {
                    spi::ClientContext context(client_);
                    ASSERT_EQ_EVENTUALLY(2, context.get_connection_manager().get_active_connections().size());
                }

                void TearDown() override {
                    // clear maps
                    employees_->destroy().get();
                    int_map_->destroy().get();
                    imap_->destroy().get();
                    client_.get_map(ONE_SECOND_MAP_NAME).get()->destroy().get();
                    client_.get_map("tradeMap").get()->destroy().get();
                }

                void fill_map() {
                    for (int i = 0; i < 10; i++) {
                        std::string key = "key";
                        key += std::to_string(i);
                        std::string value = "value";
                        value += std::to_string(i);
                        imap_->put(key, value).get();
                    }
                }

                static void try_put_thread(hazelcast::util::ThreadArgs &args) {
                    auto *latch1 = (boost::latch *) args.arg0;
                    imap *pMap = (imap *) args.arg1;
                    bool result = pMap->try_put("key1", "value3", std::chrono::seconds(1)).get();
                    if (!result) {
                        latch1->count_down();
                    }
                }

                static void try_remove_thread(hazelcast::util::ThreadArgs &args) {
                    auto *latch1 = (boost::latch *) args.arg0;
                    imap *pMap = (imap *) args.arg1;
                    bool result = pMap->try_remove("key2", std::chrono::seconds(1)).get();
                    if (!result) {
                        latch1->count_down();
                    }
                }

                static void test_lock_thread(hazelcast::util::ThreadArgs &args) {
                    auto *latch1 = (boost::latch *) args.arg0;
                    imap *pMap = (imap *) args.arg1;
                    pMap->try_put("key1", "value2", std::chrono::milliseconds(1)).get();
                    latch1->count_down();
                }

                static void test_lock_ttl_thread(hazelcast::util::ThreadArgs &args) {
                    auto *latch1 = (boost::latch *) args.arg0;
                    imap *pMap = (imap *) args.arg1;
                    pMap->try_put("key1", "value2", std::chrono::seconds(5)).get();
                    latch1->count_down();
                }

                static void test_lock_tt_l2_thread(hazelcast::util::ThreadArgs &args) {
                    auto *latch1 = (boost::latch *) args.arg0;
                    imap *pMap = (imap *) args.arg1;
                    if (!pMap->try_lock("key1").get()) {
                        latch1->count_down();
                    }
                    if (pMap->try_lock("key1", std::chrono::seconds(5)).get()) {
                        latch1->count_down();
                    }
                }

                static void test_map_try_lock_thread1(hazelcast::util::ThreadArgs &args) {
                    auto *latch1 = (boost::latch *) args.arg0;
                    imap *pMap = (imap *) args.arg1;
                    if (!pMap->try_lock("key1", std::chrono::milliseconds(2)).get()) {
                        latch1->count_down();
                    }
                }

                static void test_map_try_lock_thread2(hazelcast::util::ThreadArgs &args) {
                    auto *latch1 = (boost::latch *) args.arg0;
                    imap *pMap = (imap *) args.arg1;
                    if (pMap->try_lock("key1", std::chrono::seconds(20)).get()) {
                        latch1->count_down();
                    }
                }

                static void test_map_force_unlock_thread(hazelcast::util::ThreadArgs &args) {
                    auto *latch1 = (boost::latch *) args.arg0;
                    imap *pMap = (imap *) args.arg1;
                    pMap->force_unlock("key1").get();
                    latch1->count_down();
                }

                entry_listener make_countdown_listener(boost::latch &add_latch,
                                                       boost::latch &remove_latch,
                                                       boost::latch &update_latch,
                                                       boost::latch &expiry_latch,
                                                       boost::latch *evict_latch = nullptr) {
                    return entry_listener().
                        on_added([&add_latch](entry_event &&) {
                            add_latch.count_down();
                        }).
                        on_removed([&remove_latch](entry_event &&) {
                            remove_latch.count_down();
                        }).
                        on_updated([&update_latch](entry_event &&) {
                            update_latch.count_down();
                        }).
                        on_expired([&expiry_latch](entry_event &&) {
                            expiry_latch.count_down();
                        }).
                        on_evicted([evict_latch](entry_event &&) {
                            if (!evict_latch) {
                                return;
                            }
                            evict_latch->count_down();
                        });
                }

                void validate_expiry_invalidations(std::shared_ptr<imap> map, std::function<void()> f) {
                    boost::latch dummy(10), expiry(1);
                    auto id = map->add_entry_listener(make_countdown_listener(dummy, dummy, dummy, expiry), false).get();

                    auto nearCacheStatsImpl = std::static_pointer_cast<monitor::impl::NearCacheStatsImpl>(
                            map->get_local_map_stats().get_near_cache_stats());

                    int64_t initialInvalidationRequests = 0;
                    if (nearCacheStatsImpl) {
                        initialInvalidationRequests = nearCacheStatsImpl->get_invalidation_requests();
                    }

                    f();

                    // if near cache is enabled
                    if (nearCacheStatsImpl) {
                        ASSERT_TRUE_EVENTUALLY(
                                initialInvalidationRequests < nearCacheStatsImpl->get_invalidation_requests());
                        // When ttl expires at server, the server sends another invalidation.
                        ASSERT_EQ_EVENTUALLY(
                                initialInvalidationRequests + 2, nearCacheStatsImpl->get_invalidation_requests());
                    } else {
                        // trigger eviction
                        ASSERT_TRUE_EVENTUALLY((!map->get<std::string, std::string>("key1").get().has_value()));
                    }

                    ASSERT_OPEN_EVENTUALLY(expiry);
                    ASSERT_TRUE(map->remove_entry_listener(id).get());
                }

                hazelcast_client client_;
                std::shared_ptr<imap> imap_;
                std::shared_ptr<imap> int_map_;
                std::shared_ptr<imap> employees_;

                static HazelcastServer *instance;
                static HazelcastServer *instance2;
            };

            HazelcastServer *ClientMapTest::instance = nullptr;
            HazelcastServer *ClientMapTest::instance2 = nullptr;

            INSTANTIATE_TEST_SUITE_P(ClientMapTestWithDifferentConfigs, ClientMapTest,
                                     ::testing::Values(ClientMapTest::create_map_client_config,
                                                       ClientMapTest::create_near_cached_map_client_config,
                                                       ClientMapTest::create_near_cached_object_map_client_config));

            TEST_P(ClientMapTest, testIssue537) {
                boost::latch latch1(2);
                boost::latch nullLatch(1);

                entry_listener listener;

                listener.
                    on_added([&latch1](entry_event &&) {
                        latch1.count_down();
                    }).
                    on_expired([&latch1, &nullLatch](entry_event &&event) {
                        auto oldValue = event.get_old_value().get<std::string>();
                        if (!oldValue.has_value() || oldValue.value().compare("")) {
                            nullLatch.count_down();
                        }
                        latch1.count_down();
                    });

                auto id = imap_->add_entry_listener(std::move(listener), true).get();

                imap_->put<std::string, std::string>("key1", "value1", std::chrono::seconds(2)).get();

                ASSERT_OPEN_EVENTUALLY(latch1);
                ASSERT_OPEN_EVENTUALLY(nullLatch);

                ASSERT_TRUE(imap_->remove_entry_listener(id).get());

                imap_->put<std::string, std::string>("key2", "value2").get();
                ASSERT_EQ(1, imap_->size().get());
            }

            TEST_P(ClientMapTest, testContains) {
                fill_map();

                ASSERT_FALSE(imap_->contains_key("key10").get());
                ASSERT_TRUE(imap_->contains_key("key1").get());

                ASSERT_FALSE(imap_->contains_value("value10").get());
                ASSERT_TRUE(imap_->contains_value("value1").get());
            }

            TEST_P(ClientMapTest, testGet) {
                fill_map();
                for (int i = 0; i < 10; i++) {
                    std::string key = "key";
                    key += std::to_string(i);
                    boost::optional<std::string> temp = imap_->get<std::string, std::string>(key).get();
                    ASSERT_TRUE(temp.has_value());
                    std::string value = "value";
                    value += std::to_string(i);
                    ASSERT_EQ(temp.value(), value);
                }
            }

            TEST_P(ClientMapTest, testPartitionAwareKey) {
                int partitionKey = 5;
                int value = 25;
                std::shared_ptr<imap> map = client_.get_map(intMapName).get();
                PartitionAwareInt partitionAwareInt(partitionKey, 7);
                map->put(partitionAwareInt, value).get();
                boost::optional<int> val = map->get<PartitionAwareInt, int>(partitionAwareInt).get();
                ASSERT_TRUE(val.has_value());
                ASSERT_EQ(*val, value);
            }

            TEST_P(ClientMapTest, testRemoveAndDelete) {
                fill_map();
                boost::optional<std::string> temp = imap_->remove<std::string, std::string>("key10").get();
                ASSERT_FALSE(temp.has_value());
                imap_->delete_entry("key9").get();
                ASSERT_EQ(imap_->size().get(), 9);
                for (int i = 0; i < 9; i++) {
                    std::string key = "key";
                    key += std::to_string(i);
                    boost::optional<std::string> temp2 = imap_->remove<std::string, std::string>(key).get();
                    std::string value = "value";
                    value += std::to_string(i);
                    ASSERT_EQ(*temp2, value);
                }
                ASSERT_EQ(imap_->size().get(), 0);
            }

            TEST_P(ClientMapTest, testRemoveIfSame) {
                fill_map();

                ASSERT_FALSE(imap_->remove("key2", "value").get());
                ASSERT_EQ(10, imap_->size().get());

                ASSERT_TRUE((imap_->remove("key2", "value2").get()));
                ASSERT_EQ(9, imap_->size().get());
            }

            TEST_P(ClientMapTest, testRemoveAll) {
                fill_map();

                imap_->remove_all(
                        query::equal_predicate(client_, query::query_constants::KEY_ATTRIBUTE_NAME, "key5")).get();

                boost::optional<std::string> value = imap_->get<std::string, std::string>("key5").get();

                ASSERT_FALSE(value.has_value()) << "key5 should not exist";

                imap_->remove_all(
                        query::like_predicate(client_, query::query_constants::THIS_ATTRIBUTE_NAME, "value%")).get();

                ASSERT_TRUE(imap_->is_empty().get());
            }

            TEST_P(ClientMapTest, testGetAllPutAll) {
                std::unordered_map<std::string, std::string> mapTemp;

                for (int i = 0; i < 100; i++) {
                    mapTemp[std::to_string(i)] = std::to_string(i);
                }
                ASSERT_EQ(imap_->size().get(), 0);
                imap_->put_all(mapTemp).get();
                ASSERT_EQ(imap_->size().get(), 100);

                for (int i = 0; i < 100; i++) {
                    std::string expected = std::to_string(i);
                    boost::optional<std::string> actual = imap_->get<std::string, std::string>(
                            std::to_string(i)).get();
                    ASSERT_EQ(expected, *actual);
                }

                std::unordered_set<std::string> tempSet;
                tempSet.insert(std::to_string(1));
                tempSet.insert(std::to_string(3));
                std::unordered_map<std::string, std::string> m2 = imap_->get_all<std::string, std::string>(tempSet).get();

                ASSERT_EQ(2U, m2.size());
                ASSERT_EQ(m2[std::to_string(1)], "1");
                ASSERT_EQ(m2[std::to_string(3)], "3");

            }

            TEST_P(ClientMapTest, testTryPutRemove) {
                ASSERT_TRUE(imap_->try_put("key1", "value1", std::chrono::seconds(1)).get());
                ASSERT_TRUE(imap_->try_put("key2", "value2", std::chrono::seconds(1)).get());
                imap_->lock("key1").get();
                imap_->lock("key2").get();

                boost::latch latch1(2);

                hazelcast::util::StartedThread t1(try_put_thread, &latch1, imap_.get());
                hazelcast::util::StartedThread t2(try_remove_thread, &latch1, imap_.get());

                ASSERT_OPEN_EVENTUALLY(latch1);
                ASSERT_EQ("value1", (imap_->get<std::string, std::string>("key1").get().value()));
                ASSERT_EQ("value2", (imap_->get<std::string, std::string>("key2").get().value()));
                imap_->force_unlock("key1").get();
                imap_->force_unlock("key2").get();
            }

            TEST_P(ClientMapTest, testPutTtl) {
                validate_expiry_invalidations(imap_, [=] () { imap_->put<std::string, std::string>("key1", "value1", std::chrono::seconds(1)).get(); });
            }

            TEST_P(ClientMapTest, testPutConfigTtl) {
                std::shared_ptr<imap> map = client_.get_map(ONE_SECOND_MAP_NAME).get();
                validate_expiry_invalidations(map, [=] () { map->put<std::string, std::string>("key1", "value1").get(); });
            }

            TEST_P(ClientMapTest, testPutIfAbsent) {
                boost::optional<std::string> o = imap_->put_if_absent<std::string, std::string>("key1", "value1").get();
                ASSERT_FALSE(o.has_value());
                ASSERT_EQ("value1", (imap_->put_if_absent<std::string, std::string>("key1", "value3").get().value()));
            }

            TEST_P(ClientMapTest, testPutIfAbsentTtl) {
                ASSERT_FALSE((imap_->put_if_absent<std::string, std::string>("key1", "value1", std::chrono::seconds(1)).get().has_value()));
                ASSERT_EQ("value1", (imap_->put_if_absent<std::string, std::string>("key1", "value3", std::chrono::seconds(1)).get().value()));

                ASSERT_FALSE_EVENTUALLY((imap_->put_if_absent<std::string, std::string>("key1", "value3", std::chrono::seconds(1)).get().has_value()));
                ASSERT_EQ("value3", (imap_->put_if_absent<std::string, std::string>("key1", "value4", std::chrono::seconds(1)).get().value()));
            }

            TEST_P(ClientMapTest, testSet) {
                imap_->set("key1", "value1").get();
                ASSERT_EQ("value1", (imap_->get<std::string, std::string>("key1").get().value()));

                imap_->set("key1", "value2").get();
                ASSERT_EQ("value2", (imap_->get<std::string, std::string>("key1").get().value()));
            }

            TEST_P(ClientMapTest, testSetTtl) {
                validate_expiry_invalidations(imap_, [=] () { imap_->set("key1", "value1", std::chrono::seconds(1)).get(); });
            }

            TEST_P(ClientMapTest, testSetConfigTtl) {
                std::shared_ptr<imap> map = client_.get_map(ONE_SECOND_MAP_NAME).get();
                validate_expiry_invalidations(map, [=] () { map->set("key1", "value1").get(); });
            }

            TEST_P(ClientMapTest, testLock) {
                imap_->put<std::string, std::string>("key1", "value1").get();
                ASSERT_EQ("value1", (imap_->get<std::string, std::string>("key1").get().value()));
                imap_->lock("key1").get();
                boost::latch latch1(1);
                hazelcast::util::StartedThread t1(test_lock_thread, &latch1, imap_.get());
                ASSERT_OPEN_EVENTUALLY(latch1);
                ASSERT_EQ("value1", (imap_->get<std::string, std::string>("key1").get().value()));
                imap_->force_unlock("key1").get();
            }

            TEST_P(ClientMapTest, testLockTtl) {
                imap_->put<std::string, std::string>("key1", "value1").get();
                ASSERT_EQ("value1", (imap_->get<std::string, std::string>("key1").get().value()));
                imap_->lock("key1", std::chrono::seconds(2)).get();
                boost::latch latch1(1);
                hazelcast::util::StartedThread t1(test_lock_ttl_thread, &latch1, imap_.get());
                ASSERT_OPEN_EVENTUALLY(latch1);
                ASSERT_FALSE(imap_->is_locked("key1").get());
                ASSERT_EQ("value2", (imap_->get<std::string, std::string>("key1").get().value()));
                imap_->force_unlock("key1").get();
            }

            TEST_P(ClientMapTest, testLockTtl2) {
                imap_->lock("key1", std::chrono::seconds(3)).get();
                boost::latch latch1(2);
                hazelcast::util::StartedThread t1(test_lock_tt_l2_thread, &latch1, imap_.get());
                ASSERT_OPEN_EVENTUALLY(latch1);
                imap_->force_unlock("key1").get();
            }

            TEST_P(ClientMapTest, testTryLock) {
                ASSERT_TRUE(imap_->try_lock("key1", std::chrono::seconds(2)).get());
                boost::latch latch1(1);
                hazelcast::util::StartedThread t1(test_map_try_lock_thread1, &latch1, imap_.get());

                ASSERT_OPEN_EVENTUALLY(latch1);

                ASSERT_TRUE(imap_->is_locked("key1").get());

                boost::latch latch2(1);
                hazelcast::util::StartedThread t2(test_map_try_lock_thread2, &latch2, imap_.get());

                std::this_thread::sleep_for(std::chrono::seconds(1));
                imap_->unlock("key1").get();
                ASSERT_OPEN_EVENTUALLY(latch2);
                ASSERT_TRUE(imap_->is_locked("key1").get());
                imap_->force_unlock("key1").get();
            }

            TEST_P(ClientMapTest, testTryLockTtl) {
                ASSERT_TRUE(imap_->try_lock("key1", std::chrono::seconds(2), std::chrono::seconds(1)).get());
                boost::latch latch1(1);
                hazelcast::util::StartedThread t1(test_map_try_lock_thread1, &latch1, imap_.get());

                ASSERT_OPEN_EVENTUALLY(latch1);

                ASSERT_TRUE(imap_->is_locked("key1").get());

                boost::latch latch2(1);
                hazelcast::util::StartedThread t2(test_map_try_lock_thread2, &latch2, imap_.get());

                ASSERT_OPEN_EVENTUALLY(latch2);
                ASSERT_TRUE(imap_->is_locked("key1").get());
                imap_->force_unlock("key1").get();
            }

            TEST_P(ClientMapTest, testTryLockTtlTimeout) {
                ASSERT_TRUE(imap_->try_lock("key1", std::chrono::seconds(2), std::chrono::seconds(200)).get());
                boost::latch latch1(1);
                hazelcast::util::StartedThread t1(test_map_try_lock_thread1, &latch1, imap_.get());

                ASSERT_OPEN_EVENTUALLY(latch1);

                ASSERT_TRUE(imap_->is_locked("key1").get());
                imap_->force_unlock("key1").get();
            }

            TEST_P(ClientMapTest, testForceUnlock) {
                imap_->lock("key1").get();
                boost::latch latch1(1);
                hazelcast::util::StartedThread t2(test_map_force_unlock_thread, &latch1, imap_.get());
                ASSERT_OPEN_EVENTUALLY(latch1);
                t2.join();
                ASSERT_FALSE(imap_->is_locked("key1").get());
            }

            TEST_P(ClientMapTest, testJsonValues) {
                const int numItems = 5;
                for (int i = 0; i < numItems; ++i) {
                    imap_->put("key_" + std::to_string(i), hazelcast_json_value("{ \"value\"=\"value_" + std::to_string(i) + "\"}")).get();
                }
                auto values = imap_->values<hazelcast_json_value>().get();
                ASSERT_EQ(numItems, (int) values.size());
            }

            /**
             * Fails with `hazelcast_serialization {Not comparable { "value"="value_2"}`
             * The hazelcast_json_value should be comparable at Java server side to make it work.
             */
            TEST_P(ClientMapTest, DISABLED_testJsonValuesWithpaging_predicate) {
                const int numItems = 5;
                const int predSize = 3;
                for (int i = 0; i < numItems; ++i) {
                    imap_->put<std::string, hazelcast_json_value>("key_" + std::to_string(i), hazelcast_json_value(
                            "{ \"value\"=\"value_" + std::to_string(i) + "\"}")).get();
                }
                auto predicate = imap_->new_paging_predicate<std::string, hazelcast_json_value>((size_t) predSize);
                auto values = imap_->values<std::string, hazelcast_json_value>(predicate).get();
                ASSERT_EQ(predSize, (int) values.size());
            }


            TEST_P(ClientMapTest, testValues) {
                fill_map();
                query::sql_predicate predicate(client_, "this == value1");
                std::vector<std::string> tempVector = imap_->values<std::string>(predicate).get();
                ASSERT_EQ(1U, tempVector.size());
                ASSERT_EQ("value1", tempVector[0]);
            }

            TEST_P(ClientMapTest, testValuesWithPredicate) {
                const int numItems = 20;
                for (int i = 0; i < numItems; ++i) {
                    int_map_->put(i, 2 * i).get();
                }

                auto values = int_map_->values<int>().get();
                ASSERT_EQ(numItems, (int) values.size());
                std::sort(values.begin(), values.end());
                for (int i = 0; i < numItems; ++i) {
                    ASSERT_EQ(2 * i, values[i]);
                }

                // equal_predicate
                // key == 5
                values = int_map_->values<int>(
                        query::equal_predicate(client_, query::query_constants::KEY_ATTRIBUTE_NAME, 5)).get();
                ASSERT_EQ(1, (int) values.size());
                ASSERT_EQ(2 * 5, values[0]);

                // value == 8
                values = int_map_->values<int>(
                        query::equal_predicate(client_, query::query_constants::THIS_ATTRIBUTE_NAME, 8)).get();
                ASSERT_EQ(1, (int) values.size());
                ASSERT_EQ(8, values[0]);

                // key == numItems
                values = int_map_->values<int>(
                        query::equal_predicate(client_, query::query_constants::KEY_ATTRIBUTE_NAME, numItems)).get();
                ASSERT_EQ(0, (int) values.size());

                // NotEqual Predicate
                // key != 5
                values = int_map_->values<int>(
                        query::not_equal_predicate(client_, query::query_constants::KEY_ATTRIBUTE_NAME, 5)).get();
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
                values = int_map_->values<int>(
                        query::not_equal_predicate(client_, query::query_constants::THIS_ATTRIBUTE_NAME, 8)).get();
                ASSERT_EQ(numItems - 1, (int) values.size());
                std::sort(values.begin(), values.end());
                for (int i = 0; i < numItems - 1; ++i) {
                    if (i >= 4) {
                        ASSERT_EQ(2 * (i + 1), values[i]);
                    } else {
                        ASSERT_EQ(2 * i, values[i]);
                    }
                }

                // true_predicate
                values = int_map_->values<int>(query::true_predicate(client_)).get();
                ASSERT_EQ(numItems, (int) values.size());
                std::sort(values.begin(), values.end());
                for (int i = 0; i < numItems; ++i) {
                    ASSERT_EQ(2 * i, values[i]);
                }

                // false_predicate
                values = int_map_->values<int>(query::false_predicate(client_)).get();
                ASSERT_EQ(0, (int) values.size());

                // between_predicate
                // 5 <= key <= 10
                values = int_map_->values<int>(
                        query::between_predicate(client_, query::query_constants::KEY_ATTRIBUTE_NAME, 5, 10)).get();
                std::sort(values.begin(), values.end());
                ASSERT_EQ(6, (int) values.size());
                for (int i = 0; i < 6; ++i) {
                    ASSERT_EQ(2 * (i + 5), values[i]);
                }

                // 20 <= key <=30
                values = int_map_->values<int>(
                        query::between_predicate(client_, query::query_constants::KEY_ATTRIBUTE_NAME, 20, 30)).get();
                ASSERT_EQ(0, (int) values.size());

                // greater_less_predicate
                // value <= 10
                values = int_map_->values<int>(
                        query::greater_less_predicate(client_, query::query_constants::THIS_ATTRIBUTE_NAME, 10, true,
                                                      true)).get();
                ASSERT_EQ(6, (int) values.size());
                std::sort(values.begin(), values.end());
                for (int i = 0; i < 6; ++i) {
                    ASSERT_EQ(2 * i, values[i]);
                }

                // key < 7
                values = int_map_->values<int>(
                        query::greater_less_predicate(client_, query::query_constants::KEY_ATTRIBUTE_NAME, 7, false, true)).get();
                ASSERT_EQ(7, (int) values.size());
                std::sort(values.begin(), values.end());
                for (int i = 0; i < 7; ++i) {
                    ASSERT_EQ(2 * i, values[i]);
                }

                // value >= 15
                values = int_map_->values<int>(
                        query::greater_less_predicate(client_, query::query_constants::THIS_ATTRIBUTE_NAME, 15, true,
                                                      false)).get();
                ASSERT_EQ(12, (int) values.size());
                std::sort(values.begin(), values.end());
                for (int i = 0; i < 12; ++i) {
                    ASSERT_EQ(2 * (i + 8), values[i]);
                }

                // key > 5
                values = int_map_->values<int>(
                        query::greater_less_predicate(client_, query::query_constants::KEY_ATTRIBUTE_NAME, 5, false,
                                                      false)).get();
                ASSERT_EQ(14, (int) values.size());
                std::sort(values.begin(), values.end());
                for (int i = 0; i < 14; ++i) {
                    ASSERT_EQ(2 * (i + 6), values[i]);
                }

                // in_predicate
                // key in {4, 10, 19}
                values = int_map_->values<int>(
                        query::in_predicate(client_, query::query_constants::KEY_ATTRIBUTE_NAME, 4, 10, 19)).get();
                ASSERT_EQ(3, (int) values.size());
                std::sort(values.begin(), values.end());
                ASSERT_EQ(2 * 4, values[0]);
                ASSERT_EQ(2 * 10, values[1]);
                ASSERT_EQ(2 * 19, values[2]);

                // value in {4, 10, 19}
                values = int_map_->values<int>(
                        query::in_predicate(client_, query::query_constants::THIS_ATTRIBUTE_NAME, std::vector<int>({4, 10, 19}))).get();
                ASSERT_EQ(2, (int) values.size());
                std::sort(values.begin(), values.end());
                ASSERT_EQ(4, values[0]);
                ASSERT_EQ(10, values[1]);

                // instance_of_predicate
                // value instanceof Integer
                values = int_map_->values<int>(query::instance_of_predicate(client_, "java.lang.Integer")).get();
                ASSERT_EQ(20, (int) values.size());
                std::sort(values.begin(), values.end());
                for (int i = 0; i < numItems; ++i) {
                    ASSERT_EQ(2 * i, values[i]);
                }

                values = int_map_->values<int>(query::instance_of_predicate(client_, "java.lang.String")).get();
                ASSERT_EQ(0, (int) values.size());

                // not_predicate
                // !(5 <= key <= 10)
                query::not_predicate notPredicate(client_, query::between_predicate(client_,
                                                                                    query::query_constants::KEY_ATTRIBUTE_NAME,
                                                                                    5, 10));
                values = int_map_->values<int>(notPredicate).get();
                ASSERT_EQ(14, (int) values.size());
                std::sort(values.begin(), values.end());
                for (int i = 0; i < 14; ++i) {
                    if (i >= 5) {
                        ASSERT_EQ(2 * (i + 6), values[i]);
                    } else {
                        ASSERT_EQ(2 * i, values[i]);
                    }
                }

                // and_predicate
                // 5 <= key <= 10 AND Values in {4, 10, 19} = values {4, 10}
                values = int_map_->values<int>(query::and_predicate(client_, query::between_predicate(client_,
                                                                                                      query::query_constants::KEY_ATTRIBUTE_NAME,
                                                                                                      5, 10),
                                                                    query::in_predicate(client_,
                                                                                     query::query_constants::THIS_ATTRIBUTE_NAME,
                                                                                     4, 10, 19))).get();
                ASSERT_EQ(1, (int) values.size());
                std::sort(values.begin(), values.end());
                ASSERT_EQ(10, values[0]);

                // or_predicate
                // 5 <= key <= 10 OR Values in {4, 10, 19} = values {4, 10, 12, 14, 16, 18, 20}
                values = int_map_->values<int>(query::or_predicate(client_, query::between_predicate(client_,
                                                                                                     query::query_constants::KEY_ATTRIBUTE_NAME,
                                                                                                     5, 10),
                                                                   query::in_predicate(client_,
                                                                                    query::query_constants::THIS_ATTRIBUTE_NAME,
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
                    imap_->put(key, value).get();
                }
                imap_->put<std::string, std::string>("key_111_test", "myvalue_111_test").get();
                imap_->put<std::string, std::string>("key_22_test", "myvalue_22_test").get();

                // like_predicate
                // value LIKE "value1" : {"value1"}
                std::vector<std::string> strValues = imap_->values<std::string>(
                        query::like_predicate(client_, query::query_constants::THIS_ATTRIBUTE_NAME, "value1")).get();
                ASSERT_EQ(1, (int) strValues.size());
                ASSERT_EQ("value1", strValues[0]);

                // ilike_predicate
                // value ILIKE "%VALue%1%" : {"myvalue_111_test", "value1", "value10", "value11"}
                strValues = imap_->values<std::string>(
                        query::ilike_predicate(client_, query::query_constants::THIS_ATTRIBUTE_NAME, "%VALue%1%")).get();
                ASSERT_EQ(4, (int) strValues.size());
                std::sort(strValues.begin(), strValues.end());
                ASSERT_EQ("myvalue_111_test", strValues[0]);
                ASSERT_EQ("value1", strValues[1]);
                ASSERT_EQ("value10", strValues[2]);
                ASSERT_EQ("value11", strValues[3]);

                // value ILIKE "%VAL%2%" : {"myvalue_22_test", "value2"}
                strValues = imap_->values<std::string>(
                        query::ilike_predicate(client_, query::query_constants::THIS_ATTRIBUTE_NAME, "%VAL%2%")).get();
                ASSERT_EQ(2, (int) strValues.size());
                std::sort(strValues.begin(), strValues.end());
                ASSERT_EQ("myvalue_22_test", strValues[0]);
                ASSERT_EQ("value2", strValues[1]);

                // sql_predicate
                // __key BETWEEN 4 and 7 : {4, 5, 6, 7} -> {8, 10, 12, 14}
                char sql[100];
                hazelcast::util::hz_snprintf(sql, 50, "%s BETWEEN 4 and 7",
                                             query::query_constants::KEY_ATTRIBUTE_NAME);
                values = int_map_->values<int>(query::sql_predicate(client_, sql)).get();
                ASSERT_EQ(4, (int) values.size());
                std::sort(values.begin(), values.end());
                for (int i = 0; i < 4; ++i) {
                    ASSERT_EQ(2 * (i + 4), values[i]);
                }

                // regex_predicate
                // value matches the regex ".*value.*2.*" : {myvalue_22_test, value2}
                strValues = imap_->values<std::string>(
                        query::regex_predicate(client_, query::query_constants::THIS_ATTRIBUTE_NAME, ".*value.*2.*")).get();
                ASSERT_EQ(2, (int) strValues.size());
                std::sort(strValues.begin(), strValues.end());
                ASSERT_EQ("myvalue_22_test", strValues[0]);
                ASSERT_EQ("value2", strValues[1]);
            }

            TEST_P(ClientMapTest, testValuesWithpaging_predicate) {
                int predSize = 5;
                const int totalEntries = 25;

                for (int i = 0; i < totalEntries; ++i) {
                    int_map_->put(i, i).get();
                }

                auto predicate = int_map_->new_paging_predicate<int, int>((size_t) predSize);

                std::vector<int> values = int_map_->values<int>(predicate).get();
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    ASSERT_EQ(i, values[i]);
                }

                values = int_map_->values<int>(predicate).get();
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    ASSERT_EQ(i, values[i]);
                }

                predicate.next_page();
                values = int_map_->values<int>(predicate).get();
                ASSERT_EQ(predSize, (int) values.size());

                for (int i = 0; i < predSize; ++i) {
                    ASSERT_EQ(predSize + i, values[i]);
                }

                ASSERT_EQ(1, (int) predicate.get_page());

                predicate.set_page(4);

                values = int_map_->values<int>(predicate).get();
                ASSERT_EQ(predSize, (int) values.size());

                for (int i = 0; i < predSize; ++i) {
                    ASSERT_EQ(predSize * 4 + i, values[i]);
                }

                predicate.next_page();
                values = int_map_->values<int>(predicate).get();
                ASSERT_EQ(0, (int) values.size());

                predicate.set_page(0);
                values = int_map_->values<int>(predicate).get();
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    ASSERT_EQ(i, values[i]);
                }

                predicate.previous_page();
                ASSERT_EQ(0, (int) predicate.get_page());

                predicate.set_page(5);
                values = int_map_->values<int>(predicate).get();
                ASSERT_EQ(0, (int) values.size());

                predicate.set_page(3);
                values = int_map_->values<int>(predicate).get();
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    ASSERT_EQ(3 * predSize + i, values[i]);
                }

                predicate.previous_page();
                values = int_map_->values<int>(predicate).get();
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    ASSERT_EQ(2 * predSize + i, values[i]);
                }

                // test paging_predicate with inner predicate (value < 10)
                query::greater_less_predicate lessThanTenPredicate(client_, query::query_constants::THIS_ATTRIBUTE_NAME, 9, false, true);
                auto predicate2 = int_map_->new_paging_predicate<int, int>(5, lessThanTenPredicate);
                values = int_map_->values<int>(predicate2).get();
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    ASSERT_EQ(i, values[i]);
                }

                predicate2.next_page();
                // match values 5,6, 7, 8
                values = int_map_->values<int>(predicate2).get();
                ASSERT_EQ(predSize - 1, (int) values.size());
                for (int i = 0; i < predSize - 1; ++i) {
                    ASSERT_EQ(predSize + i, values[i]);
                }

                predicate2.next_page();
                values = int_map_->values<int>(predicate2).get();
                ASSERT_EQ(0, (int) values.size());

                // test paging predicate with comparator
                employee empl1("ahmet", 35);
                employee empl2("mehmet", 21);
                employee empl3("deniz", 25);
                employee empl4("ali", 33);
                employee empl5("veli", 44);
                employee empl6("aylin", 5);

                employees_->put(3, empl1).get();
                employees_->put(4, empl2).get();
                employees_->put(5, empl3).get();
                employees_->put(6, empl4).get();
                employees_->put(7, empl5).get();
                employees_->put(8, empl6).get();

                predSize = 2;
                auto predicate3 = int_map_->new_paging_predicate<int, employee>(EmployeeEntryComparator(), (size_t) predSize);
                std::vector<employee> result = employees_->values<int, employee>(predicate3).get();
                ASSERT_EQ(2, (int) result.size());
                ASSERT_EQ(empl6, result[0]);
                ASSERT_EQ(empl2, result[1]);

                predicate3.next_page();
                result = employees_->values<int, employee>(predicate3).get();
                ASSERT_EQ(2, (int) result.size());
                ASSERT_EQ(empl3, result[0]);
                ASSERT_EQ(empl4, result[1]);
            }

            TEST_P(ClientMapTest, testKeySetWithPredicate) {
                const int numItems = 20;
                for (int i = 0; i < numItems; ++i) {
                    int_map_->put(i, 2 * i).get();
                }

                std::vector<int> keys = int_map_->key_set<int>().get();
                ASSERT_EQ(numItems, (int) keys.size());
                std::sort(keys.begin(), keys.end());
                for (int i = 0; i < numItems; ++i) {
                    ASSERT_EQ(i, keys[i]);
                }

                // equal_predicate
                // key == 5
                keys = int_map_->key_set<int>(
                        query::equal_predicate(client_, query::query_constants::KEY_ATTRIBUTE_NAME, 5)).get();
                ASSERT_EQ(1, (int) keys.size());
                ASSERT_EQ(5, keys[0]);

                // value == 8
                keys = int_map_->key_set<int>(
                        query::equal_predicate(client_, query::query_constants::THIS_ATTRIBUTE_NAME, 8)).get();
                ASSERT_EQ(1, (int) keys.size());
                ASSERT_EQ(4, keys[0]);

// key == numItems
                keys = int_map_->key_set<int>(
                        query::equal_predicate(client_, query::query_constants::KEY_ATTRIBUTE_NAME, numItems)).get();
                ASSERT_EQ(0, (int) keys.size());

// NotEqual Predicate
// key != 5
                keys = int_map_->key_set<int>(
                        query::not_equal_predicate(client_, query::query_constants::KEY_ATTRIBUTE_NAME, 5)).get();
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
                keys = int_map_->key_set<int>(
                        query::not_equal_predicate(client_, query::query_constants::THIS_ATTRIBUTE_NAME, 8)).get();
                ASSERT_EQ(numItems - 1, (int) keys.size());
                std::sort(keys.begin(), keys.end());
                for (int i = 0; i < numItems - 1; ++i) {
                    if (i >= 4) {
                        ASSERT_EQ(i + 1, keys[i]);
                    } else {
                        ASSERT_EQ(i, keys[i]);
                    }
                }

// true_predicate
                keys = int_map_->key_set<int>(query::true_predicate(client_)).get();
                ASSERT_EQ(numItems, (int) keys.size());
                std::sort(keys.begin(), keys.end());
                for (int i = 0; i < numItems; ++i) {
                    ASSERT_EQ(i, keys[i]);
                }

// false_predicate
                keys = int_map_->key_set<int>(query::false_predicate(client_)).get();
                ASSERT_EQ(0, (int) keys.size());

// between_predicate
// 5 <= key <= 10
                keys = int_map_->key_set<int>(
                        query::between_predicate(client_, query::query_constants::KEY_ATTRIBUTE_NAME, 5, 10)).get();
                std::sort(keys.begin(), keys.end());
                ASSERT_EQ(6, (int) keys.size());
                for (int i = 0; i < 6; ++i) {
                    ASSERT_EQ((i + 5), keys[i]);
                }

// 20 <= key <=30
                keys = int_map_->key_set<int>(
                        query::between_predicate(client_, query::query_constants::KEY_ATTRIBUTE_NAME, 20, 30)).get();
                ASSERT_EQ(0, (int) keys.size());

// greater_less_predicate
// value <= 10
                keys = int_map_->key_set<int>(
                        query::greater_less_predicate(client_, query::query_constants::THIS_ATTRIBUTE_NAME, 10, true,
                                                      true)).get();
                ASSERT_EQ(6, (int) keys.size());
                std::sort(keys.begin(), keys.end());
                for (int i = 0; i < 6; ++i) {
                    ASSERT_EQ(i, keys[i]);
                }

// key < 7
                keys = int_map_->key_set<int>(
                        query::greater_less_predicate(client_, query::query_constants::KEY_ATTRIBUTE_NAME, 7, false, true)).get();
                ASSERT_EQ(7, (int) keys.size());
                std::sort(keys.begin(), keys.end());
                for (int i = 0; i < 7; ++i) {
                    ASSERT_EQ(i, keys[i]);
                }

// value >= 15
                keys = int_map_->key_set<int>(
                        query::greater_less_predicate(client_, query::query_constants::THIS_ATTRIBUTE_NAME, 15, true,
                                                      false)).get();
                ASSERT_EQ(12, (int) keys.size());
                std::sort(keys.begin(), keys.end());
                for (int i = 0; i < 12; ++i) {
                    ASSERT_EQ(i + 8, keys[i]);
                }

// key > 5
                keys = int_map_->key_set<int>(
                        query::greater_less_predicate(client_, query::query_constants::KEY_ATTRIBUTE_NAME, 5, false,
                                                      false)).get();
                ASSERT_EQ(14, (int) keys.size());
                std::sort(keys.begin(), keys.end());
                for (int i = 0; i < 14; ++i) {
                    ASSERT_EQ(i + 6, keys[i]);
                }

// in_predicate
// key in {4, 10, 19}
                keys = int_map_->key_set<int>(
                        query::in_predicate(client_, query::query_constants::KEY_ATTRIBUTE_NAME, 4, 10, 19)).get();
                ASSERT_EQ(3, (int) keys.size());
                std::sort(keys.begin(), keys.end());
                ASSERT_EQ(4, keys[0]);
                ASSERT_EQ(10, keys[1]);
                ASSERT_EQ(19, keys[2]);

// value in {4, 10, 19}
                keys = int_map_->key_set<int>(
                        query::in_predicate(client_, query::query_constants::THIS_ATTRIBUTE_NAME, 4, 10, 19)).get();
                ASSERT_EQ(2, (int) keys.size());
                std::sort(keys.begin(), keys.end());
                ASSERT_EQ(2, keys[0]);
                ASSERT_EQ(5, keys[1]);

// instance_of_predicate
// value instanceof Integer
                keys = int_map_->key_set<int>(query::instance_of_predicate(client_, "java.lang.Integer")).get();
                ASSERT_EQ(20, (int) keys.size());
                std::sort(keys.begin(), keys.end());
                for (int i = 0; i < numItems; ++i) {
                    ASSERT_EQ(i, keys[i]);
                }

                keys = int_map_->key_set<int>(query::instance_of_predicate(client_, "java.lang.String")).get();
                ASSERT_EQ(0, (int) keys.size());

                // not_predicate
                // !(5 <= key <= 10)
                query::not_predicate notPredicate(client_, query::between_predicate(client_, query::query_constants::KEY_ATTRIBUTE_NAME, 5, 10));
                keys = int_map_->key_set<int>(notPredicate).get();
                ASSERT_EQ(14, (int) keys.size());
                std::sort(keys.begin(), keys.end());
                for (int i = 0; i < 14; ++i) {
                    if (i >= 5) {
                        ASSERT_EQ(i + 6, keys[i]);
                    } else {
                        ASSERT_EQ(i, keys[i]);
                    }
                }

// and_predicate
// 5 <= key <= 10 AND Values in {4, 10, 19} = keys {4, 10}
                keys = int_map_->key_set<int>(query::and_predicate(client_,
                                                                   query::between_predicate(client_, query::query_constants::KEY_ATTRIBUTE_NAME, 5, 10),
                                                                   query::in_predicate(client_, query::query_constants::THIS_ATTRIBUTE_NAME, 4, 10, 19))).get();
                ASSERT_EQ(1, (int) keys.size());
                std::sort(keys.begin(), keys.end());
                ASSERT_EQ(5, keys[0]);

// or_predicate
// 5 <= key <= 10 OR Values in {4, 10, 19} = keys {2, 5, 6, 7, 8, 9, 10}
                keys = int_map_->key_set<int>(query::or_predicate(client_, query::between_predicate(client_, query::query_constants::KEY_ATTRIBUTE_NAME, 5, 10), query::in_predicate(client_, query::query_constants::THIS_ATTRIBUTE_NAME, 4, 10, 19))).get();
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
                    imap_->put<std::string, std::string>(key, value).get();
                }
                imap_->put<std::string, std::string>("key_111_test", "myvalue_111_test").get();
                imap_->put<std::string, std::string>("key_22_test", "myvalue_22_test").get();

// like_predicate
// value LIKE "value1" : {"value1"}
                std::vector<std::string> strKeys = imap_->key_set<std::string>(
                        query::like_predicate(client_, query::query_constants::THIS_ATTRIBUTE_NAME, "value1")).get();
                ASSERT_EQ(1, (int) strKeys.size());
                ASSERT_EQ("key1", strKeys[0]);

// ilike_predicate
// value ILIKE "%VALue%1%" : {"key_111_test", "key1", "key10", "key11"}
                strKeys = imap_->key_set<std::string>(
                        query::ilike_predicate(client_, query::query_constants::THIS_ATTRIBUTE_NAME, "%VALue%1%")).get();
                ASSERT_EQ(4, (int) strKeys.size());
                std::sort(strKeys.begin(), strKeys.end());
                ASSERT_EQ("key1", strKeys[0]);
                ASSERT_EQ("key10", strKeys[1]);
                ASSERT_EQ("key11", strKeys[2]);
                ASSERT_EQ("key_111_test", strKeys[3]);

// key ILIKE "%VAL%2%" : {"key_22_test", "key2"}
                strKeys = imap_->key_set<std::string>(
                        query::ilike_predicate(client_, query::query_constants::THIS_ATTRIBUTE_NAME, "%VAL%2%")).get();
                ASSERT_EQ(2, (int) strKeys.size());
                std::sort(strKeys.begin(), strKeys.end());
                ASSERT_EQ("key2", strKeys[0]);
                ASSERT_EQ("key_22_test", strKeys[1]);

// sql_predicate
// __key BETWEEN 4 and 7 : {4, 5, 6, 7} -> {8, 10, 12, 14}
                char sql[100];
                hazelcast::util::hz_snprintf(sql, 50, "%s BETWEEN 4 and 7",
                                             query::query_constants::KEY_ATTRIBUTE_NAME);
                keys = int_map_->key_set<int>(query::sql_predicate(client_, sql)).get();
                ASSERT_EQ(4, (int) keys.size());
                std::sort(keys.begin(), keys.end());
                for (int i = 0; i < 4; ++i) {
                    ASSERT_EQ(i + 4, keys[i]);
                }

// regex_predicate
// value matches the regex ".*value.*2.*" : {key_22_test, value2}
                strKeys = imap_->key_set<std::string>(
                        query::regex_predicate(client_, query::query_constants::THIS_ATTRIBUTE_NAME, ".*value.*2.*")).get();
                ASSERT_EQ(2, (int) strKeys.size());
                std::sort(strKeys.begin(), strKeys.end());
                ASSERT_EQ("key2", strKeys[0]);
                ASSERT_EQ("key_22_test", strKeys[1]);
            }

            TEST_P(ClientMapTest, testKeySetWithpaging_predicate) {
                int predSize = 5;
                const int totalEntries = 25;

                for (int i = 0; i < totalEntries; ++i) {
                    int_map_->put(i, i).get();
                }

                auto predicate = int_map_->new_paging_predicate<int, int>((size_t) predSize);

                std::vector<int> values = int_map_->key_set<int>(predicate).get();
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    ASSERT_EQ(i, values[i]);
                }

                values = int_map_->key_set<int>(predicate).get();
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    ASSERT_EQ(i, values[i]);
                }

                predicate.next_page();
                values = int_map_->key_set<int>(predicate).get();
                ASSERT_EQ(predSize, (int) values.size());

                for (int i = 0; i < predSize; ++i) {
                    ASSERT_EQ(predSize + i, values[i]);
                }

                ASSERT_EQ(1, (int) predicate.get_page());

                predicate.set_page(4);

                values = int_map_->key_set<int>(predicate).get();
                ASSERT_EQ(predSize, (int) values.size());

                for (int i = 0; i < predSize; ++i) {
                    ASSERT_EQ(predSize * 4 + i, values[i]);
                }

                predicate.next_page();
                values = int_map_->key_set<int>(predicate).get();
                ASSERT_EQ(0, (int) values.size());

                predicate.set_page(0);
                values = int_map_->key_set<int>(predicate).get();
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    ASSERT_EQ(i, values[i]);
                }

                predicate.previous_page();
                ASSERT_EQ(0, (int) predicate.get_page());

                predicate.set_page(5);
                values = int_map_->key_set<int>(predicate).get();
                ASSERT_EQ(0, (int) values.size());

                predicate.set_page(3);
                values = int_map_->key_set<int>(predicate).get();
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    ASSERT_EQ(3 * predSize + i, values[i]);
                }

                predicate.previous_page();
                values = int_map_->key_set<int>(predicate).get();
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    ASSERT_EQ(2 * predSize + i, values[i]);
                }

// test paging_predicate with inner predicate (value < 10)
                query::greater_less_predicate lessThanTenPredicate(client_, query::query_constants::THIS_ATTRIBUTE_NAME, 9, false, true);
                auto predicate2 = int_map_->new_paging_predicate<int, int>(5, lessThanTenPredicate);
                values = int_map_->key_set<int>(predicate2).get();
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    ASSERT_EQ(i, values[i]);
                }

                predicate2.next_page();
// match values 5,6, 7, 8
                values = int_map_->key_set<int>(predicate2).get();
                ASSERT_EQ(predSize - 1, (int) values.size());
                for (int i = 0; i < predSize - 1; ++i) {
                    ASSERT_EQ(predSize + i, values[i]);
                }

                predicate2.next_page();
                values = int_map_->key_set<int>(predicate2).get();
                ASSERT_EQ(0, (int) values.size());

                // test paging predicate with comparator
                employee empl1("ahmet", 35);
                employee empl2("mehmet", 21);
                employee empl3("deniz", 25);
                employee empl4("ali", 33);
                employee empl5("veli", 44);
                employee empl6("aylin", 5);

                employees_->put(3, empl1).get();
                employees_->put(4, empl2).get();
                employees_->put(5, empl3).get();
                employees_->put(6, empl4).get();
                employees_->put(7, empl5).get();
                employees_->put(8, empl6).get();

                predSize = 2;
                auto predicate3 = int_map_->new_paging_predicate<int, employee>(EmployeeEntryKeyComparator(), (size_t) predSize);
                std::vector<int> result = employees_->key_set<int>(predicate3).get();
                // since keyset result only returns keys from the server, no ordering based on the value but ordered based on the keys
                ASSERT_EQ(2, (int) result.size());
                ASSERT_EQ(3, result[0]);
                ASSERT_EQ(4, result[1]);

                predicate3.next_page();
                result = employees_->key_set<int>(predicate3).get();
                ASSERT_EQ(2, (int) result.size());
                ASSERT_EQ(5, result[0]);
                ASSERT_EQ(6, result[1]);
            }

            TEST_P(ClientMapTest, testEntrySetWithPredicate) {
                const int numItems = 20;
                std::vector<std::pair<int, int> > expected(numItems);
                for (int i = 0; i < numItems; ++i) {
                    int_map_->put(i, 2 * i).get();
                    expected[i] = std::pair<int, int>(i, 2 * i);
                }

                std::vector<std::pair<int, int> > entries = int_map_->entry_set<int, int>().get();
                ASSERT_EQ(numItems, (int) entries.size());
                std::sort(entries.begin(), entries.end());
                for (int i = 0; i < numItems; ++i) {
                    ASSERT_EQ(expected[i], entries[i]);
                }

// equal_predicate
// key == 5
                entries = int_map_->entry_set<int, int>(
                        query::equal_predicate(client_, query::query_constants::KEY_ATTRIBUTE_NAME, 5)).get();
                ASSERT_EQ(1, (int) entries.size());
                ASSERT_EQ(expected[5], entries[0]);

// value == 8
                entries = int_map_->entry_set<int, int>(
                        query::equal_predicate(client_, query::query_constants::THIS_ATTRIBUTE_NAME, 8)).get();
                ASSERT_EQ(1, (int) entries.size());
                ASSERT_EQ(expected[4], entries[0]);

// key == numItems
                entries = int_map_->entry_set<int, int>(
                        query::equal_predicate(client_, query::query_constants::KEY_ATTRIBUTE_NAME, numItems)).get();
                ASSERT_EQ(0, (int) entries.size());

// NotEqual Predicate
// key != 5
                entries = int_map_->entry_set<int, int>(
                        query::not_equal_predicate(client_, query::query_constants::KEY_ATTRIBUTE_NAME, 5)).get();
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
                entries = int_map_->entry_set<int, int>(
                        query::not_equal_predicate(client_, query::query_constants::THIS_ATTRIBUTE_NAME, 8)).get();
                ASSERT_EQ(numItems - 1, (int) entries.size());
                std::sort(entries.begin(), entries.end());
                for (int i = 0; i < numItems - 1; ++i) {
                    if (i >= 4) {
                        ASSERT_EQ(expected[i + 1], entries[i]);
                    } else {
                        ASSERT_EQ(expected[i], entries[i]);
                    }
                }

// true_predicate
                entries = int_map_->entry_set<int, int>(query::true_predicate(client_)).get();
                ASSERT_EQ(numItems, (int) entries.size());
                std::sort(entries.begin(), entries.end());
                for (int i = 0; i < numItems; ++i) {
                    ASSERT_EQ(expected[i], entries[i]);
                }

// false_predicate
                entries = int_map_->entry_set<int, int>(query::false_predicate(client_)).get();
                ASSERT_EQ(0, (int) entries.size());

// between_predicate
// 5 <= key <= 10
                entries = int_map_->entry_set<int, int>(
                        query::between_predicate(client_, query::query_constants::KEY_ATTRIBUTE_NAME, 5, 10)).get();
                std::sort(entries.begin(), entries.end());
                ASSERT_EQ(6, (int) entries.size());
                for (int i = 0; i < 6; ++i) {
                    ASSERT_EQ(expected[i + 5], entries[i]);
                }

// 20 <= key <=30
                entries = int_map_->entry_set<int, int>(
                        query::between_predicate(client_, query::query_constants::KEY_ATTRIBUTE_NAME, 20, 30)).get();
                ASSERT_EQ(0, (int) entries.size());

// greater_less_predicate
// value <= 10
                entries = int_map_->entry_set<int, int>(
                        query::greater_less_predicate(client_, query::query_constants::THIS_ATTRIBUTE_NAME, 10, true,
                                                      true)).get();
                ASSERT_EQ(6, (int) entries.size());
                std::sort(entries.begin(), entries.end());
                for (int i = 0; i < 6; ++i) {
                    ASSERT_EQ(expected[i], entries[i]);
                }

// key < 7
                entries = int_map_->entry_set<int, int>(
                        query::greater_less_predicate(client_, query::query_constants::KEY_ATTRIBUTE_NAME, 7, false, true)).get();
                ASSERT_EQ(7, (int) entries.size());
                std::sort(entries.begin(), entries.end());
                for (int i = 0; i < 7; ++i) {
                    ASSERT_EQ(expected[i], entries[i]);
                }

// value >= 15
                entries = int_map_->entry_set<int, int>(
                        query::greater_less_predicate(client_, query::query_constants::THIS_ATTRIBUTE_NAME, 15, true,
                                                      false)).get();
                ASSERT_EQ(12, (int) entries.size());
                std::sort(entries.begin(), entries.end());
                for (int i = 0; i < 12; ++i) {
                    ASSERT_EQ(expected[i + 8], entries[i]);
                }

// key > 5
                entries = int_map_->entry_set<int, int>(
                        query::greater_less_predicate(client_, query::query_constants::KEY_ATTRIBUTE_NAME, 5, false,
                                                      false)).get();
                ASSERT_EQ(14, (int) entries.size());
                std::sort(entries.begin(), entries.end());
                for (int i = 0; i < 14; ++i) {
                    ASSERT_EQ(expected[i + 6], entries[i]);
                }

                // in_predicate
                // key in {4, 10, 19}
                entries = int_map_->entry_set<int, int>(
                        query::in_predicate(client_, query::query_constants::KEY_ATTRIBUTE_NAME, 4, 10, 19)).get();
                ASSERT_EQ(3, (int) entries.size());
                std::sort(entries.begin(), entries.end());
                ASSERT_EQ(expected[4], entries[0]);
                ASSERT_EQ(expected[10], entries[1]);
                ASSERT_EQ(expected[19], entries[2]);

// value in {4, 10, 19}
                entries = int_map_->entry_set<int, int>(
                        query::in_predicate(client_, query::query_constants::THIS_ATTRIBUTE_NAME, 4, 10, 19)).get();
                ASSERT_EQ(2, (int) entries.size());
                std::sort(entries.begin(), entries.end());
                ASSERT_EQ(expected[2], entries[0]);
                ASSERT_EQ(expected[5], entries[1]);

// instance_of_predicate
// value instanceof Integer
                entries = int_map_->entry_set<int, int>(query::instance_of_predicate(client_, "java.lang.Integer")).get();
                ASSERT_EQ(20, (int) entries.size());
                std::sort(entries.begin(), entries.end());
                for (int i = 0; i < numItems; ++i) {
                    ASSERT_EQ(expected[i], entries[i]);
                }

                entries = int_map_->entry_set<int, int>(query::instance_of_predicate(client_, "java.lang.String")).get();
                ASSERT_EQ(0, (int) entries.size());

                // not_predicate
                // !(5 <= key <= 10)
                query::not_predicate notPredicate(client_, query::between_predicate(client_, query::query_constants::KEY_ATTRIBUTE_NAME, 5, 10));
                entries = int_map_->entry_set<int, int>(notPredicate).get();
                ASSERT_EQ(14, (int) entries.size());
                std::sort(entries.begin(), entries.end());
                for (int i = 0; i < 14; ++i) {
                    if (i >= 5) {
                        ASSERT_EQ(expected[i + 6], entries[i]);
                    } else {
                        ASSERT_EQ(expected[i], entries[i]);
                    }
                }

// and_predicate
// 5 <= key <= 10 AND Values in {4, 10, 19} = entries {4, 10}
                entries = int_map_->entry_set<int, int>(query::and_predicate(client_, query::between_predicate(client_,
                                                                                                               query::query_constants::KEY_ATTRIBUTE_NAME, 5, 10),
                                                                             query::in_predicate(client_, query::query_constants::THIS_ATTRIBUTE_NAME, 4, 10, 19))).get();
                ASSERT_EQ(1, (int) entries.size());
                std::sort(entries.begin(), entries.end());
                ASSERT_EQ(expected[5], entries[0]);

// or_predicate
// 5 <= key <= 10 OR Values in {4, 10, 19} = entries {2, 5, 6, 7, 8, 9, 10}
                entries = int_map_->entry_set<int, int>(query::or_predicate(client_,
                                                                            query::between_predicate(client_, query::query_constants::KEY_ATTRIBUTE_NAME, 5, 10),
                                                                            query::in_predicate(client_, query::query_constants::THIS_ATTRIBUTE_NAME, 4, 10, 19))).get();
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
                    imap_->put<std::string, std::string>(key, value).get();
                    expectedStrEntries[i] = std::pair<std::string, std::string>(key, value);
                }
                imap_->put<std::string, std::string>("key_111_test", "myvalue_111_test").get();
                expectedStrEntries[12] = std::pair<std::string, std::string>("key_111_test", "myvalue_111_test");
                imap_->put<std::string, std::string>("key_22_test", "myvalue_22_test").get();
                expectedStrEntries[13] = std::pair<std::string, std::string>("key_22_test", "myvalue_22_test");

// like_predicate
// value LIKE "value1" : {"value1"}
                std::vector<std::pair<std::string, std::string> > strEntries = imap_->entry_set<std::string, std::string>(
                        query::like_predicate(client_, query::query_constants::THIS_ATTRIBUTE_NAME, "value1")).get();
                ASSERT_EQ(1, (int) strEntries.size());
                ASSERT_EQ(expectedStrEntries[1], strEntries[0]);

// ilike_predicate
// value ILIKE "%VALue%1%" : {"key_111_test", "key1", "key10", "key11"}
                strEntries = imap_->entry_set<std::string, std::string>(
                        query::ilike_predicate(client_, query::query_constants::THIS_ATTRIBUTE_NAME, "%VALue%1%")).get();
                ASSERT_EQ(4, (int) strEntries.size());
                std::sort(strEntries.begin(), strEntries.end());
                ASSERT_EQ(expectedStrEntries[1], strEntries[0]);
                ASSERT_EQ(expectedStrEntries[10], strEntries[1]);
                ASSERT_EQ(expectedStrEntries[11], strEntries[2]);
                ASSERT_EQ(expectedStrEntries[12], strEntries[3]);

// key ILIKE "%VAL%2%" : {"key_22_test", "key2"}
                strEntries = imap_->entry_set<std::string, std::string>(
                        query::ilike_predicate(client_, query::query_constants::THIS_ATTRIBUTE_NAME, "%VAL%2%")).get();
                ASSERT_EQ(2, (int) strEntries.size());
                std::sort(strEntries.begin(), strEntries.end());
                ASSERT_EQ(expectedStrEntries[2], strEntries[0]);
                ASSERT_EQ(expectedStrEntries[13], strEntries[1]);

// sql_predicate
// __key BETWEEN 4 and 7 : {4, 5, 6, 7} -> {8, 10, 12, 14}
                char sql[100];
                hazelcast::util::hz_snprintf(sql, 50, "%s BETWEEN 4 and 7",
                                             query::query_constants::KEY_ATTRIBUTE_NAME);
                entries = int_map_->entry_set<int, int>(query::sql_predicate(client_, sql)).get();
                ASSERT_EQ(4, (int) entries.size());
                std::sort(entries.begin(), entries.end());
                for (int i = 0; i < 4; ++i) {
                    ASSERT_EQ(expected[i + 4], entries[i]);
                }

// regex_predicate
// value matches the regex ".*value.*2.*" : {key_22_test, value2}
                strEntries = imap_->entry_set<std::string, std::string>(
                        query::regex_predicate(client_, query::query_constants::THIS_ATTRIBUTE_NAME, ".*value.*2.*")).get();
                ASSERT_EQ(2, (int) strEntries.size());
                std::sort(strEntries.begin(), strEntries.end());
                ASSERT_EQ(expectedStrEntries[2], strEntries[0]);
                ASSERT_EQ(expectedStrEntries[13], strEntries[1]);
            }

            TEST_P(ClientMapTest, testEntrySetWithpaging_predicate) {
                int predSize = 5;
                const int totalEntries = 25;

                for (int i = 0; i < totalEntries; ++i) {
                    int_map_->put(i, i).get();
                }

                auto predicate = int_map_->new_paging_predicate<int, int>((size_t) predSize);

                std::vector<std::pair<int, int> > values = int_map_->entry_set<int, int>(predicate).get();
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    std::pair<int, int> value(i, i);
                    ASSERT_EQ(value, values[i]);
                }

                values = int_map_->entry_set<int, int>(predicate).get();
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    std::pair<int, int> value(i, i);
                    ASSERT_EQ(value, values[i]);
                }

                predicate.next_page();
                values = int_map_->entry_set<int, int>(predicate).get();
                ASSERT_EQ(predSize, (int) values.size());

                for (int i = 0; i < predSize; ++i) {
                    std::pair<int, int> value(predSize + i, predSize + i);
                    ASSERT_EQ(value, values[i]);
                }

                ASSERT_EQ(1, (int) predicate.get_page());

                predicate.set_page(4);

                values = int_map_->entry_set<int, int>(predicate).get();
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    std::pair<int, int> value(predSize * 4 + i, predSize * 4 + i);
                    ASSERT_EQ(value, values[i]);
                }

                predicate.next_page();
                values = int_map_->entry_set<int, int>(predicate).get();
                ASSERT_EQ(0, (int) values.size());

                predicate.set_page(0);
                values = int_map_->entry_set<int, int>(predicate).get();
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    std::pair<int, int> value(i, i);
                    ASSERT_EQ(value, values[i]);
                }

                predicate.previous_page();
                ASSERT_EQ(0, (int) predicate.get_page());

                predicate.set_page(5);
                values = int_map_->entry_set<int, int>(predicate).get();
                ASSERT_EQ(0, (int) values.size());

                predicate.set_page(3);
                values = int_map_->entry_set<int, int>(predicate).get();
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    std::pair<int, int> value(3 * predSize + i, 3 * predSize + i);
                    ASSERT_EQ(value, values[i]);
                }

                predicate.previous_page();
                values = int_map_->entry_set<int, int>(predicate).get();
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    std::pair<int, int> value(2 * predSize + i, 2 * predSize + i);
                    ASSERT_EQ(value, values[i]);
                }

// test paging_predicate with inner predicate (value < 10)
                query::greater_less_predicate lessThanTenPredicate(client_, query::query_constants::THIS_ATTRIBUTE_NAME, 9, false, true);
                auto predicate2 = int_map_->new_paging_predicate<int, int>(5, lessThanTenPredicate);
                values = int_map_->entry_set<int, int>(predicate2).get();
                ASSERT_EQ(predSize, (int) values.size());
                for (int i = 0; i < predSize; ++i) {
                    std::pair<int, int> value(i, i);
                    ASSERT_EQ(value, values[i]);
                }

                predicate2.next_page();
// match values 5,6, 7, 8
                values = int_map_->entry_set<int, int>(predicate2).get();
                ASSERT_EQ(predSize - 1, (int) values.size());
                for (int i = 0; i < predSize - 1; ++i) {
                    std::pair<int, int> value(predSize + i, predSize + i);
                    ASSERT_EQ(value, values[i]);
                }

                predicate2.next_page();
                values = int_map_->entry_set<int, int>(predicate2).get();
                ASSERT_EQ(0, (int) values.size());

// test paging predicate with comparator
                employee empl1("ahmet", 35);
                employee empl2("mehmet", 21);
                employee empl3("deniz", 25);
                employee empl4("ali", 33);
                employee empl5("veli", 44);
                employee empl6("aylin", 5);

                employees_->put(3, empl1).get();
                employees_->put(4, empl2).get();
                employees_->put(5, empl3).get();
                employees_->put(6, empl4).get();
                employees_->put(7, empl5).get();
                employees_->put(8, empl6).get();

                predSize = 2;
                auto predicate3 = int_map_->new_paging_predicate<int, employee>(EmployeeEntryComparator(), (size_t) predSize);
                std::vector<std::pair<int, employee> > result = employees_->entry_set<int, employee>(
                        predicate3).get();
                ASSERT_EQ(2, (int) result.size());
                std::pair<int, employee> value(8, empl6);
                ASSERT_EQ(value, result[0]);
                value = std::pair<int, employee>(4, empl2);
                ASSERT_EQ(value, result[1]);

                predicate3.next_page();
                result = employees_->entry_set<int, employee>(predicate3).get();
                ASSERT_EQ(2, (int) result.size());
                value = std::pair<int, employee>(5, empl3);
                ASSERT_EQ(value, result[0]);
                value = std::pair<int, employee>(6, empl4);
                ASSERT_EQ(value, result[1]);
            }

            TEST_P(ClientMapTest, testReplace) {
                boost::optional<std::string> temp = imap_->replace<std::string, std::string>("key1", "value").get();
                ASSERT_FALSE(temp);

                std::string tempKey = "key1";
                std::string tempValue = "value1";
                imap_->put<std::string, std::string>(tempKey, tempValue).get();

                ASSERT_EQ("value1", (imap_->replace<std::string, std::string>("key1", "value2").get().value()));
                ASSERT_EQ("value2", (imap_->get<std::string, std::string>("key1").get().value()));

                ASSERT_FALSE((imap_->replace<std::string, std::string>("key1", "value1", "value3").get()));
                ASSERT_EQ("value2", (imap_->get<std::string, std::string>("key1").get().value()));

                ASSERT_TRUE((imap_->replace<std::string, std::string>("key1", "value2", "value3").get()));
                ASSERT_EQ("value3", (imap_->get<std::string, std::string>("key1").get().value()));
            }

            TEST_P(ClientMapTest, testListenerWithPortableKey) {
                std::shared_ptr<imap> tradeMap = client_.get_map("tradeMap").get();
                boost::latch countDownLatch(1);
                std::atomic<int> atomicInteger(0);

                entry_listener listener;
                listener.
                    on_added([&countDownLatch, &atomicInteger](entry_event &&event) {
                        ++atomicInteger;
                        countDownLatch.count_down();
                    });

                employee key("a", 1);
                auto id = tradeMap->add_entry_listener(std::move(listener), true, key).get();
                employee key2("a", 2);
                tradeMap->put<employee, int>(key2, 1).get();
                tradeMap->put<employee, int>(key, 3).get();
                ASSERT_OPEN_EVENTUALLY(countDownLatch);
                ASSERT_EQ(1, (int) atomicInteger);

                ASSERT_TRUE(tradeMap->remove_entry_listener(id).get());
            }

            TEST_P(ClientMapTest, testListener) {
                boost::latch latch1Add(5);
                boost::latch latch1Remove(2);
                boost::latch dummy(10);
                boost::latch latch2Add(1);
                boost::latch latch2Remove(1);

                auto listener1 = make_countdown_listener(
                        latch1Add, latch1Remove, dummy, dummy);
                auto listener2 = make_countdown_listener(
                        latch2Add, latch2Remove, dummy, dummy);

                auto listener1ID = imap_->add_entry_listener(std::move(listener1), false).get();
                auto listener2ID = imap_->add_entry_listener(std::move(listener2), true, "key3").get();

                std::this_thread::sleep_for(std::chrono::seconds(2));

                imap_->put<std::string, std::string>("key1", "value1").get();
                imap_->put<std::string, std::string>("key2", "value2").get();
                imap_->put<std::string, std::string>("key3", "value3").get();
                imap_->put<std::string, std::string>("key4", "value4").get();
                imap_->put<std::string, std::string>("key5", "value5").get();

                imap_->remove<std::string, std::string>("key1").get();
                imap_->remove<std::string, std::string>("key3").get();

                ASSERT_OPEN_EVENTUALLY(latch1Add);
                ASSERT_OPEN_EVENTUALLY(latch1Remove);
                ASSERT_OPEN_EVENTUALLY(latch2Add);
                ASSERT_OPEN_EVENTUALLY(latch2Remove);

                ASSERT_TRUE(imap_->remove_entry_listener(listener1ID).get());
                ASSERT_TRUE(imap_->remove_entry_listener(listener2ID).get());
            }

            TEST_P(ClientMapTest, testListenerWithtrue_predicate) {
                boost::latch latchAdd(3), latchRemove(1), latchEvict(1), latchUpdate(1);

                auto listener = make_countdown_listener(latchAdd, latchRemove, latchUpdate, latchEvict);

                auto listenerId = int_map_->add_entry_listener(std::move(listener), query::true_predicate(client_), false).get();

                int_map_->put(1, 1).get();
                int_map_->put(2, 2).get();
                int_map_->put(3, 3, std::chrono::seconds(1)).get(); // evict after 1 second
                int_map_->remove<int, int>(2).get();

                std::this_thread::sleep_for(std::chrono::seconds(2));

                ASSERT_FALSE((int_map_->get<int, int>(3).get().has_value())); // trigger eviction

                // update an entry
                int_map_->set(1, 5).get();
                boost::optional<int> value = int_map_->get<int, int>(1).get();
                ASSERT_TRUE(value.has_value());
                ASSERT_EQ(5, value.value());

                CountDownLatchWaiter latches;
                latches.add(latchAdd).add(latchRemove).add(latchUpdate).add(latchEvict);
                ASSERT_EQ(boost::cv_status::no_timeout, latches.wait_for(boost::chrono::seconds(2)));

                ASSERT_TRUE(int_map_->remove_entry_listener(listenerId).get());
            }

            TEST_P(ClientMapTest, testListenerWithfalse_predicate) {
                boost::latch latchAdd(3), latchRemove(1), latchEvict(1), latchUpdate(1);

                auto listener = make_countdown_listener(latchAdd, latchRemove, latchUpdate, latchEvict);

                auto listenerId = int_map_->add_entry_listener(std::move(listener), query::false_predicate(client_), false).get();

                int_map_->put(1, 1).get();
                int_map_->put(2, 2).get();
                int_map_->put(3, 3, std::chrono::seconds(1)).get(); // evict after 1 second
                int_map_->remove<int, int>(2).get();

                std::this_thread::sleep_for(std::chrono::seconds(2));

                ASSERT_FALSE((int_map_->get<int, int>(3).get().has_value())); // trigger eviction

// update an entry
                int_map_->set(1, 5).get();
                boost::optional<int> value = int_map_->get<int, int>(1).get();
                ASSERT_TRUE(value.has_value());
                ASSERT_EQ(5, value.value());

                CountDownLatchWaiter latches;
                latches.add(latchAdd).add(latchRemove).add(latchUpdate).add(latchEvict);
                ASSERT_EQ(boost::cv_status::timeout, latches.wait_for(boost::chrono::seconds(2)));

                ASSERT_TRUE(int_map_->remove_entry_listener(listenerId).get());
            }

            TEST_P(ClientMapTest, testListenerWithequal_predicate) {
                boost::latch latchAdd(1), latchRemove(1), latchEvict(1), latchUpdate(1);

                auto listener = make_countdown_listener(latchAdd, latchRemove, latchUpdate, latchEvict);

                auto listenerId = int_map_->add_entry_listener(std::move(listener),
                                                                 query::equal_predicate(client_,
                                                                                        query::query_constants::KEY_ATTRIBUTE_NAME,
                                                                                        3), true).get();

                int_map_->put(1, 1).get();
                int_map_->put(2, 2).get();
                int_map_->put(3, 3, std::chrono::seconds(1)).get(); // evict after 1 second
                int_map_->remove<int, int>(2).get();

                std::this_thread::sleep_for(std::chrono::seconds(2));

                ASSERT_FALSE((int_map_->get<int, int>(3).get().has_value())); // trigger eviction

// update an entry
                int_map_->set(1, 5).get();
                boost::optional<int> value = int_map_->get<int, int>(1).get();
                ASSERT_TRUE(value.has_value());
                ASSERT_EQ(5, value.value());

                CountDownLatchWaiter latches;
                latches.add(latchAdd).add(latchEvict);
                ASSERT_EQ(boost::cv_status::no_timeout, latches.wait_for(boost::chrono::seconds(2)));

                latches.reset();
                latches.add(latchUpdate).add(latchRemove);
                ASSERT_EQ(boost::cv_status::timeout, latches.wait_for(boost::chrono::seconds(2)));

                ASSERT_TRUE(int_map_->remove_entry_listener(listenerId).get());
            }

            TEST_P(ClientMapTest, testListenerWithnot_equal_predicate) {
                boost::latch latchAdd(2), latchRemove(1), latchEvict(1), latchUpdate(1);

                auto listener = make_countdown_listener(latchAdd, latchRemove, latchUpdate, latchEvict);

                auto listenerId = int_map_->add_entry_listener(std::move(listener),
                                                                 query::not_equal_predicate(client_,
                                                                                            query::query_constants::KEY_ATTRIBUTE_NAME,
                                                                                            3), true).get();

                int_map_->put(1, 1).get();
                int_map_->put(2, 2).get();
                int_map_->put(3, 3, std::chrono::seconds(1)).get(); // evict after 1 second
                int_map_->remove<int, int>(2).get();

                std::this_thread::sleep_for(std::chrono::seconds(2));

                ASSERT_FALSE((int_map_->get<int, int>(3).get().has_value())); // trigger eviction

// update an entry
                int_map_->set(1, 5).get();
                boost::optional<int> value = int_map_->get<int, int>(1).get();
                ASSERT_TRUE(value.has_value());
                ASSERT_EQ(5, value.value());

                CountDownLatchWaiter latches;
                latches.add(latchAdd).add(latchRemove).add(latchUpdate);
                ASSERT_EQ(boost::cv_status::no_timeout, latches.wait_for(boost::chrono::seconds(2)));

                latches.reset();
                latches.add(latchEvict);
                ASSERT_EQ(boost::cv_status::timeout, latches.wait_for(boost::chrono::seconds(2)));

                ASSERT_TRUE(int_map_->remove_entry_listener(listenerId).get());
            }

            TEST_P(ClientMapTest, testListenerWithgreater_less_predicate) {
                boost::latch latchAdd(2), latchRemove(1), latchEvict(1), latchUpdate(1);

                auto listener = make_countdown_listener(latchAdd, latchRemove,
                                                     latchUpdate,
                                                     latchEvict);
// key <= 2
                auto listenerId = int_map_->add_entry_listener(std::move(listener),
                                                                 query::greater_less_predicate(client_,
                                                                                               query::query_constants::KEY_ATTRIBUTE_NAME,
                                                                                               2, true, true),
                                                                 false).get();

                int_map_->put(1, 1).get();
                int_map_->put(2, 2).get();
                int_map_->put(3, 3, std::chrono::seconds(1)).get(); // evict after 1 second
                int_map_->remove<int, int>(2).get();

                std::this_thread::sleep_for(std::chrono::seconds(2));

                ASSERT_FALSE((int_map_->get<int, int>(3).get().has_value())); // trigger eviction

// update an entry
                int_map_->set(1, 5).get();
                boost::optional<int> value = int_map_->get<int, int>(1).get();
                ASSERT_TRUE(value.has_value());
                ASSERT_EQ(5, value.value());

                CountDownLatchWaiter latches;
                latches.add(latchAdd).add(latchRemove).add(latchUpdate);
                ASSERT_EQ(boost::cv_status::no_timeout, latches.wait_for(boost::chrono::seconds(2)));

                ASSERT_EQ(boost::cv_status::timeout, latchEvict.wait_for(boost::chrono::seconds(2)));

                ASSERT_TRUE(int_map_->remove_entry_listener(listenerId).get());
            }

            TEST_P(ClientMapTest, testListenerWithbetween_predicate) {
                boost::latch latchAdd(2), latchRemove(1), latchEvict(1), latchUpdate(1);

                auto listener = make_countdown_listener(latchAdd, latchRemove, latchUpdate, latchEvict);

// 1 <=key <= 2
                auto listenerId = int_map_->add_entry_listener(std::move(listener),
                                                                 query::between_predicate(client_,
                                                                                          query::query_constants::KEY_ATTRIBUTE_NAME,
                                                                                          1, 2), true).get();

                int_map_->put(1, 1).get();
                int_map_->put(2, 2).get();
                int_map_->put(3, 3, std::chrono::seconds(1)).get(); // evict after 1 second
                int_map_->remove<int, int>(2).get();

                std::this_thread::sleep_for(std::chrono::seconds(2));

                ASSERT_FALSE((int_map_->get<int, int>(3).get().has_value())); // trigger eviction

// update an entry
                int_map_->set(1, 5).get();
                boost::optional<int> value = int_map_->get<int, int>(1).get();
                ASSERT_TRUE(value.has_value());
                ASSERT_EQ(5, value.value());

                CountDownLatchWaiter latches;
                latches.add(latchAdd).add(latchRemove).add(latchUpdate);
                ASSERT_EQ(boost::cv_status::no_timeout, latches.wait_for(boost::chrono::seconds(2)));

                ASSERT_EQ(boost::cv_status::timeout, latchEvict.wait_for(boost::chrono::seconds(2)));

                ASSERT_TRUE(int_map_->remove_entry_listener(listenerId).get());
            }

            TEST_P(ClientMapTest, testListenerWithsql_predicate) {
                boost::latch latchAdd(1), latchRemove(1), latchEvict(1), latchUpdate(1);

                auto listener = make_countdown_listener(latchAdd, latchRemove, latchUpdate, latchEvict);

// 1 <=key <= 2
                auto listenerId = int_map_->add_entry_listener(std::move(listener), query::sql_predicate(client_, "__key < 2"), true).get();

                int_map_->put(1, 1).get();
                int_map_->put(2, 2).get();
                int_map_->put(3, 3, std::chrono::seconds(1)).get(); // evict after 1 second
                int_map_->remove<int, int>(2).get();

                std::this_thread::sleep_for(std::chrono::seconds(2));

                ASSERT_FALSE((int_map_->get<int, int>(3).get().has_value())); // trigger eviction

// update an entry
                int_map_->set(1, 5).get();
                boost::optional<int> value = int_map_->get<int, int>(1).get();
                ASSERT_TRUE(value.has_value());
                ASSERT_EQ(5, value.value());

                CountDownLatchWaiter latches;
                latches.add(latchAdd).add(latchUpdate);
                ASSERT_EQ(boost::cv_status::no_timeout, latches.wait_for(boost::chrono::seconds(2)));

                latches.reset();
                latches.add(latchRemove).add(latchEvict);
                ASSERT_EQ(boost::cv_status::timeout, latches.wait_for(boost::chrono::seconds(2)));

                ASSERT_TRUE(int_map_->remove_entry_listener(listenerId).get());
            }

            TEST_P(ClientMapTest, testListenerWithRegExPredicate) {
                boost::latch latchAdd(2), latchRemove(1), latchEvict(1), latchUpdate(1);

                auto listener = make_countdown_listener(
                        latchAdd, latchRemove, latchUpdate, latchEvict);

// key matches any word containing ".*met.*"
                auto listenerId = imap_->add_entry_listener(std::move(listener),
                                                               query::regex_predicate(client_,
                                                                                      query::query_constants::KEY_ATTRIBUTE_NAME,
                                                                                      ".*met.*"), true).get();

                imap_->put<std::string, std::string>("ilkay", "yasar").get();
                imap_->put<std::string, std::string>("mehmet", "demir").get();
                imap_->put<std::string, std::string>("metin", "ozen", std::chrono::seconds(1)).get(); // evict after 1 second
                imap_->put<std::string, std::string>("hasan", "can").get();
                imap_->remove<std::string, std::string>("mehmet").get();

                std::this_thread::sleep_for(std::chrono::seconds(2));

                ASSERT_FALSE((imap_->get<std::string, std::string>("metin").get().has_value())); // trigger eviction

// update an entry
                imap_->set("hasan", "suphi").get();
                boost::optional<std::string> value = imap_->get<std::string, std::string>("hasan").get();
                ASSERT_TRUE(value.has_value());
                ASSERT_EQ("suphi", value.value());

                CountDownLatchWaiter latches;
                latches.add(latchAdd).add(latchRemove).add(latchEvict);
                ASSERT_EQ(boost::cv_status::no_timeout, latches.wait_for(boost::chrono::seconds(2)));

                ASSERT_EQ(boost::cv_status::timeout, latchUpdate.wait_for(boost::chrono::seconds(2)));

                ASSERT_TRUE(imap_->remove_entry_listener(listenerId).get());
            }

            TEST_P(ClientMapTest, testListenerWithinstance_of_predicate) {
                boost::latch latchAdd(3), latchRemove(1), latchEvict(1), latchUpdate(1);
                auto listener = make_countdown_listener(latchAdd, latchRemove,latchUpdate,latchEvict);
// 1 <=key <= 2
                auto listenerId = int_map_->add_entry_listener(std::move(listener),
                                                                 query::instance_of_predicate(client_,
                                                                                              "java.lang.Integer"),
                                                                 false).get();

                int_map_->put(1, 1).get();
                int_map_->put(2, 2).get();
                int_map_->put(3, 3, std::chrono::seconds(1)).get(); // evict after 1 second
                int_map_->remove<int, int>(2).get();

                std::this_thread::sleep_for(std::chrono::seconds(2));

                ASSERT_FALSE((int_map_->get<int, int>(3).get().has_value())); // trigger eviction

// update an entry
                int_map_->set(1, 5).get();
                boost::optional<int> value = int_map_->get<int, int>(1).get();
                ASSERT_TRUE(value.has_value());
                ASSERT_EQ(5, value.value());

                CountDownLatchWaiter latches;
                latches.add(latchAdd).add(latchRemove).add(latchUpdate).add(latchEvict);
                ASSERT_EQ(boost::cv_status::no_timeout, latches.wait_for(boost::chrono::seconds(2)));

                ASSERT_TRUE(int_map_->remove_entry_listener(listenerId).get());
            }

            TEST_P(ClientMapTest, testListenerWithnot_predicate) {
                boost::latch latchAdd(2), latchRemove(1), latchEvict(1), latchUpdate(1);
                auto listener = make_countdown_listener(latchAdd, latchRemove,latchUpdate,latchEvict);
                // key >= 3
                query::not_predicate notPredicate(client_, query::greater_less_predicate(client_,
                                                                                         query::query_constants::KEY_ATTRIBUTE_NAME,
                                                                                         3, true, false));
                auto listenerId = int_map_->add_entry_listener(std::move(listener), notPredicate,false).get();

                int_map_->put(1, 1).get();
                int_map_->put(2, 2).get();
                int_map_->put(3, 3, std::chrono::seconds(1)).get(); // evict after 1 second
                int_map_->remove<int, int>(2).get();

                std::this_thread::sleep_for(std::chrono::seconds(2));

                ASSERT_FALSE((int_map_->get<int, int>(3).get().has_value())); // trigger eviction

// update an entry
                int_map_->set(1, 5).get();
                boost::optional<int> value = int_map_->get<int, int>(1).get();
                ASSERT_TRUE(value.has_value());
                ASSERT_EQ(5, value.value());

                CountDownLatchWaiter latches;
                latches.add(latchAdd).add(latchRemove).add(latchUpdate);
                ASSERT_EQ(boost::cv_status::no_timeout, latches.wait_for(boost::chrono::seconds(2)));

                latches.reset();
                latches.add(latchEvict);
                ASSERT_EQ(boost::cv_status::timeout, latches.wait_for(boost::chrono::seconds(1)));

                ASSERT_TRUE(int_map_->remove_entry_listener(listenerId).get());
            }

            TEST_P(ClientMapTest, testListenerWithand_predicate) {
                boost::latch latchAdd(1), latchRemove(1), latchEvict(1), latchUpdate(1);
                auto listener = make_countdown_listener(latchAdd, latchRemove,latchUpdate,latchEvict);

// key < 3
                query::greater_less_predicate greaterLessPred(client_, query::query_constants::KEY_ATTRIBUTE_NAME, 3, false, true);
// value == 1
                query::equal_predicate equalPred(client_, query::query_constants::KEY_ATTRIBUTE_NAME, 1);
// key < 3 AND key == 1 --> (1, 1)
                query::and_predicate predicate(client_, greaterLessPred, equalPred);
                auto listenerId = int_map_->add_entry_listener(std::move(listener), predicate, false).get();

                int_map_->put(1, 1).get();
                int_map_->put(2, 2).get();
                int_map_->put(3, 3, std::chrono::seconds(1)).get(); // evict after 1 second
                int_map_->remove<int, int>(2).get();

                std::this_thread::sleep_for(std::chrono::seconds(2));

                ASSERT_FALSE((int_map_->get<int, int>(3).get().has_value())); // trigger eviction

// update an entry
                int_map_->set(1, 5).get();
                boost::optional<int> value = int_map_->get<int, int>(1).get();
                ASSERT_TRUE(value.has_value());
                ASSERT_EQ(5, value.value());

                CountDownLatchWaiter latches;
                latches.add(latchAdd).add(latchUpdate);
                ASSERT_EQ(boost::cv_status::no_timeout, latches.wait_for(boost::chrono::seconds(2)));

                latches.reset();
                latches.add(latchEvict).add(latchRemove);
                ASSERT_EQ(boost::cv_status::timeout, latches.wait_for(boost::chrono::seconds(1)));

                ASSERT_TRUE(int_map_->remove_entry_listener(listenerId).get());
            }

            TEST_P(ClientMapTest, testListenerWithor_predicate) {
                boost::latch latchAdd(2), latchRemove(1), latchEvict(1), latchUpdate(1);

                auto listener = make_countdown_listener(latchAdd, latchRemove,
                                                     latchUpdate,
                                                     latchEvict);

                // key < 3
                query::greater_less_predicate greaterLessPred(client_, query::query_constants::KEY_ATTRIBUTE_NAME, 3, true, false);
                // value == 1
                query::equal_predicate equalPred(client_, query::query_constants::THIS_ATTRIBUTE_NAME, 2);
                // key >= 3 OR value == 2 --> (1, 1), (2, 2)
                query::or_predicate predicate(client_, greaterLessPred, equalPred);
                auto listenerId = int_map_->add_entry_listener(std::move(listener), predicate, true).get();

                int_map_->put(1, 1).get();
                int_map_->put(2, 2).get();
                int_map_->put(3, 3, std::chrono::seconds(1)).get(); // evict after 1 second
                int_map_->remove<int, int>(2).get();

                std::this_thread::sleep_for(std::chrono::seconds(2));

                ASSERT_FALSE((int_map_->get<int, int>(3).get().has_value())); // trigger eviction

// update an entry
                int_map_->set(1, 5).get();
                boost::optional<int> value = int_map_->get<int, int>(1).get();
                ASSERT_TRUE(value.has_value());
                ASSERT_EQ(5, value.value());

                CountDownLatchWaiter latches;
                latches.add(latchAdd).add(latchEvict).add(latchRemove);
                ASSERT_EQ(boost::cv_status::no_timeout, latches.wait_for(boost::chrono::seconds(2)));

                ASSERT_EQ(boost::cv_status::timeout, latchUpdate.wait_for(boost::chrono::seconds(2)));

                ASSERT_TRUE(int_map_->remove_entry_listener(listenerId).get());
            }

            TEST_P(ClientMapTest, testClearEvent) {
                boost::latch latch1(1);

                entry_listener clearListener;
                clearListener.
                    on_map_cleared([&latch1](map_event &&) {
                        latch1.count_down();
                    });

                auto listenerId = imap_->add_entry_listener(std::move(clearListener), false).get();
                imap_->put<std::string, std::string>("key1", "value1").get();
                imap_->clear().get();
                ASSERT_OPEN_EVENTUALLY(latch1);
                imap_->remove_entry_listener(listenerId).get();
            }

            TEST_P(ClientMapTest, testEvictAllEvent) {
                boost::latch latch1(1);
                entry_listener evictListener;

                evictListener.
                    on_map_evicted([&latch1](map_event &&event) {
                        latch1.count_down();
                    });

                auto listenerId = imap_->add_entry_listener(std::move(evictListener), false).get();
                imap_->put<std::string, std::string>("key1", "value1").get();
                imap_->evict_all().get();
                ASSERT_OPEN_EVENTUALLY(latch1);
                imap_->remove_entry_listener(listenerId).get();
            }

            TEST_P(ClientMapTest, testMapWithPortable) {
                boost::optional<employee> n1 = employees_->get<int, employee>(1).get();
                ASSERT_FALSE(n1);
                employee e("sancar", 24);
                boost::optional<employee> ptr = employees_->put(1, e).get();
                ASSERT_FALSE(ptr);
                ASSERT_FALSE(employees_->is_empty().get());
                entry_view<int, employee> view = employees_->get_entry_view<int, employee>(1).get().value();
                ASSERT_EQ(view.value, e);
                ASSERT_EQ(view.key, 1);

                employees_->add_index(config::index_config::index_type::SORTED, std::string("a")).get();
                employees_->add_index(config::index_config::index_type::HASH, std::string("n")).get();
            }

            TEST_P(ClientMapTest, testMapStoreRelatedRequests) {
                imap_->put_transient<std::string, std::string>("ali", "veli", std::chrono::milliseconds(1100)).get();
                imap_->flush().get();
                ASSERT_EQ(1, imap_->size().get());
                ASSERT_FALSE(imap_->evict("deli").get());
                ASSERT_TRUE(imap_->evict("ali").get());
                ASSERT_FALSE((imap_->get<std::string, std::string>("ali").get().has_value()));
            }

            TEST_P(ClientMapTest, testExecuteOnKey) {
                employee empl1("ahmet", 35);
                employee empl2("mehmet", 21);

                employees_->put(3, empl1).get();
                employees_->put(4, empl2).get();

                EntryMultiplier processor(4);

                boost::optional<int> result = employees_->execute_on_key<int, int, EntryMultiplier>(4, processor).get();

                ASSERT_TRUE(result.has_value());
                ASSERT_EQ(4 * processor.get_multiplier(), result.value());
            }

            TEST_P(ClientMapTest, testSubmitToKey) {
                employee empl1("ahmet", 35);
                employee empl2("mehmet", 21);

                employees_->put(3, empl1).get();
                employees_->put(4, empl2).get();

                EntryMultiplier processor(4);

                auto result = employees_->submit_to_key<int, int, EntryMultiplier>(4, processor).get();
                ASSERT_TRUE(result.has_value());
                ASSERT_EQ(4 * processor.get_multiplier(), result.value());
            }

            TEST_P(ClientMapTest, testExecuteOnNonExistentKey) {
                EntryMultiplier processor(4);

                boost::optional<int> result = employees_->execute_on_key<int, int, EntryMultiplier>(
                        17, processor).get();

                ASSERT_TRUE(result.has_value());
                ASSERT_EQ(-1, result.value());
            }

            TEST_P(ClientMapTest, testExecuteOnKeys) {
                employee empl1("ahmet", 35);
                employee empl2("mehmet", 21);
                employee empl3("deniz", 25);

                employees_->put(3, empl1).get();
                employees_->put(4, empl2).get();
                employees_->put(5, empl3).get();

                EntryMultiplier processor(4);

                std::unordered_set<int> keys;
                keys.insert(3);
                keys.insert(5);
// put non existent key
                keys.insert(999);

                std::unordered_map<int, boost::optional<int> > result = employees_->execute_on_keys<int, int, EntryMultiplier>(
                        keys, processor).get();

                ASSERT_EQ(3, (int) result.size());
                ASSERT_NE(result.end(), result.find(3));
                ASSERT_NE(result.end(), result.find(5));
                ASSERT_NE(result.end(), result.find(999));
                ASSERT_EQ(3 * processor.get_multiplier(), result[3].value());
                ASSERT_EQ(5 * processor.get_multiplier(), result[5].value());
                ASSERT_EQ(-1, result[999].value());
            }

            TEST_P(ClientMapTest, testExecuteOnEntries) {
                employee empl1("ahmet", 35);
                employee empl2("mehmet", 21);
                employee empl3("deniz", 25);

                employees_->put(3, empl1).get();
                employees_->put(4, empl2).get();
                employees_->put(5, empl3).get();

                EntryMultiplier processor(4);

                std::unordered_map<int, boost::optional<int> > result = employees_->execute_on_entries<int, int, EntryMultiplier>(
                        processor).get();

                ASSERT_EQ(3, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(3)));
                ASSERT_TRUE((result.end() != result.find(4)));
                ASSERT_TRUE((result.end() != result.find(5)));
                ASSERT_EQ(3 * processor.get_multiplier(), result[3].value());
                ASSERT_EQ(4 * processor.get_multiplier(), result[4].value());
                ASSERT_EQ(5 * processor.get_multiplier(), result[5].value());
            }

            TEST_P(ClientMapTest, testExecuteOnEntriesWithtrue_predicate) {
                employee empl1("ahmet", 35);
                employee empl2("mehmet", 21);
                employee empl3("deniz", 25);

                employees_->put(3, empl1).get();
                employees_->put(4, empl2).get();
                employees_->put(5, empl3).get();

                EntryMultiplier processor(4);

                std::unordered_map<int, boost::optional<int>> result = employees_->execute_on_entries<int, int, EntryMultiplier>(
                        processor, query::true_predicate(client_)).get();

                ASSERT_EQ(3, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(3)));
                ASSERT_TRUE((result.end() != result.find(4)));
                ASSERT_TRUE((result.end() != result.find(5)));
                ASSERT_EQ(3 * processor.get_multiplier(), result[3].value());
                ASSERT_EQ(4 * processor.get_multiplier(), result[4].value());
                ASSERT_EQ(5 * processor.get_multiplier(), result[5].value());
            }

            TEST_P(ClientMapTest, testExecuteOnEntriesWithfalse_predicate) {
                employee empl1("ahmet", 35);
                employee empl2("mehmet", 21);
                employee empl3("deniz", 25);

                employees_->put(3, empl1).get();
                employees_->put(4, empl2).get();
                employees_->put(5, empl3).get();

                EntryMultiplier processor(4);

                std::unordered_map<int, boost::optional<int> > result = employees_->execute_on_entries<int, int, EntryMultiplier>(
                        processor, query::false_predicate(client_)).get();

                ASSERT_EQ(0, (int) result.size());
            }

            TEST_P(ClientMapTest, testExecuteOnEntriesWithand_predicate) {
                employee empl1("ahmet", 35);
                employee empl2("mehmet", 21);
                employee empl3("deniz", 25);

                employees_->put(3, empl1).get();
                employees_->put(4, empl2).get();
                employees_->put(5, empl3).get();

                /* 25 <= age <= 35 AND age = 35 */
                query::and_predicate andPredicate(client_, query::between_predicate(client_, "a", 25, 35),
                                                  query::not_predicate(client_, query::equal_predicate(client_, "a", 35)));

                EntryMultiplier processor(4);

                std::unordered_map<int, boost::optional<int> > result = employees_->execute_on_entries<int, int, EntryMultiplier>(
                        processor, andPredicate).get();

                ASSERT_EQ(1, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(5)));
                ASSERT_EQ(5 * processor.get_multiplier(), result[5].value());
            }

            TEST_P(ClientMapTest, testExecuteOnEntriesWithor_predicate) {
                employee empl1("ahmet", 35);
                employee empl2("mehmet", 21);
                employee empl3("deniz", 25);

                employees_->put(3, empl1).get();
                employees_->put(4, empl2).get();
                employees_->put(5, empl3).get();

                /* age == 21 OR age > 25 */
                query::or_predicate orPredicate(client_, query::equal_predicate(client_, "a", 21), query::greater_less_predicate(client_, "a", 25, false, false));

                EntryMultiplier processor(4);

                std::unordered_map<int, boost::optional<int> > result = employees_->execute_on_entries<int, int, EntryMultiplier>(
                        processor, orPredicate).get();

                ASSERT_EQ(2, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(3)));
                ASSERT_TRUE((result.end() != result.find(4)));
                ASSERT_EQ(3 * processor.get_multiplier(), result[3].value());
                ASSERT_EQ(4 * processor.get_multiplier(), result[4].value());
            }

            TEST_P(ClientMapTest, testExecuteOnEntriesWithbetween_predicate) {
                employee empl1("ahmet", 35);
                employee empl2("mehmet", 21);
                employee empl3("deniz", 25);

                employees_->put(3, empl1).get();
                employees_->put(4, empl2).get();
                employees_->put(5, empl3).get();

                EntryMultiplier processor(4);

                std::unordered_map<int, boost::optional<int> > result = employees_->execute_on_entries<int, int, EntryMultiplier>(
                        processor, query::between_predicate(client_, "a", 25, 35)).get();

                ASSERT_EQ(2, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(3)));
                ASSERT_TRUE((result.end() != result.find(5)));
                ASSERT_EQ(3 * processor.get_multiplier(), result[3].value());
                ASSERT_EQ(5 * processor.get_multiplier(), result[5].value());
            }

            TEST_P(ClientMapTest, testExecuteOnEntriesWithequal_predicate) {
                employee empl1("ahmet", 35);
                employee empl2("mehmet", 21);
                employee empl3("deniz", 25);

                employees_->put(3, empl1).get();
                employees_->put(4, empl2).get();
                employees_->put(5, empl3).get();

                EntryMultiplier processor(4);

                std::unordered_map<int, boost::optional<int> > result = employees_->execute_on_entries<int, int, EntryMultiplier>(
                        processor, query::equal_predicate(client_, "a", 25)).get();

                ASSERT_EQ(1, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(5)));

                result = employees_->execute_on_entries<int, int, EntryMultiplier>(
                        processor, query::equal_predicate(client_, "a", 10)).get();

                ASSERT_EQ(0, (int) result.size());
            }

            TEST_P(ClientMapTest, testExecuteOnEntriesWithnot_equal_predicate) {
                employee empl1("ahmet", 35);
                employee empl2("mehmet", 21);
                employee empl3("deniz", 25);

                employees_->put(3, empl1).get();
                employees_->put(4, empl2).get();
                employees_->put(5, empl3).get();

                EntryMultiplier processor(4);

                std::unordered_map<int, boost::optional<int> > result = employees_->execute_on_entries<int, int, EntryMultiplier>(
                        processor, query::not_equal_predicate(client_, "a", 25)).get();

                ASSERT_EQ(2, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(3)));
                ASSERT_TRUE((result.end() != result.find(4)));
            }

            TEST_P(ClientMapTest, testExecuteOnEntriesWithgreater_less_predicate) {
                employee empl1("ahmet", 35);
                employee empl2("mehmet", 21);
                employee empl3("deniz", 25);

                employees_->put(3, empl1).get();
                employees_->put(4, empl2).get();
                employees_->put(5, empl3).get();

                EntryMultiplier processor(4);

                std::unordered_map<int, boost::optional<int> > result = employees_->execute_on_entries<int, int, EntryMultiplier>(
                        processor, query::greater_less_predicate(client_, "a", 25, false, true)).get(); // <25 matching

                ASSERT_EQ(1, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(4)));

                result = employees_->execute_on_entries<int, int, EntryMultiplier>(
                        processor, query::greater_less_predicate(client_, "a", 25, true, true)).get(); // <=25 matching

                ASSERT_EQ(2, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(4)));
                ASSERT_TRUE((result.end() != result.find(5)));

                result = employees_->execute_on_entries<int, int, EntryMultiplier>(
                        processor, query::greater_less_predicate(client_, "a", 25, false, false)).get(); // >25 matching

                ASSERT_EQ(1, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(3)));

                result = employees_->execute_on_entries<int, int, EntryMultiplier>(
                        processor, query::greater_less_predicate(client_, "a", 25, true, false)).get(); // >=25 matching

                ASSERT_EQ(2, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(3)));
                ASSERT_TRUE((result.end() != result.find(5)));
            }

            TEST_P(ClientMapTest, testExecuteOnEntriesWithlike_predicate) {
                employee empl1("ahmet", 35);
                employee empl2("mehmet", 21);
                employee empl3("deniz", 25);

                employees_->put(3, empl1).get();
                employees_->put(4, empl2).get();
                employees_->put(5, empl3).get();

                EntryMultiplier processor(4);

                std::unordered_map<int, boost::optional<int> > result = employees_->execute_on_entries<int, int, EntryMultiplier>(
                        processor, query::like_predicate(client_, "n", "deniz")).get();

                ASSERT_EQ(1, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(5)));
            }

            TEST_P(ClientMapTest, testExecuteOnEntriesWithilike_predicate) {
                employee empl1("ahmet", 35);
                employee empl2("mehmet", 21);
                employee empl3("deniz", 25);

                employees_->put(3, empl1).get();
                employees_->put(4, empl2).get();
                employees_->put(5, empl3).get();

                EntryMultiplier processor(4);

                std::unordered_map<int, boost::optional<int> > result = employees_->execute_on_entries<int, int, EntryMultiplier>(
                        processor, query::ilike_predicate(client_, "n", "deniz")).get();

                ASSERT_EQ(1, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(5)));
            }

            TEST_P(ClientMapTest, testExecuteOnEntriesWithin_predicate) {
                employee empl1("ahmet", 35);
                employee empl2("mehmet", 21);
                employee empl3("deniz", 25);

                employees_->put(3, empl1).get();
                employees_->put(4, empl2).get();
                employees_->put(5, empl3).get();

                EntryMultiplier processor(4);

                query::in_predicate predicate(client_, "n", "ahmet", "mehmet");
                std::unordered_map<int, boost::optional<int> > result = employees_->execute_on_entries<int, int, EntryMultiplier>(
                        processor, predicate).get();

                ASSERT_EQ(2, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(3)));
                ASSERT_TRUE((result.end() != result.find(4)));
            }

            TEST_P(ClientMapTest, testExecuteOnEntriesWithinstance_of_predicate) {
                employee empl1("ahmet", 35);
                employee empl2("mehmet", 21);
                employee empl3("deniz", 25);

                employees_->put(3, empl1).get();
                employees_->put(4, empl2).get();
                employees_->put(5, empl3).get();

                EntryMultiplier processor(4);
                std::unordered_map<int, boost::optional<int> > result = employees_->execute_on_entries<int, int, EntryMultiplier>(
                        processor, query::instance_of_predicate(client_, "com.hazelcast.client.test.Employee")).get();

                ASSERT_EQ(3, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(3)));
                ASSERT_TRUE((result.end() != result.find(4)));
                ASSERT_TRUE((result.end() != result.find(5)));
            }

            TEST_P(ClientMapTest, testExecuteOnEntriesWithnot_predicate) {
                employee empl1("ahmet", 35);
                employee empl2("mehmet", 21);
                employee empl3("deniz", 25);

                employees_->put(3, empl1).get();
                employees_->put(4, empl2).get();
                employees_->put(5, empl3).get();

                EntryMultiplier processor(4);
                query::equal_predicate eqPredicate(client_, "a", 25);
                query::not_predicate notPredicate(client_, eqPredicate);
                std::unordered_map<int, boost::optional<int> > result = employees_->execute_on_entries<int, int, EntryMultiplier>(
                        processor, notPredicate).get();

                ASSERT_EQ(2, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(3)));
                ASSERT_TRUE((result.end() != result.find(4)));

                query::not_predicate notfalse_predicate(client_, query::false_predicate(client_));
                result = employees_->execute_on_entries<int, int, EntryMultiplier>(processor, notfalse_predicate).get();

                ASSERT_EQ(3, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(3)));
                ASSERT_TRUE((result.end() != result.find(4)));
                ASSERT_TRUE((result.end() != result.find(5)));

                query::not_predicate notbetween_predicate(client_, query::between_predicate(client_, "a", 25, 35));
                result = employees_->execute_on_entries<int, int, EntryMultiplier>(processor, notbetween_predicate).get();

                ASSERT_EQ(1, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(4)));
            }

            TEST_P(ClientMapTest, testExecuteOnEntriesWithregex_predicate) {
                employee empl1("ahmet", 35);
                employee empl2("mehmet", 21);
                employee empl3("deniz", 25);

                employees_->put(3, empl1).get();
                employees_->put(4, empl2).get();
                employees_->put(5, empl3).get();

                EntryMultiplier processor(4);

                std::unordered_map<int, boost::optional<int> > result = employees_->execute_on_entries<int, int, EntryMultiplier>(
                        processor, query::regex_predicate(client_, "n", ".*met")).get();

                ASSERT_EQ(2, (int) result.size());
                ASSERT_TRUE((result.end() != result.find(3)));
                ASSERT_TRUE((result.end() != result.find(4)));
            }

            TEST_P(ClientMapTest, testAddInterceptor) {
                std::string prefix("My Prefix");
                MapGetInterceptor interceptor(prefix);
                std::string interceptorId = imap_->add_interceptor<MapGetInterceptor>(interceptor).get();

                boost::optional<std::string> val = imap_->get<std::string, std::string>("nonexistent").get();
                ASSERT_TRUE(val);
                ASSERT_EQ(prefix, *val);

                val = imap_->put<std::string, std::string>("key1", "value1").get();
                ASSERT_FALSE(val);

                val = imap_->get<std::string, std::string>("key1").get();
                ASSERT_TRUE(val);
                ASSERT_EQ(prefix + "value1", *val);

                imap_->remove_interceptor(interceptorId).get();
            }

            TEST_P(ClientMapTest, testJsonPutGet) {
                std::shared_ptr<imap> map = client_.get_map(get_test_name()).get();
                hazelcast_json_value value("{ \"age\": 4 }");
                map->put("item1", value).get();
                boost::optional<hazelcast_json_value> retrieved = map->get<std::string, hazelcast_json_value>("item1").get();

                ASSERT_TRUE(retrieved.has_value());
                ASSERT_EQ(value, retrieved.value());
            }

            TEST_P(ClientMapTest, testQueryOverJsonObject) {
                std::shared_ptr<imap> map = client_.get_map(get_test_name()).get();
                hazelcast_json_value young("{ \"age\": 4 }");
                hazelcast_json_value old("{ \"age\": 20 }");
                map->put("item1", young).get();
                map->put("item2", old).get();

                ASSERT_EQ(2, map->size().get());

// Get the objects whose age is less than 6
                std::vector<hazelcast_json_value> result = map->values<hazelcast_json_value>(
                        query::greater_less_predicate(client_, "age", 6, false, true)).get();
                ASSERT_EQ(1U, result.size());
                ASSERT_EQ(young, result[0]);
            }

            TEST_P(ClientMapTest, testExtendedAsciiString) {
                std::string key = "Num\xc3\xa9ro key";
                std::string value = "Num\xc3\xa9ro value";
                imap_->put<std::string, std::string>(key, value).get();

                boost::optional<std::string> actualValue = imap_->get<std::string, std::string>(key).get();
                ASSERT_TRUE(actualValue.has_value());
                ASSERT_EQ(value, actualValue.value());
            }
        }

        namespace serialization {
            template<>
            struct hz_serializer<test::ClientMapTest::EntryMultiplier> : public identified_data_serializer {
                static int get_factory_id() {
                    return 666;
                }

                static int get_class_id() {
                    return 3;
                }

                static void write_data(const test::ClientMapTest::EntryMultiplier &object, object_data_output &writer) {
                    writer.write<int32_t>(object.get_multiplier());
                }

                static test::ClientMapTest::EntryMultiplier read_data(object_data_input &reader) {
                    return test::ClientMapTest::EntryMultiplier(reader.read<int32_t>());
                }
            };

            template<>
            struct hz_serializer<test::PartitionAwareInt> : public identified_data_serializer {
            public:
                static int32_t get_factory_id() {
                    return 666;
                }

                static int32_t get_class_id() {
                    return 9;
                }

                static void write_data(const test::PartitionAwareInt &object, object_data_output &out) {
                    out.write<int32_t>(object.get_actual_key());
                }

                static test::PartitionAwareInt read_data(object_data_input &in) {
                    int value = in.read<int32_t>();
                    return test::PartitionAwareInt(value, value);
                }
            };

            template<>
            struct hz_serializer<test::ClientMapTest::MapGetInterceptor> : public identified_data_serializer {
                static int get_factory_id() {
                    return 666;
                }

                static int get_class_id() {
                    return 6;
                }

                static void write_data(const test::ClientMapTest::MapGetInterceptor &object, object_data_output &writer) {
                    writer.write(object.prefix_);
                }

                static test::ClientMapTest::MapGetInterceptor read_data(object_data_input &reader) {
                    return test::ClientMapTest::MapGetInterceptor(reader.read<std::string>());
                }
            };
        }
    }
}

namespace std {
    template<> struct hash<hazelcast::client::test::PartitionAwareInt> {
        std::size_t operator()(const hazelcast::client::test::PartitionAwareInt &object) const noexcept {
            return std::hash<int>{}(object.get_actual_key());
        }
    };
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

