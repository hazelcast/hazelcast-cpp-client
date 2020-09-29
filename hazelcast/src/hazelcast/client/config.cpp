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
#include "hazelcast/client/GroupConfig.h"
#include "hazelcast/client/config/matcher/MatchingPointConfigPatternMatcher.h"
#include "hazelcast/client/query/Predicates.h"
#include "hazelcast/client/LifecycleListener.h"

namespace hazelcast {
    namespace client {
        SerializationConfig::SerializationConfig() : version_(0) {
        }

        int SerializationConfig::getPortableVersion() const {
            return version_;
        }

        SerializationConfig &SerializationConfig::setPortableVersion(int v) {
            this->version_ = v;
            return *this;
        }

        std::shared_ptr<serialization::global_serializer> SerializationConfig::getGlobalSerializer() const {
            return global_serializer_;
        }

        void SerializationConfig::setGlobalSerializer(
                const std::shared_ptr<serialization::global_serializer> &globalSerializer) {
            global_serializer_ = globalSerializer;
        }

        namespace config {
            SSLConfig::SSLConfig() : enabled_(false), ssl_protocol_(tlsv12) {
            }

            bool SSLConfig::isEnabled() const {
                return enabled_;
            }

            SSLConfig &SSLConfig::setEnabled(bool isEnabled) {
                util::Preconditions::checkSSL("getAwsConfig");
                this->enabled_ = isEnabled;
                return *this;
            }

            SSLConfig &SSLConfig::setProtocol(SSLProtocol protocol) {
                this->ssl_protocol_ = protocol;
                return *this;
            }

            SSLProtocol SSLConfig::getProtocol() const {
                return ssl_protocol_;
            }

            const std::vector<std::string> &SSLConfig::getVerifyFiles() const {
                return client_verify_files_;
            }

            SSLConfig &SSLConfig::addVerifyFile(const std::string &filename) {
                this->client_verify_files_.push_back(filename);
                return *this;
            }

            const std::string &SSLConfig::getCipherList() const {
                return cipher_list_;
            }

            SSLConfig &SSLConfig::setCipherList(const std::string &ciphers) {
                this->cipher_list_ = ciphers;
                return *this;
            }

            constexpr int64_t ClientFlakeIdGeneratorConfig::DEFAULT_PREFETCH_VALIDITY_MILLIS;

            ClientFlakeIdGeneratorConfig::ClientFlakeIdGeneratorConfig(const std::string &name)
                    : name_(name), prefetch_count_(ClientFlakeIdGeneratorConfig::DEFAULT_PREFETCH_COUNT),
                      prefetch_validity_duration_(ClientFlakeIdGeneratorConfig::DEFAULT_PREFETCH_VALIDITY_MILLIS) {}

            const std::string &ClientFlakeIdGeneratorConfig::getName() const {
                return name_;
            }

            ClientFlakeIdGeneratorConfig &ClientFlakeIdGeneratorConfig::setName(const std::string &n) {
                ClientFlakeIdGeneratorConfig::name_ = n;
                return *this;
            }

            int32_t ClientFlakeIdGeneratorConfig::getPrefetchCount() const {
                return prefetch_count_;
            }

            ClientFlakeIdGeneratorConfig &ClientFlakeIdGeneratorConfig::setPrefetchCount(int32_t count) {
                std::ostringstream out;
                out << "prefetch-count must be 1.." << MAXIMUM_PREFETCH_COUNT << ", not " << count;
                util::Preconditions::checkTrue(count > 0 && count <= MAXIMUM_PREFETCH_COUNT, out.str());
                prefetch_count_ = count;
                return *this;
            }

            std::chrono::steady_clock::duration ClientFlakeIdGeneratorConfig::getPrefetchValidityDuration() const {
                return prefetch_validity_duration_;
            }

            ClientFlakeIdGeneratorConfig &
            ClientFlakeIdGeneratorConfig::setPrefetchValidityDuration(std::chrono::steady_clock::duration duration) {
                util::Preconditions::checkNotNegative(duration.count(),
                                                      "prefetchValidityMs must be non negative");
                prefetch_validity_duration_ = duration;
                return *this;
            }

            int32_t ClientNetworkConfig::CONNECTION_ATTEMPT_PERIOD = 3000;

