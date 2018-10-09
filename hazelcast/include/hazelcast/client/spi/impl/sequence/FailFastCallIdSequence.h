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

#ifndef HAZELCAST_CLIENT_SPI_IMPL_SEQUENCE_FILFASTCALLIDSEQUENCE_H_
#define HAZELCAST_CLIENT_SPI_IMPL_SEQUENCE_FILFASTCALLIDSEQUENCE_H_

#include "hazelcast/client/spi/impl/sequence/AbstractCallIdSequence.h"

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
                    class FailFastCallIdSequence : public AbstractCallIdSequence {
                    public:
                        FailFastCallIdSequence(int32_t maxConcurrentInvocations);

                    protected:
                        virtual void handleNoSpaceLeft();
                    };
                }
            }
        }
    }
}

#endif //HAZELCAST_CLIENT_SPI_IMPL_SEQUENCE_FILFASTCALLIDSEQUENCE_H_
