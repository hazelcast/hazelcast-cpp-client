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

#include "hazelcast/util/hazelcast_dll.h"

namespace hazelcast {
    namespace client {
        namespace spi {
            namespace impl {
                namespace sequence {
                    /**
                     * Responsible for generating invocation callIds.
                     * <p>
                     * It is very important that for each {@link #next()} and {@link #forceNext()} ()}
                     * there is a matching {@link #complete()}. If they don't match, the number of concurrent
                     * invocations will grow/shrink without bound over time. This can lead to OOME or deadlock.
                     * <p>
                     * When backpressure is enabled and there are too many concurrent invocations, calls of {@link #next()}
                     * will block using a spin-loop with exponential backoff.
                     * <p/>
                     * Currently a single CallIdSequence is used for all partitions, so there is contention. Also one partition
                     * can cause problems in other partition if a lot of invocations are created for that partition. Then other
                     * partitions can't make as many invocations because a single callIdSequence is being used.
                     * <p/>
                     * In the future we could add a CallIdSequence per partition or using some 'concurrency level'
                     * and do a mod based on the partition-id. The advantage is that you reduce contention and improve isolation,
                     * at the expense of:
                     * <ol>
                     * <li>increased complexity</li>
                     * <li>not always being able to fully utilize the number of invocations.</li>
                     * </ol>
                     */
                    class HAZELCAST_API CallIdSequence {
                    public:
                        /**
                         * Destructor
                         */
                        virtual ~CallIdSequence() = default;

                        /**
                         * Returns the maximum concurrent invocations supported. INT32_MAX means there is no max.
                         *
                         * @return the maximum concurrent invocation.
                         */
                        virtual int32_t get_max_concurrent_invocations() const = 0;

                        /**
                         * Generates the next unique call ID. When the implementation
                         * supports backpressure, it will not return unless the number of outstanding invocations is within the
                         * configured limit. Instead it will block until the condition is met and eventually throw HazelcastOverloadException.
                         *
                         * @return the generated call ID
                         * @throws HazelcastOverloadException if the outstanding invocation count hasn't dropped below the configured limit
                         */
                        virtual int64_t next() = 0;

                        /**
                         * Generates the next unique call ID.
                         * This never blocks and it should be used only for urgent operation or when retrying.
                         *
                         * @return the generated call ID
                         */
                        virtual int64_t force_next() = 0;

                        /** Not idempotent: must be called exactly once per invocation. */
                        virtual void complete() = 0;

                        /** Returns the last issued call ID.
                         * <strong>ONLY FOR TESTING. Must not be used for production code.</strong>
                         */
                        virtual int64_t get_last_call_id() = 0;
                    };
                }
            }
        }
    }
}