            ClientNetworkConfig::ClientNetworkConfig()
                    : connection_timeout_(5000), smart_routing_(true), connection_attempt_limit_(-1),
                      connection_attempt_period_(CONNECTION_ATTEMPT_PERIOD) {}

            SSLConfig &ClientNetworkConfig::getSSLConfig() {
                return ssl_config_;
            }

            ClientNetworkConfig &ClientNetworkConfig::setSSLConfig(const config::SSLConfig &config) {
                ssl_config_ = config;
                return *this;
            }

            int64_t ClientNetworkConfig::getConnectionTimeout() const {
                return connection_timeout_;
            }

            ClientNetworkConfig &ClientNetworkConfig::setConnectionTimeout(int64_t connectionTimeoutInMillis) {
                this->connection_timeout_ = connectionTimeoutInMillis;
                return *this;
            }

            ClientNetworkConfig &ClientNetworkConfig::setAwsConfig(const ClientAwsConfig &clientAwsConfig) {
                this->client_aws_config_ = clientAwsConfig;
                return *this;
            }

            ClientAwsConfig &ClientNetworkConfig::getAwsConfig() {
                return client_aws_config_;
            }

            bool ClientNetworkConfig::isSmartRouting() const {
                return smart_routing_;
            }

            ClientNetworkConfig &ClientNetworkConfig::setSmartRouting(bool smartRouting) {
                ClientNetworkConfig::smart_routing_ = smartRouting;
                return *this;
            }

            int32_t ClientNetworkConfig::getConnectionAttemptLimit() const {
                return connection_attempt_limit_;
            }

            ClientNetworkConfig &ClientNetworkConfig::setConnectionAttemptLimit(int32_t connectionAttemptLimit) {
                if (connectionAttemptLimit < 0) {
                    BOOST_THROW_EXCEPTION(
                            exception::IllegalArgumentException("ClientNetworkConfig::setConnectionAttemptLimit",
                                                                "connectionAttemptLimit cannot be negative"));
                }
                this->connection_attempt_limit_ = connectionAttemptLimit;
                return *this;
            }

            int32_t ClientNetworkConfig::getConnectionAttemptPeriod() const {
                return connection_attempt_period_;
            }

            ClientNetworkConfig &ClientNetworkConfig::setConnectionAttemptPeriod(int32_t connectionAttemptPeriod) {
                if (connectionAttemptPeriod < 0) {
                    BOOST_THROW_EXCEPTION(
                            exception::IllegalArgumentException("ClientNetworkConfig::setConnectionAttemptPeriod",
                                                                "connectionAttemptPeriod cannot be negative"));
                }
                this->connection_attempt_period_ = connectionAttemptPeriod;
                return *this;
            }

            std::vector<Address> ClientNetworkConfig::getAddresses() const {
                return address_list_;
            }

            ClientNetworkConfig &ClientNetworkConfig::addAddresses(const std::vector<Address> &addresses) {
                address_list_.insert(address_list_.end(), addresses.begin(), addresses.end());
                return *this;
            }

            ClientNetworkConfig &ClientNetworkConfig::setAddresses(const std::vector<Address> &addresses) {
                address_list_ = addresses;
                return *this;
            }

            ClientNetworkConfig &ClientNetworkConfig::addAddress(const Address &address) {
                address_list_.push_back(address);
                return *this;
            }

            SocketOptions &ClientNetworkConfig::getSocketOptions() {
                return socket_options_;
            }

            const std::string &LoggerConfig::getConfigurationFileName() const {
                return configuration_file_name_;
            }

            void LoggerConfig::setConfigurationFileName(const std::string &fileName) {
                LoggerConfig::configuration_file_name_ = fileName;
            }

            LoggerConfig::LoggerConfig() : type_(Type::EASYLOGGINGPP), log_level_(LoggerLevel::INFO) {}

            LoggerConfig::Type::LoggerType LoggerConfig::getType() const {
                return type_;
            }

            void LoggerConfig::setType(LoggerConfig::Type::LoggerType type) {
                LoggerConfig::type_ = type;
            }

            LoggerLevel::Level LoggerConfig::getLogLevel() const {
                return log_level_;
            }

            void LoggerConfig::setLogLevel(LoggerLevel::Level logLevel) {
                LoggerConfig::log_level_ = logLevel;
            }

