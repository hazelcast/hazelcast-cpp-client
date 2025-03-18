/*
 * Copyright (c) 2008-2023, Hazelcast, Inc. All Rights Reserved.
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

#pragma once

#include <string>
#include <unordered_map>

#include "hazelcast/util/export.h"
#include "hazelcast/util/IOUtil.h"

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4251) // for dll export
#endif

namespace hazelcast {
namespace client {
class client_config;

/**
 * A client property is a configuration for hazelcast client. For available
 * configurations see client_properties class.
 *
 */
class HAZELCAST_API client_property
{
public:
    client_property(const std::string& name, const std::string& default_value);

    const std::string& get_name() const;

    const std::string& get_default_value() const;

    /**
     * Gets the system environment property value of the property.
     *
     * @return the value of the property. NULL if no such environment property
     * exist.
     */
    const char* get_system_property() const;

private:
    std::string name_;
    std::string default_value_;
};

/**
 * Client Properties is an internal class. Only use properties documentations as
 * references to be used in client_config#set_property
 *
 */
class HAZELCAST_API client_properties
{
public:
    client_properties(
      const std::unordered_map<std::string, std::string>& properties);

    const client_property& get_heartbeat_timeout() const;

    const client_property& get_heartbeat_interval() const;

    const client_property& get_aws_member_port() const;

    const client_property& get_invocation_retry_pause_millis() const;

    const client_property& get_invocation_timeout_seconds() const;

    const client_property& get_event_thread_count() const;

    const client_property& get_internal_executor_pool_size() const;

    const client_property& get_shuffle_member_list() const;

    const client_property& get_max_concurrent_invocations() const;

    const client_property& get_backpressure_backoff_timeout_millis() const;

    const client_property& get_statistics_enabled() const;

    const client_property& get_statistics_period_seconds() const;

    const client_property& backup_timeout_millis() const;

    const client_property& fail_on_indeterminate_state() const;

    const client_property& cloud_base_url() const;

    const client_property& partition_arg_cache_size() const;

    /**
     * Client will be sending heartbeat messages to members and this is the
     * timeout. If there is no any message passing between client and member
     * within the given time via this property in milliseconds the connection
     * will be closed.
     *
     * attribute      "hazelcast_client_heartbeat_timeout"
     * default value  "60000"
     */
    static const std::string PROP_HEARTBEAT_TIMEOUT;
    static const std::string PROP_HEARTBEAT_TIMEOUT_DEFAULT;

    /**
     * Time interval in milliseconds between the heartbeats sent by the client
     * to the nodes.
     *
     * attribute      "hazelcast_client_heartbeat_interval"
     * default value  "5000"
     */
    static const std::string PROP_HEARTBEAT_INTERVAL;
    static const std::string PROP_HEARTBEAT_INTERVAL_DEFAULT;

    /**
     * Client will retry requests which either inherently retryable(idempotent
     * client) or {@link client_network_config#redoOperation} is set to true_
     * <p/>
     * This property is to configure retry count before client give up retrying
     *
     * attribute      "hazelcast_client_request_retry_count"
     * default value  "20"
     */
    static const std::string PROP_REQUEST_RETRY_COUNT;
    static const std::string PROP_REQUEST_RETRY_COUNT_DEFAULT;

    /**
     * Client will retry requests which either inherently retryable(idempotent
     * client) or {@link client_network_config#redoOperation} is set to true.
     * <p/>
     * Time delay in seconds between retries.
     *
     * attribute      "hazelcast_client_request_retry_wait_time"
     * default value  "1"
     */
    static const std::string PROP_REQUEST_RETRY_WAIT_TIME;
    static const std::string PROP_REQUEST_RETRY_WAIT_TIME_DEFAULT;

    /**
     * The discovery mechanism will discover only IP addresses. You can define
     * the port on which Hazelcast is expected to be running here. This port
     * number is not used by the discovery mechanism itself, it is only returned
     * by the discovery mechanism. The default port is {@link
     * PROP_AWS_MEMBER_PORT_DEFAULT}
     */
    static const std::string PROP_AWS_MEMBER_PORT;
    static const std::string PROP_AWS_MEMBER_PORT_DEFAULT;

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
     * If the time is already passed, then the exception is delegated to the
     * user. If not, the invocation is retried. Note that, if invocation gets no
     * exception and it is a long running one, then it will not get any
     * exception, no matter how small this timeout is set.
     */
    static const std::string INVOCATION_TIMEOUT_SECONDS;
    static const std::string INVOCATION_TIMEOUT_SECONDS_DEFAULT;

    /**
     * Number of the threads to handle the incoming event packets.
     */
    static const std::string EVENT_THREAD_COUNT;
    static const std::string EVENT_THREAD_COUNT_DEFAULT;

    static const std::string INTERNAL_EXECUTOR_POOL_SIZE;
    static const std::string INTERNAL_EXECUTOR_POOL_SIZE_DEFAULT;

