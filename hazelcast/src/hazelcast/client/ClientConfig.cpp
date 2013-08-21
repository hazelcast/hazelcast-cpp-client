#include "hazelcast/client/ClientConfig.h"
#include "SocketInterceptor.h"

namespace hazelcast {
    namespace client {

        ClientConfig::ClientConfig()
        : smart(true)
        , redoOperation(false) //TODO make it true, after implemented isRedoOperation for request
        , poolSize(100)
        , connectionTimeout(60000)
        , connectionAttemptLimit(2)
        , attemptPeriod(3000)
        , credentials(NULL)
        , loadBalancer(NULL)
        , socketInterceptor(NULL)
        , isDefaultCredentialsInitialized(false)
        , defaultLoadBalancer(new impl::RoundRobinLB) {
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
                return defaultLoadBalancer.get();
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

        void ClientConfig::setSocketInterceptor(connection::SocketInterceptor *socketInterceptor) {
            this->socketInterceptor.reset(socketInterceptor);
        }

        std::auto_ptr<connection::SocketInterceptor> ClientConfig::getSocketInterceptor() {
            return socketInterceptor;
        };

        void ClientConfig::setSmart(bool smart) {
            this->smart = smart;
        }

        bool ClientConfig::isSmart() const {
            return smart;
        }


    }
}