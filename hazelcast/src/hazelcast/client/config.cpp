/*
 * Copyright (c) 2008-2022, Hazelcast, Inc. All Rights Reserved.
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
#include <atomic>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/trim.hpp>

#include "hazelcast/client/client_config.h"
#include "hazelcast/client/serialization_config.h"
#include "hazelcast/client/config/ssl_config.h"
#include "hazelcast/util/Preconditions.h"
#include "hazelcast/client/config/client_flake_id_generator_config.h"
#include "hazelcast/client/exception/protocol_exceptions.h"
#include "hazelcast/client/internal/partition/strategy/StringPartitioningStrategy.h"
#include "hazelcast/client/address.h"
#include "hazelcast/client/config/client_network_config.h"
#include "hazelcast/client/config/client_aws_config.h"
#include "hazelcast/client/config/reliable_topic_config.h"
#include "hazelcast/client/config/client_connection_strategy_config.h"
#include "hazelcast/client/config/logger_config.h"
#include "hazelcast/client/config/index_config.h"
#include "hazelcast/client/config/matcher/matching_point_config_pattern_matcher.h"
#include "hazelcast/client/query/predicates.h"
#include "hazelcast/client/lifecycle_listener.h"
#include "hazelcast/client/config/eviction_strategy_type.h"
#include "hazelcast/client/cluster.h"
#include "hazelcast/client/initial_membership_event.h"
#include "hazelcast/client/config/xml_config_locator.h"
#include "hazelcast/client/config/xml_dom_config_processor.h"
#include "hazelcast/client/config/xml_variable_replacer.h"

namespace hazelcast {
namespace client {
serialization_config::serialization_config()
  : version_(0)
{}

int
serialization_config::get_portable_version() const
{
    return version_;
}

serialization_config&
serialization_config::set_portable_version(int v)
{
    this->version_ = v;
    return *this;
}

std::shared_ptr<serialization::global_serializer>
serialization_config::get_global_serializer() const
{
    return global_serializer_;
}

serialization_config&
serialization_config::set_global_serializer(
  const std::shared_ptr<serialization::global_serializer>& global_serializer)
{
    global_serializer_ = global_serializer;
    return *this;
}

serialization_config&
serialization_config::set_byte_order(boost::endian::order byte_order)
{
    byte_order_ = byte_order;
    return *this;
}

boost::endian::order
serialization_config::get_byte_order() const
{
    return byte_order_;
}

namespace config {
ssl_config::ssl_config()
#ifdef HZ_BUILD_WITH_SSL
  : enabled_(false)
  , ssl_protocol_(tlsv12)
#endif
{}

#ifdef HZ_BUILD_WITH_SSL
ssl_config&
ssl_config::set_context(boost::asio::ssl::context ctx)
{
    util::Preconditions::check_ssl("ssl_config::set_context");
    if (enabled_) {
        throw exception::illegal_argument(
          "You should either use the deprecated methods or this method to "
          "enable ssl. You already used the deprecated way.");
    }
    ssl_context_ = std::make_shared<boost::asio::ssl::context>(std::move(ctx));
    return *this;
}

bool
ssl_config::is_enabled() const
{
    return ssl_context_ || enabled_;
}

ssl_config&
ssl_config::set_enabled(bool is_enabled)
{
    util::Preconditions::check_ssl("ssl_config::set_enabled");
    check_context_enabled_already();
    this->enabled_ = is_enabled;
    return *this;
}

void
ssl_config::check_context_enabled_already() const
{
    if (ssl_context_) {
        throw exception::illegal_argument(
          "You should either use set_context or this method.");
    }
}

ssl_config&
ssl_config::set_protocol(ssl_protocol protocol)
{
    check_context_enabled_already();
    this->ssl_protocol_ = protocol;
    return *this;
}

ssl_protocol
ssl_config::get_protocol() const
{
    check_context_enabled_already();
    return ssl_protocol_;
}

const std::vector<std::string>&
ssl_config::get_verify_files() const
{
    check_context_enabled_already();
    return client_verify_files_;
}

ssl_config&
ssl_config::add_verify_file(const std::string& filename)
{
    check_context_enabled_already();
    this->client_verify_files_.push_back(filename);
    return *this;
}

const std::string&
ssl_config::get_cipher_list() const
{
    return cipher_list_;
}

ssl_config&
ssl_config::set_cipher_list(const std::string& ciphers)
{
    this->cipher_list_ = ciphers;
    return *this;
}
#endif

constexpr int64_t
  client_flake_id_generator_config::DEFAULT_PREFETCH_VALIDITY_MILLIS;

client_flake_id_generator_config::client_flake_id_generator_config(
  const std::string& name)
  : name_(name)
  , prefetch_count_(client_flake_id_generator_config::DEFAULT_PREFETCH_COUNT)
  , prefetch_validity_duration_(
      client_flake_id_generator_config::DEFAULT_PREFETCH_VALIDITY_MILLIS)
{}

const std::string&
client_flake_id_generator_config::get_name() const
{
    return name_;
}

client_flake_id_generator_config&
client_flake_id_generator_config::set_name(const std::string& n)
{
    client_flake_id_generator_config::name_ = n;
    return *this;
}

int32_t
client_flake_id_generator_config::get_prefetch_count() const
{
    return prefetch_count_;
}

client_flake_id_generator_config&
client_flake_id_generator_config::set_prefetch_count(int32_t count)
{
    std::ostringstream out;
    out << "prefetch-count must be 1.." << MAXIMUM_PREFETCH_COUNT << ", not "
        << count;
    util::Preconditions::check_true(
      count > 0 && count <= MAXIMUM_PREFETCH_COUNT, out.str());
    prefetch_count_ = count;
    return *this;
}

std::chrono::milliseconds
client_flake_id_generator_config::get_prefetch_validity_duration() const
{
    return prefetch_validity_duration_;
}

client_flake_id_generator_config&
client_flake_id_generator_config::set_prefetch_validity_duration(
  std::chrono::milliseconds duration)
{
    util::Preconditions::check_not_negative(duration.count(),
                                            "duration must be nonnegative");
    prefetch_validity_duration_ = duration;
    return *this;
}

constexpr std::chrono::milliseconds connection_retry_config::INITIAL_BACKOFF;
constexpr std::chrono::milliseconds connection_retry_config::MAX_BACKOFF;
constexpr std::chrono::milliseconds
  connection_retry_config::CLUSTER_CONNECT_TIMEOUT;
constexpr double connection_retry_config::JITTER;

std::chrono::milliseconds
connection_retry_config::get_initial_backoff_duration() const
{
    return initial_backoff_duration_;
}

connection_retry_config&
connection_retry_config::set_initial_backoff_duration(
  std::chrono::milliseconds initial_backoff_duration)
{
    util::Preconditions::check_not_negative(
      initial_backoff_duration.count(),
      "Initial backoff must be non-negative!");
    initial_backoff_duration_ = initial_backoff_duration;
    return *this;
}

std::chrono::milliseconds
connection_retry_config::get_max_backoff_duration() const
{
    return max_backoff_duration_;
}

connection_retry_config&
connection_retry_config::set_max_backoff_duration(
  std::chrono::milliseconds max_backoff_duration)
{
    util::Preconditions::check_not_negative(
      max_backoff_duration.count(), "Max backoff must be non-negative!");
    max_backoff_duration_ = max_backoff_duration;
    return *this;
}

double
connection_retry_config::get_multiplier() const
{
    return multiplier_;
}

connection_retry_config&
connection_retry_config::set_multiplier(double m)
{
    util::Preconditions::check_true(
      m >= 1.0, "Multiplier must be greater than or equal to 1.0!");
    multiplier_ = m;
    return *this;
}

std::chrono::milliseconds
connection_retry_config::get_cluster_connect_timeout() const
{
    return cluster_connect_timeout_;
}

connection_retry_config&
connection_retry_config::set_cluster_connect_timeout(
  std::chrono::milliseconds cluster_connect_timeout)
{
    util::Preconditions::check_not_negative(
      cluster_connect_timeout.count(),
      "Cluster connect timeout must be non-negative!");
    cluster_connect_timeout_ = cluster_connect_timeout;
    return *this;
}

double
connection_retry_config::get_jitter() const
{
    return jitter_;
}

connection_retry_config&
connection_retry_config::set_jitter(double jitter)
{
    util::Preconditions::check_true(jitter >= 0.0 && jitter <= 1.0,
                                    "Jitter must be in range [0.0, 1.0]");
    jitter_ = jitter;
    return *this;
}

client_network_config::client_network_config()
  : connection_timeout_(5000)
  , smart_routing_(true)
{}

ssl_config&
client_network_config::get_ssl_config()
{
    return ssl_config_;
}

client_network_config&
client_network_config::set_ssl_config(const config::ssl_config& config)
{
    ssl_config_ = config;
    return *this;
}

std::chrono::milliseconds
client_network_config::get_connection_timeout() const
{
    return connection_timeout_;
}

client_network_config&
client_network_config::set_aws_config(
  const client_aws_config& client_aws_config)
{
    this->client_aws_config_ = client_aws_config;
    return *this;
}

client_aws_config&
client_network_config::get_aws_config()
{
    return client_aws_config_;
}

cloud_config&
client_network_config::get_cloud_config()
{
    return cloud_config_;
}

bool
client_network_config::is_smart_routing() const
{
    return smart_routing_;
}

client_network_config&
client_network_config::set_smart_routing(bool smart_routing)
{
    client_network_config::smart_routing_ = smart_routing;
    return *this;
}

std::vector<address>
client_network_config::get_addresses() const
{
    return address_list_;
}

client_network_config&
client_network_config::add_addresses(const std::vector<address>& addresses)
{
    address_list_.insert(
      address_list_.end(), addresses.begin(), addresses.end());
    return *this;
}

client_network_config&
client_network_config::set_addresses(const std::vector<address>& addresses)
{
    address_list_ = addresses;
    return *this;
}

client_network_config&
client_network_config::add_address(const address& address)
{
    address_list_.push_back(address);
    return *this;
}

socket_options&
client_network_config::get_socket_options()
{
    return socket_options_;
}

client_network_config&
client_network_config::set_connection_timeout(
  const std::chrono::milliseconds& timeout)
{
    connection_timeout_ = timeout;
    return *this;
}

bool
client_network_config::use_public_address() const
{
    return use_public_address_;
}

client_network_config&
client_network_config::use_public_address(bool should_use_public_address)
{
    use_public_address_ = should_use_public_address;
    return *this;
}

client_connection_strategy_config::client_connection_strategy_config()
  : async_start_(false)
  , reconnect_mode_(ON)
{}

client_connection_strategy_config::reconnect_mode
client_connection_strategy_config::get_reconnect_mode() const
{
    return reconnect_mode_;
}

bool
client_connection_strategy_config::is_async_start() const
{
    return async_start_;
}

client_connection_strategy_config&
client_connection_strategy_config::set_async_start(bool async_start)
{
    this->async_start_ = async_start;
    return *this;
}

client_connection_strategy_config&
client_connection_strategy_config::set_reconnect_mode(
  reconnect_mode reconnect_mode)
{
    this->reconnect_mode_ = reconnect_mode;
    return *this;
}

connection_retry_config&
client_connection_strategy_config::get_retry_config()
{
    return retry_config_;
}

client_connection_strategy_config&
client_connection_strategy_config::set_retry_config(
  connection_retry_config retry_config)
{
    retry_config_ = std::move(retry_config);
    return *this;
}

constexpr int reliable_topic_config::DEFAULT_READ_BATCH_SIZE;

reliable_topic_config::reliable_topic_config() = default;

reliable_topic_config::reliable_topic_config(std::string topic_name)
  : read_batch_size_(DEFAULT_READ_BATCH_SIZE)
  ,

  name_(std::move(topic_name))
{}

const std::string&
reliable_topic_config::get_name() const
{
    return name_;
}

int
reliable_topic_config::get_read_batch_size() const
{
    return read_batch_size_;
}

reliable_topic_config&
reliable_topic_config::set_read_batch_size(int batch_size)
{
    if (batch_size <= 0) {
        BOOST_THROW_EXCEPTION(
          exception::illegal_argument("ReliableTopicConfig::setReadBatchSize",
                                      "readBatchSize should be positive"));
    }

    this->read_batch_size_ = batch_size;

    return *this;
}

socket_options::socket_options()
  : tcp_no_delay_(true)
  , keep_alive_(true)
  , reuse_address_(true)
  , linger_seconds_(3)
  , buffer_size_(DEFAULT_BUFFER_SIZE_BYTE)
{}

bool
socket_options::is_tcp_no_delay() const
{
    return tcp_no_delay_;
}

socket_options&
socket_options::set_tcp_no_delay(bool tcp_no_delay)
{
    socket_options::tcp_no_delay_ = tcp_no_delay;
    return *this;
}

bool
socket_options::is_keep_alive() const
{
    return keep_alive_;
}

socket_options&
socket_options::set_keep_alive(bool keep_alive)
{
    socket_options::keep_alive_ = keep_alive;
    return *this;
}

bool
socket_options::is_reuse_address() const
{
    return reuse_address_;
}

socket_options&
socket_options::set_reuse_address(bool reuse_address)
{
    socket_options::reuse_address_ = reuse_address;
    return *this;
}

int
socket_options::get_linger_seconds() const
{
    return linger_seconds_;
}

socket_options&
socket_options::set_linger_seconds(int linger_seconds)
{
    socket_options::linger_seconds_ = linger_seconds;
    return *this;
}

int
socket_options::get_buffer_size_in_bytes() const
{
    return buffer_size_;
}

socket_options&
socket_options::set_buffer_size_in_bytes(int buffer_size)
{
    socket_options::buffer_size_ = buffer_size;
    return *this;
}

client_aws_config::client_aws_config()
  : enabled_(false)
  , region_("us-east-1")
  , host_header_("ec2.amazonaws.com")
  , inside_aws_(false)
{}

const std::string&
client_aws_config::get_access_key() const
{
    return access_key_;
}

client_aws_config&
client_aws_config::set_access_key(const std::string& access_key)
{
    this->access_key_ = util::Preconditions::check_has_text(
      access_key, "accessKey must contain text");
    return *this;
}

const std::string&
client_aws_config::get_secret_key() const
{
    return secret_key_;
}

client_aws_config&
client_aws_config::set_secret_key(const std::string& secret_key)
{
    this->secret_key_ = util::Preconditions::check_has_text(
      secret_key, "secretKey must contain text");
    return *this;
}

const std::string&
client_aws_config::get_region() const
{
    return region_;
}

client_aws_config&
client_aws_config::set_region(const std::string& region)
{
    this->region_ =
      util::Preconditions::check_has_text(region, "region must contain text");
    return *this;
}

const std::string&
client_aws_config::get_host_header() const
{
    return host_header_;
}

client_aws_config&
client_aws_config::set_host_header(const std::string& host_header)
{
    this->host_header_ = util::Preconditions::check_has_text(
      host_header, "hostHeader must contain text");
    return *this;
}

client_aws_config&
client_aws_config::set_enabled(bool enabled)
{
    util::Preconditions::check_ssl("get_aws_config");
    this->enabled_ = enabled;
    return *this;
}

bool
client_aws_config::is_enabled() const
{
    return enabled_;
}

client_aws_config&
client_aws_config::set_security_group_name(
  const std::string& security_group_name)
{
    this->security_group_name_ = security_group_name;
    return *this;
}

const std::string&
client_aws_config::get_security_group_name() const
{
    return security_group_name_;
}

client_aws_config&
client_aws_config::set_tag_key(const std::string& tag_key)
{
    this->tag_key_ = tag_key;
    return *this;
}

const std::string&
client_aws_config::get_tag_key() const
{
    return tag_key_;
}

client_aws_config&
client_aws_config::set_tag_value(const std::string& tag_value)
{
    this->tag_value_ = tag_value;
    return *this;
}

const std::string&
client_aws_config::get_tag_value() const
{
    return tag_value_;
}

const std::string&
client_aws_config::get_iam_role() const
{
    return iam_role_;
}

client_aws_config&
client_aws_config::set_iam_role(const std::string& iam_role)
{
    this->iam_role_ = iam_role;
    return *this;
}

bool
client_aws_config::is_inside_aws() const
{
    return inside_aws_;
}

client_aws_config&
client_aws_config::set_inside_aws(bool inside_aws)
{
    this->inside_aws_ = inside_aws;
    return *this;
}

std::ostream&
operator<<(std::ostream& out, const client_aws_config& config)
{
    return out << "client_aws_config{"
               << "enabled=" << config.is_enabled() << ", region='"
               << config.get_region() << '\'' << ", securityGroupName='"
               << config.get_security_group_name() << '\'' << ", tagKey='"
               << config.get_tag_key() << '\'' << ", tagValue='"
               << config.get_tag_value() << '\'' << ", hostHeader='"
               << config.get_host_header() << '\'' << ", iamRole='"
               << config.get_iam_role() << "\'}";
}

namespace matcher {
std::shared_ptr<std::string>
matching_point_config_pattern_matcher::matches(
  const std::vector<std::string>& config_patterns,
  const std::string& item_name) const
{
    std::shared_ptr<std::string> candidate;
    std::shared_ptr<std::string> duplicate;
    int lastMatchingPoint = -1;
    for (const std::string& pattern : config_patterns) {
        int matchingPoint = get_matching_point(pattern, item_name);
        if (matchingPoint > -1 && matchingPoint >= lastMatchingPoint) {
            if (matchingPoint == lastMatchingPoint) {
                duplicate = candidate;
            } else {
                duplicate.reset();
            }
            lastMatchingPoint = matchingPoint;
            candidate.reset(new std::string(pattern));
        }
    }
    if (duplicate.get() != NULL) {
        throw(exception::exception_builder<exception::invalid_configuration>(
                "MatchingPointConfigPatternMatcher::matches")
              << "Configuration " << item_name
              << " has duplicate configuration. Candidate:" << *candidate
              << ", duplicate:" << *duplicate)
          .build();
    }
    return candidate;
}

int
matching_point_config_pattern_matcher::get_matching_point(
  const std::string& pattern,
  const std::string& item_name) const
{
    size_t index = pattern.find('*');
    if (index == std::string::npos) {
        return -1;
    }

    std::string firstPart = pattern.substr(0, index);
    if (item_name.find(firstPart) != 0) {
        return -1;
    }

    std::string secondPart = pattern.substr(index + 1);
    if (item_name.rfind(secondPart) !=
        (item_name.length() - secondPart.length())) {
        return -1;
    }

    return (int)(firstPart.length() + secondPart.length());
}
} // namespace matcher

const std::string index_config::bitmap_index_options::DEFAULT_KEY =
  query::query_constants::KEY_ATTRIBUTE_NAME;
const index_config::bitmap_index_options::unique_key_transformation
  index_config::bitmap_index_options::DEFAULT_TRANSFORMATION =
    index_config::bitmap_index_options::unique_key_transformation::OBJECT;

index_config::bitmap_index_options::bitmap_index_options()
  : key(DEFAULT_KEY)
  , transformation(DEFAULT_TRANSFORMATION)
{}

const index_config::index_type index_config::DEFAULT_TYPE =
  index_config::index_type::SORTED;

index_config::index_config()
  : type(DEFAULT_TYPE)
{}

index_config::index_config(index_config::index_type type)
  : type(type)
{}

void
index_config::add_attributes()
{}

eviction_config::eviction_config()
  : size_(DEFAULT_MAX_ENTRY_COUNT)
  , max_size_policy_(DEFAULT_MAX_SIZE_POLICY)
  , eviction_policy_(DEFAULT_EVICTION_POLICY)
{}

int32_t
eviction_config::get_size() const
{
    return size_;
}

eviction_config&
eviction_config::set_size(int32_t size)
{
    this->size_ = util::Preconditions::check_positive(
      size, "Size must be positive number!");
    return *this;
}

eviction_config::max_size_policy
eviction_config::get_maximum_size_policy() const
{
    return max_size_policy_;
}

eviction_config&
eviction_config::set_maximum_size_policy(
  const eviction_config::max_size_policy& max_size_policy)
{
    this->max_size_policy_ = max_size_policy;
    return *this;
}

eviction_policy
eviction_config::get_eviction_policy() const
{
    return eviction_policy_;
}

eviction_config&
eviction_config::set_eviction_policy(eviction_policy policy)
{
    this->eviction_policy_ = policy;
    return *this;
}

eviction_strategy_type
eviction_config::get_eviction_strategy_type() const
{
    // TODO: add support for other/custom eviction strategies
    return eviction_strategy_type::DEFAULT_EVICTION_STRATEGY;
}

std::ostream&
operator<<(std::ostream& out, const eviction_config& config)
{
    out << "EvictionConfig{"
        << "size=" << config.get_size()
        << ", maxSizePolicy=" << config.get_maximum_size_policy()
        << ", evictionPolicy=" << config.get_eviction_policy() << '}';

    return out;
}

near_cache_config::near_cache_config()
  : name_("default")
  , time_to_live_seconds_(DEFAULT_TTL_SECONDS)
  , max_idle_seconds_(DEFAULT_MAX_IDLE_SECONDS)
  , in_memory_format_(DEFAULT_MEMORY_FORMAT)
  , local_update_policy_(INVALIDATE)
  , invalidate_on_change_(true)
  , cache_local_entries_(false)
{}

near_cache_config::near_cache_config(const std::string& cache_name)
  : near_cache_config()
{
    name_ = cache_name;
}

near_cache_config::near_cache_config(const std::string& cache_name,
                                     in_memory_format memory_format)
  : near_cache_config(name_)
{
    this->in_memory_format_ = memory_format;
}

near_cache_config::near_cache_config(int32_t time_to_live_seconds,
                                     int32_t max_idle_seconds,
                                     bool invalidate_on_change,
                                     in_memory_format in_memory_format,
                                     const eviction_config& evict_config)
  : near_cache_config(name_, in_memory_format)
{
    this->time_to_live_seconds_ = time_to_live_seconds;
    this->max_idle_seconds_ = max_idle_seconds;
    this->invalidate_on_change_ = invalidate_on_change;
    this->eviction_config_ = evict_config;
}

const std::string&
near_cache_config::get_name() const
{
    return name_;
}

near_cache_config&
near_cache_config::set_name(const std::string& name)
{
    this->name_ = name;
    return *this;
}

int32_t
near_cache_config::get_time_to_live_seconds() const
{
    return time_to_live_seconds_;
}

near_cache_config&
near_cache_config::set_time_to_live_seconds(int32_t time_to_live_seconds)
{
    this->time_to_live_seconds_ = util::Preconditions::check_not_negative(
      time_to_live_seconds, "TTL seconds cannot be negative!");
    return *this;
}

int32_t
near_cache_config::get_max_idle_seconds() const
{
    return max_idle_seconds_;
}

near_cache_config&
near_cache_config::set_max_idle_seconds(int32_t max_idle_seconds)
{
    this->max_idle_seconds_ = util::Preconditions::check_not_negative(
      max_idle_seconds, "Max-Idle seconds cannot be negative!");
    return *this;
}

bool
near_cache_config::is_invalidate_on_change() const
{
    return invalidate_on_change_;
}

near_cache_config&
near_cache_config::set_invalidate_on_change(bool invalidate_on_change)
{
    this->invalidate_on_change_ = invalidate_on_change;
    return *this;
}

const in_memory_format&
near_cache_config::get_in_memory_format() const
{
    return in_memory_format_;
}

near_cache_config&
near_cache_config::set_in_memory_format(
  const in_memory_format& in_memory_format)
{
    this->in_memory_format_ = in_memory_format;
    return *this;
}

bool
near_cache_config::is_cache_local_entries() const
{
    return cache_local_entries_;
}

near_cache_config&
near_cache_config::set_cache_local_entries(bool cache_local_entries)
{
    this->cache_local_entries_ = cache_local_entries;
    return *this;
}

const near_cache_config::local_update_policy&
near_cache_config::get_local_update_policy() const
{
    return local_update_policy_;
}

near_cache_config&
near_cache_config::set_local_update_policy(
  const local_update_policy& local_update_policy)
{
    this->local_update_policy_ = local_update_policy;
    return *this;
}

eviction_config&
near_cache_config::get_eviction_config()
{
    return eviction_config_;
}

near_cache_config&
near_cache_config::set_eviction_config(const eviction_config& eviction_config)
{
    this->eviction_config_ = eviction_config;
    return *this;
}

int32_t
near_cache_config::calculate_max_size(int32_t max_size)
{
    return (max_size == 0) ? INT32_MAX
                           : util::Preconditions::check_not_negative(
                               max_size, "Max-size cannot be negative!");
}

std::ostream&
operator<<(std::ostream& out, const near_cache_config& config)
{
    out << "NearCacheConfig{"
        << "timeToLiveSeconds=" << config.time_to_live_seconds_
        << ", maxIdleSeconds=" << config.max_idle_seconds_
        << ", invalidateOnChange=" << config.invalidate_on_change_
        << ", inMemoryFormat=" << config.in_memory_format_
        << ", cacheLocalEntries=" << config.cache_local_entries_
        << ", localUpdatePolicy=" << config.local_update_policy_
        << config.eviction_config_;
    out << '}';

    return out;
}

client_config_sections::client_config_sections(std::string name, bool multiple_occurrence){
    this->name = name;
    this->multiple_occurrence = multiple_occurrence;
}
std::string client_config_sections::get_name(){
    return name;
}
static client_config_sections HAZELCAST_CLIENT("hazelcast-client", false);
static client_config_sections IMPORT("import", true);
static client_config_sections SECURITY("security", false);
static client_config_sections PROXY_FACTORIES("proxy-factories", false);
static client_config_sections PROPERTIES("properties", false);
static client_config_sections SERIALIZATION("serialization", false);
static client_config_sections NATIVE_MEMORY("native-memory", false);
static client_config_sections LISTENERS("listeners", false);
static client_config_sections NETWORK("network", false);
static client_config_sections LOAD_BALANCER("load-balancer", false);
static client_config_sections NEAR_CACHE("near-cache", true);
static client_config_sections QUERY_CACHES("query-caches", false);
static client_config_sections BACKUP_ACK_TO_CLIENT("backup-ack-to-client-enabled", false);
static client_config_sections INSTANCE_NAME("instance-name", false);
static client_config_sections CONNECTION_STRATEGY("connection-strategy", false);
static client_config_sections USER_CODE_DEPLOYMENT("user-code-deployment", false);
static client_config_sections FLAKE_ID_GENERATOR("flake-id-generator", true);
static client_config_sections RELIABLE_TOPIC("reliable-topic", true);
static client_config_sections LABELS("client-labels", false);
static client_config_sections CLUSTER_NAME("cluster-name", false);
static client_config_sections METRICS("metrics", false);
static client_config_sections INSTANCE_TRACKING("instance-tracking", false);

std::vector<client_config_sections> values = {HAZELCAST_CLIENT, IMPORT, SECURITY, PROXY_FACTORIES, PROPERTIES, SERIALIZATION, NATIVE_MEMORY,
                                               LISTENERS, NETWORK, LOAD_BALANCER, NEAR_CACHE, QUERY_CACHES, BACKUP_ACK_TO_CLIENT, INSTANCE_NAME,
                                               CONNECTION_STRATEGY, USER_CODE_DEPLOYMENT, FLAKE_ID_GENERATOR, RELIABLE_TOPIC, LABELS, CLUSTER_NAME,
                                               METRICS, INSTANCE_TRACKING};

bool client_config_sections::can_occur_multiple_times(std::string name){
    for (client_config_sections section : values){
        if(name == section.get_name()){
            return section.multiple_occurrence;
        }
    }
    return true;
}

abstract_dom_config_processor::abstract_dom_config_processor(bool dom_level_3){
    this->dom_level_3 = dom_level_3;
    this->strict = true;
}
abstract_dom_config_processor::abstract_dom_config_processor(bool dom_level_3, bool strict){
    this->dom_level_3 = dom_level_3;
    this->strict = strict;
}

bool abstract_dom_config_processor::matches(std::string config1, std::string config2){
    return config1 == config2;
}
std::string abstract_dom_config_processor::get_attribute(boost::property_tree::ptree node, std::string attribute){
    return node.get_child("<xmlattr>." + attribute).data();
}
bool abstract_dom_config_processor::get_bool_value(std::string value){
    boost::algorithm::to_lower(value);
    if(value == "true"){
        return true;
    }
    else if(value == "false"){
        return false;
    }
}
int abstract_dom_config_processor::get_integer_value(std::string parameter_name, std::string value){
    try{
        return std::stoi(value);
    }catch (const std::exception& e){
        throw new hazelcast::client::exception::invalid_configuration("Invalid integer value for parameter "+parameter_name+": "+value);
    }
}
int abstract_dom_config_processor::get_integer_value(std::string parameter_name, std::string value, int default_value){
    if(value.empty()){
        return default_value;
    }
    return get_integer_value(parameter_name, value);
}
long abstract_dom_config_processor::get_long_value(std::string parameter_name, std::string value){
    try{
        return std::stol(value);
    }catch (const std::exception& e){
        throw new hazelcast::client::exception::invalid_configuration("Invalid long value for parameter "+parameter_name+": "+value);
    }
}
long abstract_dom_config_processor::get_long_value(std::string parameter_name, std::string value, long default_value){
    if(value.empty()){
        return default_value;
    }
    return get_long_value(parameter_name, value);
}
double abstract_dom_config_processor::get_double_value(std::string parameter_name, std::string value){
    try{
        return std::stod(value);
    }catch (const std::exception& e){
        throw new hazelcast::client::exception::invalid_configuration("Invalid double value for parameter "+parameter_name+": "+value);
    }
}
double abstract_dom_config_processor::get_double_value(std::string parameter_name, std::string value, double default_value){
    if(value.empty()){
        return default_value;
    }
    return get_double_value(parameter_name, value);
}

hazelcast::client::serialization_config abstract_dom_config_processor::parse_serialization(boost::property_tree::ptree node){
    auto serialization_config = new hazelcast::client::serialization_config();
    for (auto& child : node) {
        std::string name = child.first;
        if (matches("portable-version", name)) {
            std::string value = child.second.data();
            serialization_config->set_portable_version(get_integer_value(child.first,value));
        } else if (matches("check-class-def-errors",  name)) {
            std::string value = child.second.data();
            //serializationConfig.setCheckClassDefErrors(getBooleanValue(value));
        } else if (matches("use-native-byte-order" , name)) {
            serialization_config->set_byte_order(boost::endian::order::native);
        } else if (matches("byte-order" , name)) {
            std::string value = child.second.data();
            if (matches("BIG_ENDIAN", value)) {
                serialization_config->set_byte_order(boost::endian::order::big);
            } else if (matches("LITTLE_ENDIAN", value)) {
                serialization_config->set_byte_order(
                  boost::endian::order::little);
            }
        } /*else if (matches("enable-compression", name)) {
            serializationConfig.setEnableCompression(getBooleanValue(getTextContent(child)));
        } else if (matches("enable-shared-object", name)) {
            serializationConfig.setEnableSharedObject(getBooleanValue(getTextContent(child)));
        } else if (matches("allow-unsafe", name)) {
            serializationConfig.setAllowUnsafe(getBooleanValue(getTextContent(child)));
        } else if (matches("allow-override-default-serializers", name)) {
            serializationConfig.setAllowOverrideDefaultSerializers(getBooleanValue(getTextContent(child)));
        } else if (matches("data-serializable-factories", name)) {
            fillDataSerializableFactories(child, serializationConfig);
        } else if (matches("portable-factories", name)) {
            fillPortableFactories(child, serializationConfig);
        } else if (matches("serializers", name)) {
            fillSerializers(child, serializationConfig);
        } else if (matches("java-serialization-filter", name)) {
            fillJavaSerializationFilter(child, serializationConfig);
        } else if (matches("compact-serialization", name)) {
            handleCompactSerialization(child, serializationConfig);
        }*/
    }
    return *serialization_config;
}
void abstract_dom_config_processor::fill_properties(boost::property_tree::ptree node, hazelcast::client::client_properties properties){
    fill_properties(node, std::move(properties), dom_level_3);
}
void abstract_dom_config_processor::fill_properties(const boost::property_tree::ptree& node, hazelcast::client::client_properties properties, bool dom_level_3){
 /* TODO
  *
  */
}
void abstract_dom_config_processor::fill_properties(const boost::property_tree::ptree& node, std::unordered_map<std::string , std::string> properties, bool dom_level_3){
    /* TODO
    *
     */
}

