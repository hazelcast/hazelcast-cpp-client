/*
 * Copyright (c) 2008-2023, Hazelcast, Inc. All Rights Reserved.
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
#include <iostream>
#include <memory>
#include <ostream>
#include <string>
#include <thread>
#include <unordered_set>
#include <utility>
#include <vector>

#include <boost/asio.hpp>
#include <boost/thread/barrier.hpp>

#include <gtest/gtest.h>

#ifdef HZ_BUILD_WITH_SSL
#include <openssl/crypto.h>
#endif

#include <hazelcast/client/client_config.h>
#include <hazelcast/client/client_properties.h>
#include <hazelcast/client/connection/ClientConnectionManagerImpl.h>
#include <hazelcast/client/connection/Connection.h>
#include <hazelcast/client/connection/AddressProvider.h>
#include <hazelcast/client/spi/impl/discovery/remote_address_provider.h>
#include <hazelcast/client/entry_event.h>
#include <hazelcast/client/exception/protocol_exceptions.h>
#include <hazelcast/client/hazelcast_client.h>
#include <hazelcast/client/imap.h>
#include <hazelcast/client/impl/Partition.h>
#include <hazelcast/client/initial_membership_event.h>
#include <hazelcast/client/internal/nearcache/impl/store/NearCacheObjectRecordStore.h>
#include <hazelcast/client/iset.h>
#include <hazelcast/client/item_listener.h>
#include <hazelcast/client/itopic.h>
#include <hazelcast/client/multi_map.h>
#include <hazelcast/client/pipelining.h>
#include <hazelcast/client/protocol/ClientMessage.h>
#include <hazelcast/client/reliable_topic.h>
#include <hazelcast/client/serialization_config.h>
#include <hazelcast/client/serialization/serialization.h>
#include <hazelcast/client/spi/ClientContext.h>
#include <hazelcast/client/topic/reliable_listener.h>
#include <hazelcast/logger.h>
#include <hazelcast/util/AddressHelper.h>
#include <hazelcast/util/MurmurHash3.h>
#include <hazelcast/util/Util.h>

#include "ClientTest.h"
#include "HazelcastServer.h"
#include "HazelcastServerFactory.h"
#include "TestHelperFunctions.h"
#include "serialization/Serializables.h"
#include "CountDownLatchWaiter.h"
#include "remote_controller_client.h"
#include "hazelcast/client/protocol/codec/builtin/sql_page_codec.h"

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4996) // for unsafe getenv
#endif

namespace hazelcast {
namespace client {
namespace test {
class BasicClientNearCacheTest
  : public ClientTest
  , public ::testing::WithParamInterface<config::in_memory_format>
{
public:
    static void SetUpTestSuite()
    {
        instance = new HazelcastServer(default_server_factory());
        instance2 = new HazelcastServer(default_server_factory());
    }

    static void TearDownTestSuite()
    {
        delete instance2;
        delete instance;
        instance2 = nullptr;
        instance = nullptr;
    }

    void SetUp() override
    {
        near_cache_config_ = NearCacheTestUtils::create_near_cache_config(
          GetParam(), get_test_name());
    }

    void TearDown() override
    {
        if (near_cached_map_) {
            near_cached_map_->destroy().get();
        }
        if (no_near_cache_map_) {
            no_near_cache_map_->destroy().get();
        }
        if (client_) {
            client_->shutdown().get();
        }
        if (near_cached_client_) {
            near_cached_client_->shutdown().get();
        }
    }

protected:
    /**
     * Provides utility methods for unified Near Cache tests.
     */
    class NearCacheTestUtils : public ClientTest
    {
    public:
        /**
         * Creates a {@link NearCacheConfig} with a given {@link
         * InMemoryFormat}.
         *
         * @param inMemoryFormat the {@link InMemoryFormat} to set
         * @return the {@link NearCacheConfig}
         */
        static config::near_cache_config create_near_cache_config(
          config::in_memory_format in_memory_format,
          const std::string& map_name)
        {
            config::near_cache_config nearCacheConfig;
            nearCacheConfig.set_name(map_name)
              .set_in_memory_format(in_memory_format)
              .set_invalidate_on_change(true);

            return nearCacheConfig;
        }

        /**
         * Configures the {@link EvictionConfig} of the given {@link
         * NearCacheConfig}.
         *
         * @param nearCacheConfig the {@link NearCacheConfig} to configure
         * @param evictionPolicy  the {@link EvictionPolicy} to set
         * @param maxSizePolicy   the {@link MaxSizePolicy} to set
         * @param maxSize         the max size to set
         */
        static void set_eviction_config(
          config::near_cache_config& near_cache_config,
          config::eviction_policy eviction_policy,
          typename config::eviction_config::max_size_policy max_size_policy,
          int max_size)
        {
            near_cache_config.get_eviction_config()
              .set_eviction_policy(eviction_policy)
              .set_maximum_size_policy(max_size_policy)
              .set_size(max_size);
        }

        /**
         * Asserts the {@link NearCacheStats} for expected values.
         *
         * @param stats                   stats of the near cache
         * @param expectedOwnedEntryCount the expected owned entry count
         * @param expectedHits            the expected Near Cache hits
         * @param expectedMisses          the expected Near Cache misses
         */
        static void assert_near_cache_stats(monitor::near_cache_stats& stats,
                                            int64_t expected_owned_entry_count,
                                            int64_t expected_hits,
                                            int64_t expected_misses)
        {
            assert_near_cache_stats(stats,
                                    expected_owned_entry_count,
                                    expected_hits,
                                    expected_misses,
                                    0,
                                    0);
        }

        /**
         * Asserts the {@link NearCacheStats} for expected values.
         *
         * @param stats                   stats of the near cache
         * @param expectedOwnedEntryCount the expected owned entry count
         * @param expectedHits            the expected Near Cache hits
         * @param expectedMisses          the expected Near Cache misses
         * @param expectedEvictions       the expected Near Cache evictions
         * @param expectedExpirations     the expected Near Cache expirations
         */
        static void assert_near_cache_stats(monitor::near_cache_stats& stats,
                                            int64_t expected_owned_entry_count,
                                            int64_t expected_hits,
                                            int64_t expected_misses,
                                            int64_t expected_evictions,
                                            int64_t expected_expirations)
        {
            assert_equals_format(
              "Near Cache entry count should be %ld, but was %ld ",
              expected_owned_entry_count,
              stats.get_owned_entry_count(),
              stats);
            assert_equals_format("Near Cache hits should be %ld, but were %ld ",
                                 expected_hits,
                                 stats.get_hits(),
                                 stats);
            assert_equals_format(
              "Near Cache misses should be %ld, but were %ld ",
              expected_misses,
              stats.get_misses(),
              stats);
            assert_equals_format(
              "Near Cache evictions should be %ld, but were %ld ",
              expected_evictions,
              stats.get_evictions(),
              stats);
            assert_equals_format(
              "Near Cache expirations should be %ld, but were %ld ",
              expected_expirations,
              stats.get_expirations(),
              stats);
        }

        static void assert_equals_format(const char* message_format,
                                         int64_t expected,
                                         int64_t actual,
                                         monitor::near_cache_stats& stats)
        {
            char buf[300];
            hazelcast::util::hz_snprintf(
              buf, 300, message_format, expected, actual);
            ASSERT_EQ(expected, actual)
              << buf << "(" << stats.to_string() << ")";
        }

        NearCacheTestUtils() = delete;

        NearCacheTestUtils(const NearCacheTestUtils&) = delete;
    };

    /**
     * The default count to be inserted into the Near Caches.
     */
    static const int DEFAULT_RECORD_COUNT;

    void create_context()
    {
        create_no_near_cache_context();
        create_near_cache_context();
    }

    void create_no_near_cache_context()
    {
        client_ = std::unique_ptr<hazelcast_client>(
          new hazelcast_client{ new_client(get_config()).get() });
        no_near_cache_map_ = client_->get_map(get_test_name()).get();
    }

    void create_near_cache_context()
    {
        near_cached_client_config_ = get_config();
        near_cached_client_config_.add_near_cache_config(near_cache_config_);
        near_cached_client_ =
          std::unique_ptr<hazelcast_client>(new hazelcast_client{
            new_client(std::move(near_cached_client_config_)).get() });
        near_cached_map_ = near_cached_client_->get_map(get_test_name()).get();
        spi::ClientContext clientContext(*near_cached_client_);
        near_cache_manager_ = &clientContext.get_near_cache_manager();
        near_cache_ =
          near_cache_manager_->get_near_cache<serialization::pimpl::data,
                                              serialization::pimpl::data,
                                              serialization::pimpl::data>(
            get_test_name());
        this->stats_ =
          near_cache_ ? near_cache_->get_near_cache_stats() : nullptr;
    }

    void test_contains_key(bool use_near_cached_map_for_removal)
    {
        create_no_near_cache_context();

        // populate map
        no_near_cache_map_->put<int, std::string>(1, "value1").get();
        no_near_cache_map_->put<int, std::string>(2, "value2").get();
        no_near_cache_map_->put<int, std::string>(3, "value3").get();

        create_near_cache_context();

        // populate Near Cache
        near_cached_map_->get<int, std::string>(1).get();
        near_cached_map_->get<int, std::string>(2).get();
        near_cached_map_->get<int, std::string>(3).get();

        ASSERT_TRUE(near_cached_map_->contains_key(1).get());
        ASSERT_TRUE(near_cached_map_->contains_key(2).get());
        ASSERT_TRUE(near_cached_map_->contains_key(3).get());
        ASSERT_FALSE(near_cached_map_->contains_key(5).get());

        // remove a key which is in the Near Cache
        auto adapter = use_near_cached_map_for_removal ? near_cached_map_
                                                       : no_near_cache_map_;
        adapter->remove<int, std::string>(1).get();

        WAIT_TRUE_EVENTUALLY(check_contain_keys());
        ASSERT_FALSE(near_cached_map_->contains_key(1).get());
        ASSERT_TRUE(near_cached_map_->contains_key(2).get());
        ASSERT_TRUE(near_cached_map_->contains_key(3).get());
        ASSERT_FALSE(near_cached_map_->contains_key(5).get());
    }

    bool check_contain_keys()
    {
        return !near_cached_map_->contains_key(1).get() &&
               near_cached_map_->contains_key(2).get() &&
               near_cached_map_->contains_key(3).get() &&
               !near_cached_map_->contains_key(5).get();
    }

    void assert_near_cache_invalidation_requests(
      monitor::near_cache_stats& stat,
      int64_t invalidation_requests)
    {
        if (near_cache_config_.is_invalidate_on_change() &&
            invalidation_requests > 0) {
            monitor::impl::NearCacheStatsImpl& nearCacheStatsImpl =
              (monitor::impl::NearCacheStatsImpl&)stat;
            ASSERT_EQ_EVENTUALLY(
              invalidation_requests,
              nearCacheStatsImpl.get_invalidation_requests());
            nearCacheStatsImpl.reset_invalidation_events();
        }
    }

    void populate_map()
    {
        char buf[30];
        for (int i = 0; i < DEFAULT_RECORD_COUNT; i++) {
            hazelcast::util::hz_snprintf(buf, 30, "value-%d", i);
            no_near_cache_map_->put<int, std::string>(i, buf).get();
        }

        assert_near_cache_invalidation_requests(*stats_, DEFAULT_RECORD_COUNT);
    }

    void populate_near_cache()
    {
        char buf[30];
        for (int i = 0; i < DEFAULT_RECORD_COUNT; i++) {
            auto value = near_cached_map_->get<int, std::string>(i).get();
            ASSERT_TRUE(value.has_value());
            hazelcast::util::hz_snprintf(buf, 30, "value-%d", i);
            ASSERT_EQ(buf, value.value());
        }
    }

    std::shared_ptr<serialization::pimpl::data> get_near_cache_key(int key)
    {
        spi::ClientContext clientContext(*client_);
        return clientContext.get_serialization_service().to_shared_data<int>(
          &key);
    }

    int64_t get_expected_misses_with_local_update_policy()
    {
        if (near_cache_config_.get_local_update_policy() ==
            config::near_cache_config::CACHE) {
            // we expect the first and second get() to be hits, since the value
            // should be already be cached
            return stats_->get_misses();
        }
        // we expect the first get() to be a miss, due to the replaced /
        // invalidated value
        return stats_->get_misses() + 1;
    }

    int64_t get_expected_hits_with_local_update_policy()
    {
        if (near_cache_config_.get_local_update_policy() ==
            config::near_cache_config::CACHE) {
            // we expect the first and second get() to be hits, since the value
            // should be already be cached
            return stats_->get_hits() + 2;
        }
        // we expect the second get() to be a hit, since it should be served
        // from the Near Cache
        return stats_->get_hits() + 1;
    }

    bool check_misses_and_hits(int64_t& expected_misses,
                               int64_t& expected_hits,
                               boost::optional<std::string>& value)
    {
        expected_misses = get_expected_misses_with_local_update_policy();
        expected_hits = get_expected_hits_with_local_update_policy();

        value = near_cached_map_->get<int, std::string>(1).get();
        if (!value.has_value() || value.value() != "newValue") {
            return false;
        }
        value = near_cached_map_->get<int, std::string>(1).get();
        if (!value.has_value() || value.value() != "newValue") {
            return false;
        }

        return expected_hits == stats_->get_hits() &&
               expected_misses == stats_->get_misses();
    }

    void when_put_all_is_used_then_near_cache_should_be_invalidated(
      bool use_near_cache_adapter)
    {
        create_no_near_cache_context();

        create_near_cache_context();

        populate_map();

        populate_near_cache();

        std::unordered_map<int, std::string> invalidationMap;
        char buf[30];
        for (int i = 0; i < DEFAULT_RECORD_COUNT; i++) {
            hazelcast::util::hz_snprintf(buf, 30, "value-%d", i);
            invalidationMap[i] = buf;
        }

        // this should invalidate the Near Cache
        auto adapter =
          use_near_cache_adapter ? near_cached_map_ : no_near_cache_map_;
        adapter->put_all<int, std::string>(invalidationMap).get();

        WAIT_EQ_EVENTUALLY(0, near_cache_->size());
        ASSERT_EQ(0, near_cache_->size())
          << "Invalidation is not working on putAll()";
    }

    client_config client_config_;
    client_config near_cached_client_config_;
    config::near_cache_config near_cache_config_;
    std::unique_ptr<hazelcast_client> client_;
    std::unique_ptr<hazelcast_client> near_cached_client_;
    std::shared_ptr<imap> no_near_cache_map_;
    std::shared_ptr<imap> near_cached_map_;
    hazelcast::client::internal::nearcache::NearCacheManager*
      near_cache_manager_;
    std::shared_ptr<hazelcast::client::internal::nearcache::NearCache<
      serialization::pimpl::data,
      serialization::pimpl::data>>
      near_cache_;
    std::shared_ptr<monitor::near_cache_stats> stats_;
    static HazelcastServer* instance;
    static HazelcastServer* instance2;
};

