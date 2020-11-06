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

#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/SerializationConfig.h"
#include "hazelcast/client/config/SSLConfig.h"
#include "hazelcast/util/Preconditions.h"
#include "hazelcast/client/config/ClientFlakeIdGeneratorConfig.h"
#include "hazelcast/client/exception/IllegalArgumentException.h"
#include "hazelcast/client/internal/partition/strategy/StringPartitioningStrategy.h"
#include "hazelcast/client/Address.h"
#include "hazelcast/client/config/ClientNetworkConfig.h"
#include "hazelcast/client/config/ClientAwsConfig.h"
#include "hazelcast/client/config/ReliableTopicConfig.h"
#include "hazelcast/client/config/ClientConnectionStrategyConfig.h"
#include "hazelcast/client/config/LoggerConfig.h"
#include "hazelcast/client/config/index_config.h"
#include "hazelcast/client/config/matcher/MatchingPointConfigPatternMatcher.h"
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
            return globalSerializer_;
        }

        void SerializationConfig::set_global_serializer(
                const std::shared_ptr<serialization::global_serializer> &global_serializer) {
            globalSerializer_ = global_serializer;
        }

        namespace config {
            SSLConfig::SSLConfig() : enabled_(false), sslProtocol_(tlsv12) {
            }

            bool SSLConfig::is_enabled() const {
                return enabled_;
            }

            SSLConfig &SSLConfig::set_enabled(bool is_enabled) {
                util::Preconditions::check_ssl("getAwsConfig");
                this->enabled_ = is_enabled;
                return *this;
            }

            SSLConfig &SSLConfig::set_protocol(SSLProtocol protocol) {
                this->sslProtocol_ = protocol;
                return *this;
            }

            SSLProtocol SSLConfig::get_protocol() const {
                return sslProtocol_;
            }

            const std::vector<std::string> &SSLConfig::get_verify_files() const {
                return clientVerifyFiles_;
            }

            SSLConfig &SSLConfig::add_verify_file(const std::string &filename) {
                this->clientVerifyFiles_.push_back(filename);
                return *this;
            }

            const std::string &SSLConfig::get_cipher_list() const {
                return cipherList_;
            }

            SSLConfig &SSLConfig::set_cipher_list(const std::string &ciphers) {
                this->cipherList_ = ciphers;
                return *this;
            }

            constexpr int64_t ClientFlakeIdGeneratorConfig::DEFAULT_PREFETCH_VALIDITY_MILLIS;

            ClientFlakeIdGeneratorConfig::ClientFlakeIdGeneratorConfig(const std::string &name)
                    : name_(name), prefetchCount_(ClientFlakeIdGeneratorConfig::DEFAULT_PREFETCH_COUNT),
                      prefetchValidityDuration_(ClientFlakeIdGeneratorConfig::DEFAULT_PREFETCH_VALIDITY_MILLIS) {}

            const std::string &ClientFlakeIdGeneratorConfig::get_name() const {
                return name_;
            }

            ClientFlakeIdGeneratorConfig &ClientFlakeIdGeneratorConfig::set_name(const std::string &n) {
                ClientFlakeIdGeneratorConfig::name_ = n;
                return *this;
            }

            int32_t ClientFlakeIdGeneratorConfig::get_prefetch_count() const {
                return prefetchCount_;
            }

            ClientFlakeIdGeneratorConfig &ClientFlakeIdGeneratorConfig::set_prefetch_count(int32_t count) {
                std::ostringstream out;
                out << "prefetch-count must be 1.." << MAXIMUM_PREFETCH_COUNT << ", not " << count;
                util::Preconditions::check_true(count > 0 && count <= MAXIMUM_PREFETCH_COUNT, out.str());
                prefetchCount_ = count;
                return *this;
            }

            std::chrono::milliseconds ClientFlakeIdGeneratorConfig::get_prefetch_validity_duration() const {
                return prefetchValidityDuration_;
            }

            ClientFlakeIdGeneratorConfig &
            ClientFlakeIdGeneratorConfig::set_prefetch_validity_duration(std::chrono::milliseconds duration) {
                util::Preconditions::check_not_negative(duration.count(),
                                                      "prefetchValidityMs must be non negative");
                prefetchValidityDuration_ = duration;
                return *this;
            }

            int32_t ClientNetworkConfig::CONNECTION_ATTEMPT_PERIOD = 3000;

            ClientNetworkConfig::ClientNetworkConfig()
                    : connectionTimeout_(5000), smartRouting_(true), connectionAttemptLimit_(-1),
                      connectionAttemptPeriod_(CONNECTION_ATTEMPT_PERIOD) {}

            SSLConfig &ClientNetworkConfig::get_ssl_config() {
                return sslConfig_;
            }

            ClientNetworkConfig &ClientNetworkConfig::set_ssl_config(const config::SSLConfig &config) {
                sslConfig_ = config;
                return *this;
            }

            std::chrono::milliseconds ClientNetworkConfig::get_connection_timeout() const {
                return connectionTimeout_;
            }

            ClientNetworkConfig &ClientNetworkConfig::set_aws_config(const ClientAwsConfig &client_aws_config) {
                this->clientAwsConfig_ = client_aws_config;
                return *this;
            }

            ClientAwsConfig &ClientNetworkConfig::get_aws_config() {
                return clientAwsConfig_;
            }

            bool ClientNetworkConfig::is_smart_routing() const {
                return smartRouting_;
            }

            ClientNetworkConfig &ClientNetworkConfig::set_smart_routing(bool smart_routing) {
                ClientNetworkConfig::smartRouting_ = smart_routing;
                return *this;
            }

            int32_t ClientNetworkConfig::get_connection_attempt_limit() const {
                return connectionAttemptLimit_;
            }

            ClientNetworkConfig &ClientNetworkConfig::set_connection_attempt_limit(int32_t connection_attempt_limit) {
                if (connection_attempt_limit < 0) {
                    BOOST_THROW_EXCEPTION(
                            exception::IllegalArgumentException("ClientNetworkConfig::setConnectionAttemptLimit",
                                                                "connectionAttemptLimit cannot be negative"));
                }
                this->connectionAttemptLimit_ = connection_attempt_limit;
                return *this;
            }

            std::chrono::milliseconds ClientNetworkConfig::get_connection_attempt_period() const {
                return connectionAttemptPeriod_;
            }

            std::vector<Address> ClientNetworkConfig::get_addresses() const {
                return addressList_;
            }

            ClientNetworkConfig &ClientNetworkConfig::add_addresses(const std::vector<Address> &addresses) {
                addressList_.insert(addressList_.end(), addresses.begin(), addresses.end());
                return *this;
            }

            ClientNetworkConfig &ClientNetworkConfig::set_addresses(const std::vector<Address> &addresses) {
                addressList_ = addresses;
                return *this;
            }

            ClientNetworkConfig &ClientNetworkConfig::add_address(const Address &address) {
                addressList_.push_back(address);
                return *this;
            }

            SocketOptions &ClientNetworkConfig::get_socket_options() {
                return socketOptions_;
            }

            ClientNetworkConfig &ClientNetworkConfig::set_connection_timeout(const std::chrono::milliseconds &timeout) {
                connectionTimeout_ = timeout;
                return *this;
            }

            ClientNetworkConfig &
            ClientNetworkConfig::set_connection_attempt_period(const std::chrono::milliseconds &interval) {
                util::Preconditions::check_not_negative(interval.count(), (boost::format(
                        "Provided connectionAttemptPeriod(%1% msecs) cannot be negative") % interval.count()).str());
                connectionAttemptPeriod_ = interval;
                return *this;
            }

            ClientConnectionStrategyConfig::ClientConnectionStrategyConfig() : asyncStart_(false), reconnectMode_(ON) {
            }

            ClientConnectionStrategyConfig::ReconnectMode ClientConnectionStrategyConfig::get_reconnect_mode() const {
                return reconnectMode_;
            }

            bool ClientConnectionStrategyConfig::is_async_start() const {
                return asyncStart_;
            }

            ClientConnectionStrategyConfig &ClientConnectionStrategyConfig::set_async_start(bool async_start) {
                this->asyncStart_ = async_start;
                return *this;
            }

            ClientConnectionStrategyConfig &
            ClientConnectionStrategyConfig::set_reconnect_mode(ReconnectMode reconnect_mode) {
                this->reconnectMode_ = reconnect_mode;
                return *this;
            }

            const int ReliableTopicConfig::DEFAULT_READ_BATCH_SIZE = 10;

            ReliableTopicConfig::ReliableTopicConfig() = default;

            ReliableTopicConfig::ReliableTopicConfig(const char *topic_name) : readBatchSize_(DEFAULT_READ_BATCH_SIZE),
                                                                              name_(topic_name) {
            }

            const std::string &ReliableTopicConfig::get_name() const {
                return name_;
            }

            int ReliableTopicConfig::get_read_batch_size() const {
                return readBatchSize_;
            }

            ReliableTopicConfig &ReliableTopicConfig::set_read_batch_size(int batch_size) {
                if (batch_size <= 0) {
                    BOOST_THROW_EXCEPTION(exception::IllegalArgumentException("ReliableTopicConfig::setReadBatchSize",
                                                                              "readBatchSize should be positive"));
                }

                this->readBatchSize_ = batch_size;

                return *this;
            }

            SocketOptions::SocketOptions() : tcpNoDelay_(true), keepAlive_(true), reuseAddress_(true), lingerSeconds_(3),
                                             bufferSize_(DEFAULT_BUFFER_SIZE_BYTE) {}

            bool SocketOptions::is_tcp_no_delay() const {
                return tcpNoDelay_;
            }

            SocketOptions &SocketOptions::set_tcp_no_delay(bool tcp_no_delay) {
                SocketOptions::tcpNoDelay_ = tcp_no_delay;
                return *this;
            }

            bool SocketOptions::is_keep_alive() const {
                return keepAlive_;
            }

            SocketOptions &SocketOptions::set_keep_alive(bool keep_alive) {
                SocketOptions::keepAlive_ = keep_alive;
                return *this;
            }

            bool SocketOptions::is_reuse_address() const {
                return reuseAddress_;
            }

            SocketOptions &SocketOptions::set_reuse_address(bool reuse_address) {
                SocketOptions::reuseAddress_ = reuse_address;
                return *this;
            }

            int SocketOptions::get_linger_seconds() const {
                return lingerSeconds_;
            }

            SocketOptions &SocketOptions::set_linger_seconds(int linger_seconds) {
                SocketOptions::lingerSeconds_ = linger_seconds;
                return *this;
            }

            int SocketOptions::get_buffer_size_in_bytes() const {
                return bufferSize_;
            }

            SocketOptions &SocketOptions::set_buffer_size_in_bytes(int buffer_size) {
                SocketOptions::bufferSize_ = buffer_size;
                return *this;
            }

            ClientAwsConfig::ClientAwsConfig() : enabled_(false), region_("us-east-1"), hostHeader_("ec2.amazonaws.com"),
                                                 insideAws_(false) {
            }

            const std::string &ClientAwsConfig::get_access_key() const {
                return accessKey_;
            }

            ClientAwsConfig &ClientAwsConfig::set_access_key(const std::string &access_key) {
                this->accessKey_ = util::Preconditions::check_has_text(access_key, "accessKey must contain text");
                return *this;
            }

            const std::string &ClientAwsConfig::get_secret_key() const {
                return secretKey_;
            }

            ClientAwsConfig &ClientAwsConfig::set_secret_key(const std::string &secret_key) {
                this->secretKey_ = util::Preconditions::check_has_text(secret_key, "secretKey must contain text");
                return *this;
            }

            const std::string &ClientAwsConfig::get_region() const {
                return region_;
            }

            ClientAwsConfig &ClientAwsConfig::set_region(const std::string &region) {
                this->region_ = util::Preconditions::check_has_text(region, "region must contain text");
                return *this;
            }

            const std::string &ClientAwsConfig::get_host_header() const {
                return hostHeader_;
            }

            ClientAwsConfig &ClientAwsConfig::set_host_header(const std::string &host_header) {
                this->hostHeader_ = util::Preconditions::check_has_text(host_header, "hostHeader must contain text");
                return *this;
            }

            ClientAwsConfig &ClientAwsConfig::set_enabled(bool enabled) {
                util::Preconditions::check_ssl("getAwsConfig");
                this->enabled_ = enabled;
                return *this;
            }

            bool ClientAwsConfig::is_enabled() const {
                return enabled_;
            }

            ClientAwsConfig &ClientAwsConfig::set_security_group_name(const std::string &security_group_name) {
                this->securityGroupName_ = security_group_name;
                return *this;
            }

            const std::string &ClientAwsConfig::get_security_group_name() const {
                return securityGroupName_;
            }

            ClientAwsConfig &ClientAwsConfig::set_tag_key(const std::string &tag_key) {
                this->tagKey_ = tag_key;
                return *this;
            }

            const std::string &ClientAwsConfig::get_tag_key() const {
                return tagKey_;
            }

            ClientAwsConfig &ClientAwsConfig::set_tag_value(const std::string &tag_value) {
                this->tagValue_ = tag_value;
                return *this;
            }

            const std::string &ClientAwsConfig::get_tag_value() const {
                return tagValue_;
            }

            const std::string &ClientAwsConfig::get_iam_role() const {
                return iamRole_;
            }

            ClientAwsConfig &ClientAwsConfig::set_iam_role(const std::string &iam_role) {
                this->iamRole_ = iam_role;
                return *this;
            }

            bool ClientAwsConfig::is_inside_aws() const {
                return insideAws_;
            }

            ClientAwsConfig &ClientAwsConfig::set_inside_aws(bool inside_aws) {
                this->insideAws_ = inside_aws;
                return *this;
            }

            std::ostream &operator<<(std::ostream &out, const ClientAwsConfig &config) {
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
                MatchingPointConfigPatternMatcher::matches(const std::vector<std::string> &config_patterns,
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

                int MatchingPointConfigPatternMatcher::get_matching_point(const std::string &pattern,
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

            EvictionConfig::EvictionConfig() : size_(DEFAULT_MAX_ENTRY_COUNT), maxSizePolicy_(DEFAULT_MAX_SIZE_POLICY),
                               evictionPolicy_(DEFAULT_EVICTION_POLICY) {}

            int32_t EvictionConfig::get_size() const {
                return size_;
            }

            EvictionConfig &EvictionConfig::set_size(int32_t size) {
                this->size_ = util::Preconditions::check_positive(size, "Size must be positive number!");
                return *this;
            }

            EvictionConfig::MaxSizePolicy EvictionConfig::get_maximum_size_policy() const {
                return maxSizePolicy_;
            }

            EvictionConfig &EvictionConfig::set_maximum_size_policy(const EvictionConfig::MaxSizePolicy &max_size_policy) {
                this->maxSizePolicy_ = max_size_policy;
                return *this;
            }

            EvictionPolicy EvictionConfig::get_eviction_policy() const {
                return evictionPolicy_;
            }

            EvictionConfig &EvictionConfig::set_eviction_policy(EvictionPolicy policy) {
                this->evictionPolicy_ = policy;
                return *this;
            }

            internal::eviction::EvictionStrategyType::Type EvictionConfig::get_eviction_strategy_type() const {
                // TODO: add support for other/custom eviction strategies
                return internal::eviction::EvictionStrategyType::DEFAULT_EVICTION_STRATEGY;
            }

            internal::eviction::EvictionPolicyType EvictionConfig::get_eviction_policy_type() const {
                if (evictionPolicy_ == LFU) {
                    return internal::eviction::LFU;
                } else if (evictionPolicy_ == LRU) {
                    return internal::eviction::LRU;
                } else if (evictionPolicy_ == RANDOM) {
                    return internal::eviction::RANDOM;
                } else if (evictionPolicy_ == NONE) {
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

            NearCacheConfig::NearCacheConfig() : name_("default"), timeToLiveSeconds_(DEFAULT_TTL_SECONDS),
                                                 maxIdleSeconds_(DEFAULT_MAX_IDLE_SECONDS),
                                                 inMemoryFormat_(DEFAULT_MEMORY_FORMAT),
                                                 localUpdatePolicy_(INVALIDATE), invalidateOnChange_(true),
                                                 cacheLocalEntries_(false) {
            }

            NearCacheConfig::NearCacheConfig(const std::string &cache_name) : NearCacheConfig() {
                name_ = cache_name;
            }

            NearCacheConfig::NearCacheConfig(const std::string &cache_name, InMemoryFormat memory_format)
                    : NearCacheConfig(name_) {
                this->inMemoryFormat_ = memory_format;
            }

            NearCacheConfig::NearCacheConfig(int32_t time_to_live_seconds, int32_t max_idle_seconds, bool invalidate_on_change,
                                             InMemoryFormat in_memory_format, const EvictionConfig &evict_config)
                    : NearCacheConfig(name_, in_memory_format) {
                this->timeToLiveSeconds_ = time_to_live_seconds;
                this->maxIdleSeconds_ = max_idle_seconds;
                this->invalidateOnChange_ = invalidate_on_change;
                this->evictionConfig_ = evict_config;
            }

            const std::string &NearCacheConfig::get_name() const {
                return name_;
            }

            NearCacheConfig &NearCacheConfig::set_name(const std::string &name) {
                this->name_ = name;
                return *this;
            }

            int32_t NearCacheConfig::get_time_to_live_seconds() const {
                return timeToLiveSeconds_;
            }

            NearCacheConfig &NearCacheConfig::set_time_to_live_seconds(int32_t time_to_live_seconds) {
                this->timeToLiveSeconds_ = util::Preconditions::check_not_negative(time_to_live_seconds,
                                                                                "TTL seconds cannot be negative!");
                return *this;
            }

            int32_t NearCacheConfig::get_max_idle_seconds() const {
                return maxIdleSeconds_;
            }

            NearCacheConfig &NearCacheConfig::set_max_idle_seconds(int32_t max_idle_seconds) {
                this->maxIdleSeconds_ = util::Preconditions::check_not_negative(max_idle_seconds,
                                                                             "Max-Idle seconds cannot be negative!");
                return *this;
            }

            bool NearCacheConfig::is_invalidate_on_change() const {
                return invalidateOnChange_;
            }

            NearCacheConfig &NearCacheConfig::set_invalidate_on_change(bool invalidate_on_change) {
                this->invalidateOnChange_ = invalidate_on_change;
                return *this;
            }

            const InMemoryFormat &NearCacheConfig::get_in_memory_format() const {
                return inMemoryFormat_;
            }

            NearCacheConfig &NearCacheConfig::set_in_memory_format(const InMemoryFormat &in_memory_format) {
                this->inMemoryFormat_ = in_memory_format;
                return *this;
            }

            bool NearCacheConfig::is_cache_local_entries() const {
                return cacheLocalEntries_;
            }

            NearCacheConfig &NearCacheConfig::set_cache_local_entries(bool cache_local_entries) {
                this->cacheLocalEntries_ = cache_local_entries;
                return *this;
            }

            const NearCacheConfig::LocalUpdatePolicy &NearCacheConfig::get_local_update_policy() const {
                return localUpdatePolicy_;
            }

            NearCacheConfig &NearCacheConfig::set_local_update_policy(const LocalUpdatePolicy &local_update_policy) {
                this->localUpdatePolicy_ = local_update_policy;
                return *this;
            }

            EvictionConfig &NearCacheConfig::get_eviction_config() {
                return evictionConfig_;
            }

            NearCacheConfig &NearCacheConfig::set_eviction_config(const EvictionConfig &eviction_config) {
                this->evictionConfig_ = eviction_config;
                return *this;
            }

            int32_t NearCacheConfig::calculate_max_size(int32_t max_size) {
                return (max_size == 0) ? INT32_MAX : util::Preconditions::check_not_negative(max_size,
                                                                                          "Max-size cannot be negative!");
            }

            std::ostream &operator<<(std::ostream &out, const NearCacheConfig &config) {
                out << "NearCacheConfig{"
                    << "timeToLiveSeconds=" << config.timeToLiveSeconds_
                    << ", maxIdleSeconds=" << config.maxIdleSeconds_
                    << ", invalidateOnChange=" << config.invalidateOnChange_
                    << ", inMemoryFormat=" << config.inMemoryFormat_
                    << ", cacheLocalEntries=" << config.cacheLocalEntries_
                    << ", localUpdatePolicy=" << config.localUpdatePolicy_
                    << config.evictionConfig_;
                out << '}';

                return out;
            }
        }

        ClientConfig::ClientConfig() : cluster_name_("dev"), loadBalancer_(NULL), redoOperation_(false),
                                       socketInterceptor_(), executorPoolSize_(-1) {}

        ClientConfig &ClientConfig::set_redo_operation(bool redo_operation) {
            this->redoOperation_ = redo_operation;
            return *this;
        }

        bool ClientConfig::is_redo_operation() const {
            return redoOperation_;
        }

        LoadBalancer *const ClientConfig::get_load_balancer() {
            if (!loadBalancer_)
                return &defaultLoadBalancer_;
            return loadBalancer_;
        }

        ClientConfig &ClientConfig::set_load_balancer(LoadBalancer *load_balancer) {
            this->loadBalancer_ = load_balancer;
            return *this;
        }

        config::LoggerConfig &ClientConfig::get_logger_config() {
            return loggerConfig_;
        }

        ClientConfig &ClientConfig::add_listener(LifecycleListener &&listener) {
            lifecycleListeners_.emplace_back(std::move(listener));
            return *this;
        }

        ClientConfig &ClientConfig::add_listener(MembershipListener &&listener) {
            membershipListeners_.emplace_back(std::move(listener));
            return *this;
        }

        const std::vector<LifecycleListener> &ClientConfig::get_lifecycle_listeners() const {
            return lifecycleListeners_;
        }

        const std::vector<MembershipListener> &ClientConfig::get_membership_listeners() const {
            return membershipListeners_;
        }

        ClientConfig &ClientConfig::set_socket_interceptor(SocketInterceptor &&interceptor) {
            this->socketInterceptor_ = std::move(interceptor);
            return *this;
        }

        const SocketInterceptor &ClientConfig::get_socket_interceptor() const {
            return socketInterceptor_;
        }

        SerializationConfig &ClientConfig::get_serialization_config() {
            return serializationConfig_;
        }

        ClientConfig &ClientConfig::set_serialization_config(SerializationConfig const &serialization_config) {
            this->serializationConfig_ = serialization_config;
            return *this;
        }

        const std::unordered_map<std::string, std::string> &ClientConfig::get_properties() const {
            return properties_;
        }

        ClientConfig &ClientConfig::set_property(const std::string &name, const std::string &value) {
            properties_[name] = value;
            return *this;
        }

        ClientConfig &ClientConfig::add_reliable_topic_config(const config::ReliableTopicConfig &reliable_topic_config) {
            reliableTopicConfigMap_[reliable_topic_config.get_name()] = reliable_topic_config;
            return *this;
        }

        const config::ReliableTopicConfig &ClientConfig::get_reliable_topic_config(const std::string &name) {
            auto it = reliableTopicConfigMap_.find(name);
            if (reliableTopicConfigMap_.end() == it) {
                reliableTopicConfigMap_[name] = config::ReliableTopicConfig(name.c_str());
            }

            return reliableTopicConfigMap_[name];
        }

        config::ClientNetworkConfig &ClientConfig::get_network_config() {
            return networkConfig_;
        }

        ClientConfig &ClientConfig::add_near_cache_config(const config::NearCacheConfig &near_cache_config) {
            nearCacheConfigMap_.emplace(near_cache_config.get_name(), near_cache_config);
            return *this;
        }

        const config::NearCacheConfig *ClientConfig::get_near_cache_config(const std::string &name) const {
            auto nearCacheConfig = internal::config::ConfigUtils::lookup_by_pattern(
                    configPatternMatcher_, nearCacheConfigMap_, name);
            if (nearCacheConfig) {
                return nearCacheConfig;
            }

            auto config_it = nearCacheConfigMap_.find("default");
            if (config_it != nearCacheConfigMap_.end()) {
                return &nearCacheConfigMap_.find("default")->second;
            }

            // not needed for c++ client since it is always native memory
            //initDefaultMaxSizeForOnHeapMaps(nearCacheConfig);
            return nullptr;
        }

        ClientConfig &ClientConfig::set_network_config(const config::ClientNetworkConfig &network_config) {
            this->networkConfig_ = network_config;
            return *this;
        }

        const std::shared_ptr<std::string> &ClientConfig::get_instance_name() const {
            return instanceName_;
        }

        void ClientConfig::set_instance_name(const std::shared_ptr<std::string> &instance_name) {
            ClientConfig::instanceName_ = instance_name;
        }

        int32_t ClientConfig::get_executor_pool_size() const {
            return executorPoolSize_;
        }

        void ClientConfig::set_executor_pool_size(int32_t executor_pool_size) {
            ClientConfig::executorPoolSize_ = executor_pool_size;
        }

        config::ClientConnectionStrategyConfig &ClientConfig::get_connection_strategy_config() {
            return connectionStrategyConfig_;
        }

        ClientConfig &ClientConfig::set_connection_strategy_config(
                const config::ClientConnectionStrategyConfig &connection_strategy_config) {
            ClientConfig::connectionStrategyConfig_ = connection_strategy_config;
            return *this;
        }

        const config::ClientFlakeIdGeneratorConfig *
        ClientConfig::find_flake_id_generator_config(const std::string &name) {
            std::string baseName = internal::partition::strategy::StringPartitioningStrategy::get_base_name(name);
            auto config = internal::config::ConfigUtils::lookup_by_pattern<config::ClientFlakeIdGeneratorConfig>(
                    configPatternMatcher_, flakeIdGeneratorConfigMap_, baseName);
            if (config) {
                return config;
            }
            return get_flake_id_generator_config("default");
        }


        const config::ClientFlakeIdGeneratorConfig *
        ClientConfig::get_flake_id_generator_config(const std::string &name) {
            std::string baseName = internal::partition::strategy::StringPartitioningStrategy::get_base_name(name);
            auto config = internal::config::ConfigUtils::lookup_by_pattern<config::ClientFlakeIdGeneratorConfig>(
                    configPatternMatcher_, flakeIdGeneratorConfigMap_, baseName);
            if (config) {
                return config;
            }
            auto defConfig = flakeIdGeneratorConfigMap_.find("default");
            if (defConfig == flakeIdGeneratorConfigMap_.end()) {
                flakeIdGeneratorConfigMap_.emplace("default", config::ClientFlakeIdGeneratorConfig("default"));
            }
            defConfig = flakeIdGeneratorConfigMap_.find("default");
            config::ClientFlakeIdGeneratorConfig new_config = defConfig->second;
            new_config.set_name(name);
            flakeIdGeneratorConfigMap_.emplace(name, std::move(new_config));
            return &flakeIdGeneratorConfigMap_.find(name)->second;
        }

        ClientConfig &
        ClientConfig::add_flake_id_generator_config(const config::ClientFlakeIdGeneratorConfig &config) {
            flakeIdGeneratorConfigMap_.emplace(config.get_name(), config);
            return *this;
        }

        const std::string &ClientConfig::get_cluster_name() const {
            return cluster_name_;
        }

        ClientConfig &ClientConfig::set_cluster_name(const std::string &cluster_name) {
            cluster_name_ = cluster_name;
            return *this;
        }

        const std::unordered_set<std::string> &ClientConfig::get_labels() const {
            return labels_;
        }

        ClientConfig &ClientConfig::set_labels(const std::unordered_set<std::string> &labels) {
            labels_ = labels;
            return *this;
        }

        ClientConfig &ClientConfig::add_label(const std::string &label) {
            labels_.insert(label);
            return *this;
        }

        ClientConfig &ClientConfig::backup_acks_enabled(bool enabled) {
            backup_acks_enabled_ = enabled;
            return *this;
        }

        bool ClientConfig::backup_acks_enabled() {
            return backup_acks_enabled_;
        }

        const std::shared_ptr<security::credentials> &ClientConfig::get_credentials() const {
            return credentials_;
        }

        ClientConfig &ClientConfig::set_credentials(const std::shared_ptr<security::credentials> &credential) {
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