boost::property_tree::ptree abstract_dom_config_processor::pair_to_node(std::string node_name, boost::property_tree::ptree node_content){
    boost::property_tree::ptree temp;
    temp.add_child(node_name, node_content);
    return temp.get_child(node_name);
}

client_dom_config_processor::client_dom_config_processor(bool dom_level_3, hazelcast::client::client_config* client_config) : abstract_dom_config_processor(dom_level_3){
    this->client_config = client_config;
}
client_dom_config_processor::client_dom_config_processor( bool dom_level_3, hazelcast::client::client_config* client_config, bool strict) : abstract_dom_config_processor(dom_level_3,strict){
    this->client_config = client_config;
}
void client_dom_config_processor::build_config(const boost::property_tree::ptree& root_node){
    for(auto& pair : root_node){
        std::string node_name = pair.first;
        auto node = pair_to_node(pair.first, pair.second);
        if(occurrence_set->find(node_name) == occurrence_set->end()){
            handle_node(node, node_name);
        }
        else{
            throw hazelcast::client::exception::invalid_configuration("Duplicate '" + node_name + "' definition found in the configuration");
        }
        if(!client_config_sections::can_occur_multiple_times(node_name)){
            occurrence_set->insert(node_name);
        }
    }
}
void client_dom_config_processor::handle_node(const boost::property_tree::ptree& node, std::string &node_name){
    if(matches(node_name,client_config_sections::SECURITY.get_name())){
        handle_security(node);
    }
    else if(matches(node_name,client_config_sections::PROXY_FACTORIES.get_name())){
        handle_proxy_factories(node);
    }
    else if(matches(node_name,client_config_sections::PROPERTIES.get_name())){
        fill_properties(node, client_config->get_properties());
    }
    else if(matches(node_name,client_config_sections::SERIALIZATION.get_name())){
        handle_serialization(node);
    }
    else if(matches(node_name,client_config_sections::NATIVE_MEMORY.get_name())){//not supported

    }
    else if(matches(node_name,client_config_sections::LISTENERS.get_name())){
        handle_listeners(node);
    }
    else if(matches(node_name,client_config_sections::NETWORK.get_name())){
        handle_network(node);
    }
    else if(matches(node_name,client_config_sections::LOAD_BALANCER.get_name())){
        handle_load_balancer(node);
    }
    else if(matches(node_name,client_config_sections::NEAR_CACHE.get_name())){
        handle_near_cache(node);
    }
    else if(matches(node_name,client_config_sections::QUERY_CACHES.get_name())){//not supported

    }
    else if(matches(node_name,client_config_sections::INSTANCE_NAME.get_name())){
        client_config->set_instance_name(node.data());
    }
    else if(matches(node_name,client_config_sections::CONNECTION_STRATEGY.get_name())){
        handle_connection_strategy(node);
    }
    else if(matches(node_name,client_config_sections::USER_CODE_DEPLOYMENT.get_name())){//not supported

    }
    else if(matches(node_name,client_config_sections::FLAKE_ID_GENERATOR.get_name())){
        handle_flake_id_generator(node);
    }
    else if(matches(node_name,client_config_sections::RELIABLE_TOPIC.get_name())){
        handle_reliable_topic(node);
    }
    else if(matches(node_name,client_config_sections::LABELS.get_name())){
        handle_labels(node);
    }
    else if(matches(node_name,client_config_sections::BACKUP_ACK_TO_CLIENT.get_name())){
        handle_backup_ack_to_client(node);
    }
    else if(matches(node_name,client_config_sections::CLUSTER_NAME.get_name())){
        client_config->set_cluster_name(node.data());
    }
    else if(matches(node_name,client_config_sections::METRICS.get_name())){//?????

    }
    else if(matches(node_name,client_config_sections::INSTANCE_TRACKING.get_name())){//?????

    }
}