const int BasicClientNearCacheTest::DEFAULT_RECORD_COUNT = 1000;
HazelcastServer* BasicClientNearCacheTest::instance = nullptr;
HazelcastServer* BasicClientNearCacheTest::instance2 = nullptr;

/**
 * Checks that the Near Cache keys are correctly checked when {@link
 * DataStructureAdapter#contains_key(Object)} is used.
 *
 * This variant uses the {@link NearCacheTestContext#nearCacheAdapter}, so there
 * is no Near Cache invalidation necessary.
 */
TEST_P(BasicClientNearCacheTest, testContainsKey_withUpdateOnNearCacheAdapter)
{
    test_contains_key(true);
}

/**
 * Checks that the memory costs are calculated correctly.
 *
 * This variant uses the noNearCacheMap(client with no near cache), so we need
 * to configure Near Cache invalidation.
 */
TEST_P(BasicClientNearCacheTest, testContainsKey_withUpdateOnDataAdapter)
{
    near_cache_config_.set_invalidate_on_change(true);
    test_contains_key(false);
}

/**
 * Checks that the Near Cache never returns its internal {@link
 * NearCache#NULL_OBJECT} to the public API.
 */
TEST_P(BasicClientNearCacheTest,
       whenEmptyMap_thenPopulatedNearCacheShouldReturnNull_neverNULLOBJECT)
{
    create_context();

    for (int i = 0; i < DEFAULT_RECORD_COUNT; i++) {
        // populate Near Cache
        ASSERT_FALSE(
          (near_cached_map_->get<int, std::string>(i).get().has_value()))
          << "Expected null from original data structure for key " << i;
        // fetch value from Near Cache
        ASSERT_FALSE(
          (near_cached_map_->get<int, std::string>(i).get().has_value()))
          << "Expected null from Near cached data structure for key " << i;

        // fetch internal value directly from Near Cache
        std::shared_ptr<serialization::pimpl::data> key = get_near_cache_key(i);
        auto value = near_cache_->get(key);
        if (value) {
            // the internal value should either be `null` or `NULL_OBJECT`
            ASSERT_EQ(near_cache_->NULL_OBJECT, near_cache_->get(key))
              << "Expected NULL_OBJECT in Near Cache for key " << i;
        }
    }
}

/**
 * Checks that the Near Cache updates value for keys which are already in the
 * Near Cache, even if the Near Cache is full an the eviction is disabled (via
 * {@link com.hazelcast.config.EvictionPolicy#NONE}.
 *
 * This variant uses the {@link NearCacheTestContext#nearCacheAdapter}, so there
 * is no Near Cache invalidation necessary.
 */
TEST_P(
  BasicClientNearCacheTest,
  whenCacheIsFull_thenPutOnSameKeyShouldUpdateValue_withUpdateOnNearCacheAdapter)
{
    int size = DEFAULT_RECORD_COUNT / 2;
    NearCacheTestUtils::set_eviction_config(
      near_cache_config_,
      config::NONE,
      config::eviction_config::ENTRY_COUNT,
      size);
    create_no_near_cache_context();

    create_near_cache_context();

    populate_map();

    populate_near_cache();

    ASSERT_EQ(size, near_cache_->size());
    auto value = near_cached_map_->get<int, std::string>(1).get();
    ASSERT_TRUE(value.has_value());
    ASSERT_EQ("value-1", value.value());

    near_cached_map_->put<int, std::string>(1, "newValue").get();

    // wait for the invalidation to be processed
    ASSERT_EQ(size - 1, near_cache_->size());
    ASSERT_EQ(1, stats_->get_invalidations());
    auto stats_impl =
      std::static_pointer_cast<monitor::impl::NearCacheStatsImpl>(stats_);
    // one from local and one from remote
    ASSERT_EQ_EVENTUALLY(2, stats_impl->get_invalidation_requests());

    int64_t expectedMisses = get_expected_misses_with_local_update_policy();
    int64_t expectedHits = get_expected_hits_with_local_update_policy();

    value = near_cached_map_->get<int, std::string>(1).get();
    ASSERT_TRUE(value.has_value());
    ASSERT_EQ("newValue", value.value());
    value = near_cached_map_->get<int, std::string>(1).get();
    ASSERT_TRUE(value.has_value());
    ASSERT_EQ("newValue", value.value());

    NearCacheTestUtils::assert_near_cache_stats(
      *stats_, size, expectedHits, expectedMisses);
}

/**
 * Checks that the Near Cache updates value for keys which are already in the
 * Near Cache, even if the Near Cache is full an the eviction is disabled (via
 * {@link com.hazelcast.config.EvictionPolicy#NONE}.
 *
 * This variant uses the {@link NearCacheTestContext#dataAdapter}, so we need to
 * configure Near Cache invalidation.
 */
TEST_P(
  BasicClientNearCacheTest,
  whenCacheIsFull_thenPutOnSameKeyShouldUpdateValue_withUpdateOnDataAdapter)
{
    int size = DEFAULT_RECORD_COUNT / 2;
    NearCacheTestUtils::set_eviction_config(
      near_cache_config_,
      config::NONE,
      config::eviction_config::ENTRY_COUNT,
      size);
    near_cache_config_.set_invalidate_on_change(true);

    create_no_near_cache_context();

    create_near_cache_context();

    populate_map();

    populate_near_cache();

    ASSERT_EQ(size, near_cache_->size());
    auto value = near_cached_map_->get<int, std::string>(1).get();
    ASSERT_TRUE(value.has_value());
    ASSERT_EQ("value-1", value.value());

    no_near_cache_map_->put<int, std::string>(1, "newValue").get();

    // we have to use assertTrueEventually since the invalidation is done
    // asynchronously
    int64_t expectedMisses = 0;
    int64_t expectedHits = 0;
    WAIT_TRUE_EVENTUALLY(
      check_misses_and_hits(expectedMisses, expectedHits, value));
    SCOPED_TRACE("whenCacheIsFull_thenPutOnSameKeyShouldUpdateValue_"
                 "withUpdateOnDataAdapter");
    NearCacheTestUtils::assert_near_cache_stats(
      *stats_, size, expectedHits, expectedMisses);
}

/**
 * Checks that the Near Cache values are eventually invalidated when {@link
 * DataStructureAdapter#putAll(Map)} is used.
 *
 * This variant uses the nearCacheMap, so there is no Near Cache invalidation
 * necessary.
 */
