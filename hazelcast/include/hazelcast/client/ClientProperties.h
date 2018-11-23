/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
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

#ifndef HAZELCAST_ClientProperties
#define HAZELCAST_ClientProperties

#include <string>

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/util/IOUtil.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export	
#endif

namespace hazelcast {
    namespace client {
        class ClientConfig;

        /**
        * A client property is a configuration for hazelcast client. For available configurations
        * see ClientProperties class.
        *
        */
        class HAZELCAST_API ClientProperty {
        public:
            ClientProperty(ClientConfig& config, const std::string& name, const std::string& defaultValue);

            std::string getName() const;

            std::string getValue() const;

            template <typename T>
            T get() const {
                return util::IOUtil::to_value<T>(value);
            }

            int getInteger() const;

            bool getBoolean() const;

            int64_t getLong() const;

            const std::string &getDefaultValue() const;

        private:
            std::string name;
            std::string value;
            std::string defaultValue;
        };


        /**
        * Client Properties is an internal class. Only use properties documentations as references to be used
        * in ClientConfig#setProperty
        *
        */
        class HAZELCAST_API ClientProperties {
        public:
            ClientProperties(ClientConfig& clientConfig);

            const ClientProperty& getHeartbeatTimeout() const;

            const ClientProperty& getHeartbeatInterval() const;

            const ClientProperty& getAwsMemberPort() const;

            const ClientProperty &getCleanResourcesPeriodMillis() const;

            const ClientProperty &getInvocationRetryPauseMillis() const;

            const ClientProperty &getInvocationTimeoutSeconds() const;

            const ClientProperty &getEventThreadCount() const;

            const ClientProperty &getEventQueueCapacity() const;

            const ClientProperty &getInternalExecutorPoolSize() const;

            const ClientProperty &getShuffleMemberList() const;

            const ClientProperty &getMaxConcurrentInvocations() const;

            const ClientProperty &getBackpressureBackoffTimeoutMillis() const;

            const ClientProperty &getStatisticsEnabled() const;

            const ClientProperty &getStatisticsPeriodSeconds() const;

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

            /**
             * The discovery mechanism will discover only IP addresses. You can define the port on which Hazelcast is expected to be
             * running here. This port number is not used by the discovery mechanism itself, it is only returned by the discovery
             * mechanism. The default port is {@link PROP_AWS_MEMBER_PORT_DEFAULT}
             */
            static const std::string PROP_AWS_MEMBER_PORT;
            static const std::string PROP_AWS_MEMBER_PORT_DEFAULT;

            /**
             * The period in milliseconds at which the resource cleaning is run (e.g. invocations).
             */
            static const std::string CLEAN_RESOURCES_PERIOD_MILLIS;
            static const std::string CLEAN_RESOURCES_PERIOD_MILLIS_DEFAULT;

            /**
             * Pause time between each retry cycle of an invocation in milliseconds.
             */
            static const std::string INVOCATION_RETRY_PAUSE_MILLIS;
            static const std::string INVOCATION_RETRY_PAUSE_MILLIS_DEFAULT;

            /**
             * When an invocation gets an exception because :
             * - Member throws an exception.
             * - Connection between the client and member is closed.
             * - Client's heartbeat requests are timed out.
             * Time passed since invocation started is compared with this property.
             * If the time is already passed, then the exception is delegated to the user. If not, the invocation is retried.
             * Note that, if invocation gets no exception and it is a long running one, then it will not get any exception,
             * no matter how small this timeout is set.
             */
            static const std::string INVOCATION_TIMEOUT_SECONDS;
            static const std::string INVOCATION_TIMEOUT_SECONDS_DEFAULT;

            /**
             * Number of the threads to handle the incoming event packets.
             */
            static const std::string EVENT_THREAD_COUNT;
            static const std::string EVENT_THREAD_COUNT_DEFAULT;

            /**
             * Capacity of the executor that handles the incoming event packets.
             */
            static const std::string EVENT_QUEUE_CAPACITY;
            static const std::string EVENT_QUEUE_CAPACITY_DEFAULT;

            static const std::string INTERNAL_EXECUTOR_POOL_SIZE;
            static const std::string INTERNAL_EXECUTOR_POOL_SIZE_DEFAULT;

            /**
             * Client shuffles the given member list to prevent all clients to connect to the same node when
             * this property is set to true. When it is set to false, the client tries to connect to the nodes
             * in the given order.
             */
            static const std::string SHUFFLE_MEMBER_LIST;
            static const std::string SHUFFLE_MEMBER_LIST_DEFAULT;

            /**
             * The maximum number of concurrent invocations allowed.
             * <p/>
             * To prevent the system from overloading, user can apply a constraint on the number of concurrent invocations.
             * If the maximum number of concurrent invocations has been exceeded and a new invocation comes in,
             * then hazelcast will throw HazelcastOverloadException
             * <p/>
             * By default it is configured as INT32_MAX.
             */
            static const std::string MAX_CONCURRENT_INVOCATIONS;
            static const std::string MAX_CONCURRENT_INVOCATIONS_DEFAULT;

            /**
             * Control the maximum timeout in millis to wait for an invocation space to be available.
             * <p/>
             * If an invocation can't be made because there are too many pending invocations, then an exponential backoff is done
             * to give the system time to deal with the backlog of invocations. This property controls how long an invocation is
             * allowed to wait before getting a {@link com.hazelcast.core.HazelcastOverloadException}.
             * <p/>
             * <p>
             * When set to -1 then <code>HazelcastOverloadException</code> is thrown immediately without any waiting.
             * </p>
             */
            static const std::string BACKPRESSURE_BACKOFF_TIMEOUT_MILLIS;
            static const std::string BACKPRESSURE_BACKOFF_TIMEOUT_MILLIS_DEFAULT;

            /**
             * Use to enable the client statistics collection.
             * <p>
             * The default is false.
             */
            static const std::string STATISTICS_ENABLED;
            static const std::string STATISTICS_ENABLED_DEFAULT;

            /**
             * The period in seconds the statistics sent to the cluster.
             */
            static const std::string STATISTICS_PERIOD_SECONDS;
            static const std::string STATISTICS_PERIOD_SECONDS_DEFAULT;


        private:
            ClientProperty heartbeatTimeout;
            ClientProperty heartbeatInterval;
            ClientProperty retryCount;
            ClientProperty retryWaitTime;
            ClientProperty awsMemberPort;
            ClientProperty cleanResourcesPeriod;
            ClientProperty invocationRetryPauseMillis;
            ClientProperty invocationTimeoutSeconds;
            ClientProperty eventThreadCount;
            ClientProperty eventQueueCapacity;
            ClientProperty internalExecutorPoolSize;
            ClientProperty shuffleMemberList;
            ClientProperty maxConcurrentInvocations;
            ClientProperty backpressureBackoffTimeoutMillis;
            ClientProperty statisticsEnabled;
            ClientProperty statisticsPeriodSeconds;
        };

    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_ClientProperties