void client_dom_config_processor::handle_security(const boost::property_tree::ptree& node){
    for(auto& pair : node){
        auto child = pair_to_node(pair.first, pair.second);
        std::string node_name = pair.first;
        if (matches("username-password", node_name)) {
            client_config->set_credentials(
              std::make_shared<
                hazelcast::client::security::username_password_credentials>(
                get_attribute(child,"username"), get_attribute(child, "password")));
        } else if (matches("token", node_name)) {
            // TODO
        } else if (matches("credentials-factory", node_name)) {//not supported

        } else if (matches("kerberos", node_name)) {//not supported

        } else if (matches("realms", node_name)) {
            handle_realms(child);
        }
    }

}
void client_dom_config_processor::handle_serialization(const boost::property_tree::ptree& node){
    hazelcast::client::serialization_config serialization_config = parse_serialization(node);
    client_config->set_serialization_config(serialization_config);
}
void client_dom_config_processor::handle_network(const boost::property_tree::ptree& node){
    auto client_network_config = new hazelcast::client::config::client_network_config();
    for (auto& pair : node) {
        auto child = pair_to_node(pair.first, pair.second);
        std::string node_name = pair.first;
        if (matches("cluster-members", node_name)) {
            handle_cluster_members(child, client_network_config);
        } else if (matches("smart-routing",node_name)) {
            client_network_config->set_smart_routing(get_bool_value(pair.second.data()));
        } else if (matches("redo-operation", node_name)) {
            client_config->set_redo_operation(get_bool_value(pair.second.data()));
        } else if (matches("connection-timeout", node_name)){
            client_network_config->set_connection_timeout(std::chrono::milliseconds(get_integer_value(node_name, pair.second.data())));
        } else if (matches("socket-options", node_name)) {
            handle_socket_options(child, client_network_config);
        } else if (matches("socket-interceptor", node_name)) {//not supported

        } else if (matches("ssl" , node_name)) {
            handle_ssl_config(child, client_network_config);
        } //else if (AliasedDiscoveryConfigUtils.supports(node_name)) {//not supported
          // handleAliasedDiscoveryStrategy(child, clientNetworkConfig, nodeName);}
        else if (matches("discovery-strategies", node_name)) {//not supported

        } else if (matches("auto-detection", node_name)) {//not supported

        } else if (matches("outbound-ports", node_name)) {//not supported

        } else if (matches("icmp-ping", node_name)) {//not supported

        } else if (matches("hazelcast-cloud" , node_name)) {
            handle_hazelcast_cloud(child, client_network_config);
        }
    }
    client_config->set_network_config(*client_network_config);
}

