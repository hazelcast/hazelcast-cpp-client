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

#include "hazelcast/client/client_config.h"
#include "hazelcast/client/SerializationConfig.h"
#include "hazelcast/client/config/ssl_config.h"
#include "hazelcast/util/Preconditions.h"
#include "hazelcast/client/config/ClientFlakeIdGeneratorConfig.h"
#include "hazelcast/client/exception/IllegalArgumentException.h"
#include "hazelcast/client/internal/partition/strategy/StringPartitioningStrategy.h"
#include "hazelcast/client/address.h"
#include "hazelcast/client/config/ClientNetworkConfig.h"
#include "hazelcast/client/config/client_aws_config.h"
#include "hazelcast/client/config/ReliableTopicConfig.h"
#include "hazelcast/client/config/ClientConnectionStrategyConfig.h"
#include "hazelcast/client/config/LoggerConfig.h"
#include "hazelcast/client/config/index_config.h"
#include "hazelcast/client/config/matcher/matching_point_config_pattern_matcher.h"
#include "hazelcast/client/query/Predicates.h"
#include "hazelcast/client/LifecycleListener.h"

namespace hazelcast {
    namespace client {
        SerializationConfig::SerializationConfig() : version_(0) {
        }

        int SerializationConfig::get_portable_version() const {
            return version_;
        }

        SerializationConfig &SerializationConfig::set_portable_version(int v) {
            this->version_ = v;
            return *this;
        }

        std::shared_ptr<serialization::global_serializer> SerializationConfig::get_global_serializer() const {
            return global_serializer_;
        }

        void SerializationConfig::set_global_serializer(
                const std::shared_ptr<serialization::global_serializer> &global_serializer) {
            global_serializer_ = global_serializer;
        }

        namespace config {
            ssl_config::ssl_config() : enabled_(false), ssl_protocol_(tlsv12) {
            }

            bool ssl_config::is_enabled() const {
                return enabled_;
            }

            ssl_config &ssl_config::set_enabled(bool is_enabled) {
                util::Preconditions::check_ssl("getAwsConfig");
                this->enabled_ = is_enabled;
                return *this;
            }

            ssl_config &ssl_config::set_protocol(ssl_protocol protocol) {
                this->ssl_protocol_ = protocol;
                return *this;
            }

            ssl_protocol ssl_config::get_protocol() const {
                return ssl_protocol_;
            }

            const std::vector<std::string> &ssl_config::get_verify_files() const {
                return client_verify_files_;
            }

            ssl_config &ssl_config::add_verify_file(const std::string &filename) {
                this->client_verify_files_.push_back(filename);
                return *this;
            }

            const std::string &ssl_config::get_cipher_list() const {
                return cipher_list_;
            }

            ssl_config &ssl_config::set_cipher_list(const std::string &ciphers) {
                this->cipher_list_ = ciphers;
                return *this;
            }

            constexpr int64_t ClientFlakeIdGeneratorConfig::DEFAULT_PREFETCH_VALIDITY_MILLIS;

            ClientFlakeIdGeneratorConfig::ClientFlakeIdGeneratorConfig(const std::string &name)
                    : name_(name), prefetch_count_(ClientFlakeIdGeneratorConfig::DEFAULT_PREFETCH_COUNT),
                      prefetch_validity_duration_(ClientFlakeIdGeneratorConfig::DEFAULT_PREFETCH_VALIDITY_MILLIS) {}

            const std::string &ClientFlakeIdGeneratorConfig::get_name() const {
                return name_;
            }

            ClientFlakeIdGeneratorConfig &ClientFlakeIdGeneratorConfig::set_name(const std::string &n) {
                ClientFlakeIdGeneratorConfig::name_ = n;
                return *this;
            }

            int32_t ClientFlakeIdGeneratorConfig::get_prefetch_count() const {
                return prefetch_count_;
            }

