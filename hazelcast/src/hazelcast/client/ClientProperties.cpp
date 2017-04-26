/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
//
// Created by sancar koyunlu on 21/08/14.
//

#include "hazelcast/util/IOUtil.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/ClientProperties.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4996) //for strerror	
#endif

namespace hazelcast {
    namespace client {

        const std::string ClientProperties::PROP_HEARTBEAT_TIMEOUT = "hazelcast_client_heartbeat_timeout";
        const std::string ClientProperties::PROP_HEARTBEAT_TIMEOUT_DEFAULT = "60";
        const std::string ClientProperties::PROP_HEARTBEAT_INTERVAL = "hazelcast_client_heartbeat_interval";
        const std::string ClientProperties::PROP_HEARTBEAT_INTERVAL_DEFAULT = "10";
        const std::string ClientProperties::PROP_REQUEST_RETRY_COUNT = "hazelcast_client_request_retry_count";
        const std::string ClientProperties::PROP_REQUEST_RETRY_COUNT_DEFAULT = "20";
        const std::string ClientProperties::PROP_REQUEST_RETRY_WAIT_TIME = "hazelcast_client_request_retry_wait_time";
        const std::string ClientProperties::PROP_REQUEST_RETRY_WAIT_TIME_DEFAULT = "1";

        const std::string ClientProperties::PROP_AWS_MEMBER_PORT = "hz-port";
        const std::string ClientProperties::PROP_AWS_MEMBER_PORT_DEFAULT = "5701";

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
        , retryWaitTime(clientConfig, PROP_REQUEST_RETRY_WAIT_TIME, PROP_REQUEST_RETRY_WAIT_TIME_DEFAULT)
        , awsMemberPort(clientConfig, PROP_AWS_MEMBER_PORT, PROP_AWS_MEMBER_PORT_DEFAULT) {

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

        const ClientProperty& ClientProperties::getAwsMemberPort() const {
            return awsMemberPort;
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

