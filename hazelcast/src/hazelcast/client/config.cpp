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
#include "hazelcast/client/GroupConfig.h"
#include "hazelcast/client/config/matcher/MatchingPointConfigPatternMatcher.h"

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

        const std::shared_ptr<serialization::global_serializer> &SerializationConfig::getGlobalSerializer() const {
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

            std::chrono::steady_clock::duration ClientFlakeIdGeneratorConfig::getPrefetchValidityDuration() const {
                return prefetchValidityDuration;
            }

            ClientFlakeIdGeneratorConfig &
            ClientFlakeIdGeneratorConfig::setPrefetchValidityDuration(std::chrono::steady_clock::duration duration) {
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

            int64_t ClientNetworkConfig::getConnectionTimeout() const {
                return connectionTimeout;
            }

            ClientNetworkConfig &ClientNetworkConfig::setConnectionTimeout(int64_t connectionTimeoutInMillis) {
                this->connectionTimeout = connectionTimeoutInMillis;
                return *this;
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

            int32_t ClientNetworkConfig::getConnectionAttemptPeriod() const {
                return connectionAttemptPeriod;
            }

            ClientNetworkConfig &ClientNetworkConfig::setConnectionAttemptPeriod(int32_t connectionAttemptPeriod) {
                if (connectionAttemptPeriod < 0) {
                    BOOST_THROW_EXCEPTION(
                            exception::IllegalArgumentException("ClientNetworkConfig::setConnectionAttemptPeriod",
                                                                "connectionAttemptPeriod cannot be negative"));
                }
                this->connectionAttemptPeriod = connectionAttemptPeriod;
                return *this;
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

            const std::string &LoggerConfig::getConfigurationFileName() const {
                return configurationFileName;
            }

            void LoggerConfig::setConfigurationFileName(const std::string &fileName) {
                LoggerConfig::configurationFileName = fileName;
            }

            LoggerConfig::LoggerConfig() : type(Type::EASYLOGGINGPP), logLevel(LoggerLevel::INFO) {}

            LoggerConfig::Type::LoggerType LoggerConfig::getType() const {
                return type;
            }

            void LoggerConfig::setType(LoggerConfig::Type::LoggerType type) {
                LoggerConfig::type = type;
            }

            LoggerLevel::Level LoggerConfig::getLogLevel() const {
                return logLevel;
            }

            void LoggerConfig::setLogLevel(LoggerLevel::Level logLevel) {
                LoggerConfig::logLevel = logLevel;
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

            ReliableTopicConfig::ReliableTopicConfig() {

            }

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
                        throw (exception::ExceptionBuilder<exception::ConfigurationException>(
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
        }

        GroupConfig::GroupConfig() : name("dev"), password("dev-pass") {

        }

        GroupConfig::GroupConfig(const std::string &name, const std::string &password)
                : name(name), password(password) {
        }

        std::string GroupConfig::getName() const {
            return name;
        }

        GroupConfig &GroupConfig::setName(const std::string &name) {
            this->name = name;
            return (*this);
        }

        GroupConfig &GroupConfig::setPassword(const std::string &password) {
            this->password = password;
            return (*this);
        }

        std::string GroupConfig::getPassword() const {
            return password;
        }

        ClientConfig::ClientConfig()
                : loadBalancer(NULL), redoOperation(false), socketInterceptor(NULL), executorPoolSize(-1) {}

        ClientConfig &ClientConfig::addAddress(const Address &address) {
            networkConfig.addAddress(address);
            return (*this);
        }

        ClientConfig &ClientConfig::addAddresses(const std::vector<Address> &addresses) {
            networkConfig.addAddresses(addresses);
            return (*this);
        }


        std::unordered_set<Address> ClientConfig::getAddresses() {
            std::unordered_set<Address> result;
            for (const Address &address : networkConfig.getAddresses()) {
                result.insert(address);
            }
            return result;
        }

        ClientConfig &ClientConfig::setGroupConfig(const GroupConfig &groupConfig) {
            this->groupConfig = groupConfig;
            return *this;
        }
        
        GroupConfig &ClientConfig::getGroupConfig() {
            return groupConfig;
        }


        ClientConfig &ClientConfig::setConnectionAttemptLimit(int connectionAttemptLimit) {
            networkConfig.setConnectionAttemptLimit(connectionAttemptLimit);
            return *this;
        }

        int ClientConfig::getConnectionAttemptLimit() const {
            return networkConfig.getConnectionAttemptLimit();
        }

        ClientConfig &ClientConfig::setConnectionTimeout(int connectionTimeoutInMillis) {
            this->networkConfig.setConnectionTimeout(connectionTimeoutInMillis);
            return *this;
        }

        int ClientConfig::getConnectionTimeout() const {
            return (int) this->networkConfig.getConnectionTimeout();
        }

        ClientConfig &ClientConfig::setAttemptPeriod(int attemptPeriodInMillis) {
            networkConfig.setConnectionAttemptPeriod(attemptPeriodInMillis);
            return *this;
        }

        int ClientConfig::getAttemptPeriod() const {
            return networkConfig.getConnectionAttemptPeriod();
        }

        ClientConfig &ClientConfig::setRedoOperation(bool redoOperation) {
            this->redoOperation = redoOperation;
            return *this;
        }

        bool ClientConfig::isRedoOperation() const {
            return redoOperation;
        }

        LoadBalancer *const ClientConfig::getLoadBalancer() {
            if (loadBalancer == NULL)
                return &defaultLoadBalancer;
            return loadBalancer;
        }

        ClientConfig &ClientConfig::setLoadBalancer(LoadBalancer *loadBalancer) {
            this->loadBalancer = loadBalancer;
            return *this;
        }

        ClientConfig &ClientConfig::setLogLevel(LogLevel loggerLevel) {
            this->loggerConfig.setLogLevel((LoggerLevel::Level) loggerLevel);
            return *this;
        }

        config::LoggerConfig &ClientConfig::getLoggerConfig() {
            return loggerConfig;
        }

        ClientConfig &ClientConfig::addListener(LifecycleListener *listener) {
            lifecycleListeners.insert(listener);
            return *this;
        }

        ClientConfig &ClientConfig::addListener(MembershipListener *listener) {
            if (listener == NULL) {
                BOOST_THROW_EXCEPTION(exception::NullPointerException("ClientConfig::addListener(MembershipListener *)",
                                                                      "listener can't be null"));
            }

            membershipListeners.insert(listener);
            managedMembershipListeners.insert(
                    std::shared_ptr<MembershipListener>(new MembershipListenerDelegator(listener)));
            return *this;
        }

        ClientConfig &ClientConfig::addListener(InitialMembershipListener *listener) {
            if (listener == NULL) {
                BOOST_THROW_EXCEPTION(
                        exception::NullPointerException("ClientConfig::addListener(InitialMembershipListener *)",
                                                        "listener can't be null"));
            }

            membershipListeners.insert(listener);
            managedMembershipListeners.insert(
                    std::shared_ptr<MembershipListener>(new InitialMembershipListenerDelegator(listener)));
            return *this;
        }

        ClientConfig &ClientConfig::addListener(const std::shared_ptr<MembershipListener> &listener) {
            membershipListeners.insert(listener.get());
            managedMembershipListeners.insert(listener);
            return *this;
        }

        ClientConfig &ClientConfig::addListener(const std::shared_ptr<InitialMembershipListener> &listener) {
            membershipListeners.insert(listener.get());
            managedMembershipListeners.insert(listener);
            return *this;
        }

        const std::unordered_set<LifecycleListener *> &ClientConfig::getLifecycleListeners() const {
            return lifecycleListeners;
        }

        const std::unordered_set<MembershipListener *> &ClientConfig::getMembershipListeners() const {
            return membershipListeners;
        }


        const boost::optional<std::string> &ClientConfig::getPrincipal() const {
            return principal;
        }

        ClientConfig &ClientConfig::setSocketInterceptor(SocketInterceptor *interceptor) {
            this->socketInterceptor = interceptor;
            return *this;
        }

        SocketInterceptor *ClientConfig::getSocketInterceptor() {
            return socketInterceptor;
        }

        ClientConfig &ClientConfig::setSmart(bool smart) {
            networkConfig.setSmartRouting(smart);
            return *this;
        }

        bool ClientConfig::isSmart() const {
            return networkConfig.isSmartRouting();
        }

        SerializationConfig &ClientConfig::getSerializationConfig() {
            return serializationConfig;
        }

        ClientConfig &ClientConfig::setSerializationConfig(SerializationConfig const &serializationConfig) {
            this->serializationConfig = serializationConfig;
            return *this;
        }


        std::unordered_map<std::string, std::string> &ClientConfig::getProperties() {
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

        const config::ReliableTopicConfig *ClientConfig::getReliableTopicConfig(const std::string &name) {
            std::unordered_map<std::string, config::ReliableTopicConfig>::const_iterator it = reliableTopicConfigMap.find(name);
            if (reliableTopicConfigMap.end() == it) {
                reliableTopicConfigMap[name] = config::ReliableTopicConfig(name.c_str());
            }
            return &reliableTopicConfigMap[name];
        }

        config::ClientNetworkConfig &ClientConfig::getNetworkConfig() {
            return networkConfig;
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

        std::shared_ptr<config::ClientFlakeIdGeneratorConfig>
        ClientConfig::findFlakeIdGeneratorConfig(const std::string &name) {
            std::string baseName = internal::partition::strategy::StringPartitioningStrategy::getBaseName(name);
            std::shared_ptr<config::ClientFlakeIdGeneratorConfig> config = internal::config::ConfigUtils::lookupByPattern<config::ClientFlakeIdGeneratorConfig>(
                    configPatternMatcher, flakeIdGeneratorConfigMap, baseName);
            if (config.get() != NULL) {
                return config;
            }
            return getFlakeIdGeneratorConfig("default");
        }


        std::shared_ptr<config::ClientFlakeIdGeneratorConfig>
        ClientConfig::getFlakeIdGeneratorConfig(const std::string &name) {
            std::string baseName = internal::partition::strategy::StringPartitioningStrategy::getBaseName(name);
            std::shared_ptr<config::ClientFlakeIdGeneratorConfig> config = internal::config::ConfigUtils::lookupByPattern<config::ClientFlakeIdGeneratorConfig>(
                    configPatternMatcher, flakeIdGeneratorConfigMap, baseName);
            if (config.get() != NULL) {
                return config;
            }
            std::shared_ptr<config::ClientFlakeIdGeneratorConfig> defConfig = flakeIdGeneratorConfigMap.get("default");
            if (defConfig.get() == NULL) {
                defConfig.reset(new config::ClientFlakeIdGeneratorConfig("default"));
                flakeIdGeneratorConfigMap.put(defConfig->getName(), defConfig);
            }
            config.reset(new config::ClientFlakeIdGeneratorConfig(*defConfig));
            config->setName(name);
            flakeIdGeneratorConfigMap.put(config->getName(), config);
            return config;
        }

        ClientConfig &
        ClientConfig::addFlakeIdGeneratorConfig(const std::shared_ptr<config::ClientFlakeIdGeneratorConfig> &config) {
            flakeIdGeneratorConfigMap.put(config->getName(), config);
            return *this;
        }

        const std::unordered_set<std::shared_ptr<MembershipListener> > &ClientConfig::getManagedMembershipListeners() const {
            return managedMembershipListeners;
        }
    }
}