TEST_P(
  BasicClientNearCacheTest,
  whenPutAllIsUsed_thenNearCacheShouldBeInvalidated_withUpdateOnNearCacheAdapter)
{
    when_put_all_is_used_then_near_cache_should_be_invalidated(true);
}

/**
 * Checks that the Near Cache values are eventually invalidated when {@link
 * DataStructureAdapter#putAll(Map)} is used.
 *
 * This variant uses the noNearCacheMap, so we need to configure Near Cache
 * invalidation.
 */
TEST_P(
  BasicClientNearCacheTest,
  whenPutAllIsUsed_thenNearCacheShouldBeInvalidated_withUpdateOnDataAdapter)
{
    near_cache_config_.set_invalidate_on_change(true);
    when_put_all_is_used_then_near_cache_should_be_invalidated(false);
}

/**
 * Checks that the {@link com.hazelcast.monitor.NearCacheStats} are calculated
 * correctly.
 */
TEST_P(BasicClientNearCacheTest, testNearCacheStats)
{
    create_no_near_cache_context();

    create_near_cache_context();

    // populate map
    populate_map();

    {
        SCOPED_TRACE("testNearCacheStats when near cache is empty");
        NearCacheTestUtils::assert_near_cache_stats(*stats_, 0, 0, 0);
    }

    // populate Near Cache. Will cause misses and will increment the owned entry
    // count
    populate_near_cache();
    {
        SCOPED_TRACE(
          "testNearCacheStats when near cache is initially populated");
        NearCacheTestUtils::assert_near_cache_stats(
          *stats_, DEFAULT_RECORD_COUNT, 0, DEFAULT_RECORD_COUNT);
    }

    // make some hits
    populate_near_cache();
    {
        SCOPED_TRACE(
          "testNearCacheStats when near cache is hit after being populated.");
        NearCacheTestUtils::assert_near_cache_stats(*stats_,
                                                    DEFAULT_RECORD_COUNT,
                                                    DEFAULT_RECORD_COUNT,
                                                    DEFAULT_RECORD_COUNT);
    }
}

TEST_P(BasicClientNearCacheTest, testNearCacheEviction)
{
    NearCacheTestUtils::set_eviction_config(
      near_cache_config_,
      config::LRU,
      config::eviction_config::ENTRY_COUNT,
      DEFAULT_RECORD_COUNT);
    create_no_near_cache_context();

    create_near_cache_context();

    // all Near Cache implementations use the same eviction algorithm, which
    // evicts a single entry
    int64_t expectedEvictions = 1;

    // populate map with an extra entry
    populate_map();
    char buf[20];
    hazelcast::util::hz_snprintf(buf, 20, "value-%d", DEFAULT_RECORD_COUNT);
    no_near_cache_map_->put<int, std::string>(DEFAULT_RECORD_COUNT, buf).get();

    // populate Near Caches
    populate_near_cache();

    // we expect (size + the extra entry - the expectedEvictions) entries in the
    // Near Cache
    int64_t expectedOwnedEntryCount =
      DEFAULT_RECORD_COUNT + 1 - expectedEvictions;
    int64_t expectedHits = stats_->get_hits();
    int64_t expectedMisses = stats_->get_misses() + 1;

    // trigger eviction via fetching the extra entry
    near_cached_map_->get<int, std::string>(DEFAULT_RECORD_COUNT).get();

    int64_t evictions = stats_->get_evictions();
    ASSERT_GE(evictions, expectedEvictions)
      << "Near Cache eviction count didn't reach the desired value ("
      << expectedEvictions << " vs. " << evictions << ") ("
      << stats_->to_string();

    SCOPED_TRACE("testNearCacheEviction");

    NearCacheTestUtils::assert_near_cache_stats(*stats_,
                                                expectedOwnedEntryCount,
                                                expectedHits,
                                                expectedMisses,
                                                expectedEvictions,
                                                0);
}

TEST_P(BasicClientNearCacheTest, testSetCacheLocalEntries)
{
    /*set_cache_local_entries and is_cache_local_entries methods are deprecated,
    for codecoverage, these dummy tests are added*/
    EXPECT_FALSE(near_cache_config_.is_cache_local_entries());

    near_cache_config_.set_cache_local_entries(true);

    EXPECT_TRUE(near_cache_config_.is_cache_local_entries());

    near_cache_config_.set_cache_local_entries(false);

    EXPECT_FALSE(near_cache_config_.is_cache_local_entries());
}

INSTANTIATE_TEST_SUITE_P(ClientNearCacheTest,
                         BasicClientNearCacheTest,
                         ::testing::Values(config::BINARY, config::OBJECT));
} // namespace test
} // namespace client
} // namespace hazelcast

namespace hazelcast {
namespace client {
namespace test {
class ClientMapNearCacheTest : public ClientTest
{
protected:
    /**
     * The default name used for the data structures which have a Near Cache.
     */
    static constexpr const char* DEFAULT_NEAR_CACHE_NAME = "defaultNearCache";

    static void SetUpTestCase()
    {
        instance = new HazelcastServer(default_server_factory());
        instance2 = new HazelcastServer(default_server_factory());
    }

    static void TearDownTestCase()
    {
        delete instance2;
        delete instance;
        instance2 = nullptr;
        instance = nullptr;
    }

    void TearDown() override
    {
        if (map_) {
            map_->destroy().get();
        }
    }

    static config::near_cache_config new_no_invalidation_near_cache_config()
    {
        config::near_cache_config config(new_near_cache_config());
        config.set_in_memory_format(config::OBJECT);
        config.set_invalidate_on_change(false);
        return config;
    }

    static config::near_cache_config new_near_cache_config()
    {
        return config::near_cache_config();
    }

    static std::unique_ptr<client_config> new_client_config()
    {
        return std::unique_ptr<client_config>(new client_config(get_config()));
    }

    std::shared_ptr<imap> get_near_cached_map_from_client(
      config::near_cache_config config)
    {
        std::string mapName = DEFAULT_NEAR_CACHE_NAME;

        config.set_name(mapName);

        client_config_ = new_client_config();
        client_config_->add_near_cache_config(config);

        if (client_ != nullptr) {
            client_->shutdown().get();
        }
        client_.reset(
          new hazelcast_client(new_client(std::move(*client_config_)).get()));
        map_ = client_->get_map(mapName).get();
        return map_;
    }

    static std::shared_ptr<monitor::near_cache_stats> get_near_cache_stats(
      imap& m)
    {
        return m.get_local_map_stats().get_near_cache_stats();
    }

    static void assert_that_owned_entry_count_equals(imap& client_map,
                                                     int64_t expected)
    {
        ASSERT_EQ(expected,
                  get_near_cache_stats(client_map)->get_owned_entry_count());
    }

    std::unique_ptr<client_config> client_config_;
    config::near_cache_config near_cache_config_;
    std::unique_ptr<hazelcast_client> client_;
    std::shared_ptr<imap> map_;
    static HazelcastServer* instance;
    static HazelcastServer* instance2;
};

HazelcastServer* ClientMapNearCacheTest::instance = nullptr;
HazelcastServer* ClientMapNearCacheTest::instance2 = nullptr;

TEST_F(ClientMapNearCacheTest, testGetAllChecksNearCacheFirst)
{
    auto map =
      get_near_cached_map_from_client(new_no_invalidation_near_cache_config());

    std::unordered_set<int> keys;
    int size = 1003;
    for (int i = 0; i < size; i++) {
        map->put(i, i).get();
        keys.insert(i);
    }
    // populate Near Cache
    for (int i = 0; i < size; i++) {
        map->get<int, int>(i).get();
    }
    // getAll() generates the Near Cache hits
    map->get_all<int, int>(keys).get();

    auto stats = get_near_cache_stats(*map);
    ASSERT_EQ(size, stats->get_owned_entry_count());
    ASSERT_EQ(size, stats->get_hits());
}

TEST_F(ClientMapNearCacheTest, testGetAllPopulatesNearCache)
{
    auto map =
      get_near_cached_map_from_client(new_no_invalidation_near_cache_config());

    std::unordered_set<int> keys;
    int size = 1214;
    for (int i = 0; i < size; i++) {
        map->put(i, i).get();
        keys.insert(i);
    }
    // populate Near Cache
    for (int i = 0; i < size; i++) {
        map->get<int, int>(i).get();
    }
    // getAll() generates the Near Cache hits
    map->get_all<int, int>(keys).get();

    assert_that_owned_entry_count_equals(*map, size);
}

TEST_F(ClientMapNearCacheTest, testRemoveAllNearCache)
{
    auto map = get_near_cached_map_from_client(new_near_cache_config());

    std::unordered_set<int> keys;
    int size = 1214;
    for (int i = 0; i < size; i++) {
        map->put(i, i).get();
        keys.insert(i);
    }
    // populate Near Cache
    for (int i = 0; i < size; i++) {
        map->get<int, int>(i).get();
    }

    map
      ->remove_all(query::equal_predicate(
        *client_, query::query_constants::KEY_ATTRIBUTE_NAME, 20))
      .get();

    assert_that_owned_entry_count_equals(*map, 0);
}
} // namespace test
} // namespace client
} // namespace hazelcast

namespace hazelcast {
namespace client {
namespace test {

class ClientSetTest : public ClientTest
{
protected:
    void add_items(int count)
    {
        for (int i = 1; i <= count; ++i) {
            ASSERT_TRUE(
              set->add(std::string("item") + std::to_string(i)).get());
        }
    }

    void remove_items(int count)
    {
        for (int i = 1; i <= count; ++i) {
            ASSERT_TRUE(
              set->remove(std::string("item") + std::to_string(i)).get());
        }
    }

    void TearDown() override { set->clear().get(); }

    static void SetUpTestCase()
    {
        instance = new HazelcastServer(default_server_factory());
        client = new hazelcast_client{ new_client(get_config()).get() };
        set = client->get_set("MySet").get();
    }

    static void TearDownTestCase()
    {
        client->shutdown().get();
        delete client;
        delete instance;

        set = nullptr;
        client = nullptr;
        instance = nullptr;
    }

    bool item_exists(const std::vector<std::string>& items,
                     const std::string& item) const
    {
        bool found = false;
        for (std::vector<std::string>::const_iterator it = items.begin();
             it != items.end();
             ++it) {
            if (item == *it) {
                found = true;
                break;
            }
        }
        return found;
    }

