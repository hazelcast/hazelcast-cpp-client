//
// Created by sancar koyunlu on 21/08/14.
//

#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/ClientProperties.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4996) //for strerror
#endif

namespace hazelcast {
    namespace client {

        ClientProperty::ClientProperty(ClientConfig& config, const std::string& name, const std::string& defaultValue)
        : name(name) {
            if (config.getProperties().count(name) > 0) {
                value = config.getProperties()[name];
            } else if (::getenv(name.c_str()) != NULL) {
                value = std::string(::getenv(name.c_str()));
            } else {
                value = defaultValue;
            }
        }

        std::string ClientProperty::getName() const {
            return name;
        }

        std::string ClientProperty::getValue() const {
            return value;
        }

        int ClientProperty::getInteger() const {
            return util::IOUtil::to_value<int>(value);
        }

        byte ClientProperty::getByte() const {
            return util::IOUtil::to_value<byte>(value);
        }

        bool ClientProperty::getBoolean() const {
            return util::IOUtil::to_value<bool>(value);
        }

        std::string ClientProperty::getString() const {
            return value;
        }

        long ClientProperty::getLong() const {
            return util::IOUtil::to_value<long>(value);
        }


        ClientProperties::ClientProperties(ClientConfig& clientConfig)
        : heartbeatTimeout(clientConfig, PROP_HEARTBEAT_TIMEOUT, PROP_HEARTBEAT_TIMEOUT_DEFAULT)
        , heartbeatInterval(clientConfig, PROP_HEARTBEAT_INTERVAL, PROP_HEARTBEAT_INTERVAL_DEFAULT)
        , retryCount(clientConfig, PROP_REQUEST_RETRY_COUNT, PROP_REQUEST_RETRY_COUNT_DEFAULT)
        , retryWaitTime(clientConfig, PROP_REQUEST_RETRY_WAIT_TIME, PROP_REQUEST_RETRY_WAIT_TIME_DEFAULT) {

        }


        const ClientProperty& ClientProperties::getHeartbeatTimeout() const {
            return heartbeatTimeout;
        }

        const ClientProperty& ClientProperties::getHeartbeatInterval() const {
            return heartbeatInterval;
        }

        const ClientProperty& ClientProperties::getRetryCount() const {
            return retryCount;
        }

        const ClientProperty& ClientProperties::getRetryWaitTime() const {
            return retryWaitTime;
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