            ClientFlakeIdGeneratorConfig &ClientFlakeIdGeneratorConfig::set_prefetch_count(int32_t count) {
                std::ostringstream out;
                out << "prefetch-count must be 1.." << MAXIMUM_PREFETCH_COUNT << ", not " << count;
                util::Preconditions::check_true(count > 0 && count <= MAXIMUM_PREFETCH_COUNT, out.str());
                prefetch_count_ = count;
                return *this;
            }

            std::chrono::milliseconds ClientFlakeIdGeneratorConfig::get_prefetch_validity_duration() const {
                return prefetch_validity_duration_;
            }

            ClientFlakeIdGeneratorConfig &
            ClientFlakeIdGeneratorConfig::set_prefetch_validity_duration(std::chrono::milliseconds duration) {
                util::Preconditions::check_not_negative(duration.count(),
                                                      "prefetchValidityMs must be non negative");
                prefetch_validity_duration_ = duration;
                return *this;
            }

            int32_t ClientNetworkConfig::CONNECTION_ATTEMPT_PERIOD = 3000;

            ClientNetworkConfig::ClientNetworkConfig()
                    : connection_timeout_(5000), smart_routing_(true), connection_attempt_limit_(-1),
                      connection_attempt_period_(CONNECTION_ATTEMPT_PERIOD) {}

            ssl_config &ClientNetworkConfig::get_ssl_config() {
                return ssl_config_;
            }

            ClientNetworkConfig &ClientNetworkConfig::set_ssl_config(const config::ssl_config &config) {
                ssl_config_ = config;
                return *this;
            }

            std::chrono::milliseconds ClientNetworkConfig::get_connection_timeout() const {
                return connection_timeout_;
            }

            ClientNetworkConfig &ClientNetworkConfig::set_aws_config(const client_aws_config &client_aws_config) {
                this->client_aws_config_ = client_aws_config;
                return *this;
            }

            client_aws_config &ClientNetworkConfig::get_aws_config() {
                return client_aws_config_;
            }

            bool ClientNetworkConfig::is_smart_routing() const {
                return smart_routing_;
            }

            ClientNetworkConfig &ClientNetworkConfig::set_smart_routing(bool smart_routing) {
                ClientNetworkConfig::smart_routing_ = smart_routing;
                return *this;
            }

            int32_t ClientNetworkConfig::get_connection_attempt_limit() const {
                return connection_attempt_limit_;
            }

            ClientNetworkConfig &ClientNetworkConfig::set_connection_attempt_limit(int32_t connection_attempt_limit) {
                if (connection_attempt_limit < 0) {
                    BOOST_THROW_EXCEPTION(
                            exception::IllegalArgumentException("ClientNetworkConfig::setConnectionAttemptLimit",
                                                                "connectionAttemptLimit cannot be negative"));
                }
                this->connection_attempt_limit_ = connection_attempt_limit;
                return *this;
            }

            std::chrono::milliseconds ClientNetworkConfig::get_connection_attempt_period() const {
                return connection_attempt_period_;
            }

            std::vector<address> ClientNetworkConfig::get_addresses() const {
                return address_list_;
            }

            ClientNetworkConfig &ClientNetworkConfig::add_addresses(const std::vector<address> &addresses) {
                address_list_.insert(address_list_.end(), addresses.begin(), addresses.end());
                return *this;
            }

            ClientNetworkConfig &ClientNetworkConfig::set_addresses(const std::vector<address> &addresses) {
                address_list_ = addresses;
                return *this;
            }

            ClientNetworkConfig &ClientNetworkConfig::add_address(const address &address) {
                address_list_.push_back(address);
                return *this;
            }

            SocketOptions &ClientNetworkConfig::get_socket_options() {
                return socket_options_;
            }

            ClientNetworkConfig &ClientNetworkConfig::set_connection_timeout(const std::chrono::milliseconds &timeout) {
                connection_timeout_ = timeout;
                return *this;
            }

