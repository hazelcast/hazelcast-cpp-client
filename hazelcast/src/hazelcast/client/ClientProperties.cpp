/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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

        const std::string ClientProperties::CLEAN_RESOURCES_PERIOD_MILLIS = "hazelcast.client.internal.clean.resources.millis";
        const std::string ClientProperties::CLEAN_RESOURCES_PERIOD_MILLIS_DEFAULT = "100";

        const std::string ClientProperties::INVOCATION_RETRY_PAUSE_MILLIS = "hazelcast.client.invocation.retry.pause.millis";
        const std::string ClientProperties::INVOCATION_RETRY_PAUSE_MILLIS_DEFAULT = "1000";

        const std::string ClientProperties::INVOCATION_TIMEOUT_SECONDS = "hazelcast.client.invocation.timeout.seconds";
        const std::string ClientProperties::INVOCATION_TIMEOUT_SECONDS_DEFAULT = "120";

        const std::string ClientProperties::EVENT_THREAD_COUNT = "hazelcast.client.event.thread.count";
        const std::string ClientProperties::EVENT_THREAD_COUNT_DEFAULT = "5";

        const std::string ClientProperties::EVENT_QUEUE_CAPACITY = "hazelcast.client.event.queue.capacity";
        const std::string ClientProperties::EVENT_QUEUE_CAPACITY_DEFAULT = "1000000";

        const std::string ClientProperties::INTERNAL_EXECUTOR_POOL_SIZE = "hazelcast.client.internal.executor.pool.size";
        const std::string ClientProperties::INTERNAL_EXECUTOR_POOL_SIZE_DEFAULT = "3";

        const std::string ClientProperties::SHUFFLE_MEMBER_LIST = "hazelcast.client.shuffle.member.list";
        const std::string ClientProperties::SHUFFLE_MEMBER_LIST_DEFAULT = "true";

        const std::string ClientProperties::MAX_CONCURRENT_INVOCATIONS = "hazelcast.client.max.concurrent.invocations";
        const std::string ClientProperties::MAX_CONCURRENT_INVOCATIONS_DEFAULT = util::IOUtil::to_string<int32_t>(
                INT32_MAX);

        const std::string ClientProperties::BACKPRESSURE_BACKOFF_TIMEOUT_MILLIS = "hazelcast.client.invocation.backoff.timeout.millis";
        const std::string ClientProperties::BACKPRESSURE_BACKOFF_TIMEOUT_MILLIS_DEFAULT = "-1";

        const std::string ClientProperties::STATISTICS_ENABLED = "hazelcast.client.statistics.enabled";
        const std::string ClientProperties::STATISTICS_ENABLED_DEFAULT = "false";

        const std::string ClientProperties::STATISTICS_PERIOD_SECONDS = "hazelcast.client.statistics.period.seconds";
        const std::string ClientProperties::STATISTICS_PERIOD_SECONDS_DEFAULT = "3";

        ClientProperty::ClientProperty(const std::string &name, const std::string &defaultValue)
                : name(name), defaultValue(defaultValue) {
        }

        const std::string &ClientProperty::getName() const {
            return name;
        }

        const std::string &ClientProperty::getDefaultValue() const {
            return defaultValue;
        }

        const char *ClientProperty::getSystemProperty() const {
            return ::getenv(name.c_str());
        }

        ClientProperties::ClientProperties(const std::map<std::string, std::string> &properties)
                : heartbeatTimeout(PROP_HEARTBEAT_TIMEOUT, PROP_HEARTBEAT_TIMEOUT_DEFAULT),
                  heartbeatInterval(PROP_HEARTBEAT_INTERVAL, PROP_HEARTBEAT_INTERVAL_DEFAULT),
                  retryCount(PROP_REQUEST_RETRY_COUNT, PROP_REQUEST_RETRY_COUNT_DEFAULT),
                  retryWaitTime(PROP_REQUEST_RETRY_WAIT_TIME, PROP_REQUEST_RETRY_WAIT_TIME_DEFAULT),
                  awsMemberPort(PROP_AWS_MEMBER_PORT, PROP_AWS_MEMBER_PORT_DEFAULT),
                  cleanResourcesPeriod(CLEAN_RESOURCES_PERIOD_MILLIS,
                                       CLEAN_RESOURCES_PERIOD_MILLIS_DEFAULT),
                  invocationRetryPauseMillis(INVOCATION_RETRY_PAUSE_MILLIS,
                                             INVOCATION_RETRY_PAUSE_MILLIS_DEFAULT),
                  invocationTimeoutSeconds(INVOCATION_TIMEOUT_SECONDS,
                                           INVOCATION_TIMEOUT_SECONDS_DEFAULT),
                  eventThreadCount(EVENT_THREAD_COUNT, EVENT_THREAD_COUNT_DEFAULT),
                  eventQueueCapacity(EVENT_QUEUE_CAPACITY, EVENT_QUEUE_CAPACITY_DEFAULT),
                  internalExecutorPoolSize(INTERNAL_EXECUTOR_POOL_SIZE,
                                           INTERNAL_EXECUTOR_POOL_SIZE_DEFAULT),
                  shuffleMemberList(SHUFFLE_MEMBER_LIST, SHUFFLE_MEMBER_LIST_DEFAULT),
                  maxConcurrentInvocations(MAX_CONCURRENT_INVOCATIONS,
                                           MAX_CONCURRENT_INVOCATIONS_DEFAULT),
                  backpressureBackoffTimeoutMillis(BACKPRESSURE_BACKOFF_TIMEOUT_MILLIS,
                                                   BACKPRESSURE_BACKOFF_TIMEOUT_MILLIS_DEFAULT),
                  statisticsEnabled(STATISTICS_ENABLED, STATISTICS_ENABLED_DEFAULT),
                  statisticsPeriodSeconds(STATISTICS_PERIOD_SECONDS, STATISTICS_PERIOD_SECONDS_DEFAULT),
                  propertiesMap(properties) {
        }

        const ClientProperty &ClientProperties::getHeartbeatTimeout() const {
            return heartbeatTimeout;
        }

        const ClientProperty &ClientProperties::getHeartbeatInterval() const {
            return heartbeatInterval;
        }

        const ClientProperty &ClientProperties::getAwsMemberPort() const {
            return awsMemberPort;
        }

        const ClientProperty &ClientProperties::getCleanResourcesPeriodMillis() const {
            return cleanResourcesPeriod;
        }

        const ClientProperty &ClientProperties::getInvocationRetryPauseMillis() const {
            return invocationRetryPauseMillis;
        }

        const ClientProperty &ClientProperties::getInvocationTimeoutSeconds() const {
            return invocationTimeoutSeconds;
        }

        const ClientProperty &ClientProperties::getEventThreadCount() const {
            return eventThreadCount;
        }

        const ClientProperty &ClientProperties::getEventQueueCapacity() const {
            return eventQueueCapacity;
        }

        const ClientProperty &ClientProperties::getInternalExecutorPoolSize() const {
            return internalExecutorPoolSize;
        }

        const ClientProperty &ClientProperties::getShuffleMemberList() const {
            return shuffleMemberList;
        }

        const ClientProperty &ClientProperties::getMaxConcurrentInvocations() const {
            return maxConcurrentInvocations;
        }

        const ClientProperty &ClientProperties::getBackpressureBackoffTimeoutMillis() const {
            return backpressureBackoffTimeoutMillis;
        }

        const ClientProperty &ClientProperties::getStatisticsEnabled() const {
            return statisticsEnabled;
        }

        const ClientProperty &ClientProperties::getStatisticsPeriodSeconds() const {
            return statisticsPeriodSeconds;
        }

        std::string ClientProperties::getString(const ClientProperty &property) const {
            std::map<std::string, std::string>::const_iterator valueIt = propertiesMap.find(property.getName());
            if (valueIt != propertiesMap.end()) {
                return valueIt->second;
            }

            const char *value = property.getSystemProperty();
            if (value != NULL) {
                return value;
            }

            return property.getDefaultValue();
        }

        bool ClientProperties::getBoolean(const ClientProperty &property) const {
            return util::IOUtil::to_value<bool>(getString(property));
        }

        int32_t ClientProperties::getInteger(const ClientProperty &property) const {
            return util::IOUtil::to_value<int32_t>(getString(property));
        }

        int64_t ClientProperties::getLong(const ClientProperty &property) const {
            return util::IOUtil::to_value<int64_t>(getString(property));
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