    static HazelcastServer* instance;
    static hazelcast_client* client;
    static std::shared_ptr<iset> set;
};

HazelcastServer* ClientSetTest::instance = nullptr;
hazelcast_client* ClientSetTest::client = nullptr;
std::shared_ptr<iset> ClientSetTest::set;

TEST_F(ClientSetTest, testAddAll)
{
    std::vector<std::string> l;
    l.emplace_back("item1");
    l.emplace_back("item2");

    ASSERT_TRUE(set->add_all(l).get());
    ASSERT_EQ(2, set->size().get());

    ASSERT_FALSE(set->add_all(l).get());
    ASSERT_EQ(2, set->size().get());
}

TEST_F(ClientSetTest, testAddRemove)
{
    add_items(3);
    ASSERT_EQ(3, set->size().get());

    ASSERT_FALSE(set->add("item3").get());
    ASSERT_EQ(3, set->size().get());

    ASSERT_FALSE(set->remove("item4").get());
    ASSERT_TRUE(set->remove("item3").get());
}

TEST_F(ClientSetTest, testContains)
{
    add_items(4);

    ASSERT_FALSE(set->contains("item5").get());
    ASSERT_TRUE(set->contains("item2").get());

    std::vector<std::string> l;
    l.emplace_back("item6");
    l.emplace_back("item3");
    ASSERT_FALSE(set->contains_all(l).get());
    ASSERT_TRUE(set->add("item6").get());
    ASSERT_TRUE(set->contains_all(l).get());
}

TEST_F(ClientSetTest, testToArray)
{
    add_items(4);
    ASSERT_FALSE(set->add("item4").get());

    std::vector<std::string> items = set->to_array<std::string>().get();

    ASSERT_EQ((size_t)4, items.size());
    ASSERT_TRUE(item_exists(items, "item1"));
    ASSERT_TRUE(item_exists(items, "item2"));
    ASSERT_TRUE(item_exists(items, "item3"));
    ASSERT_TRUE(item_exists(items, "item4"));
}

TEST_F(ClientSetTest, testRemoveRetainAll)
{
    add_items(4);
    std::vector<std::string> l;
    l.emplace_back("item4");
    l.emplace_back("item3");
    ASSERT_TRUE(set->remove_all(l).get());
    ASSERT_EQ(2, set->size().get());
    ASSERT_FALSE(set->remove_all(l).get());
    ASSERT_EQ(2, set->size().get());

    l.clear();
    l.emplace_back("item1");
    l.emplace_back("item2");
    ASSERT_FALSE(set->retain_all(l).get());
    ASSERT_EQ(2, set->size().get());

    l.clear();
    ASSERT_TRUE(set->retain_all(l).get());
    ASSERT_EQ(0, set->size().get());
}

TEST_F(ClientSetTest, testListener)
{
    boost::latch latch1(6);

    auto registrationId =
      set
        ->add_item_listener(
          item_listener().on_added(
            [&latch1](item_event&& item_event) { latch1.count_down(); }),
          true)
        .get();

    add_items(5);
    set->add("done").get();
    ASSERT_OPEN_EVENTUALLY(latch1);

    ASSERT_TRUE(set->remove_item_listener(registrationId).get());
}

TEST_F(ClientSetTest, testListenerOnRemoved)
{
    constexpr int num_of_entry = 5;
    boost::latch latch1(num_of_entry + 1);

    add_items(num_of_entry);
    set->add("done").get();

    auto registrationId =
      set
        ->add_item_listener(
          item_listener().on_removed(
            [&latch1](item_event&& item_event) { latch1.count_down(); }),
          true)
        .get();

    remove_items(num_of_entry);
    set->remove("done").get();
    ASSERT_OPEN_EVENTUALLY(latch1);

    EXPECT_TRUE(set->remove_item_listener(registrationId).get());
}

TEST_F(ClientSetTest, testIsEmpty)
{
    ASSERT_TRUE(set->is_empty().get());
    ASSERT_TRUE(set->add("item1").get());
    ASSERT_FALSE(set->is_empty().get());
}
} // namespace test
} // namespace client
} // namespace hazelcast

namespace hazelcast {
namespace client {
namespace test {
class ReliableTopicTest : public ClientTest
{
protected:
    struct ListenerState
    {
        explicit ListenerState(int latch_count, int64_t start_sequence = -1)
          : latch1(latch_count)
          , latch_for_seq_id(latch_count)
          , latch_for_termination(latch_count)
          , start_sequence(start_sequence)
        {}

        boost::latch latch1;
        boost::latch latch_for_seq_id;
        boost::latch latch_for_termination;
        int64_t start_sequence;
        std::vector<topic::message> messages;
        std::vector<int64_t> seq_ids;
    };

    topic::reliable_listener make_listener(std::shared_ptr<ListenerState> state,
                                           bool is_throw_exception = false,
                                           bool is_lvalue = false)
    {
        auto on_received = [state,
                            is_throw_exception](topic::message&& message) {
            state->messages.push_back(std::move(message));
            state->latch1.count_down();
            if (is_throw_exception) {
                BOOST_THROW_EXCEPTION(exception::cancellation(
                  "reliable_listener user_code::throw_exception", ""));
            }
        };

        auto on_store_sequence_id = [state](int64_t seq_no) {
            state->seq_ids.push_back(seq_no);
            state->latch_for_seq_id.count_down();
        };

        auto terminate_on_exception =
          [state](const exception::iexception& par_exception) -> bool {
            state->latch_for_termination.count_down();
            return true;
        };

        if (is_lvalue) {
            topic::reliable_listener tmp_listener(false, state->start_sequence);
            return tmp_listener.on_received(std::move(on_received))
              .on_store_sequence_id(std::move(on_store_sequence_id))
              .terminate_on_exception(std::move(terminate_on_exception));
        } else {
            return topic::reliable_listener(false, state->start_sequence)
              .on_received(std::move(on_received))
              .on_store_sequence_id(std::move(on_store_sequence_id))
              .terminate_on_exception(std::move(terminate_on_exception));
        }
    }

protected:
    void TearDown() override
    {
        if (topic_) {
            topic_->destroy().get();
        }
    }

    static void SetUpTestCase()
    {
        instance = new HazelcastServer(default_server_factory());
        client = new hazelcast_client{ new_client(get_config()).get() };
    }

    static void TearDownTestCase()
    {
        client->shutdown().get();
        delete client;
        delete instance;

        client = nullptr;
        instance = nullptr;
    }