            ClientNetworkConfig &
            ClientNetworkConfig::set_connection_attempt_period(const std::chrono::milliseconds &interval) {
                util::Preconditions::check_not_negative(interval.count(), (boost::format(
                        "Provided connectionAttemptPeriod(%1% msecs) cannot be negative") % interval.count()).str());
                connection_attempt_period_ = interval;
                return *this;
            }

            ClientConnectionStrategyConfig::ClientConnectionStrategyConfig() : async_start_(false), reconnect_mode_(ON) {
            }

            ClientConnectionStrategyConfig::reconnect_mode ClientConnectionStrategyConfig::get_reconnect_mode() const {
                return reconnect_mode_;
            }

            bool ClientConnectionStrategyConfig::is_async_start() const {
                return async_start_;
            }

            ClientConnectionStrategyConfig &ClientConnectionStrategyConfig::set_async_start(bool async_start) {
                this->async_start_ = async_start;
                return *this;
            }

            ClientConnectionStrategyConfig &
            ClientConnectionStrategyConfig::set_reconnect_mode(reconnect_mode reconnect_mode) {
                this->reconnect_mode_ = reconnect_mode;
                return *this;
            }

            const int ReliableTopicConfig::DEFAULT_READ_BATCH_SIZE = 10;

            ReliableTopicConfig::ReliableTopicConfig() = default;

            ReliableTopicConfig::ReliableTopicConfig(const char *topic_name) : read_batch_size_(DEFAULT_READ_BATCH_SIZE),
                                                                              name_(topic_name) {
            }

            const std::string &ReliableTopicConfig::get_name() const {
                return name_;
            }

            int ReliableTopicConfig::get_read_batch_size() const {
                return read_batch_size_;
            }

            ReliableTopicConfig &ReliableTopicConfig::set_read_batch_size(int batch_size) {
                if (batch_size <= 0) {
                    BOOST_THROW_EXCEPTION(exception::IllegalArgumentException("ReliableTopicConfig::setReadBatchSize",
                                                                              "readBatchSize should be positive"));
                }

                this->read_batch_size_ = batch_size;

                return *this;
            }

            SocketOptions::SocketOptions() : tcp_no_delay_(true), keep_alive_(true), reuse_address_(true), linger_seconds_(3),
                                             buffer_size_(DEFAULT_BUFFER_SIZE_BYTE) {}

            bool SocketOptions::is_tcp_no_delay() const {
                return tcp_no_delay_;
            }

            SocketOptions &SocketOptions::set_tcp_no_delay(bool tcp_no_delay) {
                SocketOptions::tcp_no_delay_ = tcp_no_delay;
                return *this;
            }

            bool SocketOptions::is_keep_alive() const {
                return keep_alive_;
            }

            SocketOptions &SocketOptions::set_keep_alive(bool keep_alive) {
                SocketOptions::keep_alive_ = keep_alive;
                return *this;
            }

            bool SocketOptions::is_reuse_address() const {
                return reuse_address_;
            }

            SocketOptions &SocketOptions::set_reuse_address(bool reuse_address) {
                SocketOptions::reuse_address_ = reuse_address;
                return *this;
            }

            int SocketOptions::get_linger_seconds() const {
                return linger_seconds_;
            }

            SocketOptions &SocketOptions::set_linger_seconds(int linger_seconds) {
                SocketOptions::linger_seconds_ = linger_seconds;
                return *this;
            }

            int SocketOptions::get_buffer_size_in_bytes() const {
                return buffer_size_;
            }

            SocketOptions &SocketOptions::set_buffer_size_in_bytes(int buffer_size) {
                SocketOptions::buffer_size_ = buffer_size;
                return *this;
            }

            client_aws_config::client_aws_config() : enabled_(false), region_("us-east-1"), host_header_("ec2.amazonaws.com"),
                                                     inside_aws_(false) {
            }