            ClientConnectionStrategyConfig::ClientConnectionStrategyConfig() : async_start_(false), reconnect_mode_(ON) {
            }

            ClientConnectionStrategyConfig::ReconnectMode ClientConnectionStrategyConfig::getReconnectMode() const {
                return reconnect_mode_;
            }

            bool ClientConnectionStrategyConfig::isAsyncStart() const {
                return async_start_;
            }

            ClientConnectionStrategyConfig &ClientConnectionStrategyConfig::setAsyncStart(bool asyncStart) {
                this->async_start_ = asyncStart;
                return *this;
            }

            ClientConnectionStrategyConfig &
            ClientConnectionStrategyConfig::setReconnectMode(ReconnectMode reconnectMode) {
                this->reconnect_mode_ = reconnectMode;
                return *this;
            }

            const int ReliableTopicConfig::DEFAULT_READ_BATCH_SIZE = 10;

            ReliableTopicConfig::ReliableTopicConfig() = default;

            ReliableTopicConfig::ReliableTopicConfig(const char *topicName) : read_batch_size_(DEFAULT_READ_BATCH_SIZE),
                                                                              name_(topicName) {
            }

            const std::string &ReliableTopicConfig::getName() const {
                return name_;
            }

            int ReliableTopicConfig::getReadBatchSize() const {
                return read_batch_size_;
            }

            ReliableTopicConfig &ReliableTopicConfig::setReadBatchSize(int batchSize) {
                if (batchSize <= 0) {
                    BOOST_THROW_EXCEPTION(exception::IllegalArgumentException("ReliableTopicConfig::setReadBatchSize",
                                                                              "readBatchSize should be positive"));
                }

                this->read_batch_size_ = batchSize;

                return *this;
            }

            SocketOptions::SocketOptions() : tcp_no_delay_(true), keep_alive_(true), reuse_address_(true), linger_seconds_(3),
                                             buffer_size_(DEFAULT_BUFFER_SIZE_BYTE) {}

            bool SocketOptions::isTcpNoDelay() const {
                return tcp_no_delay_;
            }

            SocketOptions &SocketOptions::setTcpNoDelay(bool tcpNoDelay) {
                SocketOptions::tcp_no_delay_ = tcpNoDelay;
                return *this;
            }

            bool SocketOptions::isKeepAlive() const {
                return keep_alive_;
            }

            SocketOptions &SocketOptions::setKeepAlive(bool keepAlive) {
                SocketOptions::keep_alive_ = keepAlive;
                return *this;
            }

            bool SocketOptions::isReuseAddress() const {
                return reuse_address_;
            }

            SocketOptions &SocketOptions::setReuseAddress(bool reuseAddress) {
                SocketOptions::reuse_address_ = reuseAddress;
                return *this;
            }

            int SocketOptions::getLingerSeconds() const {
                return linger_seconds_;
            }

            SocketOptions &SocketOptions::setLingerSeconds(int lingerSeconds) {
                SocketOptions::linger_seconds_ = lingerSeconds;
                return *this;
            }

            int SocketOptions::getBufferSizeInBytes() const {
                return buffer_size_;
            }

            SocketOptions &SocketOptions::setBufferSizeInBytes(int bufferSize) {
                SocketOptions::buffer_size_ = bufferSize;
                return *this;
            }

            ClientAwsConfig::ClientAwsConfig() : enabled_(false), region_("us-east-1"), host_header_("ec2.amazonaws.com"),
                                                 inside_aws_(false) {
            }

            const std::string &ClientAwsConfig::getAccessKey() const {
                return access_key_;
            }

            ClientAwsConfig &ClientAwsConfig::setAccessKey(const std::string &accessKey) {
                this->access_key_ = util::Preconditions::checkHasText(accessKey, "accessKey must contain text");
                return *this;
            }

            const std::string &ClientAwsConfig::getSecretKey() const {
                return secret_key_;
            }

            ClientAwsConfig &ClientAwsConfig::setSecretKey(const std::string &secretKey) {
                this->secret_key_ = util::Preconditions::checkHasText(secretKey, "secretKey must contain text");
                return *this;
            }

            const std::string &ClientAwsConfig::getRegion() const {
                return region_;
            }

