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

#include "hazelcast/util/UTFUtil.h"
#include "hazelcast/client/exception/UTFDataFormatException.h"

namespace hazelcast {
    namespace util {
        int32_t UTFUtil::isValidUTF8(const std::string &str) {
            int32_t numberOfUtf8Chars = 0;
            for (size_t i = 0, len = str.length(); i < len; ++i) {
                unsigned char c = (unsigned char) str[i];
                size_t n = 0;
                // is ascii
                if (c <= 0x7f) {
                    n = 0; // 0bbbbbbb
                } else if ((c & 0xE0) == 0xC0) {
                    n = 1; // 110bbbbb
                } else if (c == 0xed && i < (len - 1) && ((unsigned char) str[i + 1] & 0xa0) == 0xa0) {
                    return -1; //U+d800 to U+dfff
                } else if ((c & 0xF0) == 0xE0) {
                    n = 2; // 1110bbbb
                } else if ((c & 0xF8) == 0xF0) {
                    n = 3; // 11110bbb
                } else {
                    return -1;
                }

                for (size_t j = 0; j < n && i < len; j++) { // n bytes matching 10bbbbbb follow ?
                    if ((++i == len) || (((unsigned char) str[i] & 0xC0) != 0x80)) {
                        return -1;
                    }
                }

                ++numberOfUtf8Chars;
            }

            return numberOfUtf8Chars;
        }

        void UTFUtil::readUTF8Char(UTFUtil::ByteReadable &in, byte firstByte, std::vector<char> &utfBuffer) {
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
                byte b = in.readByte();
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

        UTFUtil::ByteReadable::~ByteReadable() {
        }
    }
}
