/*
 * Copyright (c) 2008-2026, Hazelcast, Inc. All Rights Reserved.
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

#include <cstdint>
#include <iterator>

#include "hazelcast/util/export.h"

namespace hazelcast {
namespace client {
namespace impl {

/**
 * Set of IDs returned from a flake ID generator.
 * <p>
 * IDs can be iterated using a foreach loop:
 * <pre>{@code
 *    for (auto it = batch.iterator(); it != id_batch::end(); ++it) {
 *        // ... use *it
 *    }
 * }</pre>
 * <p>
 * Object is immutable.
 *
 * Mirrors com.hazelcast.flakeidgen.impl.IdBatch in the Java client.
 */
class HAZELCAST_API id_batch
{
public:
    id_batch(int64_t base, int64_t increment, int32_t batch_size);

    /**
     * Returns the first ID in the set.
     */
    int64_t get_base() const;

    /**
     * Returns increment from {@link #get_base()} for the next ID in the set.
     */
    int64_t get_increment() const;

    /**
     * Returns number of IDs in the set.
     */
    int32_t get_batch_size() const;

    class HAZELCAST_API id_iterator
    {
    public:
        using iterator_category = std::input_iterator_tag;
        using value_type = int64_t;
        using difference_type = std::ptrdiff_t;
        using pointer = int64_t*;
        using reference = int64_t&;

        id_iterator();
        id_iterator(int64_t base2, int64_t increment, int32_t remaining);

        id_iterator& operator++();
        bool operator==(const id_iterator& rhs) const;
        bool operator!=(const id_iterator& rhs) const;
        const int64_t& operator*() const;

    private:
        int64_t base2_;
        int64_t increment_;
        int32_t remaining_;
    };

    id_iterator iterator();

    static id_iterator& end();

private:
    int64_t base_;
    int64_t increment_;
    int32_t batch_size_;

    static id_iterator end_of_batch_;
};

} // namespace impl
} // namespace client
} // namespace hazelcast
