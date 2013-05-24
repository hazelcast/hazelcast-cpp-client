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
        , credentials(NULL){
        };

        ClientConfig::ClientConfig(const ClientConfig& rhs)
        : smart(rhs.smart)
        , redoOperation(rhs.redoOperation)
        , poolSize(rhs.poolSize)
        , connectionTimeout(rhs.connectionTimeout)
        , connectionAttemptLimit(rhs.connectionAttemptLimit)
        , attemptPeriod(rhs.attemptPeriod)
        , groupConfig(rhs.groupConfig)
        , addressList(rhs.addressList){
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

        ClientConfig::~ClientConfig() {
            if(credentials != NULL)
                delete credentials;
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

    }
}