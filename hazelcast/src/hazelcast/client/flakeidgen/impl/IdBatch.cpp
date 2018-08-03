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

#include <hazelcast/client/flakeidgen/impl/IdBatch.h>

#include "hazelcast/client/flakeidgen/impl/IdBatch.h"

namespace hazelcast {
    namespace client {
        namespace flakeidgen {
            namespace impl {
                IdBatch::IdIterator IdBatch::endOfBatch;

                const int64_t IdBatch::getBase() const {
                    return base;
                }

                const int64_t IdBatch::getIncrement() const {
                    return increment;
                }

                const int32_t IdBatch::getBatchSize() const {
                    return batchSize;
                }

                IdBatch::IdBatch(const int64_t base, const int64_t increment, const int32_t batchSize)
                        : base(base), increment(increment), batchSize(batchSize) {}

                IdBatch::IdIterator &IdBatch::end() {
                    return endOfBatch;
                }

                IdBatch::IdIterator IdBatch::iterator() {
                    return IdBatch::IdIterator(base, increment, batchSize);
                }

                IdBatch::IdIterator::IdIterator(int64_t base2, const int64_t increment, int32_t remaining) : base2(
                        base2), increment(increment), remaining(remaining) {}

                bool IdBatch::IdIterator::operator==(const IdBatch::IdIterator &rhs) const {
                    return base2 == rhs.base2 && increment == rhs.increment && remaining == rhs.remaining;
                }

                bool IdBatch::IdIterator::operator!=(const IdBatch::IdIterator &rhs) const {
                    return !(rhs == *this);
                }

                IdBatch::IdIterator::IdIterator() : base2(-1), increment(-1), remaining(-1) {
                }

                IdBatch::IdIterator &IdBatch::IdIterator::operator++() {
                    if (remaining == 0) {
                        return IdBatch::end();
                    }

                    --remaining;

                    base2 += increment;

                    return *this;
                }
            }
        }
    }
}
