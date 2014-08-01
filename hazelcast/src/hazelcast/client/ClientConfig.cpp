#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/SocketInterceptor.h"
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
        , credentials(NULL) {
        }


        ClientConfig::~ClientConfig() {
            if (credentials != NULL)
                delete credentials;
        }

        ClientConfig &ClientConfig::addAddress(const Address &address) {
            addressList.insert(address);
            return (*this);
        }

        ClientConfig &ClientConfig::addAddresses(const std::vector<Address> &addresses) {
            addressList.insert(addresses.begin(), addresses.end());
            return (*this);
        }


        std::set<Address, addressComparator> &ClientConfig::getAddresses() {
            return addressList;
        }

        ClientConfig &ClientConfig::setGroupConfig(GroupConfig &groupConfig) {
            this->groupConfig = groupConfig;
            return *this;
        }


        GroupConfig &ClientConfig::getGroupConfig() {
            return groupConfig;
        }


        ClientConfig &ClientConfig::setConnectionAttemptLimit(int connectionAttemptLimit) {
            this->connectionAttemptLimit = connectionAttemptLimit;
            return *this;
        }

        int ClientConfig::getConnectionAttemptLimit() const {
            return connectionAttemptLimit;
        }

        ClientConfig &ClientConfig::setConnectionTimeout(int connectionTimeoutInMillis) {
            this->connectionTimeout = connectionTimeoutInMillis;
            return *this;
        }

        int ClientConfig::getConnectionTimeout() const {
            return connectionTimeout;
        }

        ClientConfig &ClientConfig::setAttemptPeriod(int attemptPeriodInMillis) {
            this->attemptPeriod = attemptPeriodInMillis;
            return *this;
        }

        int ClientConfig::getAttemptPeriod() const {
            return attemptPeriod;
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
                return defaultLoadBalancer.get();
            return loadBalancer;
        }

        ClientConfig & ClientConfig::setLoadBalancer(LoadBalancer *loadBalancer) {
            this->loadBalancer = loadBalancer;
            return *this;
        }

        ClientConfig &ClientConfig::setLogLevel(LogLevel loggerLevel) {
            util::ILogger::getLogger().setLogLevel(loggerLevel);
            return *this;
        }

        ClientConfig &ClientConfig::addListener(LifecycleListener *listener) {
            lifecycleListeners.insert(listener);
            return *this;
        }

        ClientConfig &ClientConfig::addListener(MembershipListener *listener) {
            membershipListeners.insert(listener);
            return *this;
        }

        ClientConfig &ClientConfig::addListener(InitialMembershipListener *listener) {
            initialMembershipListeners.insert(listener);
            return *this;
        }


        const std::set<LifecycleListener *> &ClientConfig::getLifecycleListeners() const {
            return lifecycleListeners;
        }

        const std::set<MembershipListener *> &ClientConfig::getMembershipListeners() const {
            return membershipListeners;
        }

        const std::set<InitialMembershipListener *> &ClientConfig::getInitialMembershipListeners() const {
            return initialMembershipListeners;
        }

        ClientConfig & ClientConfig::setCredentials(Credentials *credentials) {
            this->credentials = credentials;
            return *this;
        }

        Credentials &ClientConfig::getCredentials() {
            if (credentials == NULL) {
                credentials = new protocol::UsernamePasswordCredentials(groupConfig.getName(), groupConfig.getPassword());
            }
            return *credentials;
        }

        ClientConfig & ClientConfig::setSocketInterceptor(SocketInterceptor *socketInterceptor) {
            this->socketInterceptor.reset(socketInterceptor);
            return *this;
        }

        std::auto_ptr<SocketInterceptor> ClientConfig::getSocketInterceptor() {
            return socketInterceptor;
        }

        ClientConfig & ClientConfig::setSmart(bool smart) {
            this->smart = smart;
            return *this;
        }

        bool ClientConfig::isSmart() const {
            return smart;
        }

        SerializationConfig const &ClientConfig::getSerializationConfig() const {
            return serializationConfig;
        }

        ClientConfig & ClientConfig::setSerializationConfig(SerializationConfig const &serializationConfig){
            this->serializationConfig = serializationConfig;
            return *this;
        }
    }
}
