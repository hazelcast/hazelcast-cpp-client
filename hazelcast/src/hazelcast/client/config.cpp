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

        int SerializationConfig::getPortableVersion() const {
            return version_;
        }

        SerializationConfig &SerializationConfig::setPortableVersion(int v) {
            this->version_ = v;
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
            SSLConfig::SSLConfig() : enabled_(false), sslProtocol_(tlsv12) {
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
                this->sslProtocol_ = protocol;
                return *this;
            }

            SSLProtocol SSLConfig::getProtocol() const {
                return sslProtocol_;
            }

            const std::vector<std::string> &SSLConfig::getVerifyFiles() const {
                return clientVerifyFiles_;
            }

            SSLConfig &SSLConfig::addVerifyFile(const std::string &filename) {
                this->clientVerifyFiles_.push_back(filename);
                return *this;
            }

            const std::string &SSLConfig::getCipherList() const {
                return cipherList_;
            }

            SSLConfig &SSLConfig::setCipherList(const std::string &ciphers) {
                this->cipherList_ = ciphers;
                return *this;
            }

            constexpr int64_t ClientFlakeIdGeneratorConfig::DEFAULT_PREFETCH_VALIDITY_MILLIS;

            ClientFlakeIdGeneratorConfig::ClientFlakeIdGeneratorConfig(const std::string &name)
                    : name_(name), prefetchCount_(ClientFlakeIdGeneratorConfig::DEFAULT_PREFETCH_COUNT),
                      prefetchValidityDuration_(ClientFlakeIdGeneratorConfig::DEFAULT_PREFETCH_VALIDITY_MILLIS) {}

            const std::string &ClientFlakeIdGeneratorConfig::getName() const {
                return name_;
            }

            ClientFlakeIdGeneratorConfig &ClientFlakeIdGeneratorConfig::setName(const std::string &n) {
                ClientFlakeIdGeneratorConfig::name_ = n;
                return *this;
            }

            int32_t ClientFlakeIdGeneratorConfig::getPrefetchCount() const {
                return prefetchCount_;
            }

            ClientFlakeIdGeneratorConfig &ClientFlakeIdGeneratorConfig::setPrefetchCount(int32_t count) {
                std::ostringstream out;
                out << "prefetch-count must be 1.." << MAXIMUM_PREFETCH_COUNT << ", not " << count;
                util::Preconditions::checkTrue(count > 0 && count <= MAXIMUM_PREFETCH_COUNT, out.str());
                prefetchCount_ = count;
                return *this;
            }

            std::chrono::milliseconds ClientFlakeIdGeneratorConfig::getPrefetchValidityDuration() const {
                return prefetchValidityDuration_;
            }

            ClientFlakeIdGeneratorConfig &
            ClientFlakeIdGeneratorConfig::setPrefetchValidityDuration(std::chrono::milliseconds duration) {
                util::Preconditions::checkNotNegative(duration.count(),
                                                      "prefetchValidityMs must be non negative");
                prefetchValidityDuration_ = duration;
                return *this;
            }

            int32_t ClientNetworkConfig::CONNECTION_ATTEMPT_PERIOD = 3000;

            ClientNetworkConfig::ClientNetworkConfig()
                    : connectionTimeout_(5000), smartRouting_(true), connectionAttemptLimit_(-1),
                      connectionAttemptPeriod_(CONNECTION_ATTEMPT_PERIOD) {}

            SSLConfig &ClientNetworkConfig::getSSLConfig() {
                return sslConfig_;
            }

            ClientNetworkConfig &ClientNetworkConfig::setSSLConfig(const config::SSLConfig &config) {
                sslConfig_ = config;
                return *this;
            }

            std::chrono::milliseconds ClientNetworkConfig::getConnectionTimeout() const {
                return connectionTimeout_;
            }

            ClientNetworkConfig &ClientNetworkConfig::setAwsConfig(const ClientAwsConfig &clientAwsConfig) {
                this->clientAwsConfig_ = clientAwsConfig;
                return *this;
            }

            ClientAwsConfig &ClientNetworkConfig::getAwsConfig() {
                return clientAwsConfig_;
            }

            bool ClientNetworkConfig::isSmartRouting() const {
                return smartRouting_;
            }

            ClientNetworkConfig &ClientNetworkConfig::setSmartRouting(bool smartRouting) {
                ClientNetworkConfig::smartRouting_ = smartRouting;
                return *this;
            }

            int32_t ClientNetworkConfig::getConnectionAttemptLimit() const {
                return connectionAttemptLimit_;
            }

            ClientNetworkConfig &ClientNetworkConfig::setConnectionAttemptLimit(int32_t connectionAttemptLimit) {
                if (connectionAttemptLimit < 0) {
                    BOOST_THROW_EXCEPTION(
                            exception::IllegalArgumentException("ClientNetworkConfig::setConnectionAttemptLimit",
                                                                "connectionAttemptLimit cannot be negative"));
                }
                this->connectionAttemptLimit_ = connectionAttemptLimit;
                return *this;
            }

            std::chrono::milliseconds ClientNetworkConfig::getConnectionAttemptPeriod() const {
                return connectionAttemptPeriod_;
            }

            std::vector<Address> ClientNetworkConfig::getAddresses() const {
                return addressList_;
            }

            ClientNetworkConfig &ClientNetworkConfig::addAddresses(const std::vector<Address> &addresses) {
                addressList_.insert(addressList_.end(), addresses.begin(), addresses.end());
                return *this;
            }

            ClientNetworkConfig &ClientNetworkConfig::setAddresses(const std::vector<Address> &addresses) {
                addressList_ = addresses;
                return *this;
            }

            ClientNetworkConfig &ClientNetworkConfig::addAddress(const Address &address) {
                addressList_.push_back(address);
                return *this;
            }

            SocketOptions &ClientNetworkConfig::getSocketOptions() {
                return socketOptions_;
            }

            ClientNetworkConfig &ClientNetworkConfig::setConnectionTimeout(const std::chrono::milliseconds &timeout) {
                connectionTimeout_ = timeout;
                return *this;
            }

            ClientNetworkConfig &
            ClientNetworkConfig::setConnectionAttemptPeriod(const std::chrono::milliseconds &interval) {
                util::Preconditions::checkNotNegative(interval.count(), (boost::format(
                        "Provided connectionAttemptPeriod(%1% msecs) cannot be negative") % interval.count()).str());
                connectionAttemptPeriod_ = interval;
                return *this;
            }

            ClientConnectionStrategyConfig::ClientConnectionStrategyConfig() : asyncStart_(false), reconnectMode_(ON) {
            }

            ClientConnectionStrategyConfig::ReconnectMode ClientConnectionStrategyConfig::getReconnectMode() const {
                return reconnectMode_;
            }

            bool ClientConnectionStrategyConfig::isAsyncStart() const {
                return asyncStart_;
            }

            ClientConnectionStrategyConfig &ClientConnectionStrategyConfig::setAsyncStart(bool asyncStart) {
                this->asyncStart_ = asyncStart;
                return *this;
            }

            ClientConnectionStrategyConfig &
            ClientConnectionStrategyConfig::setReconnectMode(ReconnectMode reconnectMode) {
                this->reconnectMode_ = reconnectMode;
                return *this;
            }

            const int ReliableTopicConfig::DEFAULT_READ_BATCH_SIZE = 10;

            ReliableTopicConfig::ReliableTopicConfig() = default;

            ReliableTopicConfig::ReliableTopicConfig(const char *topicName) : readBatchSize_(DEFAULT_READ_BATCH_SIZE),
                                                                              name_(topicName) {
            }

            const std::string &ReliableTopicConfig::getName() const {
                return name_;
            }

            int ReliableTopicConfig::getReadBatchSize() const {
                return readBatchSize_;
            }

            ReliableTopicConfig &ReliableTopicConfig::setReadBatchSize(int batchSize) {
                if (batchSize <= 0) {
                    BOOST_THROW_EXCEPTION(exception::IllegalArgumentException("ReliableTopicConfig::setReadBatchSize",
                                                                              "readBatchSize should be positive"));
                }

                this->readBatchSize_ = batchSize;

                return *this;
            }

            SocketOptions::SocketOptions() : tcpNoDelay_(true), keepAlive_(true), reuseAddress_(true), lingerSeconds_(3),
                                             bufferSize_(DEFAULT_BUFFER_SIZE_BYTE) {}

            bool SocketOptions::isTcpNoDelay() const {
                return tcpNoDelay_;
            }

            SocketOptions &SocketOptions::setTcpNoDelay(bool tcpNoDelay) {
                SocketOptions::tcpNoDelay_ = tcpNoDelay;
                return *this;
            }

            bool SocketOptions::isKeepAlive() const {
                return keepAlive_;
            }

            SocketOptions &SocketOptions::setKeepAlive(bool keepAlive) {
                SocketOptions::keepAlive_ = keepAlive;
                return *this;
            }

            bool SocketOptions::isReuseAddress() const {
                return reuseAddress_;
            }

            SocketOptions &SocketOptions::setReuseAddress(bool reuseAddress) {
                SocketOptions::reuseAddress_ = reuseAddress;
                return *this;
            }

            int SocketOptions::getLingerSeconds() const {
                return lingerSeconds_;
            }

            SocketOptions &SocketOptions::setLingerSeconds(int lingerSeconds) {
                SocketOptions::lingerSeconds_ = lingerSeconds;
                return *this;
            }

            int SocketOptions::getBufferSizeInBytes() const {
                return bufferSize_;
            }

            SocketOptions &SocketOptions::setBufferSizeInBytes(int bufferSize) {
                SocketOptions::bufferSize_ = bufferSize;
                return *this;
            }

            ClientAwsConfig::ClientAwsConfig() : enabled_(false), region_("us-east-1"), hostHeader_("ec2.amazonaws.com"),
                                                 insideAws_(false) {
            }

            const std::string &ClientAwsConfig::getAccessKey() const {
                return accessKey_;
            }

            ClientAwsConfig &ClientAwsConfig::setAccessKey(const std::string &accessKey) {
                this->accessKey_ = util::Preconditions::checkHasText(accessKey, "accessKey must contain text");
                return *this;
            }

            const std::string &ClientAwsConfig::getSecretKey() const {
                return secretKey_;
            }

            ClientAwsConfig &ClientAwsConfig::setSecretKey(const std::string &secretKey) {
                this->secretKey_ = util::Preconditions::checkHasText(secretKey, "secretKey must contain text");
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
                return hostHeader_;
            }

            ClientAwsConfig &ClientAwsConfig::setHostHeader(const std::string &hostHeader) {
                this->hostHeader_ = util::Preconditions::checkHasText(hostHeader, "hostHeader must contain text");
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
                this->securityGroupName_ = securityGroupName;
                return *this;
            }

            const std::string &ClientAwsConfig::getSecurityGroupName() const {
                return securityGroupName_;
            }

            ClientAwsConfig &ClientAwsConfig::setTagKey(const std::string &tagKey) {
                this->tagKey_ = tagKey;
                return *this;
            }

            const std::string &ClientAwsConfig::getTagKey() const {
                return tagKey_;
            }

            ClientAwsConfig &ClientAwsConfig::setTagValue(const std::string &tagValue) {
                this->tagValue_ = tagValue;
                return *this;
            }

            const std::string &ClientAwsConfig::getTagValue() const {
                return tagValue_;
            }

            const std::string &ClientAwsConfig::getIamRole() const {
                return iamRole_;
            }

            ClientAwsConfig &ClientAwsConfig::setIamRole(const std::string &iamRole) {
                this->iamRole_ = iamRole;
                return *this;
            }

            bool ClientAwsConfig::isInsideAws() const {
                return insideAws_;
            }

            ClientAwsConfig &ClientAwsConfig::setInsideAws(bool insideAws) {
                this->insideAws_ = insideAws;
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

            EvictionConfig::EvictionConfig() : size_(DEFAULT_MAX_ENTRY_COUNT), maxSizePolicy_(DEFAULT_MAX_SIZE_POLICY),
                               evictionPolicy_(DEFAULT_EVICTION_POLICY) {}

            int32_t EvictionConfig::getSize() const {
                return size_;
            }

            EvictionConfig &EvictionConfig::setSize(int32_t size) {
                this->size_ = util::Preconditions::checkPositive(size, "Size must be positive number!");
                return *this;
            }

            EvictionConfig::MaxSizePolicy EvictionConfig::getMaximumSizePolicy() const {
                return maxSizePolicy_;
            }

            EvictionConfig &EvictionConfig::setMaximumSizePolicy(const EvictionConfig::MaxSizePolicy &maxSizePolicy) {
                this->maxSizePolicy_ = maxSizePolicy;
                return *this;
            }

            EvictionPolicy EvictionConfig::getEvictionPolicy() const {
                return evictionPolicy_;
            }

            EvictionConfig &EvictionConfig::setEvictionPolicy(EvictionPolicy policy) {
                this->evictionPolicy_ = policy;
                return *this;
            }

            internal::eviction::EvictionStrategyType::Type EvictionConfig::getEvictionStrategyType() const {
                // TODO: add support for other/custom eviction strategies
                return internal::eviction::EvictionStrategyType::DEFAULT_EVICTION_STRATEGY;
            }

            internal::eviction::EvictionPolicyType EvictionConfig::getEvictionPolicyType() const {
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
                    << "size=" << config.getSize()
                    << ", maxSizePolicy=" << config.getMaximumSizePolicy()
                    << ", evictionPolicy=" << config.getEvictionPolicy()
                    << '}';

                return out;
            }

            NearCacheConfig::NearCacheConfig() : name_("default"), timeToLiveSeconds_(DEFAULT_TTL_SECONDS),
                                                 maxIdleSeconds_(DEFAULT_MAX_IDLE_SECONDS),
                                                 inMemoryFormat_(DEFAULT_MEMORY_FORMAT),
                                                 localUpdatePolicy_(INVALIDATE), invalidateOnChange_(true),
                                                 cacheLocalEntries_(false) {
            }

            NearCacheConfig::NearCacheConfig(const std::string &cacheName) : NearCacheConfig() {
                name_ = cacheName;
            }

            NearCacheConfig::NearCacheConfig(const std::string &cacheName, InMemoryFormat memoryFormat)
                    : NearCacheConfig(name_) {
                this->inMemoryFormat_ = memoryFormat;
            }

            NearCacheConfig::NearCacheConfig(int32_t timeToLiveSeconds, int32_t maxIdleSeconds, bool invalidateOnChange,
                                             InMemoryFormat inMemoryFormat, const EvictionConfig &evictConfig)
                    : NearCacheConfig(name_, inMemoryFormat) {
                this->timeToLiveSeconds_ = timeToLiveSeconds;
                this->maxIdleSeconds_ = maxIdleSeconds;
                this->invalidateOnChange_ = invalidateOnChange;
                this->evictionConfig_ = evictConfig;
            }

            const std::string &NearCacheConfig::getName() const {
                return name_;
            }

            NearCacheConfig &NearCacheConfig::setName(const std::string &name) {
                this->name_ = name;
                return *this;
            }

            int32_t NearCacheConfig::getTimeToLiveSeconds() const {
                return timeToLiveSeconds_;
            }

            NearCacheConfig &NearCacheConfig::setTimeToLiveSeconds(int32_t timeToLiveSeconds) {
                this->timeToLiveSeconds_ = util::Preconditions::checkNotNegative(timeToLiveSeconds,
                                                                                "TTL seconds cannot be negative!");
                return *this;
            }

            int32_t NearCacheConfig::getMaxIdleSeconds() const {
                return maxIdleSeconds_;
            }

            NearCacheConfig &NearCacheConfig::setMaxIdleSeconds(int32_t maxIdleSeconds) {
                this->maxIdleSeconds_ = util::Preconditions::checkNotNegative(maxIdleSeconds,
                                                                             "Max-Idle seconds cannot be negative!");
                return *this;
            }

            bool NearCacheConfig::isInvalidateOnChange() const {
                return invalidateOnChange_;
            }

            NearCacheConfig &NearCacheConfig::setInvalidateOnChange(bool invalidateOnChange) {
                this->invalidateOnChange_ = invalidateOnChange;
                return *this;
            }

            const InMemoryFormat &NearCacheConfig::getInMemoryFormat() const {
                return inMemoryFormat_;
            }

            NearCacheConfig &NearCacheConfig::setInMemoryFormat(const InMemoryFormat &inMemoryFormat) {
                this->inMemoryFormat_ = inMemoryFormat;
                return *this;
            }

            bool NearCacheConfig::isCacheLocalEntries() const {
                return cacheLocalEntries_;
            }

            NearCacheConfig &NearCacheConfig::setCacheLocalEntries(bool cacheLocalEntries) {
                this->cacheLocalEntries_ = cacheLocalEntries;
                return *this;
            }

            const NearCacheConfig::LocalUpdatePolicy &NearCacheConfig::getLocalUpdatePolicy() const {
                return localUpdatePolicy_;
            }

            NearCacheConfig &NearCacheConfig::setLocalUpdatePolicy(const LocalUpdatePolicy &localUpdatePolicy) {
                this->localUpdatePolicy_ = localUpdatePolicy;
                return *this;
            }

            EvictionConfig &NearCacheConfig::getEvictionConfig() {
                return evictionConfig_;
            }

            NearCacheConfig &NearCacheConfig::setEvictionConfig(const EvictionConfig &evictionConfig) {
                this->evictionConfig_ = evictionConfig;
                return *this;
            }

            int32_t NearCacheConfig::calculateMaxSize(int32_t maxSize) {
                return (maxSize == 0) ? INT32_MAX : util::Preconditions::checkNotNegative(maxSize,
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

        ClientConfig &ClientConfig::setRedoOperation(bool redoOperation) {
            this->redoOperation_ = redoOperation;
            return *this;
        }

        bool ClientConfig::isRedoOperation() const {
            return redoOperation_;
        }

        LoadBalancer *const ClientConfig::getLoadBalancer() {
            if (!loadBalancer_)
                return &defaultLoadBalancer_;
            return loadBalancer_;
        }

        ClientConfig &ClientConfig::setLoadBalancer(LoadBalancer *loadBalancer) {
            this->loadBalancer_ = loadBalancer;
            return *this;
        }

        config::LoggerConfig &ClientConfig::getLoggerConfig() {
            return loggerConfig_;
        }

        ClientConfig &ClientConfig::addListener(LifecycleListener &&listener) {
            lifecycleListeners_.emplace_back(std::move(listener));
            return *this;
        }

        ClientConfig &ClientConfig::addListener(MembershipListener &&listener) {
            membershipListeners_.emplace_back(std::move(listener));
            return *this;
        }

        const std::vector<LifecycleListener> &ClientConfig::getLifecycleListeners() const {
            return lifecycleListeners_;
        }

        const std::vector<MembershipListener> &ClientConfig::getMembershipListeners() const {
            return membershipListeners_;
        }

        ClientConfig &ClientConfig::setSocketInterceptor(SocketInterceptor &&interceptor) {
            this->socketInterceptor_ = std::move(interceptor);
            return *this;
        }

        const SocketInterceptor &ClientConfig::getSocketInterceptor() const {
            return socketInterceptor_;
        }

        SerializationConfig &ClientConfig::getSerializationConfig() {
            return serializationConfig_;
        }

        ClientConfig &ClientConfig::setSerializationConfig(SerializationConfig const &serializationConfig) {
            this->serializationConfig_ = serializationConfig;
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
            reliableTopicConfigMap_[reliableTopicConfig.getName()] = reliableTopicConfig;
            return *this;
        }

        const config::ReliableTopicConfig &ClientConfig::getReliableTopicConfig(const std::string &name) {
            auto it = reliableTopicConfigMap_.find(name);

            if (reliableTopicConfigMap_.end() == it) {
                reliableTopicConfigMap_[name] = config::ReliableTopicConfig(name.c_str());
            }

            return reliableTopicConfigMap_[name];
        }

        config::ClientNetworkConfig &ClientConfig::getNetworkConfig() {
            return networkConfig_;
        }

        ClientConfig &ClientConfig::addNearCacheConfig(const config::NearCacheConfig &nearCacheConfig) {
            nearCacheConfigMap_.emplace(nearCacheConfig.getName(), nearCacheConfig);
            return *this;
        }

        const config::NearCacheConfig *ClientConfig::getNearCacheConfig(const std::string &name) const {
            auto nearCacheConfig = internal::config::ConfigUtils::lookupByPattern(
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

        ClientConfig &ClientConfig::setNetworkConfig(const config::ClientNetworkConfig &networkConfig) {
            this->networkConfig_ = networkConfig;
            return *this;
        }

        const std::shared_ptr<std::string> &ClientConfig::getInstanceName() const {
            return instanceName_;
        }

        void ClientConfig::setInstanceName(const std::shared_ptr<std::string> &instanceName) {
            ClientConfig::instanceName_ = instanceName;
        }

        int32_t ClientConfig::getExecutorPoolSize() const {
            return executorPoolSize_;
        }

        void ClientConfig::setExecutorPoolSize(int32_t executorPoolSize) {
            ClientConfig::executorPoolSize_ = executorPoolSize;
        }

        config::ClientConnectionStrategyConfig &ClientConfig::getConnectionStrategyConfig() {
            return connectionStrategyConfig_;
        }

        ClientConfig &ClientConfig::setConnectionStrategyConfig(
                const config::ClientConnectionStrategyConfig &connectionStrategyConfig) {
            ClientConfig::connectionStrategyConfig_ = connectionStrategyConfig;
            return *this;
        }

        const config::ClientFlakeIdGeneratorConfig *
        ClientConfig::findFlakeIdGeneratorConfig(const std::string &name) {
            std::string baseName = internal::partition::strategy::StringPartitioningStrategy::getBaseName(name);
            auto config = internal::config::ConfigUtils::lookupByPattern<config::ClientFlakeIdGeneratorConfig>(
                    configPatternMatcher_, flakeIdGeneratorConfigMap_, baseName);
            if (config) {
                return config;
            }
            return getFlakeIdGeneratorConfig("default");
        }


        const config::ClientFlakeIdGeneratorConfig *
        ClientConfig::getFlakeIdGeneratorConfig(const std::string &name) {
            std::string baseName = internal::partition::strategy::StringPartitioningStrategy::getBaseName(name);
            auto config = internal::config::ConfigUtils::lookupByPattern<config::ClientFlakeIdGeneratorConfig>(
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
            new_config.setName(name);
            flakeIdGeneratorConfigMap_.emplace(name, std::move(new_config));
            return &flakeIdGeneratorConfigMap_.find(name)->second;
        }

        ClientConfig &
        ClientConfig::addFlakeIdGeneratorConfig(const config::ClientFlakeIdGeneratorConfig &config) {
            flakeIdGeneratorConfigMap_.emplace(config.getName(), config);
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
