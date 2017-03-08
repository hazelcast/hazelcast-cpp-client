/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
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
#include "hazelcast/client/config/NearCacheConfig.h"
#include "hazelcast/client/LifecycleListener.h"
#include "hazelcast/client/InitialMembershipListener.h"
#include "hazelcast/client/protocol/UsernamePasswordCredentials.h"

namespace hazelcast {
    namespace client {

        ClientConfig::ClientConfig()
        : loadBalancer(NULL)
        , defaultLoadBalancer(new impl::RoundRobinLB)
        , smart(true)
        , redoOperation(false)
        , connectionTimeout(5000)
        , connectionAttemptLimit(2)
        , attemptPeriod(3000)
        , socketInterceptor(NULL)
        , credentials(NULL)
        , defaultCredentials(NULL){
        }

        ClientConfig& ClientConfig::addAddress(const Address& address) {
            addressList.insert(address);
            return (*this);
        }

        ClientConfig& ClientConfig::addAddresses(const std::vector<Address>& addresses) {
            addressList.insert(addresses.begin(), addresses.end());
            return (*this);
        }


        std::set<Address, addressComparator>& ClientConfig::getAddresses() {
            return addressList;
        }

        ClientConfig& ClientConfig::setGroupConfig(GroupConfig& groupConfig) {
            this->groupConfig = groupConfig;
            return *this;
        }


        GroupConfig& ClientConfig::getGroupConfig() {
            return groupConfig;
        }


        ClientConfig& ClientConfig::setConnectionAttemptLimit(int connectionAttemptLimit) {
            this->connectionAttemptLimit = connectionAttemptLimit;
            return *this;
        }

        int ClientConfig::getConnectionAttemptLimit() const {
            return connectionAttemptLimit;
        }

        ClientConfig& ClientConfig::setConnectionTimeout(int connectionTimeoutInMillis) {
            this->connectionTimeout = connectionTimeoutInMillis;
            return *this;
        }

        int ClientConfig::getConnectionTimeout() const {
            return connectionTimeout;
        }

        ClientConfig& ClientConfig::setAttemptPeriod(int attemptPeriodInMillis) {
            this->attemptPeriod = attemptPeriodInMillis;
            return *this;
        }

        int ClientConfig::getAttemptPeriod() const {
            return attemptPeriod;
        }

        ClientConfig& ClientConfig::setRedoOperation(bool redoOperation) {
            this->redoOperation = redoOperation;
            return *this;
        }

        bool ClientConfig::isRedoOperation() const {
            return redoOperation;
        }

        LoadBalancer *const ClientConfig::getLoadBalancer() {
            if (loadBalancer == NULL)
                return defaultLoadBalancer.get();
            return loadBalancer;
        }

        ClientConfig& ClientConfig::setLoadBalancer(LoadBalancer *loadBalancer) {
            this->loadBalancer = loadBalancer;
            return *this;
        }

        ClientConfig& ClientConfig::setLogLevel(LogLevel loggerLevel) {
            util::ILogger::getLogger().setLogLevel(loggerLevel);
            return *this;
        }

        ClientConfig& ClientConfig::addListener(LifecycleListener *listener) {
            lifecycleListeners.insert(listener);
            return *this;
        }

        ClientConfig& ClientConfig::addListener(MembershipListener *listener) {
            membershipListeners.insert(listener);
            return *this;
        }

        ClientConfig& ClientConfig::addListener(InitialMembershipListener *listener) {
            initialMembershipListeners.insert(listener);
            return *this;
        }


        const std::set<LifecycleListener *>& ClientConfig::getLifecycleListeners() const {
            return lifecycleListeners;
        }

        const std::set<MembershipListener *>& ClientConfig::getMembershipListeners() const {
            return membershipListeners;
        }

        const std::set<InitialMembershipListener *>& ClientConfig::getInitialMembershipListeners() const {
            return initialMembershipListeners;
        }

        ClientConfig& ClientConfig::setCredentials(Credentials *credentials) {
            this->credentials = credentials;
            return *this;
        }

        const Credentials *ClientConfig::getCredentials() {
            return defaultCredentials.get();
        }

        ClientConfig& ClientConfig::setSocketInterceptor(SocketInterceptor *socketInterceptor) {
            this->socketInterceptor = socketInterceptor;
            return *this;
        }

        SocketInterceptor* ClientConfig::getSocketInterceptor() {
            return socketInterceptor;
        }

        ClientConfig& ClientConfig::setSmart(bool smart) {
            this->smart = smart;
            return *this;
        }

        bool ClientConfig::isSmart() const {
            return smart;
        }

        SerializationConfig const& ClientConfig::getSerializationConfig() const {
            return serializationConfig;
        }

        ClientConfig& ClientConfig::setSerializationConfig(SerializationConfig const& serializationConfig) {
            this->serializationConfig = serializationConfig;
            return *this;
        }


        std::map<std::string, std::string>& ClientConfig::getProperties() {
            return properties;
        }

        ClientConfig& ClientConfig::setProperty(const std::string& name, const std::string& value) {
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
    }
}
