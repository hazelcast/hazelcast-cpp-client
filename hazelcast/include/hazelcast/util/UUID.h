/*
 * Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
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
//
// Created by ihsan demir on 9 Dec 2016.

#pragma once
#include <stdint.h>
#include <string>
#include <ostream>

#include "hazelcast/util/HazelcastDll.h"

namespace hazelcast {
    namespace util {
        class HAZELCAST_API UUID {
        public:
            UUID();

            /**
             * Constructs a new <tt>UUID</tt> using the specified data.
             * <tt>mostSigBits</tt> is used for the most significant 64 bits
             * of the <tt>UUID</tt> and <tt>leastSigBits</tt> becomes the
             * least significant 64 bits of the <tt>UUID</tt>.
             *
             * @param  mostSigBits
             * @param  leastSigBits
             */
            UUID(int64_t mostSigBits, int64_t leastSigBits);

            // Field Accessor Methods

            /**
             * Returns the least significant 64 bits of this UUID's 128 bit value.
             *
             * @return the least significant 64 bits of this UUID's 128 bit value.
             */
            int64_t getLeastSignificantBits() const;

            /**
             * Returns the most significant 64 bits of this UUID's 128 bit value.
             *
             * @return the most significant 64 bits of this UUID's 128 bit value.
             */
            int64_t getMostSignificantBits() const;

            /**
             * Compares this object to the specified object.  The result is
             * <tt>true</tt> if and only if the argument contains the same value, bit for bit,
             * as this <tt>UUID</tt>.
             *
             * @param   obj   the object to compare with.
             * @return  <code>true</code> if the objects are the same;
             *          <code>false</code> otherwise.
             */
            bool equals(const UUID &rhs) const;

            bool operator==(const UUID &rhs) const;

            bool operator!=(const UUID &rhs) const;

            /**
             * Returns a {@code String} object representing this {@code UUID}.
             *
             * <p> The UUID string representation is as described by this BNF:
             * <blockquote><pre>
             * {@code
             * UUID                   = <time_low> "-" <time_mid> "-"
             *                          <time_high_and_version> "-"
             *                          <variant_and_sequence> "-"
             *                          <node>
             * time_low               = 4*<hexOctet>
             * time_mid               = 2*<hexOctet>
             * time_high_and_version  = 2*<hexOctet>
             * variant_and_sequence   = 2*<hexOctet>
             * node                   = 6*<hexOctet>
             * hexOctet               = <hexDigit><hexDigit>
             * hexDigit               =
             *       "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9"
             *       | "a" | "b" | "c" | "d" | "e" | "f"
             *       | "A" | "B" | "C" | "D" | "E" | "F"
             * }</pre></blockquote>
             *
             * @return  A string representation of this {@code UUID}
             */
            std::string toString() const;
        private:
            static std::string digits(int64_t val, int32_t digits);

            int64_t mostSigBits;
            int64_t leastSigBits;
        };

    }
}



