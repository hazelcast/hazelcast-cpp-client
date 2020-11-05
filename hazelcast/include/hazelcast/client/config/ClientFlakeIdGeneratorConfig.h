/*
 * Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
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

#include <stdint.h>
#include <string>

#include "hazelcast/util/HazelcastDll.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace config {
            /**
             * The {@code ClientFlakeIdGeneratorConfig} contains the configuration for the client
             * regarding {@link HazelcastClient#getFlakeIdGenerator(const std::string &)
             * Flake ID Generator}.
             */
            class HAZELCAST_API ClientFlakeIdGeneratorConfig {
            public:
                /**
                 * Default value for {@link #getPrefetchCount()}.
                 */
                static constexpr int32_t DEFAULT_PREFETCH_COUNT = 100;

                /**
                 * Default value for {@link #getPrefetchValidityMillis()}.
                 */
                static constexpr int64_t DEFAULT_PREFETCH_VALIDITY_MILLIS = 600000;

                /**
                 * Maximum value for prefetch count. The limit is ~10% of the time we allow the IDs to be from the future
                 * (see {@link com.hazelcast.flakeidgen.impl.FlakeIdGeneratorProxy#ALLOWED_FUTURE_MILLIS}).
                 * <p>
                 * The reason to limit the prefetch count is that a single call to {@link FlakeIdGenerator#newId()} might
                 * be blocked if the future allowance is exceeded: we want to avoid a single call for large batch to block
                 * another call for small batch.
                 */
                static constexpr int32_t MAXIMUM_PREFETCH_COUNT = 100000;

                explicit ClientFlakeIdGeneratorConfig(const std::string &name);

                /**
                 * Returns the configuration name. This can be actual object name or pattern.
                 */
                const std::string &getName() const;

                /**
                 * Sets the name or name pattern for this config. Must not be modified after this
                 * instance is added to {@link ClientConfig}.
                 *
                 * @return this instance for fluent API
                 */
                ClientFlakeIdGeneratorConfig &setName(const std::string &n);

                /**
                 * @see #setPrefetchCount(int)
                 */
                int32_t getPrefetchCount() const;

                /**
                 * Sets how many IDs are pre-fetched on the background when one call to
                 * {@link FlakeIdGenerator#newId()} is made. Default is 100.
                 *
                 * @param count the desired prefetch count, in the range 1..MAXIMUM_PREFETCH_COUNT.
                 * @return this instance for fluent API
                 */
                ClientFlakeIdGeneratorConfig &setPrefetchCount(int32_t count);

                /**
                 * @see #setPrefetchValidityDuration(std::chrono::milliseconds)
                 */
                std::chrono::milliseconds getPrefetchValidityDuration() const;

                /**
                 * Sets for how long the pre-fetched IDs can be used. If this time elapses, a new batch of IDs will be
                 * fetched. Time unit resolution is milliseconds, default is 600,000msecs (10 minutes).
                 * <p>
                 * The IDs contain timestamp component, which ensures rough global ordering of IDs. If an ID
                 * is assigned to an object that was created much later, it will be much out of order. If you don't care
                 * about ordering, set this value to 0.
                 * <p>
                 * This setting pertains only to {@link FlakeIdGenerator#newId newId} calls made on the member
                 * that configured it.
                 *
                 * @param durations the desired ID validity or unlimited, if configured to 0.
                 * @return this instance for fluent API
                 *
                 * @throws client::exception::IllegalArgumentException if duration is negative.
                 */
                ClientFlakeIdGeneratorConfig &setPrefetchValidityDuration(std::chrono::milliseconds duration);

            private:
                std::string name_;
                int32_t prefetchCount_;
                std::chrono::milliseconds prefetchValidityDuration_;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


