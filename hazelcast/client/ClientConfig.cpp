#include <map>
#include "ClientConfig.h"

namespace hazelcast {
    namespace client {

        ClientConfig::ClientConfig() {

        };

        ClientConfig::ClientConfig(const ClientConfig& rhs) {
            groupConfig = rhs.groupConfig;
            address = rhs.address;
            portableFactories = rhs.portableFactories;
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

        void ClientConfig::setAddress(std::string address, std::string port) {//TODO if address is not set
//            int middle = addressStr.find_first_of(':', 0);
//            std::string address = addressStr.substr(0, middle);
//            std::string port = addressStr.substr(middle + 1, addressStr.length() - middle);
            this->address.setAddress(address);
            this->address.setPort(port);
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