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
//
// Created by ihsan demir on 9 Dec 2016.

#ifndef HAZELCAST_UTIL_UUID_H_
#define HAZELCAST_UTIL_UUID_H_

#include <stdint.h>

#include "hazelcast/util/HazelcastDll.h"

namespace hazelcast {
    namespace util {
        class HAZELCAST_API UUID {
        public:
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
        private:
            const int64_t mostSigBits;
            const int64_t leastSigBits;
        };
    }
}


#endif //HAZELCAST_UTIL_UUID_H_

