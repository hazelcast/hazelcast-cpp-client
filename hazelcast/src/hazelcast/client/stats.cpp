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
#include <iomanip>

#include <boost/algorithm/string/replace.hpp>

#include "hazelcast/client/impl/statistics/Statistics.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/client_properties.h"
#include "hazelcast/client/spi/impl/ClientExecutionServiceImpl.h"
#include "hazelcast/client/spi/impl/ClientInvocation.h"
#include "hazelcast/client/spi/lifecycle_service.h"
#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/client/connection/ClientConnectionManagerImpl.h"
#include "hazelcast/client/protocol/codec/codecs.h"
#include "hazelcast/client/protocol/codec/codecs.h"
#include "hazelcast/client/internal/nearcache/NearCache.h"
#include "hazelcast/client/internal/nearcache/NearCacheManager.h"
#include "hazelcast/client/monitor/impl/NearCacheStatsImpl.h"
#include "hazelcast/client/monitor/impl/LocalMapStatsImpl.h"
#include "hazelcast/client/client_config.h"

namespace hazelcast {
namespace client {
namespace impl {
namespace statistics {
const std::string Statistics::NEAR_CACHE_CATEGORY_PREFIX("nc.");

Statistics::Statistics(spi::ClientContext& client_context)
  : client_context_(client_context)
  , client_properties_(client_context.get_client_properties())
  , logger_(client_context.get_logger())
  , periodic_stats_(*this)
{
    this->enabled_ = client_properties_.get_boolean(client_properties_.get_statistics_enabled());
}

void
Statistics::start()
{
    if (!enabled_) {
        return;
    }

    int64_t periodSeconds =
      client_properties_.get_long(client_properties_.get_statistics_period_seconds());
    if (periodSeconds <= 0) {

        int64_t defaultValue = util::IOUtil::to_value<int64_t>(
          client_properties_.get_statistics_period_seconds().get_default_value());
        HZ_LOG(logger_,
               warning,
               boost::str(
                 boost::format("Provided client statistics %1% cannot be less than or equal to 0. "
                               "You provided %2% seconds as the configuration. "
                               "Client will use the default value of %3% instead.") %
                 client_properties_.get_statistics_period_seconds().get_name() % periodSeconds %
                 defaultValue));
        periodSeconds = defaultValue;
    }

    schedule_periodic_statistics_send_task(periodSeconds);

    HZ_LOG(logger_,
           info,
           boost::str(boost::format("Client statistics is enabled with period %1% seconds.") %
                      periodSeconds));
}

void
Statistics::shutdown()
{
    if (send_task_timer_) {
        boost::system::error_code ignored;
        send_task_timer_->cancel(ignored);
    }
}

void
Statistics::schedule_periodic_statistics_send_task(int64_t period_seconds)
{
    send_task_timer_ = client_context_.get_client_execution_service().schedule_with_repetition(
      [=]() {
          if (!client_context_.get_lifecycle_service().is_running()) {
              return;
          }

          auto collection_timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                                        std::chrono::system_clock::now().time_since_epoch())
                                        .count();
          std::shared_ptr<connection::Connection> connection = get_connection();
          if (!connection) {
              HZ_LOG(logger_,
                     finest,
                     "Cannot send client statistics to the server. No connection found.");
              return;
          }

          std::ostringstream stats;

          periodic_stats_.fill_metrics(stats, connection);

          periodic_stats_.add_near_cache_stats(stats);

          send_stats(collection_timestamp, stats.str(), connection);
      },
      std::chrono::seconds(0),
      std::chrono::seconds(period_seconds));
}

std::shared_ptr<connection::Connection>
Statistics::get_connection()
{
    return client_context_.get_connection_manager().get_random_connection();
}

void
Statistics::send_stats(int64_t timestamp,
                       const std::string& new_stats,
                       const std::shared_ptr<connection::Connection>& connection)
{
    // TODO: implement metrics blob
    auto request =
      protocol::codec::client_statistics_encode(timestamp, new_stats, std::vector<byte>());
    try {
        spi::impl::ClientInvocation::create(client_context_, request, "", connection)
          ->invoke()
          .get();
    } catch (exception::iexception& e) {
        // suppress exception, do not print too many messages
        HZ_LOG(logger_, finest, boost::str(boost::format("Could not send stats %1%") % e));
    }
}

void
Statistics::PeriodicStatistics::fill_metrics(
  std::ostringstream& stats,
  const std::shared_ptr<connection::Connection>& connection)
{
    stats << "lastStatisticsCollectionTime" << KEY_VALUE_SEPARATOR << util::current_time_millis();
    add_stat(stats, "enterprise", false);
    add_stat(stats, "clientType", protocol::ClientTypes::CPP);
    add_stat(stats, "clientVersion", HAZELCAST_VERSION);
    add_stat(stats,
             "clusterConnectionTimestamp",
             std::chrono::duration_cast<std::chrono::milliseconds>(
               connection->get_start_time().time_since_epoch())
               .count());

    auto localSocketAddress = connection->get_local_socket_address();
    stats << STAT_SEPARATOR << "clientAddress" << KEY_VALUE_SEPARATOR;
    if (localSocketAddress) {
        stats << localSocketAddress->get_host() << ":" << localSocketAddress->get_port();
    }

    add_stat(stats, "clientName", statistics_.client_context_.get_name());

    auto credential = statistics_.client_context_.get_client_config().get_credentials();
    if (credential) {
        add_stat(stats, "credentials.principal", credential->name());
    }
}

void
Statistics::PeriodicStatistics::add_near_cache_stats(std::ostringstream& stats)
{
    for (auto nearCache :
         statistics_.client_context_.get_near_cache_manager().list_all_near_caches()) {
        std::string nearCacheName = nearCache->get_name();
        std::ostringstream nearCacheNameWithPrefix;
        get_name_with_prefix(nearCacheName, nearCacheNameWithPrefix);

        nearCacheNameWithPrefix << '.';

        auto nearCacheStats = std::static_pointer_cast<monitor::impl::NearCacheStatsImpl>(
          nearCache->get_near_cache_stats());

        std::string prefix = nearCacheNameWithPrefix.str();

        add_stat(stats, prefix, "creationTime", nearCacheStats->get_creation_time());
        add_stat(stats, prefix, "evictions", nearCacheStats->get_evictions());
        add_stat(stats, prefix, "hits", nearCacheStats->get_hits());
        add_stat(stats,
                 prefix,
                 "lastPersistenceDuration",
                 nearCacheStats->get_last_persistence_duration());
        add_stat(stats,
                 prefix,
                 "lastPersistenceKeyCount",
                 nearCacheStats->get_last_persistence_key_count());
        add_stat(stats, prefix, "lastPersistenceTime", nearCacheStats->get_last_persistence_time());
        add_stat(stats,
                 prefix,
                 "lastPersistenceWrittenBytes",
                 nearCacheStats->get_last_persistence_written_bytes());
        add_stat(stats, prefix, "misses", nearCacheStats->get_misses());
        add_stat(stats, prefix, "ownedEntryCount", nearCacheStats->get_owned_entry_count());
        add_stat(stats, prefix, "expirations", nearCacheStats->get_expirations());
        add_stat(stats, prefix, "invalidations", nearCacheStats->get_invalidations());
        add_stat(
          stats, prefix, "invalidationRequests", nearCacheStats->get_invalidation_requests());
        add_stat(
          stats, prefix, "ownedEntryMemoryCost", nearCacheStats->get_owned_entry_memory_cost());
        std::string persistenceFailure = nearCacheStats->get_last_persistence_failure();
        if (!persistenceFailure.empty()) {
            add_stat(stats, prefix, "lastPersistenceFailure", persistenceFailure);
        }
    }
}

Statistics::PeriodicStatistics::PeriodicStatistics(Statistics& statistics)
  : statistics_(statistics)
{}

std::string
Statistics::escape_special_characters(std::string& name)
{
    boost::replace_all(name, ",", "\\,");
    boost::replace_all(name, "=", "\\=");
    boost::replace_all(name, "\\", "\\\\");

    return name[0] == '/' ? name.substr(1) : name;
}

void
Statistics::PeriodicStatistics::get_name_with_prefix(std::string& name, std::ostringstream& out)
{
    out << NEAR_CACHE_CATEGORY_PREFIX << Statistics::escape_special_characters(name);
}

template<>
void
Statistics::PeriodicStatistics::add_stat(std::ostringstream& stats,
                                         const std::string& name,
                                         const bool& value)
{
    stats << STAT_SEPARATOR << name << KEY_VALUE_SEPARATOR << (value ? "true" : "false");
}

} // namespace statistics
} // namespace impl

namespace monitor {
const int64_t local_instance_stats::STAT_NOT_AVAILABLE = -99L;

namespace impl {
LocalMapStatsImpl::LocalMapStatsImpl() = default;

LocalMapStatsImpl::LocalMapStatsImpl(const std::shared_ptr<monitor::near_cache_stats>& s)
  : near_cache_stats_(s)
{}

std::shared_ptr<monitor::near_cache_stats>
LocalMapStatsImpl::get_near_cache_stats() const
{
    return near_cache_stats_;
}

NearCacheStatsImpl::NearCacheStatsImpl()
  : creation_time_(util::current_time_millis())
  , owned_entry_count_(0)
  , owned_entry_memory_cost_(0)
  , hits_(0)
  , misses_(0)
  , evictions_(0)
  , expirations_(0)
  , invalidations_(0)
  , invalidation_requests_(0)
  , persistence_count_(0)
  , last_persistence_time_(0)
  , last_persistence_duration_(0)
  , last_persistence_written_bytes_(0)
  , last_persistence_key_count_(0)
  , last_persistence_failure_("")
{}

int64_t
NearCacheStatsImpl::get_creation_time()
{
    return creation_time_;
}

int64_t
NearCacheStatsImpl::get_owned_entry_count()
{
    return owned_entry_count_;
}

void
NearCacheStatsImpl::set_owned_entry_count(int64_t owned_entry_count)
{
    this->owned_entry_count_ = owned_entry_count;
}

void
NearCacheStatsImpl::increment_owned_entry_count()
{
    ++owned_entry_count_;
}

void
NearCacheStatsImpl::decrement_owned_entry_count()
{
    --owned_entry_count_;
}

int64_t
NearCacheStatsImpl::get_owned_entry_memory_cost()
{
    return owned_entry_memory_cost_;
}

void
NearCacheStatsImpl::set_owned_entry_memory_cost(int64_t owned_entry_memory_cost)
{
    this->owned_entry_memory_cost_ = owned_entry_memory_cost;
}

void
NearCacheStatsImpl::increment_owned_entry_memory_cost(int64_t owned_entry_memory_cost)
{
    this->owned_entry_memory_cost_ += owned_entry_memory_cost;
}

void
NearCacheStatsImpl::decrement_owned_entry_memory_cost(int64_t owned_entry_memory_cost)
{
    this->owned_entry_memory_cost_ -= owned_entry_memory_cost;
}

int64_t
NearCacheStatsImpl::get_hits()
{
    return hits_;
}

// just for testing
void
NearCacheStatsImpl::set_hits(int64_t hits)
{
    this->hits_ = hits;
}

void
NearCacheStatsImpl::increment_hits()
{
    ++hits_;
}

int64_t
NearCacheStatsImpl::get_misses()
{
    return misses_;
}

// just for testing
void
NearCacheStatsImpl::set_misses(int64_t misses)
{
    this->misses_ = misses;
}

void
NearCacheStatsImpl::increment_misses()
{
    ++misses_;
}

double
NearCacheStatsImpl::get_ratio()
{
    if (misses_ == (int64_t)0) {
        if (hits_ == (int64_t)0) {
            return std::numeric_limits<double>::signaling_NaN();
        } else {
            return std::numeric_limits<double>::infinity();
        }
    } else {
        return ((double)hits_ / misses_) * PERCENTAGE;
    }
}

int64_t
NearCacheStatsImpl::get_evictions()
{
    return evictions_;
}

void
NearCacheStatsImpl::increment_evictions()
{
    ++evictions_;
}

int64_t
NearCacheStatsImpl::get_expirations()
{
    return expirations_;
}

void
NearCacheStatsImpl::increment_expirations()
{
    ++expirations_;
}

int64_t
NearCacheStatsImpl::get_invalidations()
{
    return invalidations_.load();
}

void
NearCacheStatsImpl::increment_invalidations()
{
    ++invalidations_;
}

int64_t
NearCacheStatsImpl::get_invalidation_requests()
{
    return invalidation_requests_.load();
}

void
NearCacheStatsImpl::increment_invalidation_requests()
{
    ++invalidation_requests_;
}

void
NearCacheStatsImpl::reset_invalidation_events()
{
    invalidation_requests_ = 0;
}

int64_t
NearCacheStatsImpl::get_persistence_count()
{
    return persistence_count_;
}

void
NearCacheStatsImpl::add_persistence(int64_t duration, int32_t written_bytes, int32_t key_count)
{
    ++persistence_count_;
    last_persistence_time_ = util::current_time_millis();
    last_persistence_duration_ = duration;
    last_persistence_written_bytes_ = written_bytes;
    last_persistence_key_count_ = key_count;
    last_persistence_failure_ = "";
}

int64_t
NearCacheStatsImpl::get_last_persistence_time()
{
    return last_persistence_time_;
}

int64_t
NearCacheStatsImpl::get_last_persistence_duration()
{
    return last_persistence_duration_;
}

int64_t
NearCacheStatsImpl::get_last_persistence_written_bytes()
{
    return last_persistence_written_bytes_;
}

int64_t
NearCacheStatsImpl::get_last_persistence_key_count()
{
    return last_persistence_key_count_;
}

std::string
NearCacheStatsImpl::get_last_persistence_failure()
{
    return last_persistence_failure_;
}

std::string
NearCacheStatsImpl::to_string()
{
    std::ostringstream out;
    std::string failureString = last_persistence_failure_;
    out << "NearCacheStatsImpl{"
        << "ownedEntryCount=" << owned_entry_count_
        << ", ownedEntryMemoryCost=" << owned_entry_memory_cost_
        << ", creationTime=" << creation_time_ << ", hits=" << hits_ << ", misses=" << misses_
        << ", ratio=" << std::setprecision(1) << get_ratio() << ", evictions=" << evictions_
        << ", expirations=" << expirations_ << ", invalidations=" << invalidations_.load()
        << ", invalidationRequests=" << invalidation_requests_.load()
        << ", lastPersistenceTime=" << last_persistence_time_
        << ", persistenceCount=" << persistence_count_
        << ", lastPersistenceDuration=" << last_persistence_duration_
        << ", lastPersistenceWrittenBytes=" << last_persistence_written_bytes_
        << ", lastPersistenceKeyCount=" << last_persistence_key_count_
        << ", lastPersistenceFailure='" << failureString << "'" << '}';

    return out.str();
}

const double NearCacheStatsImpl::PERCENTAGE = 100.0;
} // namespace impl
} // namespace monitor
} // namespace client
} // namespace hazelcast
