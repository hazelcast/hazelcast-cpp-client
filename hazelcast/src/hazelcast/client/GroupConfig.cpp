#include "hazelcast/client/GroupConfig.h"


namespace hazelcast {
    namespace client {

        GroupConfig::GroupConfig() : name("dev"), password("dev-pass") {

        };

        GroupConfig::GroupConfig(std::string name, std::string password) : name(name), password(password) {

        };

        std::string GroupConfig::getName() const {
            return name;
        };

        GroupConfig& GroupConfig::setName(std::string name) {
            this->name = name;
            return (*this);
        };

        GroupConfig& GroupConfig::setPassword(std::string password) {
            this->password = password;
            return (*this);
        };

        std::string GroupConfig::getPassword() const {
            return password;
        };

    }
}

