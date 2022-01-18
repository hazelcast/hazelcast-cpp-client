/*
 * Copyright (c) 2008-2021, Hazelcast, Inc. All Rights Reserved.
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

#include <chrono>

#include "hazelcast/util/export.h"

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4251) // for dll export
#endif

namespace hazelcast {
namespace client {
namespace config {
/**
 * Connection Retry Config is controls the period among the retries and when
 * should a client gave up retrying. Exponential behaviour can be chosen or
 * jitter can be added to wait periods.
 */
class HAZELCAST_API connection_retry_config
{
public:
    /**
     * how long to wait after the first failure before retrying
     *
     * @return initial_backoff_duration_
     */
    std::chrono::milliseconds get_initial_backoff_duration() const;

    /**
     * @param initial_backoff_duration how long to wait after the first failure
     * before retrying
     * @return updated connection_retry_config
     */
    connection_retry_config& set_initial_backoff_duration(
      std::chrono::milliseconds initial_backoff_duration);

    /**
     * When backoff reaches this upper bound, it does not increase any more.
     *
     * @return max_backoff_duration_
     */
    std::chrono::milliseconds get_max_backoff_duration() const;

    /**
     * When backoff reaches this upper bound, it does not increase any more.
     *
     * @param max_backoff_duration upper bound on backoff
     * @return updated connection_retry_config
     */
    connection_retry_config& set_max_backoff_duration(
      std::chrono::milliseconds max_backoff_duration);

    /**
     * factor with which to multiply backoff time after a failed retry
     *
     * @return multiplier_
     */
    double get_multiplier() const;

    /**
     * @param multiplier factor with which to multiply backoff after a failed
     * retry
     * @return updated connection_retry_config
     */
    connection_retry_config& set_multiplier(double multiplier);

    /**
     * Timeout value for the client to give up to connect to the current cluster
     *  Theclient can shutdown after reaching the timeout.
     *
     * @return cluster_connect_timeout_
     */
    std::chrono::milliseconds get_cluster_connect_timeout() const;

    /**
     * @param cluster_connect_timeout timeout for the client to give up to
     * connect to the current cluster The client can shutdown after reaching the
     * timeout.
     * @return updated connection_retry_config
     */
    connection_retry_config& set_cluster_connect_timeout(
      std::chrono::milliseconds cluster_connect_timeout);

    /**
     * by how much to randomize backoffs.
     * At each iteration calculated back-off is randomized via following method
     * random(-jitter * current_backoff, jitter * current_backoff)
     *
     * @return jitter_
     */
    double get_jitter() const;

    /**
     * At each iteration calculated back-off is randomized via following method
     * random(-jitter * current_backoff, jitter * current_backoff)
     *
     * @param jitter by how much to randomize backoffs
     * @return updated connection_retry_config
     */
    connection_retry_config& set_jitter(double jitter);

private:
    static constexpr std::chrono::milliseconds INITIAL_BACKOFF{ 1000 };
    static constexpr std::chrono::milliseconds MAX_BACKOFF{ 30000 };
    static constexpr std::chrono::milliseconds CLUSTER_CONNECT_TIMEOUT{ (
      std::chrono::milliseconds::max)() };
    static constexpr double JITTER = 0;
    std::chrono::milliseconds initial_backoff_duration_ = INITIAL_BACKOFF;
    std::chrono::milliseconds max_backoff_duration_ = MAX_BACKOFF;
    double multiplier_ = 1.05;
    std::chrono::milliseconds cluster_connect_timeout_ =
      CLUSTER_CONNECT_TIMEOUT;
    double jitter_ = JITTER;
};

} // namespace config
} // namespace client
} // namespace hazelcast

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