void client_dom_config_processor::handle_cluster_members(const boost::property_tree::ptree& node, hazelcast::client::config::client_network_config* client_network_config){
    for(auto& pair : node){
        if(matches("address" , pair.first)){
            int port;//port is not included in the example usage in hazelcast-client-full-example.xml
            client_network_config->add_address(hazelcast::client::address(pair.second.data(),port));
        }
    }
}

void client_dom_config_processor::handle_socket_options(const boost::property_tree::ptree& node, hazelcast::client::config::client_network_config* client_network_config){
    for(auto& pair : node){
        auto child = pair_to_node(pair.first, pair.second);
        std::string node_name = pair.first;
        if (matches("tcp-no-delay", node_name)) {
            client_network_config->get_socket_options().set_tcp_no_delay(get_bool_value(pair.second.data()));
        } else if (matches("keep-alive", node_name)) {
            client_network_config->get_socket_options().set_keep_alive(get_bool_value(pair.second.data()));
        } else if (matches("reuse-address" , node_name)) {
            client_network_config->get_socket_options().set_reuse_address(get_bool_value(pair.second.data()));
        } else if (matches("linger-seconds" , node_name)) {
            client_network_config->get_socket_options().set_linger_seconds(get_integer_value(pair.first, pair.second.data()));
        } else if (matches("buffer-size" ,node_name)) {
            client_network_config->get_socket_options().set_buffer_size_in_bytes(get_integer_value(pair.first, pair.second.data()));
        }
    }
}
void client_dom_config_processor::handle_ssl_config(const boost::property_tree::ptree& node, hazelcast::client::config::client_network_config* client_network_config){

}
void client_dom_config_processor::handle_hazelcast_cloud(const boost::property_tree::ptree& node, hazelcast::client::config::client_network_config* client_network_config){
    bool enabled = false;
    try{
        std::string enable = get_attribute(node, "enabled");
        boost::algorithm::trim(enable);
        enabled = get_bool_value(enable);
    }catch (const boost::exception& e){

    }
    client_network_config->get_cloud_config().enabled = enabled;
    for (auto& pair : node) {
        auto child = pair_to_node(pair.first, pair.second);
        std::string node_name = pair.first;
        if(matches("discovery-token" , pair.first)){
            client_network_config->get_cloud_config().discovery_token = pair.second.data();
        }
    }
}
void client_dom_config_processor::client_dom_config_processor::handle_flake_id_generator(const boost::property_tree::ptree& node){
    handle_flake_id_generator_node(node);
}