            ClientAwsConfig &ClientAwsConfig::setRegion(const std::string &region) {
                this->region_ = util::Preconditions::checkHasText(region, "region must contain text");
                return *this;
            }

            const std::string &ClientAwsConfig::getHostHeader() const {
                return host_header_;
            }

            ClientAwsConfig &ClientAwsConfig::setHostHeader(const std::string &hostHeader) {
                this->host_header_ = util::Preconditions::checkHasText(hostHeader, "hostHeader must contain text");
                return *this;
            }

            ClientAwsConfig &ClientAwsConfig::setEnabled(bool enabled) {
                util::Preconditions::checkSSL("getAwsConfig");
                this->enabled_ = enabled;
                return *this;
            }

            bool ClientAwsConfig::isEnabled() const {
                return enabled_;
            }

            ClientAwsConfig &ClientAwsConfig::setSecurityGroupName(const std::string &securityGroupName) {
                this->security_group_name_ = securityGroupName;
                return *this;
            }

            const std::string &ClientAwsConfig::getSecurityGroupName() const {
                return security_group_name_;
            }

            ClientAwsConfig &ClientAwsConfig::setTagKey(const std::string &tagKey) {
                this->tag_key_ = tagKey;
                return *this;
            }

            const std::string &ClientAwsConfig::getTagKey() const {
                return tag_key_;
            }

            ClientAwsConfig &ClientAwsConfig::setTagValue(const std::string &tagValue) {
                this->tag_value_ = tagValue;
                return *this;
            }

            const std::string &ClientAwsConfig::getTagValue() const {
                return tag_value_;
            }

            const std::string &ClientAwsConfig::getIamRole() const {
                return iam_role_;
            }

            ClientAwsConfig &ClientAwsConfig::setIamRole(const std::string &iamRole) {
                this->iam_role_ = iamRole;
                return *this;
            }

            bool ClientAwsConfig::isInsideAws() const {
                return inside_aws_;
            }

            ClientAwsConfig &ClientAwsConfig::setInsideAws(bool insideAws) {
                this->inside_aws_ = insideAws;
                return *this;
            }

            std::ostream &operator<<(std::ostream &out, const ClientAwsConfig &config) {
                return out << "ClientAwsConfig{"
                           << "enabled=" << config.isEnabled()
                           << ", region='" << config.getRegion() << '\''
                           << ", securityGroupName='" << config.getSecurityGroupName() << '\''
                           << ", tagKey='" << config.getTagKey() << '\''
                           << ", tagValue='" << config.getTagValue() << '\''
                           << ", hostHeader='" << config.getHostHeader() << '\''
                           << ", iamRole='" << config.getIamRole() << "\'}";
            }

            namespace matcher {
                std::shared_ptr<std::string>
                MatchingPointConfigPatternMatcher::matches(const std::vector<std::string> &configPatterns,
                                                           const std::string &itemName) const {
                    std::shared_ptr<std::string> candidate;
                    std::shared_ptr<std::string> duplicate;
                    int lastMatchingPoint = -1;
                    for (const std::string &pattern  : configPatterns) {
                        int matchingPoint = getMatchingPoint(pattern, itemName);
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
                                "MatchingPointConfigPatternMatcher::matches") << "Configuration " << itemName
                                                                              << " has duplicate configuration. Candidate:"
                                                                              << *candidate << ", duplicate:"
                                                                              << *duplicate).build();
                    }
                    return candidate;
                }

