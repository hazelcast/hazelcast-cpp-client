#include "ClientConfig.h"

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
        , isDefaultCredentialsInitialized(false) {
        };


        ClientConfig::~ClientConfig() {
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

        GroupConfig& ClientConfig::getGroupConfig() {
            return groupConfig;
        };


        int ClientConfig::getConnectionAttemptLimit() const {
            return connectionAttemptLimit;
        };


        int ClientConfig::getConnectionTimeout() const {
            return connectionTimeout;
        }

        int ClientConfig::getAttemptPeriod() const {
            return attemptPeriod;
        };


        bool ClientConfig::isRedoOperation() const {
            return redoOperation;
        };

        LoadBalancer *const ClientConfig::getLoadBalancer() {
            if (loadBalancer == NULL)
                return &defaultLoadBalancer;
            return loadBalancer;
        };

        void ClientConfig::setLoadBalancer(LoadBalancer *loadBalancer) {
            this->loadBalancer = loadBalancer;
        };


        ClientConfig ClientConfig::addListener(spi::EventListener *listener) {
            listeners.insert(listener);
            return *this;
        };

        std::set<spi::EventListener *>  ClientConfig::getListeners() const {
            return listeners;
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

        void ClientConfig::setCredentials(protocol::Credentials *credentials) {
            this->credentials = credentials;
        };


    }
}