void client_dom_config_processor::client_dom_config_processor::handle_flake_id_generator_node(const boost::property_tree::ptree& node){
    std::string name = get_attribute(node, "name");
    auto config = new hazelcast::client::config::client_flake_id_generator_config(name);
    for (auto& pair : node) {
        auto child = pair_to_node(pair.first, pair.second);
        std::string node_name = pair.first;
        boost::algorithm::to_lower(node_name);
        if(matches("prefetch-count" , node_name)){
            config->set_prefetch_count(get_integer_value(pair.first, pair.second.data()));
        }
        else if(matches("prefetch-validity-millis" ,node_name )){
            config->set_prefetch_validity_duration(std::chrono::milliseconds(get_long_value(node_name, pair.second.data())));
        }

    }
    client_config->add_flake_id_generator_config(*config);
}

void client_dom_config_processor::handle_reliable_topic(const boost::property_tree::ptree& node){
    handle_reliable_topic_node(node);
}
void client_dom_config_processor::handle_reliable_topic_node(const boost::property_tree::ptree& node){
    std::string name = get_attribute(node, "name");
    auto config = new hazelcast::client::config::reliable_topic_config(name);
    for (auto& pair : node) {
        auto child = pair_to_node(pair.first, pair.second);
        std::string node_name = pair.first;
        if(matches("topic-overload-policy" , node_name)){//doesnt exist

        }
        else if(matches("read-batch-size" , node_name)){
            config->set_read_batch_size(get_integer_value(pair.first, pair.second.data()));
        }
    }
    client_config->add_reliable_topic_config(*config);
}
void client_dom_config_processor::handle_near_cache(const boost::property_tree::ptree& node){
    handle_near_cache_node(node);
}
void client_dom_config_processor::handle_near_cache_node(const boost::property_tree::ptree& node){
    hazelcast::client::config::near_cache_config * near_cache_config;
    try{
        std::string name = get_attribute(node,"name");
        near_cache_config = new hazelcast::client::config::near_cache_config(name);
    } catch(const boost::exception& e){
        near_cache_config = new hazelcast::client::config::near_cache_config("default");
    }
    for (auto& pair: node) {
        auto child = pair_to_node(pair.first, pair.second);
        std::string node_name = pair.first;
        if (matches("time-to-live-seconds" , node_name)) {
            near_cache_config->set_time_to_live_seconds(get_integer_value(pair.first, pair.second.data()));
        } else if (matches("max-idle-seconds" , node_name)) {
            near_cache_config->set_max_idle_seconds(get_integer_value(pair.first, pair.second.data()));
        } else if (matches("in-memory-format" , node_name)) {//NO NATIVE ??
            if(pair.second.data() == "BINARY"){
                near_cache_config->set_in_memory_format(hazelcast::client::config::in_memory_format::BINARY);
            }
            else if(pair.second.data() == "OBJECT"){
                near_cache_config->set_in_memory_format(hazelcast::client::config::in_memory_format::OBJECT);
            }
        } else if (matches("serialize-keys", node_name)) {//??
            //serializeKeys = Boolean.parseBoolean(getTextContent(child));
            //nearCacheConfig.setSerializeKeys(serializeKeys);
        } else if (matches("invalidate-on-change", node_name)) {
            near_cache_config->set_invalidate_on_change(get_bool_value(pair.second.data()));
        } else if (matches("local-update-policy", node_name)){
            if(pair.second.data() == "CACHE"){
                near_cache_config->set_local_update_policy(hazelcast::client::config::near_cache_config::CACHE);
            }
            else if (pair.second.data() == "INVALIDATE"){
                near_cache_config->set_local_update_policy(hazelcast::client::config::near_cache_config::INVALIDATE);
            }
        } else if (matches("eviction" , node_name)) {
            near_cache_config->set_eviction_config(get_eviction_config(pair.second));
        } else if (matches("preloader", node_name)) {//??
                                                      // nearCacheConfig.setPreloaderConfig(getNearCachePreloaderConfig(child));
        }
    }
    /*if (serializeKeys != null && !serializeKeys && nearCacheConfig.getInMemoryFormat() == InMemoryFormat.NATIVE) {
        std::cout << "WARNING: " << "The Near Cache doesn't support keys by-reference with NATIVE in-memory-format. This setting will have no effect!" << std::endl;
    }*/
    client_config->add_near_cache_config(*near_cache_config);
}
hazelcast::client::config::eviction_config client_dom_config_processor::get_eviction_config(const boost::property_tree::ptree& node){
    auto eviction_config = hazelcast::client::config::eviction_config();
    try{
        auto attr = get_attribute(node, "size");
        eviction_config.set_size(get_integer_value("size", attr));
    }catch (const boost::exception& e){

    }
    try{
        auto attr = get_attribute(node, "max-size-policy");
        if(attr == "ENTRY_COUNT"){// other policies are not implemented
            eviction_config.set_maximum_size_policy(hazelcast::client::config::eviction_config::ENTRY_COUNT);
        }
    } catch (const boost::exception& e){

    }
    try{
        auto attr = get_attribute(node, "eviction-policy");
        if(attr == "LRU"){
            eviction_config.set_eviction_policy(hazelcast::client::config::eviction_policy::LRU);
        }
        else if(attr == "LFU"){
            eviction_config.set_eviction_policy(hazelcast::client::config::eviction_policy::LFU);
        }
        else if(attr == "NONE"){
            eviction_config.set_eviction_policy(hazelcast::client::config::eviction_policy::NONE);
        }
        else if(attr == "RANDOM"){
            eviction_config.set_eviction_policy(hazelcast::client::config::eviction_policy::RANDOM);
        }
    } catch (const boost::exception& e){

    }
    try{
        get_attribute(node, "comparator-class-name");//couldn't find this option
    } catch (const boost::exception& e){

    }
    return eviction_config;
}
void client_dom_config_processor::handle_connection_strategy(const boost::property_tree::ptree& node){
    auto strategy_config = hazelcast::client::config::client_connection_strategy_config();
    try{
        std::string attr_val = get_attribute(node, "async-start");
        boost::algorithm::trim(attr_val);
        strategy_config.set_async_start(get_bool_value(attr_val));
    }catch (const boost::exception& e){
        strategy_config.set_async_start(false);
    }
    try{
        std::string attr_val1 = get_attribute(node, "reconnect-mode");
        boost::algorithm::trim(attr_val1);
        if(attr_val1 == "ON"){
            strategy_config.set_reconnect_mode(hazelcast::client::config::client_connection_strategy_config::reconnect_mode::ON);
        } else if(attr_val1 == "OFF"){
            strategy_config.set_reconnect_mode(hazelcast::client::config::client_connection_strategy_config::reconnect_mode::OFF);
        } else if(attr_val1 == "ASYNC"){
            strategy_config.set_reconnect_mode(hazelcast::client::config::client_connection_strategy_config::reconnect_mode::ASYNC);
        }
    }catch (const boost::exception& e){

    }
    for(auto& pair : node ){
        auto child = pair_to_node(pair.first, pair.second);
        std::string node_name = pair.first;
        if (matches("connection-retry", node_name)) {
            handle_connection_retry(child,strategy_config);
        }
    }
    client_config->set_connection_strategy_config(strategy_config);
}
void client_dom_config_processor::handle_connection_retry(const boost::property_tree::ptree& node,hazelcast::client::config::client_connection_strategy_config strategy_config){
    auto connection_retry_config = new hazelcast::client::config::connection_retry_config();
    for(auto& pair : node){
        auto child = pair_to_node(pair.first, pair.second);
        std::string node_name = pair.first;
        if (matches("initial-backoff-millis", node_name)) {
            connection_retry_config->set_initial_backoff_duration(std::chrono::milliseconds(get_integer_value(node_name, pair.second.data())));
        } else if (matches("max-backoff-millis", node_name)) {
            connection_retry_config->set_max_backoff_duration(std::chrono::milliseconds(get_integer_value(node_name, pair.second.data())));
        } else if (matches("multiplier", node_name)) {
            connection_retry_config->set_multiplier(get_double_value(node_name, pair.second.data()));
        } else if (matches("cluster-connect-timeout-millis", node_name)) {
            connection_retry_config->set_cluster_connect_timeout(std::chrono::milliseconds(get_long_value(node_name, pair.second.data())));
        } else if (matches("jitter", node_name)) {
            connection_retry_config->set_jitter(get_double_value(node_name, pair.second.data()));
        }
    }
    strategy_config.set_retry_config(*connection_retry_config);
}

