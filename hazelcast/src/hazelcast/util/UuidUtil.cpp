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

#include "hazelcast/util/UuidUtil.h"

namespace hazelcast {
    namespace util {
        std::string UuidUtil::newUnsecureUuidString() {
            return newUnsecureUUID().toString();
        }

        UUID UuidUtil::newUnsecureUUID() {
            byte data[16];
            // TODO: Use a better random bytes generator
            for (int j = 0; j < 16; ++j) {
                data[j] = rand() % 16;
            }

            // clear version
            data[6] &= 0x0f;
            // set to version 4
            data[6] |= 0x40;
            // clear variant
            data[8] &= 0x3f;
            // set to IETF variant
            data[8] |= 0x80;

            int64_t mostSigBits = 0;
            int64_t leastSigBits = 0;
            for (int i = 0; i < 8; i++) {
                mostSigBits = (mostSigBits << 8) | (data[i] & 0xff);
            }
            for (int i = 8; i < 16; i++) {
                leastSigBits = (leastSigBits << 8) | (data[i] & 0xff);
            }
            return UUID(mostSigBits, leastSigBits);
        }
    }
}