    static HazelcastServer* instance;
    static hazelcast_client* client;
    std::shared_ptr<reliable_topic> topic_;
    std::string listener_id_;
};

HazelcastServer* ReliableTopicTest::instance = nullptr;
hazelcast_client* ReliableTopicTest::client = nullptr;

TEST_F(ReliableTopicTest, testBasics)
{
    ASSERT_NO_THROW(topic_ = client->get_reliable_topic("testBasics").get());
    ASSERT_EQ("testBasics", topic_->get_name());

    auto state = std::make_shared<ListenerState>(1);
    ASSERT_NO_THROW(listener_id_ =
                      topic_->add_message_listener(make_listener(state)));

    employee empl1("first", 20);
    ASSERT_NO_THROW(topic_->publish(empl1).get());

    ASSERT_OPEN_EVENTUALLY(state->latch1);
    ASSERT_EQ(1, state->messages.size());
    ASSERT_EQ("testBasics", state->messages[0].get_name());
    auto e = state->messages[0].get_message_object().get<employee>();
    ASSERT_TRUE(e.has_value());
    ASSERT_EQ(empl1, e.value());

    // remove listener
    ASSERT_TRUE(topic_->remove_message_listener(listener_id_));
    ASSERT_FALSE(topic_->remove_message_listener(listener_id_));
}

TEST_F(ReliableTopicTest, testListenerSequence)
{
    ASSERT_NO_THROW(topic_ =
                      client->get_reliable_topic("testListenerSequence").get());

    employee empl1("first", 10);
    employee empl2("second", 20);

    ASSERT_NO_THROW(topic_->publish(empl1).get());
    ASSERT_NO_THROW(topic_->publish(empl2).get());

    auto state = std::make_shared<ListenerState>(1, 1);
    ASSERT_NO_THROW(listener_id_ =
                      topic_->add_message_listener(make_listener(state)));

    ASSERT_OPEN_EVENTUALLY(state->latch1);
    ASSERT_EQ(1, state->messages.size());
    auto e = state->messages[0].get_message_object().get<employee>();
    ASSERT_TRUE(e.has_value());
    ASSERT_EQ(empl2, e.value());

    // remove listener
    ASSERT_TRUE(topic_->remove_message_listener(listener_id_));
}

TEST_F(ReliableTopicTest, removeMessageListener_whenExisting)
{
    ASSERT_NO_THROW(
      topic_ =
        client->get_reliable_topic("removeMessageListener_whenExisting").get());

    employee empl1("first", 10);

    auto state = std::make_shared<ListenerState>(1);
    ASSERT_NO_THROW(listener_id_ =
                      topic_->add_message_listener(make_listener(state)));

    // remove listener
    ASSERT_TRUE(topic_->remove_message_listener(listener_id_));

    ASSERT_NO_THROW(topic_->publish(empl1).get());

    ASSERT_EQ(boost::cv_status::timeout,
              state->latch1.wait_for(boost::chrono::seconds(2)));
    ASSERT_EQ(0, state->messages.size());
}

TEST_F(ReliableTopicTest, removeMessageListener_whenNonExisting)
{
    ASSERT_NO_THROW(
      topic_ =
        client->get_reliable_topic("removeMessageListener_whenNonExisting")
          .get());

    // remove listener
    ASSERT_FALSE(topic_->remove_message_listener("abc"));
}

TEST_F(ReliableTopicTest, publishMultiple)
{
    ASSERT_NO_THROW(topic_ =
                      client->get_reliable_topic("publishMultiple").get());

    auto state = std::make_shared<ListenerState>(5);
    ASSERT_NO_THROW(listener_id_ =
                      topic_->add_message_listener(make_listener(state)));

    std::vector<std::string> items;
    for (int k = 0; k < 5; k++) {
        std::string item = std::to_string(k);
        topic_->publish(item).get();
        items.push_back(item);
    }

    ASSERT_OPEN_EVENTUALLY(state->latch1);
    ASSERT_EQ(5, state->messages.size());
    for (int k = 0; k < 5; k++) {
        const auto& msg = state->messages[k];
        auto val = msg.get_message_object().get<std::string>();
        ASSERT_TRUE(val.has_value());
        ASSERT_EQ(items[k], val.value());
    }

    ASSERT_TRUE(topic_->remove_message_listener(listener_id_));
}

TEST_F(ReliableTopicTest, testConfig)
{
    client_config clientConfig;
    clientConfig.get_network_config().add_address(
      address(remote_controller_address(), 5701));
    std::string topic_name("testConfig");
    config::reliable_topic_config relConfig(topic_name);
    relConfig.set_read_batch_size(2);
    clientConfig.add_reliable_topic_config(relConfig);

    auto& readedClientConfig =
      clientConfig.get_reliable_topic_config(topic_name);
    ASSERT_EQ(2, readedClientConfig.get_read_batch_size());
    ASSERT_EQ(topic_name, readedClientConfig.get_name());

    auto configClient = hazelcast::new_client(std::move(clientConfig)).get();

    ASSERT_NO_THROW(topic_ = configClient.get_reliable_topic(topic_name).get());

    auto state = std::make_shared<ListenerState>(5);
    ASSERT_NO_THROW(listener_id_ =
                      topic_->add_message_listener(make_listener(state)));

    std::vector<std::string> items;
    for (int k = 0; k < 5; k++) {
        std::string item = std::to_string(k);
        topic_->publish(item).get();
        items.push_back(item);
    }

    ASSERT_OPEN_EVENTUALLY(state->latch1);
    ASSERT_EQ(5, state->messages.size());
    for (int k = 0; k < 5; k++) {
        const auto& msg = state->messages[k];
        auto val = msg.get_message_object().get<std::string>();
        EXPECT_TRUE(val.has_value());
        EXPECT_EQ(items[k], val.value());
    }
    EXPECT_TRUE(topic_->remove_message_listener(listener_id_));
    topic_.reset();

    configClient.shutdown().get();
}

TEST_F(ReliableTopicTest, testConfigWithSetNetworkMethod)
{
    client_config clientConfig;
    auto curr_network_config = clientConfig.get_network_config();
    curr_network_config.add_address(address(remote_controller_address(), 5701));

    // To increase the code coverage, set_network_config method is used instead
    // of get_network_config
    clientConfig.set_network_config(curr_network_config);

    std::string topic_name(get_test_name());
    config::reliable_topic_config relConfig(topic_name);
    relConfig.set_read_batch_size(2);
    clientConfig.add_reliable_topic_config(relConfig);

    auto& readedClientConfig =
      clientConfig.get_reliable_topic_config(topic_name);
    ASSERT_EQ(2, readedClientConfig.get_read_batch_size());
    ASSERT_EQ(topic_name, readedClientConfig.get_name());

    auto configClient = hazelcast::new_client(std::move(clientConfig)).get();

    ASSERT_NO_THROW(topic_ = configClient.get_reliable_topic(topic_name).get());

    auto state = std::make_shared<ListenerState>(5);
    ASSERT_NO_THROW(listener_id_ =
                      topic_->add_message_listener(make_listener(state)));

    std::vector<std::string> items;
    for (int k = 0; k < 5; k++) {
        std::string item = std::to_string(k);
        topic_->publish(item).get();
        items.push_back(item);
    }

    ASSERT_OPEN_EVENTUALLY(state->latch1);
    ASSERT_EQ(5, state->messages.size());
    for (int k = 0; k < 5; k++) {
        const auto& msg = state->messages[k];
        auto val = msg.get_message_object().get<std::string>();
        ASSERT_TRUE(val.has_value());
        ASSERT_EQ(items[k], val.value());
    }
    ASSERT_TRUE(topic_->remove_message_listener(listener_id_));
    topic_.reset();

    configClient.shutdown().get();
}

TEST_F(ReliableTopicTest, testMessageFieldSetCorrectly)
{
    ASSERT_NO_THROW(
      topic_ =
        client->get_reliable_topic("testMessageFieldSetCorrectly").get());

    auto state = std::make_shared<ListenerState>(1);
    ASSERT_NO_THROW(listener_id_ =
                      topic_->add_message_listener(make_listener(state)));

    auto timeBeforePublish = std::chrono::system_clock::now();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    topic_->publish<int>(3).get();
    auto timeAfterPublish = std::chrono::system_clock::now();
    ASSERT_OPEN_EVENTUALLY(state->latch1);
    ASSERT_EQ(1, state->messages.size());
    const auto& msg = state->messages[0];
    auto val = msg.get_message_object().get<int>();
    ASSERT_TRUE(val.has_value());
    ASSERT_EQ(3, val.value());

    ASSERT_LE(timeBeforePublish, msg.get_publish_time());
    ASSERT_GE(timeAfterPublish, msg.get_publish_time());
    ASSERT_EQ(topic_->get_name(), msg.get_source());
    ASSERT_EQ(nullptr, msg.get_publishing_member());

    ASSERT_TRUE(topic_->remove_message_listener(listener_id_));
}

TEST_F(ReliableTopicTest, testAlwaysStartAfterTail)
{
    ASSERT_NO_THROW(
      topic_ = client->get_reliable_topic("testAlwaysStartAfterTail").get());
    ASSERT_NO_THROW(topic_->publish(1).get());
    ASSERT_NO_THROW(topic_->publish(2).get());
    ASSERT_NO_THROW(topic_->publish(3).get());

    auto state = std::make_shared<ListenerState>(3);
    ASSERT_NO_THROW(listener_id_ =
                      topic_->add_message_listener(make_listener(state)));

    std::vector<int> expectedValues = { 4, 5, 6 };
    // spawn a thread for publishing new data
    std::thread([=]() {
        for (auto val : expectedValues) {
            topic_->publish(val).get();
        }
    }).detach();

    ASSERT_OPEN_EVENTUALLY(state->latch1);
    ASSERT_EQ(expectedValues.size(), state->messages.size());

    for (std::size_t i = 0; i < expectedValues.size(); i++) {
        auto received_val = state->messages[i].get_message_object().get<int>();
        auto expected_val = expectedValues[i];
        ASSERT_TRUE(received_val.has_value());
        ASSERT_EQ(expected_val, received_val.value());
    }

    ASSERT_TRUE(topic_->remove_message_listener(listener_id_));
}

TEST_F(ReliableTopicTest, testSequenceId)
{
    ASSERT_NO_THROW(topic_ =
                      client->get_reliable_topic("testSequenceId").get());

    int64_t start_seq = 0;
    int msg_count = 5;
    auto state = std::make_shared<ListenerState>(msg_count);
    ASSERT_NO_THROW(listener_id_ =
                      topic_->add_message_listener(make_listener(state)));

    for (int k = 0; k < 5; k++) {
        std::string item = std::to_string(k);
        topic_->publish(item).get();
    }

    ASSERT_OPEN_EVENTUALLY(state->latch_for_seq_id);
    ASSERT_EQ(msg_count, state->seq_ids.size());
    for (int k = 0; k < msg_count; k++) {
        auto curr_seq_id = state->seq_ids[k];
        ASSERT_EQ(curr_seq_id, start_seq + k);
    }

    ASSERT_TRUE(topic_->remove_message_listener(listener_id_));
}

TEST_F(ReliableTopicTest, testSequenceIdForLvalue)
{
    ASSERT_NO_THROW(
      topic_ = client->get_reliable_topic("testSequenceIdForLvalue").get());

    int64_t start_seq = 0;
    int msg_count = 5;
    auto state = std::make_shared<ListenerState>(msg_count);
    ASSERT_NO_THROW(listener_id_ = topic_->add_message_listener(
                      make_listener(state, false, true)));

    for (int k = 0; k < 5; k++) {
        std::string item = std::to_string(k);
        topic_->publish(item).get();
    }

    ASSERT_OPEN_EVENTUALLY(state->latch_for_seq_id);
    ASSERT_EQ(msg_count, state->seq_ids.size());
    for (int k = 0; k < msg_count; k++) {
        auto curr_seq_id = state->seq_ids[k];
        ASSERT_EQ(curr_seq_id, start_seq + k);
    }

    ASSERT_TRUE(topic_->remove_message_listener(listener_id_));
}

TEST_F(ReliableTopicTest, testTerminateCase)
{
    ASSERT_NO_THROW(topic_ =
                      client->get_reliable_topic("testTerminateCase").get());

    auto state = std::make_shared<ListenerState>(1);
    ASSERT_NO_THROW(listener_id_ =
                      topic_->add_message_listener(make_listener(state, true)));

    std::string item = std::to_string(0);
    ASSERT_NO_THROW(topic_->publish(item).get());

    ASSERT_OPEN_EVENTUALLY(state->latch_for_termination);

    // listener is removed when the exception occured
}

TEST_F(ReliableTopicTest, testTerminateCaseForLValue)
{
    ASSERT_NO_THROW(
      topic_ = client->get_reliable_topic("testTerminateCaseForLValue").get());

    auto state = std::make_shared<ListenerState>(1);
    ASSERT_NO_THROW(listener_id_ = topic_->add_message_listener(
                      make_listener(state, true, true)));

    std::string item = std::to_string(0);
    ASSERT_NO_THROW(topic_->publish(item).get());

    ASSERT_OPEN_EVENTUALLY(state->latch_for_termination);

    // listener is removed when the exception occured
}

} // namespace test
} // namespace client
} // namespace hazelcast

using namespace hazelcast::client;

namespace hazelcast {
namespace client {
namespace test {
namespace performance {
class SimpleMapTest : public ClientTest
{
protected:
    static const int THREAD_COUNT = 40;
    static const int ENTRY_COUNT = 10000;
    static const int VALUE_SIZE = 10;
    static const int STATS_SECONDS = 10;
    static const int GET_PERCENTAGE = 40;
    static const int PUT_PERCENTAGE = 40;

    class Stats
    {
    public:
        Stats()
          : get_count(0)
          , put_count(0)
          , remove_count(0)
        {}

        Stats(const Stats& rhs)
          : get_count(const_cast<Stats&>(rhs).get_count.load())
          , put_count(const_cast<Stats&>(rhs).put_count.load())
          , remove_count(const_cast<Stats&>(rhs).remove_count.load())
        {}

        Stats get_and_reset()
        {
            Stats newOne(*this);
            get_count = 0;
            put_count = 0;
            remove_count = 0;
            return newOne;
        }

        mutable std::atomic<int64_t> get_count;
        mutable std::atomic<int64_t> put_count;
        mutable std::atomic<int64_t> remove_count;

        void print() const
        {
            std::cerr << "Total = " << total() << ", puts = " << put_count
                      << " , gets = " << get_count
                      << " , removes = " << remove_count << std::endl;
        }

        int total() const
        {
            return (int)get_count + (int)put_count + (int)remove_count;
        }
    };

    class StatsPrinterTask
    {
    public:
        explicit StatsPrinterTask(Stats& stats)
          : stats_(stats)
        {}

        void run()
        {
            while (true) {
                try {
                    hazelcast::util::sleep((unsigned int)STATS_SECONDS);
                    const Stats statsNow = stats_.get_and_reset();
                    statsNow.print();
                    std::cerr << "Operations per Second : "
                              << statsNow.total() / STATS_SECONDS << std::endl;
                } catch (std::exception& e) {
                    std::cerr << e.what() << std::endl;
                }
            }
        }

        const std::string get_name() const { return "StatPrinterTask"; }

    private:
        Stats& stats_;
    };

    class Task
    {
    public:
        Task(Stats& stats,
             std::shared_ptr<imap> map,
             std::shared_ptr<logger> lg)
          : stats_(stats)
          , map_(map)
          , logger_(std::move(lg))
        {}