            const std::string &client_aws_config::get_access_key() const {
                return access_key_;
            }

            client_aws_config &client_aws_config::set_access_key(const std::string &access_key) {
                this->access_key_ = util::Preconditions::check_has_text(access_key, "accessKey must contain text");
                return *this;
            }

            const std::string &client_aws_config::get_secret_key() const {
                return secret_key_;
            }

            client_aws_config &client_aws_config::set_secret_key(const std::string &secret_key) {
                this->secret_key_ = util::Preconditions::check_has_text(secret_key, "secretKey must contain text");
                return *this;
            }

            const std::string &client_aws_config::get_region() const {
                return region_;
            }

            client_aws_config &client_aws_config::set_region(const std::string &region) {
                this->region_ = util::Preconditions::check_has_text(region, "region must contain text");
                return *this;
            }

            const std::string &client_aws_config::get_host_header() const {
                return host_header_;
            }

            client_aws_config &client_aws_config::set_host_header(const std::string &host_header) {
                this->host_header_ = util::Preconditions::check_has_text(host_header, "hostHeader must contain text");
                return *this;
            }

            client_aws_config &client_aws_config::set_enabled(bool enabled) {
                util::Preconditions::check_ssl("getAwsConfig");
                this->enabled_ = enabled;
                return *this;
            }

            bool client_aws_config::is_enabled() const {
                return enabled_;
            }

            client_aws_config &client_aws_config::set_security_group_name(const std::string &security_group_name) {
                this->security_group_name_ = security_group_name;
                return *this;
            }

            const std::string &client_aws_config::get_security_group_name() const {
                return security_group_name_;
            }

            client_aws_config &client_aws_config::set_tag_key(const std::string &tag_key) {
                this->tag_key_ = tag_key;
                return *this;
            }

            const std::string &client_aws_config::get_tag_key() const {
                return tag_key_;
            }

            client_aws_config &client_aws_config::set_tag_value(const std::string &tag_value) {
                this->tag_value_ = tag_value;
                return *this;
            }

            const std::string &client_aws_config::get_tag_value() const {
                return tag_value_;
            }

            const std::string &client_aws_config::get_iam_role() const {
                return iam_role_;
            }

            client_aws_config &client_aws_config::set_iam_role(const std::string &iam_role) {
                this->iam_role_ = iam_role;
                return *this;
            }

            bool client_aws_config::is_inside_aws() const {
                return inside_aws_;
            }

            client_aws_config &client_aws_config::set_inside_aws(bool inside_aws) {
                this->inside_aws_ = inside_aws;
                return *this;
            }

            std::ostream &operator<<(std::ostream &out, const client_aws_config &config) {
                return out << "ClientAwsConfig{"
                           << "enabled=" << config.is_enabled()
                           << ", region='" << config.get_region() << '\''
                           << ", securityGroupName='" << config.get_security_group_name() << '\''
                           << ", tagKey='" << config.get_tag_key() << '\''
                           << ", tagValue='" << config.get_tag_value() << '\''
                           << ", hostHeader='" << config.get_host_header() << '\''
                           << ", iamRole='" << config.get_iam_role() << "\'}";
            }

            namespace matcher {
                std::shared_ptr<std::string>
                matching_point_config_pattern_matcher::matches(const std::vector<std::string> &config_patterns,
                                                               const std::string &item_name) const {
                    std::shared_ptr<std::string> candidate;
                    std::shared_ptr<std::string> duplicate;
                    int lastMatchingPoint = -1;
                    for (const std::string &pattern  : config_patterns) {
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
                        throw (exception::ExceptionBuilder<exception::InvalidConfigurationException>(
                                "MatchingPointConfigPatternMatcher::matches") << "Configuration " << item_name
                                                                              << " has duplicate configuration. Candidate:"
                                                                              << *candidate << ", duplicate:"
                                                                              << *duplicate).build();
                    }
                    return candidate;
                }

