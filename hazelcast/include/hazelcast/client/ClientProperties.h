//
// Created by sancar koyunlu on 21/08/14.
//


#ifndef HAZELCAST_ClientProperties
#define HAZELCAST_ClientProperties

#include "hazelcast/util/HazelcastDll.h"
#include <string>
#include <map>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export	
#endif

namespace hazelcast {
    namespace client {
        class ClientConfig;

        class HAZELCAST_API ClientProperty {
        public:
            ClientProperty(ClientConfig& config, const std::string& name, const std::string& defaultValue);

            std::string getName() const;

            std::string getValue() const;

            int getInteger() const;

            byte getByte() const;

            bool getBoolean() const;

            std::string getString() const;

            long getLong() const;

        private:
            std::string name;
            std::string value;
        };


        class HAZELCAST_API ClientProperties {
        public:
            ClientProperties(ClientConfig& clientConfig);

            const ClientProperty& getHeartbeatTimeout() const;

            const ClientProperty& getHeartbeatInterval() const;

            const ClientProperty& getRetryCount() const;

            const ClientProperty& getRetryWaitTime() const;


            /**
            * Client will be sending heartbeat messages to members and this is the timeout. If there is no any message
            * passing between client and member within the given time via this property in seconds the connection
            * will be closed.
            *
            * attribute      "hazelcast_client_heartbeat_timeout"
            * default value  "60"
            */
            static const std::string PROP_HEARTBEAT_TIMEOUT;
            static const std::string PROP_HEARTBEAT_TIMEOUT_DEFAULT;

            /**
            * Time interval in seconds between heartbeats to nodes from client
            *
            * attribute      "hazelcast_client_heartbeat_interval"
            * default value  "10"
            */
            static const std::string PROP_HEARTBEAT_INTERVAL;
            static const std::string PROP_HEARTBEAT_INTERVAL_DEFAULT;

            /**
            * Client will retry requests which either inherently retryable(idempotent client)
            * or {@link ClientNetworkConfig#redoOperation} is set to true_
            * <p/>
            * This property is to configure retry count before client give up retrying
            *
            * attribute      "hazelcast_client_request_retry_count"
            * default value  "20"
            */
            static const std::string PROP_REQUEST_RETRY_COUNT;
            static const std::string PROP_REQUEST_RETRY_COUNT_DEFAULT;

            /**
            * Client will retry requests which either inherently retryable(idempotent client)
            * or {@link ClientNetworkConfig#redoOperation} is set to true.
            * <p/>
            * Time delay in seconds between retries.
            *
            * attribute      "hazelcast_client_request_retry_wait_time"
            * default value  "1"
            */
            static const std::string PROP_REQUEST_RETRY_WAIT_TIME;
            static const std::string PROP_REQUEST_RETRY_WAIT_TIME_DEFAULT;
        private:
            ClientProperty heartbeatTimeout;
            ClientProperty heartbeatInterval;
            ClientProperty retryCount;
            ClientProperty retryWaitTime;
        };

    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_ClientProperties
