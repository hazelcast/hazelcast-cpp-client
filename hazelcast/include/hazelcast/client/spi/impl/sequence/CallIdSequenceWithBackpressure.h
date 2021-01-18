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

#include "hazelcast/client/spi/impl/sequence/AbstractCallIdSequence.h"
#include "hazelcast/util/concurrent/IdleStrategy.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace spi {
            namespace impl {
                namespace sequence {
                    /**
                     * A {@link CallIdSequence} that provides backpressure by taking
                     * the number of in-flight operations into account when before creating a new call-id.
                     * <p>
                     * It is possible to temporarily create more concurrent invocations than the declared capacity due to:
                     * <ul>
                     *     <li>system operations</li>
                     *     <li>the racy nature of checking if space is available and getting the next sequence. </li>
                     * </ul>
                     * The latter cause is not a problem since the capacity is exceeded temporarily and it isn't sustainable.
                     * So perhaps there are a few threads that at the same time see that the there is space and do a next.
                     * But any following invocation needs to wait till there is is capacity.
                     */
                    class HAZELCAST_API CallIdSequenceWithBackpressure : public AbstractCallIdSequence {
                    public:
                        CallIdSequenceWithBackpressure(int32_t max_concurrent_invocations, int64_t backoff_timeout_ms);

                    protected:
                        void handle_no_space_left() override;

                    private:
                        int64_t backoff_timeout_nanos_;
                        static const int MAX_DELAY_MS = 500;
                        static const std::unique_ptr<util::concurrent::IdleStrategy> IDLER;
                    };
                }
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