                int matching_point_config_pattern_matcher::get_matching_point(const std::string &pattern,
                                                                              const std::string &item_name) const {
                    size_t index = pattern.find('*');
                    if (index == std::string::npos) {
                        return -1;
                    }

                    std::string firstPart = pattern.substr(0, index);
                    if (item_name.find(firstPart) != 0) {
                        return -1;
                    }

                    std::string secondPart = pattern.substr(index + 1);
                    if (item_name.rfind(secondPart) != (item_name.length() - secondPart.length())) {
                        return -1;
                    }

                    return (int) (firstPart.length() + secondPart.length());
                }
            }

            const std::string index_config::bitmap_index_options::DEFAULT_KEY = query::QueryConstants::KEY_ATTRIBUTE_NAME;
            const index_config::bitmap_index_options::unique_key_transformation index_config::bitmap_index_options::DEFAULT_TRANSFORMATION = index_config::bitmap_index_options::unique_key_transformation::OBJECT;

            index_config::bitmap_index_options::bitmap_index_options() : key(DEFAULT_KEY),
                                                                         transformation(DEFAULT_TRANSFORMATION) {}

            const index_config::index_type index_config::DEFAULT_TYPE = index_config::index_type::SORTED;

            index_config::index_config() : type(DEFAULT_TYPE) {}

            index_config::index_config(index_config::index_type type) : type(type) {}

            void index_config::add_attributes() {}

            EvictionConfig::EvictionConfig() : size_(DEFAULT_MAX_ENTRY_COUNT), max_size_policy_(DEFAULT_MAX_SIZE_POLICY),
                               eviction_policy_(DEFAULT_EVICTION_POLICY) {}

            int32_t EvictionConfig::get_size() const {
                return size_;
            }

            EvictionConfig &EvictionConfig::set_size(int32_t size) {
                this->size_ = util::Preconditions::check_positive(size, "Size must be positive number!");
                return *this;
            }

            EvictionConfig::max_size_policy EvictionConfig::get_maximum_size_policy() const {
                return max_size_policy_;
            }

            EvictionConfig &EvictionConfig::set_maximum_size_policy(const EvictionConfig::max_size_policy &max_size_policy) {
                this->max_size_policy_ = max_size_policy;
                return *this;
            }

            eviction_policy EvictionConfig::get_eviction_policy() const {
                return eviction_policy_;
            }

            EvictionConfig &EvictionConfig::set_eviction_policy(eviction_policy policy) {
                this->eviction_policy_ = policy;
                return *this;
            }

            internal::eviction::EvictionStrategyType::type EvictionConfig::get_eviction_strategy_type() const {
                // TODO: add support for other/custom eviction strategies
                return internal::eviction::EvictionStrategyType::DEFAULT_EVICTION_STRATEGY;
            }

            internal::eviction::eviction_policy_type EvictionConfig::get_eviction_policy_type() const {
                if (eviction_policy_ == LFU) {
                    return internal::eviction::LFU;
                } else if (eviction_policy_ == LRU) {
                    return internal::eviction::LRU;
                } else if (eviction_policy_ == RANDOM) {
                    return internal::eviction::RANDOM;
                } else if (eviction_policy_ == NONE) {
                    return internal::eviction::NONE;
                } else {
                    assert(0);
                }
                return internal::eviction::NONE;
            }

            std::ostream &operator<<(std::ostream &out, const EvictionConfig &config) {
                out << "EvictionConfig{"
                    << "size=" << config.get_size()
                    << ", maxSizePolicy=" << config.get_maximum_size_policy()
                    << ", evictionPolicy=" << config.get_eviction_policy()
                    << '}';

                return out;
            }

            NearCacheConfig::NearCacheConfig() : name_("default"), time_to_live_seconds_(DEFAULT_TTL_SECONDS),
                                                 max_idle_seconds_(DEFAULT_MAX_IDLE_SECONDS),
                                                 in_memory_format_(DEFAULT_MEMORY_FORMAT),
                                                 local_update_policy_(INVALIDATE), invalidate_on_change_(true),
                                                 cache_local_entries_(false) {
            }

