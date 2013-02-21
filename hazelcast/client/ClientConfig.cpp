#include "ClientConfig.h"

namespace hazelcast {
    namespace client {

        ClientConfig::ClientConfig() {

        };

        ClientConfig::ClientConfig(const ClientConfig& rhs) {
            groupConfig = rhs.groupConfig;
            address = rhs.address;
            portableFactory = rhs.portableFactory;
        };

        ClientConfig::~ClientConfig() {

        };

        ClientConfig& ClientConfig::operator = (const ClientConfig& rhs) {
            groupConfig = rhs.groupConfig;
            address = rhs.address;
            portableFactory = rhs.portableFactory;
            return (*this);
        };

        GroupConfig& ClientConfig::getGroupConfig() {
            return groupConfig;
        };

        void ClientConfig::setAddress(std::string addressStr) {//TODO if address is not set
            int middle = addressStr.find_first_of(':', 0);
            std::string address = addressStr.substr(0, middle);
            std::string port = addressStr.substr(middle + 1, addressStr.length() - middle);
            this->address.setAddress(address);
            this->address.setPort(port);
        };

        Address ClientConfig::getAddress() const {
            return address;
        };

        serialization::PortableFactory const *ClientConfig::getPortableFactory() const {
            return portableFactory;
        };

        void ClientConfig::setPortableFactory(serialization::PortableFactory *portableFactory) {
            this->portableFactory = portableFactory;
        };

    }
}