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
#ifndef HAZELCAST_CLIENT_FLAKEIDGEN_IMPL_IDBATCH_H
#define HAZELCAST_CLIENT_FLAKEIDGEN_IMPL_IDBATCH_H


#include <cstdint>
#include <iterator>

namespace hazelcast {
    namespace client {
        namespace flakeidgen {
            namespace impl {
                /**
                 * Set of IDs returned from {@link FlakeIdGenerator}.
                 * <p>
                 * IDs can be iterated using a foreach loop:
                 * <pre>{@code
                 *    IdBatch idBatch = myFlakeIdGenerator.newIdBatch(100);
                 *    for (Long id : idBatch) {
                 *        // ... use the id
                 *    }
                 * }</pre>
                 * <p>
                 * Object is immutable.
                 */
                class IdBatch {
                public:
                    IdBatch(const int64_t base, const int64_t increment, const int32_t batchSize);

                    /**
                     * Returns the first ID in the set.
                     */
                    const int64_t getBase() const;

                    /**
                     * Returns increment from {@link #base()} for the next ID in the set.
                     */
                    const int64_t getIncrement() const;

                    /**
                     * Returns number of IDs in the set.
                     */
                    const int32_t getBatchSize() const;

                    class IdIterator : public std::iterator<std::input_iterator_tag, int64_t> {
                    public:
                        IdIterator();

                        IdIterator(int64_t base2, const int64_t increment, int32_t remaining);

                        IdIterator& operator++();

                        bool operator==(const IdIterator &rhs) const;

                        bool operator!=(const IdIterator &rhs) const;

                        const int64_t &operator*() {return base2;}
                    private:

                        int64_t base2;
                        const int64_t increment;
                        int32_t remaining;
                    };

                    IdIterator iterator();

                    static IdIterator &end();

                private:

                    const int64_t base;
                    const int64_t increment;
                    const int32_t batchSize;

                    static IdIterator endOfBatch;
                };
            }
        }
    }
}

#endif /* HAZELCAST_CLIENT_FLAKEIDGEN_IMPL_IDBATCH_H */