            NearCacheConfig::NearCacheConfig(const std::string &cache_name) : NearCacheConfig() {
                name_ = cache_name;
            }

            NearCacheConfig::NearCacheConfig(const std::string &cache_name, in_memory_format memory_format)
                    : NearCacheConfig(name_) {
                this->in_memory_format_ = memory_format;
            }

            NearCacheConfig::NearCacheConfig(int32_t time_to_live_seconds, int32_t max_idle_seconds, bool invalidate_on_change,
                                             in_memory_format in_memory_format, const EvictionConfig &evict_config)
                    : NearCacheConfig(name_, in_memory_format) {
                this->time_to_live_seconds_ = time_to_live_seconds;
                this->max_idle_seconds_ = max_idle_seconds;
                this->invalidate_on_change_ = invalidate_on_change;
                this->eviction_config_ = evict_config;
            }

            const std::string &NearCacheConfig::get_name() const {
                return name_;
            }

            NearCacheConfig &NearCacheConfig::set_name(const std::string &name) {
                this->name_ = name;
                return *this;
            }

            int32_t NearCacheConfig::get_time_to_live_seconds() const {
                return time_to_live_seconds_;
            }

            NearCacheConfig &NearCacheConfig::set_time_to_live_seconds(int32_t time_to_live_seconds) {
                this->time_to_live_seconds_ = util::Preconditions::check_not_negative(time_to_live_seconds,
                                                                                "TTL seconds cannot be negative!");
                return *this;
            }

            int32_t NearCacheConfig::get_max_idle_seconds() const {
                return max_idle_seconds_;
            }

            NearCacheConfig &NearCacheConfig::set_max_idle_seconds(int32_t max_idle_seconds) {
                this->max_idle_seconds_ = util::Preconditions::check_not_negative(max_idle_seconds,
                                                                             "Max-Idle seconds cannot be negative!");
                return *this;
            }

            bool NearCacheConfig::is_invalidate_on_change() const {
                return invalidate_on_change_;
            }

            NearCacheConfig &NearCacheConfig::set_invalidate_on_change(bool invalidate_on_change) {
                this->invalidate_on_change_ = invalidate_on_change;
                return *this;
            }

            const in_memory_format &NearCacheConfig::get_in_memory_format() const {
                return in_memory_format_;
            }

            NearCacheConfig &NearCacheConfig::set_in_memory_format(const in_memory_format &in_memory_format) {
                this->in_memory_format_ = in_memory_format;
                return *this;
            }

            bool NearCacheConfig::is_cache_local_entries() const {
                return cache_local_entries_;
            }

            NearCacheConfig &NearCacheConfig::set_cache_local_entries(bool cache_local_entries) {
                this->cache_local_entries_ = cache_local_entries;
                return *this;
            }

            const NearCacheConfig::local_update_policy &NearCacheConfig::get_local_update_policy() const {
                return local_update_policy_;
            }

            NearCacheConfig &NearCacheConfig::set_local_update_policy(const local_update_policy &local_update_policy) {
                this->local_update_policy_ = local_update_policy;
                return *this;
            }

            EvictionConfig &NearCacheConfig::get_eviction_config() {
                return eviction_config_;
            }

            NearCacheConfig &NearCacheConfig::set_eviction_config(const EvictionConfig &eviction_config) {
                this->eviction_config_ = eviction_config;
                return *this;
            }

            int32_t NearCacheConfig::calculate_max_size(int32_t max_size) {
                return (max_size == 0) ? INT32_MAX : util::Preconditions::check_not_negative(max_size,
                                                                                          "Max-size cannot be negative!");
            }

            std::ostream &operator<<(std::ostream &out, const NearCacheConfig &config) {
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
        }