                int MatchingPointConfigPatternMatcher::getMatchingPoint(const std::string &pattern,
                                                                        const std::string &itemName) const {
                    size_t index = pattern.find('*');
                    if (index == std::string::npos) {
                        return -1;
                    }

                    std::string firstPart = pattern.substr(0, index);
                    if (itemName.find(firstPart) != 0) {
                        return -1;
                    }

                    std::string secondPart = pattern.substr(index + 1);
                    if (itemName.rfind(secondPart) != (itemName.length() - secondPart.length())) {
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
        }

        GroupConfig::GroupConfig() {}

        GroupConfig::GroupConfig(const std::string &name, const std::string &password)
                : name_(name), password_(password) {
        }

        std::string GroupConfig::getName() const {
            return name_;
        }

        GroupConfig &GroupConfig::setName(const std::string &name) {
            this->name_ = name;
            return (*this);
        }

        GroupConfig &GroupConfig::setPassword(const std::string &password) {
            this->password_ = password;
            return (*this);
        }

        std::string GroupConfig::getPassword() const {
            return password_;
        }

        ClientConfig::ClientConfig() : cluster_name_("dev"), load_balancer_(NULL), redo_operation_(false),
                                       socket_interceptor_(), executor_pool_size_(-1) {}

        ClientConfig &ClientConfig::setGroupConfig(const GroupConfig &groupConfig) {
            this->group_config_ = groupConfig;
            return *this;
        }
        
        GroupConfig &ClientConfig::getGroupConfig() {
            return group_config_;
        }

        ClientConfig &ClientConfig::setRedoOperation(bool redoOperation) {
            this->redo_operation_ = redoOperation;
            return *this;
        }

        bool ClientConfig::isRedoOperation() const {
            return redo_operation_;
        }

        LoadBalancer *const ClientConfig::getLoadBalancer() {
            if (!load_balancer_)
                return &default_load_balancer_;
            return load_balancer_;
        }

        ClientConfig &ClientConfig::setLoadBalancer(LoadBalancer *loadBalancer) {
            this->load_balancer_ = loadBalancer;
            return *this;
        }

        ClientConfig &ClientConfig::setLogLevel(LogLevel loggerLevel) {
            this->logger_config_.setLogLevel((LoggerLevel::Level) loggerLevel);
            return *this;
        }

        config::LoggerConfig &ClientConfig::getLoggerConfig() {
            return logger_config_;
        }

        ClientConfig &ClientConfig::addListener(LifecycleListener &&listener) {
            lifecycle_listeners_.emplace_back(std::move(listener));
            return *this;
        }

        ClientConfig &ClientConfig::addListener(MembershipListener &&listener) {
            membership_listeners_.emplace_back(std::move(listener));
            return *this;
        }

        const std::vector<LifecycleListener> &ClientConfig::getLifecycleListeners() const {
            return lifecycle_listeners_;
        }

        const std::vector<MembershipListener> &ClientConfig::getMembershipListeners() const {
            return membership_listeners_;
        }

        ClientConfig &ClientConfig::setSocketInterceptor(SocketInterceptor &&interceptor) {
            this->socket_interceptor_ = std::move(interceptor);
            return *this;
        }

        const SocketInterceptor &ClientConfig::getSocketInterceptor() const {
            return socket_interceptor_;
        }

        SerializationConfig &ClientConfig::getSerializationConfig() {
            return serialization_config_;
        }

        ClientConfig &ClientConfig::setSerializationConfig(SerializationConfig const &serializationConfig) {
            this->serialization_config_ = serializationConfig;
            return *this;
        }

        const std::unordered_map<std::string, std::string> &ClientConfig::getProperties() const {
            return properties_;
        }

        ClientConfig &ClientConfig::setProperty(const std::string &name, const std::string &value) {
            properties_[name] = value;
            return *this;
        }

        ClientConfig &ClientConfig::addReliableTopicConfig(const config::ReliableTopicConfig &reliableTopicConfig) {
            reliable_topic_config_map_[reliableTopicConfig.getName()] = reliableTopicConfig;
            return *this;
        }

        const config::ReliableTopicConfig *ClientConfig::getReliableTopicConfig(const std::string &name) {
            std::unordered_map<std::string, config::ReliableTopicConfig>::const_iterator it = reliable_topic_config_map_.find(name);
            if (reliable_topic_config_map_.end() == it) {
                reliable_topic_config_map_[name] = config::ReliableTopicConfig(name.c_str());
            }
            return &reliable_topic_config_map_[name];
        }

        config::ClientNetworkConfig &ClientConfig::getNetworkConfig() {
            return network_config_;
        }

        ClientConfig &ClientConfig::setNetworkConfig(const config::ClientNetworkConfig &networkConfig) {
            this->network_config_ = networkConfig;
            return *this;
        }

        const std::shared_ptr<std::string> &ClientConfig::getInstanceName() const {
            return instance_name_;
        }

        void ClientConfig::setInstanceName(const std::shared_ptr<std::string> &instanceName) {
            ClientConfig::instance_name_ = instanceName;
        }

        int32_t ClientConfig::getExecutorPoolSize() const {
            return executor_pool_size_;
        }

        void ClientConfig::setExecutorPoolSize(int32_t executorPoolSize) {
            ClientConfig::executor_pool_size_ = executorPoolSize;
        }

        config::ClientConnectionStrategyConfig &ClientConfig::getConnectionStrategyConfig() {
            return connection_strategy_config_;
        }

        ClientConfig &ClientConfig::setConnectionStrategyConfig(
                const config::ClientConnectionStrategyConfig &connectionStrategyConfig) {
            ClientConfig::connection_strategy_config_ = connectionStrategyConfig;
            return *this;
        }

        std::shared_ptr<config::ClientFlakeIdGeneratorConfig>
        ClientConfig::findFlakeIdGeneratorConfig(const std::string &name) {
            std::string baseName = internal::partition::strategy::StringPartitioningStrategy::getBaseName(name);
            std::shared_ptr<config::ClientFlakeIdGeneratorConfig> config = internal::config::ConfigUtils::lookupByPattern<config::ClientFlakeIdGeneratorConfig>(
                    config_pattern_matcher_, flake_id_generator_config_map_, baseName);
            if (config.get() != NULL) {
                return config;
            }
            return getFlakeIdGeneratorConfig("default");
        }


        std::shared_ptr<config::ClientFlakeIdGeneratorConfig>
        ClientConfig::getFlakeIdGeneratorConfig(const std::string &name) {
            std::string baseName = internal::partition::strategy::StringPartitioningStrategy::getBaseName(name);
            std::shared_ptr<config::ClientFlakeIdGeneratorConfig> config = internal::config::ConfigUtils::lookupByPattern<config::ClientFlakeIdGeneratorConfig>(
                    config_pattern_matcher_, flake_id_generator_config_map_, baseName);
            if (config.get() != NULL) {
                return config;
            }
            std::shared_ptr<config::ClientFlakeIdGeneratorConfig> defConfig = flake_id_generator_config_map_.get("default");
            if (defConfig.get() == NULL) {
                defConfig.reset(new config::ClientFlakeIdGeneratorConfig("default"));
                flake_id_generator_config_map_.put(defConfig->getName(), defConfig);
            }
            config.reset(new config::ClientFlakeIdGeneratorConfig(*defConfig));
            config->setName(name);
            flake_id_generator_config_map_.put(config->getName(), config);
            return config;
        }

        ClientConfig &
        ClientConfig::addFlakeIdGeneratorConfig(const std::shared_ptr<config::ClientFlakeIdGeneratorConfig> &config) {
            flake_id_generator_config_map_.put(config->getName(), config);
            return *this;
        }

        const std::string &ClientConfig::getClusterName() const {
            return cluster_name_;
        }

        ClientConfig &ClientConfig::setClusterName(const std::string &clusterName) {
            cluster_name_ = clusterName;
            return *this;
        }

        const std::unordered_set<std::string> &ClientConfig::getLabels() const {
            return labels_;
        }

        ClientConfig &ClientConfig::setLabels(const std::unordered_set<std::string> &labels) {
            labels_ = labels;
            return *this;
        }

        ClientConfig &ClientConfig::addLabel(const std::string &label) {
            labels_.insert(label);
            return *this;
        }
        
        namespace security {
            username_password_credentials::username_password_credentials(const std::string &name,
                                                                                   const std::string &password) : name_(
                    name), password_(password) {}

            const std::string &username_password_credentials::get_name() const {
                return name_;
            }

            const std::string &username_password_credentials::get_password() const {
                return password_;
            }

            const credentials::type username_password_credentials::get_type() const {
                return credentials::type::username_password;
            }

            const std::string &token_credentials::get_name() const {
                return name_;
            }

            const std::vector<byte> &token_credentials::get_secret() const {
                return secret_data_;
            }

            const credentials::type token_credentials::get_type() const {
                return credentials::type::token;
            }

            token_credentials::token_credentials(const std::string &name, const std::vector<byte> &secretData) : name_(
                    name), secret_data_(secretData) {}

            secret_credential::secret_credential(const std::string &name, const std::vector<byte> &secretData)
                    : token_credentials(name, secretData) {}

            const credentials::type secret_credential::get_type() const {
                return credentials::type::secret;
            }

            credentials::~credentials() {}
        }
    }
}
