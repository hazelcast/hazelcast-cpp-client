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

#include "hazelcast/util/Preconditions.h"
#include "hazelcast/util/Bits.h"
#include "hazelcast/client/spi/impl/sequence/AbstractCallIdSequence.h"

namespace hazelcast {
    namespace client {
        namespace spi {
            namespace impl {
                namespace sequence {
                    AbstractCallIdSequence::AbstractCallIdSequence(int32_t maxConcurrentInvocations) : longs(
                            3 * util::Bits::CACHE_LINE_LENGTH / util::Bits::LONG_SIZE_IN_BYTES) {
                        std::ostringstream out;
                        out << "maxConcurrentInvocations should be a positive number. maxConcurrentInvocations="
                                << maxConcurrentInvocations;
                        util::Preconditions::checkPositive(maxConcurrentInvocations, out.str());

                        this->maxConcurrentInvocations = maxConcurrentInvocations;
                    }

                    AbstractCallIdSequence::~AbstractCallIdSequence() {
                    }

                    int32_t AbstractCallIdSequence::getMaxConcurrentInvocations() const {
                        return maxConcurrentInvocations;
                    }

                    int64_t AbstractCallIdSequence::next() {
                        if (!hasSpace()) {
                            handleNoSpaceLeft();
                        }
                        return forceNext();
                    }

                    int64_t AbstractCallIdSequence::forceNext() {
                        return longs.incrementAndGet(INDEX_HEAD);
                    }

                    void AbstractCallIdSequence::complete() {
                        int64_t newTail = longs.incrementAndGet(INDEX_TAIL);
                        assert(newTail <= longs.get(INDEX_HEAD));
                    }

                    int64_t AbstractCallIdSequence::getLastCallId() {
                        return longs.get(INDEX_HEAD);
                    }

                    bool AbstractCallIdSequence::hasSpace() {
                        return longs.get(INDEX_HEAD) - longs.get(INDEX_TAIL) < maxConcurrentInvocations;
                    }

                    int64_t AbstractCallIdSequence::getTail() {
                        return longs.get(INDEX_TAIL);
                    }
                }
            }
        }

    }
}
