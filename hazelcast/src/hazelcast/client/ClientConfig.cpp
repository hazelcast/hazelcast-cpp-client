#include "hazelcast/client/ClientConfig.h"

namespace hazelcast {
    namespace client {

        ClientConfig::ClientConfig()
        : smart(true)
        , redoOperation(true)
        , poolSize(100)
        , connectionTimeout(60000)
        , connectionAttemptLimit(2)
        , attemptPeriod(3000)
        , credentials(NULL)
        , loadBalancer(NULL)
        , isDefaultCredentialsInitialized(false)
        , defaultLoadBalancer(new impl::RoundRobinLB) {
        };


        ClientConfig::~ClientConfig() {
            delete defaultLoadBalancer;
        };

        ClientConfig & ClientConfig::addAddress(const Address  & address) {
            addressList.push_back(address);
            return (*this);
        };

        ClientConfig & ClientConfig::addAddresses(const std::vector<Address>  & addresses) {
            addressList.insert(addressList.end(), addresses.begin(), addresses.end());
            return (*this);
        };


        std::vector<Address>  & ClientConfig::getAddresses() {
            return addressList;
        };

        ClientConfig& ClientConfig::setGroupConfig(GroupConfig& groupConfig) {
            this->groupConfig = groupConfig;
            return *this;
        };


        GroupConfig& ClientConfig::getGroupConfig() {
            return groupConfig;
        };


        ClientConfig& ClientConfig::setConnectionAttemptLimit(int connectionAttemptLimit) {
            this->connectionAttemptLimit = connectionAttemptLimit;
            return *this;
        };

        int ClientConfig::getConnectionAttemptLimit() const {
            return connectionAttemptLimit;
        };

        ClientConfig& ClientConfig::setConnectionTimeout(int connectionTimeoutInMillis) {
            this->connectionTimeout = connectionTimeoutInMillis;
            return *this;
        };

        int ClientConfig::getConnectionTimeout() const {
            return connectionTimeout;
        };

        ClientConfig& ClientConfig::setAttemptPeriod(int attemptPeriodInMillis) {
            this->attemptPeriod = attemptPeriodInMillis;
            return *this;
        };

        int ClientConfig::getAttemptPeriod() const {
            return attemptPeriod;
        };

        ClientConfig& ClientConfig::setRedoOperation(bool redoOperation) {
            this->redoOperation = redoOperation;
            return *this;
        };

        bool ClientConfig::isRedoOperation() const {
            return redoOperation;
        };

        LoadBalancer *const ClientConfig::getLoadBalancer() {
            if (loadBalancer == NULL)
                return defaultLoadBalancer;
            return loadBalancer;
        };

        void ClientConfig::setLoadBalancer(LoadBalancer *loadBalancer) {
            this->loadBalancer = loadBalancer;
        };


        ClientConfig& ClientConfig::addListener(spi::EventListener *listener) {
            listeners.insert(listener);
            return *this;
        };

        std::set<spi::EventListener *>  ClientConfig::getListeners() const {
            return listeners;
        };

        void ClientConfig::setCredentials(protocol::Credentials *credentials) {
            this->credentials = credentials;
        };

        protocol::Credentials & ClientConfig::getCredentials() {
            if (credentials == NULL) {
                if (isDefaultCredentialsInitialized)
                    return defaultCredentials;
                defaultCredentials.setPrincipal(groupConfig.getName());
                defaultCredentials.setPassword(groupConfig.getPassword());
                isDefaultCredentialsInitialized = true;
                return defaultCredentials;
            }
            return *credentials;
        };

    }
}