void client_dom_config_processor::handle_labels(const boost::property_tree::ptree& node){
    for(auto& child : node){
        client_config->add_label(child.first);
    }
}
void client_dom_config_processor::handle_proxy_factories(const boost::property_tree::ptree& node){
    for(auto& pair : node){
        auto child = pair_to_node(pair.first, pair.second);
        handle_proxy_factories_node(child);
    }
}
void client_dom_config_processor::handle_proxy_factories_node(const boost::property_tree::ptree& node){
    for(auto& pair : node){
        auto child = pair_to_node(pair.first, pair.second);
        std::string node_name = pair.first;
        if(matches(node_name , "proxy-factory")){//not sure if it is supported

        }
    }
}
void client_dom_config_processor::handle_listeners(const boost::property_tree::ptree& node){
    for(auto& pair : node){
        auto child = pair_to_node(pair.first, pair.second);
        if(matches(pair.first, "listener")){//not sure how to add listeners

        }
    }
}
void client_dom_config_processor::handle_backup_ack_to_client(const boost::property_tree::ptree& node){
    client_config->backup_acks_enabled(get_bool_value(node.data()));
}
void client_dom_config_processor::handle_realms(const boost::property_tree::ptree& node){
    for (auto& pair: node) {
        auto child = pair_to_node(pair.first, pair.second);
        if (matches("realm", pair.first)) {
            handle_realm(child);
        }
    }
}
void client_dom_config_processor::handle_realm(const boost::property_tree::ptree& node){
    try{
        auto realm_name = get_attribute(node, "name");
    }
    catch(const boost::exception& e){

    }
    for (auto& pair : node) {
        auto child = pair_to_node(pair.first, pair.second);
        std::string node_name = pair.first;
        if (matches("authentication", node_name)) {//authentication not supported

        }
    }
}
void client_dom_config_processor::handle_load_balancer(const boost::property_tree::ptree& node){//not sure how to configure load_balancer
    /*
    std::string type = get_attribute(node,"type");
    if (matches("random", type)) {
        clientConfig.setLoadBalancer(new RandomLB());
    } else if (matches("round-robin", type)) {
        clientConfig.setLoadBalancer(new RoundRobinLB());
    } else if (matches("custom", type)) {
        String loadBalancerClassName = parseCustomLoadBalancerClassName(node);
        clientConfig.setLoadBalancerClassName(loadBalancerClassName);
    }*/
}



} // namespace config

