#include <map>
#include "ClientConfig.h"

namespace hazelcast {
    namespace client {

        ClientConfig::ClientConfig(const Address& address):address(address) {
        };

        ClientConfig::ClientConfig(const ClientConfig& rhs)
        :address(address)
        , groupConfig(groupConfig)
        , portableFactories(portableFactories) {
        };

        ClientConfig::~ClientConfig() {

        };

        ClientConfig& ClientConfig::operator = (const ClientConfig& rhs) {
            groupConfig = rhs.groupConfig;
            address = rhs.address;
            portableFactories = rhs.portableFactories;
            return (*this);
        };

        GroupConfig& ClientConfig::getGroupConfig() {
            return groupConfig;
        };

        Address ClientConfig::getAddress() const {
            return address;
        };

        std::map< int, serialization::PortableFactory const * > const *const ClientConfig::getPortableFactories() {
            return &portableFactories;
        };

        void ClientConfig::addPortableFactory(int factoryId, serialization::PortableFactory *portableFactory) {
            portableFactories[factoryId] = portableFactory;
        };

    }
}