        client_config::client_config() : cluster_name_("dev"), load_balancer_(NULL), redo_operation_(false),
                                       socket_interceptor_(), executor_pool_size_(-1) {}

        client_config &client_config::set_redo_operation(bool redo_operation) {
            this->redo_operation_ = redo_operation;
            return *this;
        }

        bool client_config::is_redo_operation() const {
            return redo_operation_;
        }

        LoadBalancer *const client_config::get_load_balancer() {
            if (!load_balancer_)
                return &default_load_balancer_;
            return load_balancer_;
        }

        client_config &client_config::set_load_balancer(LoadBalancer *load_balancer) {
            this->load_balancer_ = load_balancer;
            return *this;
        }

        config::LoggerConfig &client_config::get_logger_config() {
            return logger_config_;
        }

        client_config &client_config::add_listener(LifecycleListener &&listener) {
            lifecycle_listeners_.emplace_back(std::move(listener));
            return *this;
        }

        client_config &client_config::add_listener(MembershipListener &&listener) {
            membership_listeners_.emplace_back(std::move(listener));
            return *this;
        }

        const std::vector<LifecycleListener> &client_config::get_lifecycle_listeners() const {
            return lifecycle_listeners_;
        }

        const std::vector<MembershipListener> &client_config::get_membership_listeners() const {
            return membership_listeners_;
        }

        client_config &client_config::set_socket_interceptor(SocketInterceptor &&interceptor) {
            this->socket_interceptor_ = std::move(interceptor);
            return *this;
        }

        const SocketInterceptor &client_config::get_socket_interceptor() const {
            return socket_interceptor_;
        }

        SerializationConfig &client_config::get_serialization_config() {
            return serialization_config_;
        }

        client_config &client_config::set_serialization_config(SerializationConfig const &serialization_config) {
            this->serialization_config_ = serialization_config;
            return *this;
        }

        const std::unordered_map<std::string, std::string> &client_config::get_properties() const {
            return properties_;
        }

        client_config &client_config::set_property(const std::string &name, const std::string &value) {
            properties_[name] = value;
            return *this;
        }

        client_config &client_config::add_reliable_topic_config(const config::ReliableTopicConfig &reliable_topic_config) {
            reliable_topic_config_map_[reliable_topic_config.get_name()] = reliable_topic_config;
            return *this;
        }

        const config::ReliableTopicConfig &client_config::get_reliable_topic_config(const std::string &name) {
            auto it = reliable_topic_config_map_.find(name);
            if (reliable_topic_config_map_.end() == it) {
                reliable_topic_config_map_[name] = config::ReliableTopicConfig(name.c_str());
            }

            return reliable_topic_config_map_[name];
        }

        config::ClientNetworkConfig &client_config::get_network_config() {
            return network_config_;
        }

        client_config &client_config::add_near_cache_config(const config::NearCacheConfig &near_cache_config) {
            near_cache_config_map_.emplace(near_cache_config.get_name(), near_cache_config);
            return *this;
        }

        const config::NearCacheConfig *client_config::get_near_cache_config(const std::string &name) const {
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
            //initDefaultMaxSizeForOnHeapMaps(nearCacheConfig);
            return nullptr;
        }

        client_config &client_config::set_network_config(const config::ClientNetworkConfig &network_config) {
            this->network_config_ = network_config;
            return *this;
        }

        const std::shared_ptr<std::string> &client_config::get_instance_name() const {
            return instance_name_;
        }

        void client_config::set_instance_name(const std::shared_ptr<std::string> &instance_name) {
            client_config::instance_name_ = instance_name;
        }

        int32_t client_config::get_executor_pool_size() const {
            return executor_pool_size_;
        }

        void client_config::set_executor_pool_size(int32_t executor_pool_size) {
            client_config::executor_pool_size_ = executor_pool_size;
        }

        config::ClientConnectionStrategyConfig &client_config::get_connection_strategy_config() {
            return connection_strategy_config_;
        }

