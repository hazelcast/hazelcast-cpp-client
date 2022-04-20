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

#include "hazelcast/client/decimal.h"

namespace hazelcast {
namespace client {

bool
operator==(const decimal& lhs, const decimal& rhs)
{
    return lhs.unscaled == rhs.unscaled && lhs.scale == rhs.scale;
}

namespace pimpl {

void
twos_complement(std::vector<int8_t>& a)
{
    for (auto& item : a) {
        item = ~item;
    }
    int carry = 1;
    for (int i = a.size() - 1; i >= 0; i--) {
        a[i] = a[i] + carry;
        if (a[i] == 0) {
            carry = 1;
        } else {
            break;
        }
    }
}

boost::multiprecision::cpp_int
from_bytes(std::vector<int8_t> v)
{
    boost::multiprecision::cpp_int i;
    bool is_negative = v[0] < 0;
    if (is_negative) {
        twos_complement(v);
    }
    import_bits(i, v.begin(), v.end(), 8);
    if (is_negative) {
        return -i;
    }
    return i;
}

std::vector<int8_t>
to_bytes(const boost::multiprecision::cpp_int& i)
{
    std::vector<int8_t> v;
    export_bits(i, std::back_inserter(v), 8);
    if (i < 0) {
        twos_complement(v);
        if (v[0] > 0) {
            v.insert(v.begin(), 1, -1);
        }
    } else {
        if (v[0] < 0) {
            v.insert(v.begin(), 1, 0);
        }
    }
    return v;
}
} // namespace pimpl
} // namespace client
} // namespace hazelcast