        void run()
        {
            std::vector<char> value(VALUE_SIZE);
            bool running = true;
            int getCount = 0;
            int putCount = 0;
            int removeCount = 0;

            int updateIntervalCount = 1000;
            while (running) {
                int key = rand() % ENTRY_COUNT;
                int operation = (rand() % 100);
                try {
                    if (operation < GET_PERCENTAGE) {
                        map_->get<int, std::vector<char>>(key).get();
                        ++getCount;
                    } else if (operation < GET_PERCENTAGE + PUT_PERCENTAGE) {
                        map_->put<int, std::vector<char>>(key, value).get();
                        ++putCount;
                    } else {
                        map_->remove<int, std::vector<char>>(key).get();
                        ++removeCount;
                    }
                    update_stats(
                      updateIntervalCount, getCount, putCount, removeCount);
                } catch (hazelcast::client::exception::io& e) {
                    HZ_LOG(*logger_,
                           warning,
                           std::string("[SimpleMapTest io_exception] ") +
                             e.what());
                } catch (
                  hazelcast::client::exception::hazelcast_client_not_active&
                    e) {
                    HZ_LOG(*logger_,
                           warning,
                           std::string("[SimpleMapTest::run] ") + e.what());
                } catch (hazelcast::client::exception::iexception& e) {
                    HZ_LOG(*logger_,
                           warning,
                           std::string("[SimpleMapTest::run] ") + e.what());
                } catch (...) {
                    HZ_LOG(*logger_,
                           warning,
                           "[SimpleMapTest:run] unknown exception!");
                    running = false;
                    throw;
                }
            }
        }

        const std::string get_name() { return "SimpleMapTest Task"; }

    private:
        void update_stats(int update_interval_count,
                          int& get_count,
                          int& put_count,
                          int& remove_count) const
        {
            if ((get_count + put_count + remove_count) %
                  update_interval_count ==
                0) {
                int64_t current = stats_.get_count;
                stats_.get_count = current + get_count;
                get_count = 0;

                current = stats_.put_count;
                stats_.put_count = current + put_count;
                put_count = 0;

                current = stats_.remove_count;
                stats_.remove_count = current + remove_count;
                remove_count = 0;
            }
        }

        Stats& stats_;
        std::shared_ptr<imap> map_;
        std::shared_ptr<logger> logger_;
    };

    static void start(const HazelcastServer& server)
    {
        std::cerr << "Starting Test with  " << std::endl;
        std::cerr << "      Thread Count: " << THREAD_COUNT << std::endl;
        std::cerr << "       Entry Count: " << ENTRY_COUNT << std::endl;
        std::cerr << "        Value Size: " << VALUE_SIZE << std::endl;
        std::cerr << "    Get Percentage: " << GET_PERCENTAGE << std::endl;
        std::cerr << "    Put Percentage: " << PUT_PERCENTAGE << std::endl;
        std::cerr << " Remove Percentage: "
                  << (100 - (PUT_PERCENTAGE + GET_PERCENTAGE)) << std::endl;
        client_config clientConfig;
        clientConfig.set_property(client_properties::PROP_HEARTBEAT_TIMEOUT,
                                  "10");
        auto member = server.get_member();
        clientConfig.get_connection_strategy_config()
          .get_retry_config()
          .set_cluster_connect_timeout(std::chrono::seconds(10));
        clientConfig.get_network_config().add_address(
          address(member.host, member.port));

        Stats stats;
        auto lg = std::make_shared<logger>("SimpleMapTest",
                                           "SimpleMapTest",
                                           logger::level::finest,
                                           logger::default_handler);

        auto monitor = std::async([&]() { StatsPrinterTask(stats).run(); });

        auto hazelcastClient = new_client(std::move(clientConfig)).get();

        auto map = hazelcastClient.get_map("cppDefault").get();

        std::vector<std::future<void>> futures;
        for (int i = 0; i < THREAD_COUNT; i++) {
            futures.push_back(
              std::async([&]() { Task(stats, map, lg).run(); }));
        }

        monitor.wait();

        hazelcastClient.shutdown().get();
    }
};

TEST_F(SimpleMapTest, DISABLED_testThroughput)
{
    HazelcastServer server(default_server_factory());

    start(server);
}

} // namespace performance
} // namespace test
} // namespace client
} // namespace hazelcast

namespace hazelcast {
namespace client {
namespace test {
class IssueTest : public ClientTest
{
public:
    IssueTest();

protected:
    boost::latch latch1_;
    boost::latch latch2_;
    entry_listener issue864_map_listener_;
};

IssueTest::IssueTest()
  : latch1_(1)
  , latch2_(1)
  , issue864_map_listener_()
{
    issue864_map_listener_
      .on_added([this](entry_event&& event) {
          auto key = event.get_key().get<int>().value();
          ASSERT_TRUE(1 == key || 2 == key);
          if (key == 1) {
              // The received event should be the addition of key value: 1, 10
              ASSERT_EQ(10, event.get_value().get<int>().value());
              this->latch1_.count_down();
          } else {
              // The received event should be the addition of key value: 2, 20
              ASSERT_EQ(20, event.get_value().get<int>().value());
              this->latch2_.count_down();
          }
      })
      .on_updated([this](entry_event&& event) {
          ASSERT_EQ(2, event.get_key().get<int>().value());
          ASSERT_EQ(20, event.get_value().get<int>().value());
          this->latch1_.count_down();
      });
}

TEST_F(IssueTest, testOperationRedo_smartRoutingDisabled)
{
    HazelcastServer hz1(default_server_factory());
    HazelcastServer hz2(default_server_factory());

    client_config clientConfig(get_config());
    clientConfig.set_redo_operation(true);
    clientConfig.get_network_config().set_smart_routing(false);

    auto client = hazelcast::new_client(std::move(clientConfig)).get();

    auto map = client.get_map("m").get();
    int expected = 1000;
    std::thread t;
    for (int i = 0; i < expected; i++) {
        if (i == 5) {
            t = std::thread([&]() { hz1.shutdown(); });
        }
        map->put(i, i).get();
    }
    t.join();
    ASSERT_EQ(expected, map->size().get());

    client.shutdown().get();
}

TEST_F(IssueTest, testListenerSubscriptionOnSingleServerRestart)
{
    HazelcastServer server(default_server_factory());

    // Start a client
    client_config clientConfig = get_config();
    clientConfig.get_connection_strategy_config()
      .get_retry_config()
      .set_cluster_connect_timeout(std::chrono::seconds(10));

    auto client = hazelcast::new_client(std::move(clientConfig)).get();

    // Get a map
    auto map = client.get_map("IssueTest_map").get();

    // Subscribe client to entry added event
    map->add_entry_listener(std::move(issue864_map_listener_), true).get();

    // Put a key, value to the map
    ASSERT_FALSE(map->put(1, 10).get().has_value());

    ASSERT_OPEN_EVENTUALLY(latch1_);

    // Restart the server
    ASSERT_TRUE(server.shutdown());
    HazelcastServer server2(default_server_factory());

    // Put a 2nd entry to the map
    (void)map->put(2, 20).get();

    // Verify that the 2nd entry is received by the listener
    ASSERT_OPEN_EVENTUALLY(latch2_);

    // Shut down the server
    ASSERT_TRUE(server2.shutdown());

    client.shutdown().get();
}

TEST_F(IssueTest, testIssue221)
{
    // start a server
    HazelcastServer server(default_server_factory());

    // start a client
    auto config = get_config();
    config.get_connection_strategy_config()
      .get_retry_config()
      .set_cluster_connect_timeout(std::chrono::seconds(3));
    hazelcast_client client{ new_client(std::move(config)).get() };

    auto map = client.get_map("Issue221_test_map").get();

    server.shutdown();

    ASSERT_THROW((map->get<int, int>(1).get()),
                 exception::hazelcast_client_not_active);

    client.shutdown().get();
}

TEST_F(IssueTest, issue_888)
{
    // start a server
    HazelcastServer server(default_server_factory());

    auto hz = hazelcast::new_client().get();
    auto map = hz.get_map("testmap").get();
    map->put<std::string, std::string>("12", "hello").get();
    map->put<std::string, std::string>("23", "world").get();
    map->put<std::string, std::string>("34", "!!!!").get();

    std::vector<std::string> myKeys{ "12", "34" };

    auto v = map
               ->entry_set<std::string, std::string>(query::in_predicate(
                 hz, query::query_constants::KEY_ATTRIBUTE_NAME, myKeys))
               .get();

    ASSERT_EQ(2, v.size());

    ASSERT_NO_THROW(map
                      ->remove_all(query::in_predicate(
                        hz, query::query_constants::KEY_ATTRIBUTE_NAME, myKeys))
                      .get());
    hz.shutdown().get();
}

TEST_F(IssueTest,
       invocation_should_not_block_indefinitely_during_client_shutdown)
{
    HazelcastServer server(default_server_factory());
    auto hz = new_client().get();
    auto map = hz.get_map("my_map").get();

    ASSERT_NO_THROW(map->put(1, 5).get());

    std::thread t([&]() { hz.shutdown(); });

    try {
        int i = 0;
        while (true) {
            map->put(++i, 5).get();
        }
    } catch (std::exception&) {
        // ignore
    }

    try {
        map->put(10, 5).get();
    } catch (std::exception&) {
        // ignore
    }

    t.join();
    hz.shutdown().get();
}

TEST_F(IssueTest, testIssue753)
{
    HazelcastServer server(default_server_factory());

    auto hz = new_client().get();

    auto map = hz.get_map("my_map").get();

    ASSERT_NO_THROW(map->put(1, 5).get());
    auto result = map->get<int, int>(1)
                    .then(boost::launch::async,
                          [](boost::future<boost::optional<int>> f) {
                              return 3 * f.get().value();
                          })
                    .get();

    ASSERT_EQ(3 * 5, result);

    boost::latch success_deferred(1);
    map->lock<int>(1)
      .then(boost::launch::deferred, [](boost::future<void> f) { f.get(); })
      .then(boost::launch::deferred,
            [&](boost::future<void> f) {
                map->unlock<int>(1).get();
                success_deferred.count_down();
            })
      .get();

    ASSERT_OPEN_EVENTUALLY(success_deferred);

    boost::latch success_async_deferred(1);
    map->lock<int>(1)
      .then(boost::launch::async, [](boost::future<void> f) { f.get(); })
      .then(boost::launch::deferred,
            [&](boost::future<void> f) {
                map->unlock<int>(1).get();
                success_async_deferred.count_down();
            })
      .get();

    ASSERT_OPEN_EVENTUALLY(success_async_deferred);

    hz.shutdown().get();
}

TEST_F(
  IssueTest,
  issue_980_frequent_heartbeat_should_not_interleave_data_messages_causing_corrupt_data)
{
    HazelcastServer server(default_server_factory());

    hazelcast::client::client_config config;
    config.set_property("hazelcast_client_heartbeat_interval", "10");
    config.set_property("hazelcast_client_heartbeat_timeout", "50000");

    auto hz = hazelcast::new_client(std::move(config)).get();

    auto map = hz.get_map("large-payload-test-map").get();

    std::string str("abcdefghijklmnopqrstuvwxyz");
    std::string output_data;

    // prepare a string of at least 800K characters
    while (output_data.length() < 800000) {
        output_data += str;
    }

    // put data 100 times to keep the test short enough
    constexpr int num_iterations = 100;
    auto pipe =
      hazelcast::client::pipelining<std::string>::create(num_iterations);
    std::vector<boost::future<boost::optional<std::string>>> futures;
    for (int i = 0; i < num_iterations; ++i) {
        auto key = "testKeyForLargePayload_" + std::to_string(i);
        pipe->add(map->put(key, output_data));
    }

    ASSERT_NO_THROW(pipe->results());

    hz.shutdown().get();
}
TEST_F(IssueTest,TestIssue1005){
    HazelcastServerFactory fac("hazelcast/test/resources/lock-expiration.xml");
    HazelcastServer serv1(fac);
    HazelcastServer serv2(fac);
    HazelcastServer serv3(fac);
    client_config con;
    con.set_cluster_name("TestIssue1005");
    auto c = hazelcast::new_client(std::move(con)).get();
    auto exp_lock = c.get_cp_subsystem().get_lock("exp_lock").get();
    exp_lock->lock().get();
    exp_lock->unlock().get();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    exp_lock->lock().get();
    exp_lock->unlock().get();

    c.shutdown().get();
}
} // namespace test
} // namespace client
} // namespace hazelcast