client_config::client_config()
  : cluster_name_("dev")
  , redo_operation_(false)
  , socket_interceptor_()
  , executor_pool_size_(-1)
{}

client_config::client_config(client_config&& rhs) = default;

client_config&
client_config::operator=(client_config&& rhs) = default;

client_config&
client_config::set_redo_operation(bool redo_operation)
{
    this->redo_operation_ = redo_operation;
    return *this;
}

bool
client_config::is_redo_operation() const
{
    return redo_operation_;
}

load_balancer&
client_config::get_load_balancer()
{
    if (!load_balancer_) {
        auto index = std::make_shared<std::atomic<size_t>>(0);
        load_balancer_ = load_balancer().next([=](cluster& c) {
            auto members = c.get_members();
            if (members.empty()) {
                return boost::optional<member>();
            }
            auto i = index->fetch_add(1);
            return boost::make_optional(std::move(members[i % members.size()]));
        });
    }
    return *load_balancer_;
}

client_config&
client_config::set_load_balancer(load_balancer&& load_balancer)
{
    this->load_balancer_ = std::move(load_balancer);
    return *this;
}

config::logger_config&
client_config::get_logger_config()
{
    return logger_config_;
}

client_config&
client_config::add_listener(lifecycle_listener&& listener)
{
    lifecycle_listeners_.emplace_back(std::move(listener));
    return *this;
}

