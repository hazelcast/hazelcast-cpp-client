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

#ifndef HAZELCAST_UTIL_UTFUTIL_H_
#define HAZELCAST_UTIL_UTFUTIL_H_

#include <string>
#include <vector>
#include <stdint.h>

#include "hazelcast/util/HazelcastDll.h"

namespace hazelcast {
    namespace util {
        class HAZELCAST_API UTFUtil {
        public:
            class ByteReadable {
            public:
                virtual ~ByteReadable();

                virtual byte readByte() = 0;
            };

            /**  UTF-8 Encoding
            Number      Bits for        First           Last        Byte 1	    Byte 2	    Byte 3	    Byte 4
            of bytes	code point      code point      code point

            1	            7	        U+0000	        U+007F	    0xxxxxxx
            2	            11	        U+0080	        U+07FF	    110xxxxx	10xxxxxx
            3	            16	        U+0800	        U+FFFF	    1110xxxx	10xxxxxx	10xxxxxx
            4	            21	        U+10000	        U+10FFFF	11110xxx	10xxxxxx	10xxxxxx	10xxxxxx
            */

            /**
             *
             *
             * @param str The string whose UTF-8 byte format will be validated.
             * @return The number of UTF-8 encoded bytes. Returns -1 if the format is incorrect.
             */
            static int32_t isValidUTF8(const std::string &str);

            static void readUTF8Char(ByteReadable &in, byte c, std::vector<char> &utfBuffer, size_t &index);

        private:
            UTFUtil();

            UTFUtil(const UTFUtil &);
        };
    }
}
#endif //HAZELCAST_UTIL_UTFUTIL_H_
