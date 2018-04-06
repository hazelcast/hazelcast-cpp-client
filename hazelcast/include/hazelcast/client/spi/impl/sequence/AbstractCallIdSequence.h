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

#ifndef HAZELCAST_CLIENT_SPI_IMPL_SEQUENCE_ABSTRACTCALLIDSEQUENCE_H_
#define HAZELCAST_CLIENT_SPI_IMPL_SEQUENCE_ABSTRACTCALLIDSEQUENCE_H_

#include <stdint.h>

#include "hazelcast/client/spi/impl/sequence/CallIdSequence.h"
#include "hazelcast/util/AtomicArray.h"

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
                     */
                    class HAZELCAST_API AbstractCallIdSequence : public CallIdSequence {
                    public:
                        virtual ~AbstractCallIdSequence();

                        AbstractCallIdSequence(int32_t maxConcurrentInvocations);

                        virtual int32_t getMaxConcurrentInvocations() const;

                        virtual int64_t next();

                        virtual int64_t forceNext();

                        virtual void complete();

                        virtual int64_t getLastCallId();

                    protected:
                        virtual void handleNoSpaceLeft() = 0;

                        bool hasSpace();
                    private:
                        static const size_t INDEX_HEAD = 7;
                        static const size_t INDEX_TAIL = 15;

                        int32_t maxConcurrentInvocations;

                        // instead of using 2 AtomicLongs, we use an array if width of 3 cache lines to prevent any false sharing.
                        util::AtomicArray<int64_t> longs;
                    };
                }
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_CLIENT_SPI_IMPL_SEQUENCE_ABSTRACTCALLIDSEQUENCE_H_