namespace hazelcast {
namespace client {
namespace test {
HazelcastServer::HazelcastServer(HazelcastServerFactory& factory)
  : factory_(factory)
  , is_started_(false)
  , is_shutdown_(false)
  , logger_(std::make_shared<logger>("HazelcastServer",
                                     "HazelcastServer",
                                     logger::level::info,
                                     logger::default_handler))
{
    start();
}

bool
HazelcastServer::start()
{
    bool expected = false;
    if (!is_started_.compare_exchange_strong(expected, true)) {
        return true;
    }

    try {
        member_ = factory_.start_server();
        is_started_ = true;
        return true;
    } catch (exception::illegal_state& e) {
        HZ_LOG(*logger_,
               severe,
               boost::str(boost::format("Could not start new member!!! %1%") %
                          e.what()));
        is_started_ = false;
        return false;
    }
}

bool
HazelcastServer::shutdown()
{
    bool expected = false;
    if (!is_shutdown_.compare_exchange_strong(expected, true)) {
        return false;
    }

    if (!is_started_) {
        return true;
    }

    if (!factory_.shutdown_server(member_)) {
        return false;
    }

    is_started_ = false;
    return true;
}

bool
HazelcastServer::terminate()
{
    bool expected = false;
    if (!is_shutdown_.compare_exchange_strong(expected, true)) {
        return false;
    }

    if (!is_started_) {
        return true;
    }

    if (!factory_.terminate_server(member_)) {
        return false;
    }

    is_started_ = false;
    return true;
}

HazelcastServer::~HazelcastServer()
{
    shutdown();
}

const remote::Member&
HazelcastServer::get_member() const
{
    return member_;
}

const std::string&
HazelcastServer::cluster_id() const
{
    return factory_.get_cluster_id();
}
} // namespace test
} // namespace client
} // namespace hazelcast

namespace hazelcast {
namespace client {
namespace test {

CountDownLatchWaiter&
CountDownLatchWaiter::add(boost::latch& latch1)
{
    latches_.push_back(&latch1);
    return *this;
}

boost::cv_status
CountDownLatchWaiter::wait_for(boost::chrono::steady_clock::duration duration)
{
    if (latches_.empty()) {
        return boost::cv_status::no_timeout;
    }

    auto end = boost::chrono::steady_clock::now() + duration;
    for (auto& l : latches_) {
        auto waitDuration = end - boost::chrono::steady_clock::now();
        auto status = l->wait_for(waitDuration);
        if (boost::cv_status::timeout == status) {
            return boost::cv_status::timeout;
        }
    }
    return boost::cv_status::no_timeout;
}

void
CountDownLatchWaiter::reset()
{
    latches_.clear();
}

} // namespace test
} // namespace client
} // namespace hazelcast

namespace hazelcast {
namespace client {
namespace test {

TEST(VersionTest, test_client_version)
{
    ASSERT_EQ(HAZELCAST_VERSION, version());
}

namespace {
struct BufferedMessageHandler
{
    std::shared_ptr<protocol::ClientMessage> msg;

    void handle_client_message(
      const std::shared_ptr<protocol::ClientMessage>& message)
    {
        msg = message;
    }
};
} // namespace

TEST(ClientMessageTest, testOperationNameGetSet)
{
    protocol::ClientMessage message(8);
    constexpr const char* operation_name = "OPERATION_NAME";
    message.set_operation_name(operation_name);
    ASSERT_EQ(message.get_operation_name(), operation_name);
}

TEST(ClientMessageTest, testOperationNameAfterRequestEncoding)
{
    auto request = protocol::codec::map_size_encode("map_name");
    ASSERT_EQ(request.get_operation_name(), "map.size");
}

TEST(ClientMessageTest, testFragmentedMessageHandling)
{
    std::ifstream file("hazelcast/test/resources/fragments_bytes.bin",
                       std::ios::in | std::ios::binary | std::ios::ate);
    ASSERT_TRUE(file.is_open());
    auto size = file.tellg();
    auto memblock = std::vector<char>(size);
    file.seekg(0, std::ios::beg);
    file.read(&memblock[0], size);
    file.close();

    util::ByteBuffer buffer(&memblock[0], size);

    BufferedMessageHandler handler;
    protocol::ClientMessageBuilder<BufferedMessageHandler> builder(handler);
    // it is important to check the ondata return value since there may be left
    // data less than a message header size, and this may cause an infinite
    // loop.
    while (buffer.has_remaining() && builder.on_data(buffer)) {
    }

    // the client message should be ready at this point
    // the expected message is
    ASSERT_TRUE(handler.msg);

    auto datas_opt = handler.msg->get_first_var_sized_field<std::vector<
      std::pair<serialization::pimpl::data, serialization::pimpl::data>>>();
    ASSERT_TRUE(datas_opt);
    auto& datas = datas_opt.value();
    ASSERT_EQ(10, datas.size());
    HazelcastServerFactory factory(
      "hazelcast/test/resources/serialization.xml");
    HazelcastServer member(factory);
    client_config conf;
    conf.set_cluster_name("serialization-dev");
    auto client = new_client(std::move(conf)).get();
    remote_controller_client().ping();

    serialization_config serializationConfig;
    serialization::pimpl::SerializationService ss{
        serializationConfig, spi::ClientContext{ client }.get_schema_service()
    };
    for (int32_t i = 0; i < 10; ++i) {
        ASSERT_EQ(i, ss.to_object<int32_t>(&datas[i].first));
        ASSERT_EQ(i, ss.to_object<int32_t>(&datas[i].second));
    }
    client.shutdown().get();
}

TEST(ClientMessageTest, test_encode_sql_query_id)
{
    // TODO this test can be removed once the query_id encoder is generated.

    protocol::ClientMessage msg;

    boost::uuids::uuid server_uuid{ 1, 2,  3,  4,  5,  6,  7,  8,
                                    9, 10, 11, 12, 13, 14, 15, 16 };
    boost::uuids::uuid client_uuid{ 21, 22, 23, 24, 25, 26, 27, 28,
                                    29, 30, 31, 32, 33, 34, 35, 36 };
    msg.set(sql::impl::query_id{ server_uuid, client_uuid });

    const std::vector<unsigned char> expected_bytes{
        6,  0,  0,  0,  0,  16, // begin frame
        38, 0,  0,  0,  0,  0,  // query id frame header
        1,  2,  3,  4,  5,  6,  7,  8,
        9,  10, 11, 12, 13, 14, 15, 16, // server uuid
        21, 22, 23, 24, 25, 26, 27, 28,
        29, 30, 31, 32, 33, 34, 35, 36, // client uuid
        6,  0,  0,  0,  0,  8           // end frame
    };

    std::vector<unsigned char> actual_bytes;
    for (const auto& piece : msg.get_buffer()) {
        actual_bytes.insert(actual_bytes.end(), piece.begin(), piece.end());
    }

    EXPECT_EQ(expected_bytes, actual_bytes);
}

TEST(ClientMessageTest, test_decode_sql_column_metadata)
{
    const unsigned char bytes[] = {
        6, 0, 0, 0, 0, 16,  11,  0,   0,  0,  0,   0, 1, 0, 0, 0, 0, 12,
        0, 0, 0, 0, 0, 102, 111, 111, 98, 97, 114, 6, 0, 0, 0, 0, 8,
    };

    protocol::ClientMessage msg;
    std::memcpy(msg.wr_ptr(sizeof(bytes)), bytes, sizeof(bytes));
    msg.wrap_for_read();

    auto col_metadata = msg.get<sql::sql_column_metadata>();

    EXPECT_EQ("foobar", col_metadata.name);
    EXPECT_EQ(sql::sql_column_type::boolean, col_metadata.type);
    EXPECT_EQ(false, col_metadata.nullable);
}

TEST(ClientMessageTest, test_decode_sql_page)
{
    const unsigned char bytes[] = {
        6,  0,  0,   0, 0, 16,  7,   0,   0,   0,   0, 0, 1, 14, 0,  0,
        0,  0,  0,   0, 0, 0,   0,   0,   0,   0,   0, 6, 0, 0,  0,  0,
        16, 9,  0,   0, 0, 0,   0,   102, 111, 111, 9, 0, 0, 0,  0,  0,
        98, 97, 114, 6, 0, 0,   0,   0,   8,   6,   0, 0, 0, 0,  16, 10,
        0,  0,  0,   0, 0, 116, 101, 115, 116, 6,   0, 0, 0, 0,  0,  6,
        0,  0,  0,   0, 8, 6,   0,   0,   0,   0,   8,
    };

    protocol::ClientMessage msg;
    std::memcpy(msg.wr_ptr(sizeof(bytes)), bytes, sizeof(bytes));
    msg.wrap_for_read();

    std::vector<sql::sql_column_metadata> columns_metadata{
        {"foo", sql::sql_column_type::varchar, true},
        {"test", sql::sql_column_type::varchar, true},
    };
    auto row_metadata =
      std::make_shared<sql::sql_row_metadata>(std::move(columns_metadata));
    auto page = protocol::codec::builtin::sql_page_codec::decode(msg,
                                                                 row_metadata);

    EXPECT_EQ(true, page->last());
    EXPECT_EQ((std::vector<sql::sql_column_type>{
                sql::sql_column_type::varchar,
                sql::sql_column_type::varchar,
              }),
              page->column_types());
    auto& all_rows = page->rows();
    ASSERT_EQ(2, all_rows.size());
    auto& row1 = all_rows[0];
    ASSERT_EQ(boost::make_optional<std::string>("foo"),
              row1.get_object<std::string>(0));
    ASSERT_EQ(boost::make_optional<std::string>("test"),
              row1.get_object<std::string>(1));

    auto& row2 = all_rows[1];
    ASSERT_EQ(boost::make_optional<std::string>("bar"),
              row2.get_object<std::string>(0));
    ASSERT_EQ(boost::make_optional<std::string>(""),
              row2.get_object<std::string>(1));
}

TEST(ClientMessageTest, test_decode_sql_error)
{
    const unsigned char bytes[] = {
        6,  0,   0,   0,  0,   16,  27, 0,  0,   0,   0,   0,   42,  0,  0,
        0,  0,   235, 72, 204, 144, 69, 69, 157, 199, 201, 252, 246, 76, 74,
        22, 201, 185, 9,  0,   0,   0,  0,  0,   102, 111, 111, 9,   0,  0,
        0,  0,   0,   98, 97,  114, 6,  0,  0,   0,   0,   8,
    };

    protocol::ClientMessage msg;
    std::memcpy(msg.wr_ptr(sizeof(bytes)), bytes, sizeof(bytes));
    msg.wrap_for_read();

    auto err = msg.get<sql::impl::sql_error>();

    EXPECT_EQ(42, err.code);
    EXPECT_EQ(boost::optional<std::string>{ "foo" }, err.message);
    EXPECT_EQ(
      boost::uuids::string_generator{}("c79d4545-90cc-48eb-b9c9-164a4cf6fcc9"),
      err.originating_member_id);
    EXPECT_EQ(boost::optional<std::string>{ "bar" }, err.suggestion);
}

} // namespace test
} // namespace client
} // namespace hazelcast

