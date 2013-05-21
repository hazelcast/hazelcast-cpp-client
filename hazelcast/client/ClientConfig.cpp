#include "ClientConfig.h"

namespace hazelcast {
    namespace client {

        ClientConfig::ClientConfig() {
        };

        ClientConfig::ClientConfig(const ClientConfig& rhs) {
        };

        ClientConfig::~ClientConfig() {
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
        }


    }
}