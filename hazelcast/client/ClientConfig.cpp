//#include "ClientConfig.h"
//
//namespace hazelcast {
//    namespace client {
//
//        ClientConfig::ClientConfig(const Address& address):address(address) {
//        };
//
//        ClientConfig::ClientConfig(const ClientConfig& rhs)
//        :address(address)
//        , groupConfig(groupConfig) {
//        };
//
//        ClientConfig::~ClientConfig() {
//
//        };
//
//        ClientConfig& ClientConfig::operator = (const ClientConfig& rhs) {
//            groupConfig = rhs.groupConfig;
//            address = rhs.address;
//            return (*this);
//        };
//
//        GroupConfig& ClientConfig::getGroupConfig() {
//            return groupConfig;
//        };
//
//        Address ClientConfig::getAddress() const {
//            return address;
//        };
//
//    }
//}