namespace hazelcast {
namespace client {
namespace test {

TEST(hot_restart_test, test_membership_events)
{
    HazelcastServerFactory server_factory(
      "hazelcast/test/resources/hot-restart.xml");
    HazelcastServer server(server_factory);

    client_config conf;
    conf.set_cluster_name("hot-restart-test");

    auto client = new_client(std::move(conf)).get();

    boost::latch join{ 1 }, leave{ 1 };

    client.get_cluster().add_membership_listener(
      membership_listener()
        .on_joined([&join](const membership_event& ev) { join.count_down(); })
        .on_left([&leave](const membership_event& ev) { leave.count_down(); }));

    auto old_uuid = server.get_member().uuid;

    ASSERT_TRUE(server.shutdown());
    ASSERT_TRUE(server.start());

    ASSERT_EQ(old_uuid, server.get_member().uuid);

    ASSERT_TRUE_EVENTUALLY(client.get_lifecycle_service().is_running());

    ASSERT_OPEN_EVENTUALLY(leave);
    ASSERT_OPEN_EVENTUALLY(join);

    client.shutdown().get();
}

class connection_manager_translate : public ClientTest
{
public:
    static const address private_address;
    static const address public_address;

    class test_address_provider : public connection::AddressProvider
    {
    public:
        boost::optional<address> translate(const address& addr) override
        {
            if (addr == private_address) {
                return public_address;
            }

            return boost::none;
        }

        std::vector<address> load_addresses() override
        {
            return std::vector<address>{ { "localhost", 5701 } };
        }
    };

protected:
    static HazelcastServer* instance_;

    static void SetUpTestCase()
    {
        instance_ = new HazelcastServer(default_server_factory());
    }

    static void TearDownTestCase() { delete instance_; }
};

HazelcastServer* connection_manager_translate::instance_ = nullptr;
const address connection_manager_translate::private_address{ "localhost",
                                                             5701 };
const address connection_manager_translate::public_address{ "245.245.245.245",
                                                            5701 };

TEST_F(connection_manager_translate, test_translate_is_used)
{
    // given
    client_config config;
    config.get_connection_strategy_config()
      .get_retry_config()
      .set_cluster_connect_timeout(std::chrono::seconds(1));
    auto client = new_client(std::move(config)).get();
    spi::ClientContext ctx(client);
    connection::ClientConnectionManagerImpl connection_manager(
      ctx,
      std::unique_ptr<connection::AddressProvider>(
        new test_address_provider{}));

    // throws exception because it can't connect to the cluster using translated
    // public unreachable address
    ASSERT_THROW(connection_manager.start(), exception::illegal_state);

    client.shutdown().get();
}

TEST_F(connection_manager_translate,
       test_translate_is_used_when_member_has_public_client_address)
{
    // given
    client_config config;
    config.get_network_config().use_public_address(true);
    config.get_connection_strategy_config()
      .get_retry_config()
      .set_cluster_connect_timeout(std::chrono::seconds(1));
    auto client = new_client(std::move(config)).get();
    spi::ClientContext ctx(client);

    member m(
      public_address,
      ctx.get_hazelcast_client_implementation()->random_uuid(),
      false,
      {},
      { { endpoint_qualifier{ 1, "public" }, address{ "127.0.0.1", 5701 } } });

    auto conn = ctx.get_connection_manager().get_or_connect(m);
    ASSERT_TRUE(conn);

    client.shutdown().get();
}

TEST_F(connection_manager_translate,
       test_translate_is_not_used_when_public_ip_is_disabled)
{
    // given
    client_config config;
    config.get_network_config().use_public_address(false);
    config.get_connection_strategy_config()
      .get_retry_config()
      .set_cluster_connect_timeout(std::chrono::seconds(1));
    auto client = new_client(std::move(config)).get();
    spi::ClientContext ctx(client);

    member m(
      public_address,
      ctx.get_hazelcast_client_implementation()->random_uuid(),
      false,
      {},
      { { endpoint_qualifier{ 1, "public" }, address{ "127.0.0.1", 5701 } } });

    ASSERT_THROW(ctx.get_connection_manager().get_or_connect(m),
                 boost::system::system_error);

    client.shutdown().get();
}

TEST_F(connection_manager_translate, default_config_uses_private_addresses)
{
    ASSERT_FALSE(client_config().get_network_config().use_public_address());
}

TEST_F(
  connection_manager_translate,
  if_remote_adress_provider_cannot_translate_adress_translate_should_throw_an_exception)
{
    auto client = new_client().get();
    spi::ClientContext ctx(client);
    connection::ClientConnectionManagerImpl connection_manager(
      ctx,
      std::unique_ptr<connection::AddressProvider>(
        new spi::impl::discovery::remote_address_provider{
          []() { return std::unordered_map<address, address>{}; }, true }));

    std::random_device rand{};
    member dummy_member(
      address{ "255.255.255.255", 40000 },
      boost::uuids::basic_random_generator<std::random_device>{ rand }(),
      false,
      std::unordered_map<std::string, std::string>{},
      std::unordered_map<endpoint_qualifier, address>{});
    EXPECT_THROW(connection_manager.get_or_connect(dummy_member),
                 exception::hazelcast_);

    client.shutdown().get();
}

struct ClientStateOutput : ::testing::Test
{};

TEST_F(ClientStateOutput, test_output)
{
    using client_state = connection::ClientConnectionManagerImpl::client_state;

    auto to_string = [](client_state st){
        std::stringstream ss;

        ss << st;

        return ss.str();
    };

    EXPECT_EQ(to_string(client_state::CONNECTED_TO_CLUSTER), "CONNECTED_TO_CLUSTER");
    EXPECT_EQ(to_string(client_state::DISCONNECTED_FROM_CLUSTER), "DISCONNECTED_FROM_CLUSTER");
    EXPECT_EQ(to_string(client_state::INITIAL), "INITIAL");
    EXPECT_EQ(to_string(client_state::INITIALIZED_ON_CLUSTER), "INITIALIZED_ON_CLUSTER");
}
} // namespace test
} // namespace client
} // namespace hazelcast

namespace hazelcast {
namespace client {
namespace test {
namespace thread_pool {

class ThreadPoolTest
  : public ClientTest
  , public testing::WithParamInterface<int32_t>
{
protected:
    struct ThreadState
    {
        explicit ThreadState(int latch_count)
          : latch1(latch_count)
        {
        }

        boost::latch latch1;
        std::set<boost::thread::id> thread_ids;
    };

protected:
    static void SetUpTestCase()
    {
        instance = new HazelcastServer(default_server_factory());
    }

    static void TearDownTestCase()
    {
        delete client;
        delete instance;

        client = nullptr;
        instance = nullptr;
    }

    static HazelcastServer* instance;
    static hazelcast_client* client;
};

HazelcastServer* ThreadPoolTest::instance = nullptr;
hazelcast_client* ThreadPoolTest::client = nullptr;

TEST_P(ThreadPoolTest, testEqualThreadAndJobs)
{
    int32_t num_of_thread = 5;
    int32_t num_of_jobs = GetParam();
    client_config config;
    config.set_executor_pool_size(num_of_thread);

    if (client != nullptr) {
        client->shutdown().get();
    }
    client = new hazelcast_client{ new_client(std::move(config)).get() };

    spi::ClientContext ctx(*client);
    auto state = std::make_shared<ThreadState>(num_of_jobs);
    std::mutex mutex_for_thread_id;
    uint32_t expected_thread_num = std::min(num_of_jobs, num_of_thread);
    boost::barrier sync_barrier(expected_thread_num);

    ASSERT_EQ(0, state->thread_ids.size());
    for (int i = 0; i < num_of_jobs; i++) {
        ctx.get_client_execution_service().get_user_executor().submit(
          [state, &mutex_for_thread_id, &sync_barrier]() {
              sync_barrier.count_down_and_wait();
              auto curr_thread_id = boost::this_thread::get_id();
              {
                  std::lock_guard<std::mutex> lg(mutex_for_thread_id);
                  state->thread_ids.insert(curr_thread_id);
              }
              state->latch1.count_down();
          });
    }
    ASSERT_OPEN_EVENTUALLY(state->latch1);
    ASSERT_EQ( expected_thread_num, state->thread_ids.size());
}

INSTANTIATE_TEST_SUITE_P(ThreadPoolTestSuite,
                         ThreadPoolTest,
                         ::testing::Values(5, 10, 2));

} // namespace thread_pool
} // namespace test
} // namespace client
} // namespace hazelcast

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
