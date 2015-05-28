#include "hazelcast/client/GroupConfig.h"


namespace hazelcast {
    namespace client {

        GroupConfig::GroupConfig() : name("dev"), password("dev-pass"), licenseKey("") {

        }

        GroupConfig::GroupConfig(const std::string &name, const std::string &password)
        : name(name)
        , password(password)
        , licenseKey("") {
        }

        std::string GroupConfig::getName() const {
            return name;
        }

        GroupConfig &GroupConfig::setName(const std::string &name) {
            this->name = name;
            return (*this);
        }

        GroupConfig &GroupConfig::setPassword(const std::string &password) {
            this->password = password;
            return (*this);
        }

        std::string GroupConfig::getPassword() const {
            return password;
        }

        const std::string &GroupConfig::getLicenseKey() const {
            return licenseKey;
        }

        void GroupConfig::setLicenseKey(const std::string &key) {
            licenseKey = key;
        }
    }
}


