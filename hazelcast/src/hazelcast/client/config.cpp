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
        SerializationConfig::SerializationConfig() : version(0) {
        }

        int SerializationConfig::getPortableVersion() const {
            return version;
        }

        SerializationConfig &SerializationConfig::setPortableVersion(int v) {
            this->version = v;
            return *this;
        }

        std::shared_ptr<serialization::global_serializer> SerializationConfig::getGlobalSerializer() const {
            return globalSerializer_;
        }

        void SerializationConfig::setGlobalSerializer(
                const std::shared_ptr<serialization::global_serializer> &globalSerializer) {
            globalSerializer_ = globalSerializer;
        }

        namespace config {
            SSLConfig::SSLConfig() : enabled(false), sslProtocol(tlsv12) {
            }

            bool SSLConfig::isEnabled() const {
                return enabled;
            }

            SSLConfig &SSLConfig::setEnabled(bool isEnabled) {
                util::Preconditions::checkSSL("getAwsConfig");
                this->enabled = isEnabled;
                return *this;
            }

            SSLConfig &SSLConfig::setProtocol(SSLProtocol protocol) {
                this->sslProtocol = protocol;
                return *this;
            }

            SSLProtocol SSLConfig::getProtocol() const {
                return sslProtocol;
            }

            const std::vector<std::string> &SSLConfig::getVerifyFiles() const {
                return clientVerifyFiles;
            }

            SSLConfig &SSLConfig::addVerifyFile(const std::string &filename) {
                this->clientVerifyFiles.push_back(filename);
                return *this;
            }

            const std::string &SSLConfig::getCipherList() const {
                return cipherList;
            }

            SSLConfig &SSLConfig::setCipherList(const std::string &ciphers) {
                this->cipherList = ciphers;
                return *this;
            }

            constexpr int64_t ClientFlakeIdGeneratorConfig::DEFAULT_PREFETCH_VALIDITY_MILLIS;

            ClientFlakeIdGeneratorConfig::ClientFlakeIdGeneratorConfig(const std::string &name)
                    : name(name), prefetchCount(ClientFlakeIdGeneratorConfig::DEFAULT_PREFETCH_COUNT),
                      prefetchValidityDuration(ClientFlakeIdGeneratorConfig::DEFAULT_PREFETCH_VALIDITY_MILLIS) {}

            const std::string &ClientFlakeIdGeneratorConfig::getName() const {
                return name;
            }

            ClientFlakeIdGeneratorConfig &ClientFlakeIdGeneratorConfig::setName(const std::string &n) {
                ClientFlakeIdGeneratorConfig::name = n;
                return *this;
            }

            int32_t ClientFlakeIdGeneratorConfig::getPrefetchCount() const {
                return prefetchCount;
            }

            ClientFlakeIdGeneratorConfig &ClientFlakeIdGeneratorConfig::setPrefetchCount(int32_t count) {
                std::ostringstream out;
                out << "prefetch-count must be 1.." << MAXIMUM_PREFETCH_COUNT << ", not " << count;
                util::Preconditions::checkTrue(count > 0 && count <= MAXIMUM_PREFETCH_COUNT, out.str());
                prefetchCount = count;
                return *this;
            }

            std::chrono::milliseconds ClientFlakeIdGeneratorConfig::getPrefetchValidityDuration() const {
                return prefetchValidityDuration;
            }

            ClientFlakeIdGeneratorConfig &
            ClientFlakeIdGeneratorConfig::setPrefetchValidityDuration(std::chrono::milliseconds duration) {
                util::Preconditions::checkNotNegative(duration.count(),
                                                      "prefetchValidityMs must be non negative");
                prefetchValidityDuration = duration;
                return *this;
            }

            int32_t ClientNetworkConfig::CONNECTION_ATTEMPT_PERIOD = 3000;

            ClientNetworkConfig::ClientNetworkConfig()
                    : connectionTimeout(5000), smartRouting(true), connectionAttemptLimit(-1),
                      connectionAttemptPeriod(CONNECTION_ATTEMPT_PERIOD) {}

            SSLConfig &ClientNetworkConfig::getSSLConfig() {
                return sslConfig;
            }

            ClientNetworkConfig &ClientNetworkConfig::setSSLConfig(const config::SSLConfig &config) {
                sslConfig = config;
                return *this;
            }

            std::chrono::milliseconds ClientNetworkConfig::getConnectionTimeout() const {
                return connectionTimeout;
            }

            ClientNetworkConfig &ClientNetworkConfig::setAwsConfig(const ClientAwsConfig &clientAwsConfig) {
                this->clientAwsConfig = clientAwsConfig;
                return *this;
            }

            ClientAwsConfig &ClientNetworkConfig::getAwsConfig() {
                return clientAwsConfig;
            }

            bool ClientNetworkConfig::isSmartRouting() const {
                return smartRouting;
            }

            ClientNetworkConfig &ClientNetworkConfig::setSmartRouting(bool smartRouting) {
                ClientNetworkConfig::smartRouting = smartRouting;
                return *this;
            }

            int32_t ClientNetworkConfig::getConnectionAttemptLimit() const {
                return connectionAttemptLimit;
            }

            ClientNetworkConfig &ClientNetworkConfig::setConnectionAttemptLimit(int32_t connectionAttemptLimit) {
                if (connectionAttemptLimit < 0) {
                    BOOST_THROW_EXCEPTION(
                            exception::IllegalArgumentException("ClientNetworkConfig::setConnectionAttemptLimit",
                                                                "connectionAttemptLimit cannot be negative"));
                }
                this->connectionAttemptLimit = connectionAttemptLimit;
                return *this;
            }

            std::chrono::milliseconds ClientNetworkConfig::getConnectionAttemptPeriod() const {
                return connectionAttemptPeriod;
            }

            std::vector<Address> ClientNetworkConfig::getAddresses() const {
                return addressList;
            }

            ClientNetworkConfig &ClientNetworkConfig::addAddresses(const std::vector<Address> &addresses) {
                addressList.insert(addressList.end(), addresses.begin(), addresses.end());
                return *this;
            }

            ClientNetworkConfig &ClientNetworkConfig::setAddresses(const std::vector<Address> &addresses) {
                addressList = addresses;
                return *this;
            }

            ClientNetworkConfig &ClientNetworkConfig::addAddress(const Address &address) {
                addressList.push_back(address);
                return *this;
            }

            SocketOptions &ClientNetworkConfig::getSocketOptions() {
                return socketOptions;
            }

            ClientNetworkConfig &ClientNetworkConfig::setConnectionTimeout(const std::chrono::milliseconds &timeout) {
                connectionTimeout = timeout;
                return *this;
            }

            ClientNetworkConfig &
            ClientNetworkConfig::setConnectionAttemptPeriod(const std::chrono::milliseconds &interval) {
                util::Preconditions::checkNotNegative(interval.count(), (boost::format(
                        "Provided connectionAttemptPeriod(%1% msecs) cannot be negative") % interval.count()).str());
                connectionAttemptPeriod = interval;
                return *this;
            }

            ClientConnectionStrategyConfig::ClientConnectionStrategyConfig() : asyncStart(false), reconnectMode(ON) {
            }

            ClientConnectionStrategyConfig::ReconnectMode ClientConnectionStrategyConfig::getReconnectMode() const {
                return reconnectMode;
            }

            bool ClientConnectionStrategyConfig::isAsyncStart() const {
                return asyncStart;
            }

            ClientConnectionStrategyConfig &ClientConnectionStrategyConfig::setAsyncStart(bool asyncStart) {
                this->asyncStart = asyncStart;
                return *this;
            }

            ClientConnectionStrategyConfig &
            ClientConnectionStrategyConfig::setReconnectMode(ReconnectMode reconnectMode) {
                this->reconnectMode = reconnectMode;
                return *this;
            }

            const int ReliableTopicConfig::DEFAULT_READ_BATCH_SIZE = 10;

            ReliableTopicConfig::ReliableTopicConfig() = default;

            ReliableTopicConfig::ReliableTopicConfig(const char *topicName) : readBatchSize(DEFAULT_READ_BATCH_SIZE),
                                                                              name(topicName) {
            }

            const std::string &ReliableTopicConfig::getName() const {
                return name;
            }

            int ReliableTopicConfig::getReadBatchSize() const {
                return readBatchSize;
            }

            ReliableTopicConfig &ReliableTopicConfig::setReadBatchSize(int batchSize) {
                if (batchSize <= 0) {
                    BOOST_THROW_EXCEPTION(exception::IllegalArgumentException("ReliableTopicConfig::setReadBatchSize",
                                                                              "readBatchSize should be positive"));
                }

                this->readBatchSize = batchSize;

                return *this;
            }

            SocketOptions::SocketOptions() : tcpNoDelay(true), keepAlive(true), reuseAddress(true), lingerSeconds(3),
                                             bufferSize(DEFAULT_BUFFER_SIZE_BYTE) {}

            bool SocketOptions::isTcpNoDelay() const {
                return tcpNoDelay;
            }

            SocketOptions &SocketOptions::setTcpNoDelay(bool tcpNoDelay) {
                SocketOptions::tcpNoDelay = tcpNoDelay;
                return *this;
            }

            bool SocketOptions::isKeepAlive() const {
                return keepAlive;
            }

            SocketOptions &SocketOptions::setKeepAlive(bool keepAlive) {
                SocketOptions::keepAlive = keepAlive;
                return *this;
            }

            bool SocketOptions::isReuseAddress() const {
                return reuseAddress;
            }

            SocketOptions &SocketOptions::setReuseAddress(bool reuseAddress) {
                SocketOptions::reuseAddress = reuseAddress;
                return *this;
            }

            int SocketOptions::getLingerSeconds() const {
                return lingerSeconds;
            }

            SocketOptions &SocketOptions::setLingerSeconds(int lingerSeconds) {
                SocketOptions::lingerSeconds = lingerSeconds;
                return *this;
            }

            int SocketOptions::getBufferSizeInBytes() const {
                return bufferSize;
            }

            SocketOptions &SocketOptions::setBufferSizeInBytes(int bufferSize) {
                SocketOptions::bufferSize = bufferSize;
                return *this;
            }

            ClientAwsConfig::ClientAwsConfig() : enabled(false), region("us-east-1"), hostHeader("ec2.amazonaws.com"),
                                                 insideAws(false) {
            }

            const std::string &ClientAwsConfig::getAccessKey() const {
                return accessKey;
            }

            ClientAwsConfig &ClientAwsConfig::setAccessKey(const std::string &accessKey) {
                this->accessKey = util::Preconditions::checkHasText(accessKey, "accessKey must contain text");
                return *this;
            }

            const std::string &ClientAwsConfig::getSecretKey() const {
                return secretKey;
            }

            ClientAwsConfig &ClientAwsConfig::setSecretKey(const std::string &secretKey) {
                this->secretKey = util::Preconditions::checkHasText(secretKey, "secretKey must contain text");
                return *this;
            }

            const std::string &ClientAwsConfig::getRegion() const {
                return region;
            }

            ClientAwsConfig &ClientAwsConfig::setRegion(const std::string &region) {
                this->region = util::Preconditions::checkHasText(region, "region must contain text");
                return *this;
            }

            const std::string &ClientAwsConfig::getHostHeader() const {
                return hostHeader;
            }

            ClientAwsConfig &ClientAwsConfig::setHostHeader(const std::string &hostHeader) {
                this->hostHeader = util::Preconditions::checkHasText(hostHeader, "hostHeader must contain text");
                return *this;
            }

            ClientAwsConfig &ClientAwsConfig::setEnabled(bool enabled) {
                util::Preconditions::checkSSL("getAwsConfig");
                this->enabled = enabled;
                return *this;
            }

            bool ClientAwsConfig::isEnabled() const {
                return enabled;
            }

            ClientAwsConfig &ClientAwsConfig::setSecurityGroupName(const std::string &securityGroupName) {
                this->securityGroupName = securityGroupName;
                return *this;
            }

            const std::string &ClientAwsConfig::getSecurityGroupName() const {
                return securityGroupName;
            }

            ClientAwsConfig &ClientAwsConfig::setTagKey(const std::string &tagKey) {
                this->tagKey = tagKey;
                return *this;
            }

            const std::string &ClientAwsConfig::getTagKey() const {
                return tagKey;
            }

            ClientAwsConfig &ClientAwsConfig::setTagValue(const std::string &tagValue) {
                this->tagValue = tagValue;
                return *this;
            }

            const std::string &ClientAwsConfig::getTagValue() const {
                return tagValue;
            }

            const std::string &ClientAwsConfig::getIamRole() const {
                return iamRole;
            }

            ClientAwsConfig &ClientAwsConfig::setIamRole(const std::string &iamRole) {
                this->iamRole = iamRole;
                return *this;
            }

            bool ClientAwsConfig::isInsideAws() const {
                return insideAws;
            }

            ClientAwsConfig &ClientAwsConfig::setInsideAws(bool insideAws) {
                this->insideAws = insideAws;
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

            EvictionConfig::EvictionConfig() : size(DEFAULT_MAX_ENTRY_COUNT), maxSizePolicy(DEFAULT_MAX_SIZE_POLICY),
                               evictionPolicy(DEFAULT_EVICTION_POLICY) {}

            int32_t EvictionConfig::getSize() const {
                return size;
            }

            EvictionConfig &EvictionConfig::setSize(int32_t size) {
                this->size = util::Preconditions::checkPositive(size, "Size must be positive number!");
                return *this;
            }

            EvictionConfig::MaxSizePolicy EvictionConfig::getMaximumSizePolicy() const {
                return maxSizePolicy;
            }

            EvictionConfig &EvictionConfig::setMaximumSizePolicy(const EvictionConfig::MaxSizePolicy &maxSizePolicy) {
                this->maxSizePolicy = maxSizePolicy;
                return *this;
            }

            EvictionPolicy EvictionConfig::getEvictionPolicy() const {
                return evictionPolicy;
            }

            EvictionConfig &EvictionConfig::setEvictionPolicy(EvictionPolicy policy) {
                this->evictionPolicy = policy;
                return *this;
            }

            internal::eviction::EvictionStrategyType::Type EvictionConfig::getEvictionStrategyType() const {
                // TODO: add support for other/custom eviction strategies
                return internal::eviction::EvictionStrategyType::DEFAULT_EVICTION_STRATEGY;
            }

            internal::eviction::EvictionPolicyType EvictionConfig::getEvictionPolicyType() const {
                if (evictionPolicy == LFU) {
                    return internal::eviction::LFU;
                } else if (evictionPolicy == LRU) {
                    return internal::eviction::LRU;
                } else if (evictionPolicy == RANDOM) {
                    return internal::eviction::RANDOM;
                } else if (evictionPolicy == NONE) {
                    return internal::eviction::NONE;
                } else {
                    assert(0);
                }
                return internal::eviction::NONE;
            }

            std::ostream &operator<<(std::ostream &out, const EvictionConfig &config) {
                out << "EvictionConfig{"
                    << "size=" << config.getSize()
                    << ", maxSizePolicy=" << config.getMaximumSizePolicy()
                    << ", evictionPolicy=" << config.getEvictionPolicy()
                    << '}';

                return out;
            }

            NearCacheConfig::NearCacheConfig() : name("default"), timeToLiveSeconds(DEFAULT_TTL_SECONDS),
                                                 maxIdleSeconds(DEFAULT_MAX_IDLE_SECONDS),
                                                 inMemoryFormat(DEFAULT_MEMORY_FORMAT),
                                                 localUpdatePolicy(INVALIDATE), invalidateOnChange(true),
                                                 cacheLocalEntries(false) {
            }

            NearCacheConfig::NearCacheConfig(const std::string &cacheName) : NearCacheConfig() {
                name = cacheName;
            }

            NearCacheConfig::NearCacheConfig(const std::string &cacheName, InMemoryFormat memoryFormat)
                    : NearCacheConfig(name) {
                this->inMemoryFormat = memoryFormat;
            }

            NearCacheConfig::NearCacheConfig(int32_t timeToLiveSeconds, int32_t maxIdleSeconds, bool invalidateOnChange,
                                             InMemoryFormat inMemoryFormat, const EvictionConfig &evictConfig)
                    : NearCacheConfig(name, inMemoryFormat) {
                this->timeToLiveSeconds = timeToLiveSeconds;
                this->maxIdleSeconds = maxIdleSeconds;
                this->invalidateOnChange = invalidateOnChange;
                this->evictionConfig = evictConfig;
            }

            const std::string &NearCacheConfig::getName() const {
                return name;
            }

            NearCacheConfig &NearCacheConfig::setName(const std::string &name) {
                this->name = name;
                return *this;
            }

            int32_t NearCacheConfig::getTimeToLiveSeconds() const {
                return timeToLiveSeconds;
            }

            NearCacheConfig &NearCacheConfig::setTimeToLiveSeconds(int32_t timeToLiveSeconds) {
                this->timeToLiveSeconds = util::Preconditions::checkNotNegative(timeToLiveSeconds,
                                                                                "TTL seconds cannot be negative!");
                return *this;
            }

            int32_t NearCacheConfig::getMaxIdleSeconds() const {
                return maxIdleSeconds;
            }

            NearCacheConfig &NearCacheConfig::setMaxIdleSeconds(int32_t maxIdleSeconds) {
                this->maxIdleSeconds = util::Preconditions::checkNotNegative(maxIdleSeconds,
                                                                             "Max-Idle seconds cannot be negative!");
                return *this;
            }

            bool NearCacheConfig::isInvalidateOnChange() const {
                return invalidateOnChange;
            }

            NearCacheConfig &NearCacheConfig::setInvalidateOnChange(bool invalidateOnChange) {
                this->invalidateOnChange = invalidateOnChange;
                return *this;
            }

            const InMemoryFormat &NearCacheConfig::getInMemoryFormat() const {
                return inMemoryFormat;
            }

            NearCacheConfig &NearCacheConfig::setInMemoryFormat(const InMemoryFormat &inMemoryFormat) {
                this->inMemoryFormat = inMemoryFormat;
                return *this;
            }

            bool NearCacheConfig::isCacheLocalEntries() const {
                return cacheLocalEntries;
            }

            NearCacheConfig &NearCacheConfig::setCacheLocalEntries(bool cacheLocalEntries) {
                this->cacheLocalEntries = cacheLocalEntries;
                return *this;
            }

            const NearCacheConfig::LocalUpdatePolicy &NearCacheConfig::getLocalUpdatePolicy() const {
                return localUpdatePolicy;
            }

            NearCacheConfig &NearCacheConfig::setLocalUpdatePolicy(const LocalUpdatePolicy &localUpdatePolicy) {
                this->localUpdatePolicy = localUpdatePolicy;
                return *this;
            }

            EvictionConfig &NearCacheConfig::getEvictionConfig() {
                return evictionConfig;
            }

            NearCacheConfig &NearCacheConfig::setEvictionConfig(const EvictionConfig &evictionConfig) {
                this->evictionConfig = evictionConfig;
                return *this;
            }

            int32_t NearCacheConfig::calculateMaxSize(int32_t maxSize) {
                return (maxSize == 0) ? INT32_MAX : util::Preconditions::checkNotNegative(maxSize,
                                                                                          "Max-size cannot be negative!");
            }

            std::ostream &operator<<(std::ostream &out, const NearCacheConfig &config) {
                out << "NearCacheConfig{"
                    << "timeToLiveSeconds=" << config.timeToLiveSeconds
                    << ", maxIdleSeconds=" << config.maxIdleSeconds
                    << ", invalidateOnChange=" << config.invalidateOnChange
                    << ", inMemoryFormat=" << config.inMemoryFormat
                    << ", cacheLocalEntries=" << config.cacheLocalEntries
                    << ", localUpdatePolicy=" << config.localUpdatePolicy
                    << config.evictionConfig;
                out << '}';

                return out;
            }
        }

        ClientConfig::ClientConfig() : cluster_name_("dev"), loadBalancer(NULL), redoOperation(false),
                                       socketInterceptor(), executorPoolSize(-1) {}

        ClientConfig &ClientConfig::setRedoOperation(bool redoOperation) {
            this->redoOperation = redoOperation;
            return *this;
        }

        bool ClientConfig::isRedoOperation() const {
            return redoOperation;
        }

        LoadBalancer *const ClientConfig::getLoadBalancer() {
            if (!loadBalancer)
                return &defaultLoadBalancer;
            return loadBalancer;
        }

        ClientConfig &ClientConfig::setLoadBalancer(LoadBalancer *loadBalancer) {
            this->loadBalancer = loadBalancer;
            return *this;
        }

        config::LoggerConfig &ClientConfig::getLoggerConfig() {
            return loggerConfig;
        }

        ClientConfig &ClientConfig::addListener(LifecycleListener &&listener) {
            lifecycleListeners.emplace_back(std::move(listener));
            return *this;
        }

        ClientConfig &ClientConfig::addListener(MembershipListener &&listener) {
            membershipListeners.emplace_back(std::move(listener));
            return *this;
        }

        const std::vector<LifecycleListener> &ClientConfig::getLifecycleListeners() const {
            return lifecycleListeners;
        }

        const std::vector<MembershipListener> &ClientConfig::getMembershipListeners() const {
            return membershipListeners;
        }

        ClientConfig &ClientConfig::setSocketInterceptor(SocketInterceptor &&interceptor) {
            this->socketInterceptor = std::move(interceptor);
            return *this;
        }

        const SocketInterceptor &ClientConfig::getSocketInterceptor() const {
            return socketInterceptor;
        }

        SerializationConfig &ClientConfig::getSerializationConfig() {
            return serializationConfig;
        }

        ClientConfig &ClientConfig::setSerializationConfig(SerializationConfig const &serializationConfig) {
            this->serializationConfig = serializationConfig;
            return *this;
        }

        const std::unordered_map<std::string, std::string> &ClientConfig::getProperties() const {
            return properties;
        }

        ClientConfig &ClientConfig::setProperty(const std::string &name, const std::string &value) {
            properties[name] = value;
            return *this;
        }

        ClientConfig &ClientConfig::addReliableTopicConfig(const config::ReliableTopicConfig &reliableTopicConfig) {
            reliableTopicConfigMap[reliableTopicConfig.getName()] = reliableTopicConfig;
            return *this;
        }

        const config::ReliableTopicConfig &ClientConfig::getReliableTopicConfig(const std::string &name) {
            auto it = reliableTopicConfigMap.find(name);
            
            if (reliableTopicConfigMap.end() == it) {
                reliableTopicConfigMap[name] = config::ReliableTopicConfig(name.c_str());
            }
            
            return reliableTopicConfigMap[name];
        }

        config::ClientNetworkConfig &ClientConfig::getNetworkConfig() {
            return networkConfig;
        }

        ClientConfig &ClientConfig::addNearCacheConfig(const config::NearCacheConfig &nearCacheConfig) {
            nearCacheConfigMap.emplace(nearCacheConfig.getName(), nearCacheConfig);
            return *this;
        }

        const config::NearCacheConfig *ClientConfig::getNearCacheConfig(const std::string &name) const {
            auto nearCacheConfig = internal::config::ConfigUtils::lookupByPattern(
                    configPatternMatcher, nearCacheConfigMap, name);
            if (nearCacheConfig) {
                return nearCacheConfig;
            }

            auto config_it = nearCacheConfigMap.find("default");
            if (config_it != nearCacheConfigMap.end()) {
                return &nearCacheConfigMap.find("default")->second;
            }

            // not needed for c++ client since it is always native memory
            //initDefaultMaxSizeForOnHeapMaps(nearCacheConfig);
            return nullptr;
        }

        ClientConfig &ClientConfig::setNetworkConfig(const config::ClientNetworkConfig &networkConfig) {
            this->networkConfig = networkConfig;
            return *this;
        }

        const std::shared_ptr<std::string> &ClientConfig::getInstanceName() const {
            return instanceName;
        }

        void ClientConfig::setInstanceName(const std::shared_ptr<std::string> &instanceName) {
            ClientConfig::instanceName = instanceName;
        }

        int32_t ClientConfig::getExecutorPoolSize() const {
            return executorPoolSize;
        }

        void ClientConfig::setExecutorPoolSize(int32_t executorPoolSize) {
            ClientConfig::executorPoolSize = executorPoolSize;
        }

        config::ClientConnectionStrategyConfig &ClientConfig::getConnectionStrategyConfig() {
            return connectionStrategyConfig;
        }

        ClientConfig &ClientConfig::setConnectionStrategyConfig(
                const config::ClientConnectionStrategyConfig &connectionStrategyConfig) {
            ClientConfig::connectionStrategyConfig = connectionStrategyConfig;
            return *this;
        }

        const config::ClientFlakeIdGeneratorConfig *
        ClientConfig::findFlakeIdGeneratorConfig(const std::string &name) {
            std::string baseName = internal::partition::strategy::StringPartitioningStrategy::getBaseName(name);
            auto config = internal::config::ConfigUtils::lookupByPattern<config::ClientFlakeIdGeneratorConfig>(
                    configPatternMatcher, flakeIdGeneratorConfigMap, baseName);
            if (config) {
                return config;
            }
            return getFlakeIdGeneratorConfig("default");
        }


        const config::ClientFlakeIdGeneratorConfig *
        ClientConfig::getFlakeIdGeneratorConfig(const std::string &name) {
            std::string baseName = internal::partition::strategy::StringPartitioningStrategy::getBaseName(name);
            auto config = internal::config::ConfigUtils::lookupByPattern<config::ClientFlakeIdGeneratorConfig>(
                    configPatternMatcher, flakeIdGeneratorConfigMap, baseName);
            if (config) {
                return config;
            }
            auto defConfig = flakeIdGeneratorConfigMap.find("default");
            if (defConfig == flakeIdGeneratorConfigMap.end()) {
                flakeIdGeneratorConfigMap.emplace("default", config::ClientFlakeIdGeneratorConfig("default"));
            }
            defConfig = flakeIdGeneratorConfigMap.find("default");
            config::ClientFlakeIdGeneratorConfig new_config = defConfig->second;
            new_config.setName(name);
            flakeIdGeneratorConfigMap.emplace(name, std::move(new_config));
            return &flakeIdGeneratorConfigMap.find(name)->second;
        }

        ClientConfig &
        ClientConfig::addFlakeIdGeneratorConfig(const config::ClientFlakeIdGeneratorConfig &config) {
            flakeIdGeneratorConfigMap.emplace(config.getName(), config);
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

        ClientConfig &ClientConfig::backup_acks_enabled(bool enabled) {
            backup_acks_enabled_ = enabled;
            return *this;
        }

        bool ClientConfig::backup_acks_enabled() {
            return backup_acks_enabled_;
        }

        const std::shared_ptr<security::credentials> &ClientConfig::getCredentials() const {
            return credentials_;
        }

        ClientConfig &ClientConfig::setCredentials(const std::shared_ptr<security::credentials> &credential) {
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