        client_config &client_config::set_connection_strategy_config(
                const config::ClientConnectionStrategyConfig &connection_strategy_config) {
            client_config::connection_strategy_config_ = connection_strategy_config;
            return *this;
        }

        const config::ClientFlakeIdGeneratorConfig *
        client_config::find_flake_id_generator_config(const std::string &name) {
            std::string baseName = internal::partition::strategy::StringPartitioningStrategy::get_base_name(name);
            auto config = internal::config::ConfigUtils::lookup_by_pattern<config::ClientFlakeIdGeneratorConfig>(
                    config_pattern_matcher_, flake_id_generator_config_map_, baseName);
            if (config) {
                return config;
            }
            return get_flake_id_generator_config("default");
        }


        const config::ClientFlakeIdGeneratorConfig *
        client_config::get_flake_id_generator_config(const std::string &name) {
            std::string baseName = internal::partition::strategy::StringPartitioningStrategy::get_base_name(name);
            auto config = internal::config::ConfigUtils::lookup_by_pattern<config::ClientFlakeIdGeneratorConfig>(
                    config_pattern_matcher_, flake_id_generator_config_map_, baseName);
            if (config) {
                return config;
            }
            auto defConfig = flake_id_generator_config_map_.find("default");
            if (defConfig == flake_id_generator_config_map_.end()) {
                flake_id_generator_config_map_.emplace("default", config::ClientFlakeIdGeneratorConfig("default"));
            }
            defConfig = flake_id_generator_config_map_.find("default");
            config::ClientFlakeIdGeneratorConfig new_config = defConfig->second;
            new_config.set_name(name);
            flake_id_generator_config_map_.emplace(name, std::move(new_config));
            return &flake_id_generator_config_map_.find(name)->second;
        }

        client_config &
        client_config::add_flake_id_generator_config(const config::ClientFlakeIdGeneratorConfig &config) {
            flake_id_generator_config_map_.emplace(config.get_name(), config);
            return *this;
        }

        const std::string &client_config::get_cluster_name() const {
            return cluster_name_;
        }

        client_config &client_config::set_cluster_name(const std::string &cluster_name) {
            cluster_name_ = cluster_name;
            return *this;
        }

        const std::unordered_set<std::string> &client_config::get_labels() const {
            return labels_;
        }

        client_config &client_config::set_labels(const std::unordered_set<std::string> &labels) {
            labels_ = labels;
            return *this;
        }

        client_config &client_config::add_label(const std::string &label) {
            labels_.insert(label);
            return *this;
        }

        client_config &client_config::backup_acks_enabled(bool enabled) {
            backup_acks_enabled_ = enabled;
            return *this;
        }

        bool client_config::backup_acks_enabled() {
            return backup_acks_enabled_;
        }

        const std::shared_ptr<security::credentials> &client_config::get_credentials() const {
            return credentials_;
        }

        client_config &client_config::set_credentials(const std::shared_ptr<security::credentials> &credential) {
            credentials_ = credential;
            return *this;
        }

        namespace security {
            username_password_credentials::username_password_credentials(const std::string &name,
                                                                         const std::string &password) : credentials(name),
                                                                                                        password_(password) {}

            const std::string &username_password_credentials::password() const {
                return password_;
            }

            const credentials::credential_type username_password_credentials::type() const {
                return credentials::credential_type::username_password;
            }

            const std::vector<byte> &token_credentials::token() const {
                return token_;
            }

            const credentials::credential_type token_credentials::type() const {
                return credentials::credential_type::token;
            }

            token_credentials::token_credentials(const std::vector<byte> &token) : credentials(
                    token.empty() ? "<empty>" : "<token>"), token_(token) {}

            credentials::~credentials() {}

            const std::string &credentials::name() const {
                return name_;
            }

            credentials::credentials(const std::string &name) : name_(name) {}
        }
    }
}
