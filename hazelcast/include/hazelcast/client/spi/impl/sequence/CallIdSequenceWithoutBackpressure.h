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

#ifndef HAZELCAST_CLIENT_SPI_IMPL_SEQUENCE_CALLIDSEQUENCEWITHOUTBACKPRESSURE_H_
#define HAZELCAST_CLIENT_SPI_IMPL_SEQUENCE_CALLIDSEQUENCEWITHOUTBACKPRESSURE_H_

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

                        virtual ~CallIdSequenceWithoutBackpressure();

                        virtual int32_t getMaxConcurrentInvocations() const;

                        virtual int64_t next();

                        virtual int64_t forceNext();

                        virtual void complete();

                        virtual int64_t getLastCallId();

                    private:
                        std::atomic<int64_t> head;
                    };
                }
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_CLIENT_SPI_IMPL_SEQUENCE_CALLIDSEQUENCEWITHOUTBACKPRESSURE_H_
