/*
 * Copyright (c) 2008-2023, Hazelcast, Inc. All Rights Reserved.
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

#include <vector>
#include <boost/multiprecision/cpp_int.hpp>
#include "hazelcast/util/export.h"

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4251) // for dll export
#endif

namespace hazelcast {
namespace client {
/**
 *  An arbitrary precision and scale floating point number.
 *  unscaledValue x 10 ^ -scale
 *
 *  For arithmetic operations support, it is suggested to use external
 *  libraries. An usage example with boost::multiprecision::cpp_dec_float
 *  could be as follows:
 *  <pre><code>
 * hazelcast::big_decimal dec{ u, 2 };
 * boost::multiprecision::cpp_dec_float<10> f(
 *  (dec.unscaled.str() + "e-" + std::to_string(dec.scale)).c_str());
 *  std::cout << f.str(100, std::ios_base::dec) << std::endl;
 *  </code></pre>
 */
struct HAZELCAST_API big_decimal
{
    boost::multiprecision::cpp_int unscaled;
    int32_t scale;
};

bool HAZELCAST_API
operator==(const big_decimal& lhs, const big_decimal& rhs);

bool HAZELCAST_API
operator<(const big_decimal& lhs, const big_decimal& rhs);
} // namespace client
} // namespace hazelcast
namespace std {
template<>
struct HAZELCAST_API hash<hazelcast::client::big_decimal>
{
    std::size_t operator()(const hazelcast::client::big_decimal& f) const;
};
} // namespace std
namespace hazelcast {
namespace client {
namespace pimpl {

/**
 * Takes twos complement of given array where most significant value is first.
 * This method modifies the vector in place.
 * @param a the array to take twos complement of
 */

void HAZELCAST_API
twos_complement(std::vector<int8_t>& a);

/**
 * Creates a cpp_int from a vector of int8_t respecting the sign.
 *
 * boost::import_bits does not respect the sign, so we have to do it manually.
 * if v represents a negative number, we take the two's complement of it,
 * to get positive representation of the same number. Then we add the sign
 * at the end.
 * @param v int8_t array to read from
 * @return cpp_int
 */
boost::multiprecision::cpp_int HAZELCAST_API
from_bytes(std::vector<int8_t> v);
/**
 * Creates a twos complement byte array from cpp_int respecting the sign.
 *
 * boost::export_bits does not respect the sign, so we have to do it manually.
 * if i is a negative number, we take the two's complement on resulting vector,
 * to get negative representation of the number.
 * We also add one extra byte to the end of the vector to preserve the sign if
 * sign of the integer is not the same as the most significant byte's sign.
 * Otherwise we don't add it to have minimum size vector to represent the value.
 * @param i the number to convert to bytes
 * @return the vector of int8_t representing the number
 */
std::vector<int8_t> HAZELCAST_API
to_bytes(const boost::multiprecision::cpp_int& i);
} // namespace pimpl
} // namespace client
} // namespace hazelcast
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
