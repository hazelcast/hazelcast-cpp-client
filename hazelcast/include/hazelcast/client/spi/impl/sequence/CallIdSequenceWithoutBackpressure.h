/*
 * Copyright (c) 2008-2022, Hazelcast, Inc. All Rights Reserved.
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
#include <memory>
#include <atomic>

#include "hazelcast/client/spi/impl/sequence/CallIdSequence.h"


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace spi {
            namespace impl {
                namespace sequence {
                    class HAZELCAST_API CallIdSequenceWithoutBackpressure : public CallIdSequence {
                    public:
                        CallIdSequenceWithoutBackpressure();

                        ~CallIdSequenceWithoutBackpressure() override;

                        int32_t get_max_concurrent_invocations() const override;

                        int64_t next() override;

                        int64_t force_next() override;

                        void complete() override;

                        int64_t get_last_call_id() override;

                    private:
                        std::atomic<int64_t> head_;
                    };
                }
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


