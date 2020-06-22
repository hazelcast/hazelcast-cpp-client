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

#pragma once

#include <string>
#include <vector>
#include <stdint.h>

#include "hazelcast/util/HazelcastDll.h"

namespace hazelcast {
    namespace util {
        class HAZELCAST_API UTFUtil {
        public:

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

            template<typename Readable>
            static void readUTF8Char(Readable &in, byte firstByte, std::string &utfBuffer) {
                size_t n = 0;
                // ascii
                if (firstByte <= 0x7f) {
                    n = 0; // 0bbbbbbb
                } else if ((firstByte & 0xE0) == 0xC0) {
                    n = 1; // 110bbbbb
                } else if ((firstByte & 0xF0) == 0xE0) {
                    n = 2; // 1110bbbb
                } else if ((firstByte & 0xF8) == 0xF0) {
                    n = 3; // 11110bbb
                } else {
                    throw client::exception::UTFDataFormatException("Bits::readUTF8Char", "Malformed byte sequence");
                }

                utfBuffer.push_back((char) firstByte);
                for (size_t j = 0; j < n; j++) {
                    byte b = in.template read<byte>();
                    if (firstByte == 0xed && (b & 0xa0) == 0xa0) {
                        throw client::exception::UTFDataFormatException("Bits::readUTF8Char",
                                                                        "Malformed byte sequence U+d800 to U+dfff"); //U+d800 to U+dfff
                    }

                    if ((b & 0xC0) != 0x80) { // n bytes matching 10bbbbbb follow ?
                        throw client::exception::UTFDataFormatException("Bits::readUTF8Char", "Malformed byte sequence");
                    }
                    utfBuffer.push_back((char) b);
                }
            }

        private:
            UTFUtil();

            UTFUtil(const UTFUtil &);
        };
    }
}

