/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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


#include "hazelcast/client/internal/partition/strategy/StringPartitioningStrategy.h"
#include "hazelcast/client/internal/config/ConfigUtils.h"
#include "hazelcast/client/config/ClientConnectionStrategyConfig.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/LifecycleListener.h"

namespace hazelcast {
    namespace client {

        ClientConfig::ClientConfig()
                : loadBalancer(NULL), redoOperation(false), socketInterceptor(NULL), credentials(NULL),
                  executorPoolSize(-1) {
        }

        ClientConfig &ClientConfig::addAddress(const Address &address) {
            networkConfig.addAddress(address);
            return (*this);
        }

        ClientConfig &ClientConfig::addAddresses(const std::vector<Address> &addresses) {
            networkConfig.addAddresses(addresses);
            return (*this);
        }


        std::set<Address> ClientConfig::getAddresses() {
            std::set<Address> result;
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
                throw exception::NullPointerException("ClientConfig::addListener(MembershipListener *)",
                                                      "listener can't be null");
            }

            membershipListeners.insert(listener);
            managedMembershipListeners.insert(
                    std::shared_ptr<MembershipListener>(new MembershipListenerDelegator(listener)));
            return *this;
        }

        ClientConfig &ClientConfig::addListener(InitialMembershipListener *listener) {
            if (listener == NULL) {
                throw exception::NullPointerException("ClientConfig::addListener(InitialMembershipListener *)",
                                                      "listener can't be null");
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

        const std::set<LifecycleListener *> &ClientConfig::getLifecycleListeners() const {
            return lifecycleListeners;
        }

        const std::set<MembershipListener *> &ClientConfig::getMembershipListeners() const {
            return membershipListeners;
        }

        ClientConfig &ClientConfig::setCredentials(Credentials *credentials) {
            this->credentials = credentials;
            return *this;
        }

        const Credentials *ClientConfig::getCredentials() {
            return credentials;
        }

        ClientConfig &ClientConfig::setSocketInterceptor(SocketInterceptor *socketInterceptor) {
            this->socketInterceptor = socketInterceptor;
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


        std::map<std::string, std::string> &ClientConfig::getProperties() {
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
            std::map<std::string, config::ReliableTopicConfig>::const_iterator it = reliableTopicConfigMap.find(name);
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

        const std::shared_ptr<mixedtype::config::MixedNearCacheConfig>
        ClientConfig::getMixedNearCacheConfig(const std::string &name) {
            return std::static_pointer_cast<mixedtype::config::MixedNearCacheConfig>(
                    getNearCacheConfig<TypedData, TypedData>(name));
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

        std::shared_ptr<config::ClientFlakeIdGeneratorConfig> ClientConfig::findFlakeIdGeneratorConfig(const std::string &name) {
            std::string baseName = internal::partition::strategy::StringPartitioningStrategy::getBaseName(name);
            std::shared_ptr<config::ClientFlakeIdGeneratorConfig> config = internal::config::ConfigUtils::lookupByPattern<config::ClientFlakeIdGeneratorConfig>(
                    configPatternMatcher, flakeIdGeneratorConfigMap, baseName);
            if (config.get() != NULL) {
                return config;
            }
            return getFlakeIdGeneratorConfig("default");
        }


        std::shared_ptr<config::ClientFlakeIdGeneratorConfig> ClientConfig::getFlakeIdGeneratorConfig(const std::string &name) {
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

        ClientConfig &ClientConfig::addFlakeIdGeneratorConfig(const std::shared_ptr<config::ClientFlakeIdGeneratorConfig> &config) {
            flakeIdGeneratorConfigMap.put(config->getName(), config);
            return *this;
        }

        const std::set<std::shared_ptr<MembershipListener> > &ClientConfig::getManagedMembershipListeners() const {
            return managedMembershipListeners;
        }

    }
}
