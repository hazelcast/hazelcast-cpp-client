#include "ClientConfig.h"
#include "impl/RoundRobinLB.h"

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
        , loadBalancer(dynamic_cast<LoadBalancer *>(new impl::RoundRobinLB)) {
        };

        ClientConfig::ClientConfig(ClientConfig& rhs)
        : smart(rhs.smart)
        , redoOperation(rhs.redoOperation)
        , poolSize(rhs.poolSize)
        , connectionTimeout(rhs.connectionTimeout)
        , connectionAttemptLimit(rhs.connectionAttemptLimit)
        , attemptPeriod(rhs.attemptPeriod)
        , groupConfig(rhs.groupConfig)
        , addressList(rhs.addressList)
        , loadBalancer(rhs.loadBalancer.release()) {
        };

        ClientConfig::~ClientConfig() {
            if (credentials != NULL)
                delete credentials;
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

        ClientConfig& ClientConfig::operator = (const ClientConfig& rhs) {
            groupConfig = rhs.groupConfig;
            return (*this);
        };

        GroupConfig& ClientConfig::getGroupConfig() {
            return groupConfig;
        };

        hazelcast::client::protocol::Credentials & ClientConfig::getCredentials() {
            if (credentials == NULL) {
                credentials = new hazelcast::client::protocol::Credentials(groupConfig.getName(), groupConfig.getPassword());
            }
            return *credentials;
        };

        int ClientConfig::getConnectionAttemptLimit() const {
            return connectionAttemptLimit;
        };

        int ClientConfig::getAttemptPeriod() const {
            return attemptPeriod;
        };

        LoadBalancer *const ClientConfig::getLoadBalancer() const {
            return loadBalancer.get();
        };

        void ClientConfig::setLoadBalancer(LoadBalancer *loadBalancer) {
            this->loadBalancer.reset(loadBalancer);
        };

    }
}