    /**
     * Client shuffles the given member list to prevent all clients to connect
     * to the same node when this property is set to true. When it is set to
     * false, the client tries to connect to the nodes in the given order.
     */
    static const std::string SHUFFLE_MEMBER_LIST;
    static const std::string SHUFFLE_MEMBER_LIST_DEFAULT;

    /**
     * The maximum number of concurrent invocations allowed.
     * <p/>
     * To prevent the system from overloading, user can apply a constraint on
     * the number of concurrent invocations. If the maximum number of concurrent
     * invocations has been exceeded and a new invocation comes in, then
     * hazelcast will throw hazelcast_overload_exception <p/> By default it is
     * configured as INT32_MAX.
     */
    static const std::string MAX_CONCURRENT_INVOCATIONS;
    static const std::string MAX_CONCURRENT_INVOCATIONS_DEFAULT;

    /**
     * Control the maximum timeout in millis to wait for an invocation space to
     * be available. <p/> If an invocation can't be made because there are too
     * many pending invocations, then an exponential backoff is done to give the
     * system time to deal with the backlog of invocations. This property
     * controls how long an invocation is allowed to wait before getting a
     * {@link com.hazelcast.core.hazelcast_overload}. <p/> <p> When set to -1
     * then <code>hazelcast_overload_exception</code> is thrown immediately
     * without any waiting.
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

    /**
     * If an operation has backups, this property specifies how long the
     * invocation will wait for acks from the backup replicas. If acks are not
     * received from some backups, there will not be any rollback on other
     * successful replicas.
     */
    static constexpr const char* OPERATION_BACKUP_TIMEOUT_MILLIS =
      "hazelcast.client.operation.backup.timeout.millis";
    static constexpr const char* OPERATION_BACKUP_TIMEOUT_MILLIS_DEFAULT =
      "5000";

    /**
     * When this configuration is enabled, if an operation has sync backups and
     * acks are not received from backup replicas in time, or the member which
     * owns primary replica of the target partition leaves the cluster, then the
     * invocation fails with \indeterminate_operation_state. However, even if
     * the invocation fails, there will not be any rollback on other successful
     * replicas.
     */
    static constexpr const char* FAIL_ON_INDETERMINATE_OPERATION_STATE =
      "hazelcast.client.operation.fail.on.indeterminate.state";
    static constexpr const char* FAIL_ON_INDETERMINATE_OPERATION_STATE_DEFAULT =
      "false";

    /**
     * Internal client property to change base url of cloud discovery endpoint.
     * Used for testing cloud discovery.
     */
    static constexpr const char* CLOUD_URL_BASE = "hazelcast.client.cloud.url";
    static constexpr const char* CLOUD_URL_BASE_DEFAULT =
      "api.cloud.hazelcast.com";

    /**
     * Parametrized SQL queries touching only a single partition benefit from
     * using the partition owner as the query coordinator, if the partition
     * owner can be determined from one of the query parameters. When such a
     * query is executed, the cluster sends the index of such argument to the
     * client. This parameter configures the size of the cache the client uses
     * for storing this information.
     */
    static constexpr const char* PARTITION_ARGUMENT_CACHE_SIZE =
      "hazelcast.client.sql.partition.argument.cache.size";
    static constexpr const char* PARTITION_ARGUMENT_CACHE_SIZE_DEFAULT = "1024";

    /**
     * Returns the configured boolean value of a {@link ClientProperty}.
     *
     * @param property the {@link ClientProperty} to get the value from
     * @return the value as bool
     */
    bool get_boolean(const client_property& property) const;

    /**
     * Returns the configured int32_t value of a {@link ClientProperty}.
     *
     * @param property the {@link ClientProperty} to get the value from
     * @return the value as int32_t
     */
    int32_t get_integer(const client_property& property) const;

    /**
     * Returns the configured int64_t value of a {@link ClientProperty}.
     *
     * @param property the {@link ClientProperty} to get the value from
     * @return the value as int64_t
     */
    int64_t get_long(const client_property& property) const;

    /**
     * Returns the configured value of a {@link ClientProperty} as std::string.
     *
     * @param property the {@link ClientProperty} to get the value from
     * @return the value
     */
    std::string get_string(const client_property& property) const;

private:
    client_property heartbeat_timeout_;
    client_property heartbeat_interval_;
    client_property retry_count_;
    client_property retry_wait_time_;
    client_property aws_member_port_;
    client_property invocation_retry_pause_millis_;
    client_property invocation_timeout_seconds_;
    client_property event_thread_count_;
    client_property internal_executor_pool_size_;
    client_property shuffle_member_list_;
    client_property max_concurrent_invocations_;
    client_property backpressure_backoff_timeout_millis_;
    client_property statistics_enabled_;
    client_property statistics_period_seconds_;
    client_property backup_timeout_millis_;
    client_property fail_on_indeterminate_state_;
    client_property cloud_base_url_;
    client_property partition_arg_cache_size_;

    std::unordered_map<std::string, std::string> properties_map_;
};

} // namespace client
} // namespace hazelcast

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