client_config&
client_config::add_listener(membership_listener&& listener)
{
    membership_listeners_.emplace_back(std::move(listener));
    return *this;
}

const std::vector<lifecycle_listener>&
client_config::get_lifecycle_listeners() const
{
    return lifecycle_listeners_;
}

const std::vector<membership_listener>&
client_config::get_membership_listeners() const
{
    return membership_listeners_;
}

client_config&
client_config::set_socket_interceptor(socket_interceptor&& interceptor)
{
    this->socket_interceptor_ = std::move(interceptor);
    return *this;
}

const socket_interceptor&
client_config::get_socket_interceptor() const
{
    return socket_interceptor_;
}

serialization_config&
client_config::get_serialization_config()
{
    return serialization_config_;
}

client_config&
client_config::set_serialization_config(
  serialization_config const& serialization_config)
{
    this->serialization_config_ = serialization_config;
    return *this;
}

const std::unordered_map<std::string, std::string>&
client_config::get_properties() const
{
    return properties_;
}

client_config&
client_config::set_property(const std::string& name, const std::string& value)
{
    properties_[name] = value;
    return *this;
}

client_config&
client_config::add_reliable_topic_config(
  const config::reliable_topic_config& reliable_topic_config)
{
    reliable_topic_config_map_[reliable_topic_config.get_name()] =
      reliable_topic_config;
    return *this;
}

const config::reliable_topic_config&
client_config::get_reliable_topic_config(const std::string& name)
{
    auto it = reliable_topic_config_map_.find(name);
    if (it != reliable_topic_config_map_.end()) {
        return it->second;
    }

    return reliable_topic_config_map_
      .emplace(name, config::reliable_topic_config{ name })
      .first->second;
}

const config::reliable_topic_config*
client_config::lookup_reliable_topic_config(const std::string& name) const
{
    auto it = reliable_topic_config_map_.find(name);
    if (it != reliable_topic_config_map_.end()) {
        return &it->second;
    }

    return nullptr;
}

config::client_network_config&
client_config::get_network_config()
{
    return network_config_;
}

client_config&
client_config::add_near_cache_config(
  const config::near_cache_config& near_cache_config)
{
    near_cache_config_map_.emplace(near_cache_config.get_name(),
                                   near_cache_config);
    return *this;
}

const config::near_cache_config*
client_config::get_near_cache_config(const std::string& name) const
{
    auto nearCacheConfig = internal::config::ConfigUtils::lookup_by_pattern(
      config_pattern_matcher_, near_cache_config_map_, name);
    if (nearCacheConfig) {
        return nearCacheConfig;
    }

    auto config_it = near_cache_config_map_.find("default");
    if (config_it != near_cache_config_map_.end()) {
        return &near_cache_config_map_.find("default")->second;
    }

    // not needed for c++ client since it is always native memory
    // initDefaultMaxSizeForOnHeapMaps(nearCacheConfig);
    return nullptr;
}

client_config&
client_config::set_network_config(
  const config::client_network_config& network_config)
{
    this->network_config_ = network_config;
    return *this;
}

const boost::optional<std::string>&
client_config::get_instance_name() const
{
    return instance_name_;
}

client_config&
client_config::set_instance_name(const std::string& instance_name)
{
    client_config::instance_name_ = instance_name;
    return *this;
}

int32_t
client_config::get_executor_pool_size() const
{
    return executor_pool_size_;
}

void
client_config::set_executor_pool_size(int32_t executor_pool_size)
{
    client_config::executor_pool_size_ = executor_pool_size;
}

config::client_connection_strategy_config&
client_config::get_connection_strategy_config()
{
    return connection_strategy_config_;
}

client_config&
client_config::set_connection_strategy_config(
  const config::client_connection_strategy_config& connection_strategy_config)
{
    client_config::connection_strategy_config_ = connection_strategy_config;
    return *this;
}

const config::client_flake_id_generator_config*
client_config::find_flake_id_generator_config(const std::string& name)
{
    std::string baseName =
      internal::partition::strategy::StringPartitioningStrategy::get_base_name(
        name);
    auto config = internal::config::ConfigUtils::lookup_by_pattern<
      config::client_flake_id_generator_config>(
      config_pattern_matcher_, flake_id_generator_config_map_, baseName);
    if (config) {
        return config;
    }
    return get_flake_id_generator_config("default");
}

const config::client_flake_id_generator_config*
client_config::get_flake_id_generator_config(const std::string& name)
{
    std::string baseName =
      internal::partition::strategy::StringPartitioningStrategy::get_base_name(
        name);
    auto config = internal::config::ConfigUtils::lookup_by_pattern<
      config::client_flake_id_generator_config>(
      config_pattern_matcher_, flake_id_generator_config_map_, baseName);
    if (config) {
        return config;
    }
    auto defConfig = flake_id_generator_config_map_.find("default");
    if (defConfig == flake_id_generator_config_map_.end()) {
        flake_id_generator_config_map_.emplace(
          "default", config::client_flake_id_generator_config("default"));
    }
    defConfig = flake_id_generator_config_map_.find("default");
    config::client_flake_id_generator_config new_config = defConfig->second;
    new_config.set_name(name);
    flake_id_generator_config_map_.emplace(name, std::move(new_config));
    return &flake_id_generator_config_map_.find(name)->second;
}

client_config&
client_config::add_flake_id_generator_config(
  const config::client_flake_id_generator_config& config)
{
    flake_id_generator_config_map_.emplace(config.get_name(), config);
    return *this;
}

const std::string&
client_config::get_cluster_name() const
{
    return cluster_name_;
}

client_config&
client_config::set_cluster_name(const std::string& cluster_name)
{
    cluster_name_ = cluster_name;
    return *this;
}

const std::unordered_set<std::string>&
client_config::get_labels() const
{
    return labels_;
}

client_config&
client_config::set_labels(const std::unordered_set<std::string>& labels)
{
    labels_ = labels;
    return *this;
}

client_config&
client_config::add_label(const std::string& label)
{
    labels_.insert(label);
    return *this;
}

client_config&
client_config::backup_acks_enabled(bool enabled)
{
    backup_acks_enabled_ = enabled;
    return *this;
}

bool
client_config::backup_acks_enabled()
{
    return backup_acks_enabled_;
}

const std::shared_ptr<security::credentials>&
client_config::get_credentials() const
{
    return credentials_;
}

client_config&
client_config::set_credentials(
  const std::shared_ptr<security::credentials>& credential)
{
    credentials_ = credential;
    return *this;
}

namespace security {
username_password_credentials::username_password_credentials(
  const std::string& name,
  const std::string& password)
  : credentials(name)
  , password_(password)
{}

const std::string&
username_password_credentials::password() const
{
    return password_;
}

const credentials::credential_type
username_password_credentials::type() const
{
    return credentials::credential_type::username_password;
}

const std::vector<byte>&
token_credentials::token() const
{
    return token_;
}

const credentials::credential_type
token_credentials::type() const
{
    return credentials::credential_type::token;
}

token_credentials::token_credentials(const std::vector<byte>& token)
  : credentials(token.empty() ? "<empty>" : "<token>")
  , token_(token)
{}

credentials::~credentials() {}

const std::string&
credentials::name() const
{
    return name_;
}

credentials::credentials(const std::string& name)
  : name_(name)
{}
} // namespace security
} // namespace client
} // namespace hazelcast
