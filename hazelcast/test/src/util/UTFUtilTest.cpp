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

#include <gtest/gtest.h>

#include <hazelcast/client/exception/IOException.h>
#include <hazelcast/client/serialization/pimpl/DataInput.h>
#include "hazelcast/util/UTFUtil.h"

namespace hazelcast {
    namespace client {
        namespace test {
            namespace util {
                class UTFUtilTest : public ::testing::Test {
                protected:
                    // includes one, two three and 4 byte code points
                    static const std::string VALID_UTF_STRING;

                    static const std::string INVALID_UTF_STRING_INSUFFICIENT_BYTES;

                    class ByteReader : public hazelcast::util::UTFUtil::ByteReadable {
                    public:
                        ByteReader(const std::string &value) : internalString(value) {
                            it = internalString.begin();
                        }

                        virtual byte readByte() {
                            if (it == internalString.end()) {
                                throw exception::IOException("ByteReader::readByte", "End of string is reached!");
                            }
                            byte b = *it;
                            ++it;
                            return b;
                        }

                    private:
                        std::string internalString;
                        std::string::const_iterator it;
                    };
                };

                const std::string UTFUtilTest::VALID_UTF_STRING = "a \xc3\xa9 \xe5\x92\xa7 \xf6\xa7\x93\xb5";
                const std::string UTFUtilTest::INVALID_UTF_STRING_INSUFFICIENT_BYTES = "a \xc3\xa9 \xe5\x92 \xf6\xa7\x93\xb5";

                TEST_F(UTFUtilTest, validUTF8) {
                    ASSERT_GT(hazelcast::util::UTFUtil::isValidUTF8(VALID_UTF_STRING), 0);
                }

                TEST_F(UTFUtilTest, invalidUTF8) {
                    ASSERT_EQ(-1, hazelcast::util::UTFUtil::isValidUTF8(INVALID_UTF_STRING_INSUFFICIENT_BYTES));
                }

                TEST_F(UTFUtilTest, readValidUTF8) {
                    ByteReader in(VALID_UTF_STRING);
                    std::vector<char> utfBuffer;
                    utfBuffer.reserve(
                            client::serialization::pimpl::DataInput::MAX_UTF_CHAR_SIZE * VALID_UTF_STRING.size());
                    int numberOfUtfChars = hazelcast::util::UTFUtil::isValidUTF8(VALID_UTF_STRING);
                    size_t index = 0;
                    for (int i = 0; i < numberOfUtfChars; ++i) {
                        byte c = in.readByte();
                        hazelcast::util::UTFUtil::readUTF8Char(in, c, utfBuffer, index);
                    }

                    std::string result(utfBuffer.begin(), utfBuffer.begin() + index);
                    ASSERT_EQ(VALID_UTF_STRING, result);
                }

                TEST_F(UTFUtilTest, readInvalidUTF8) {
                    ByteReader in(INVALID_UTF_STRING_INSUFFICIENT_BYTES);
                    std::vector<char> utfBuffer;
                    utfBuffer.reserve(
                            client::serialization::pimpl::DataInput::MAX_UTF_CHAR_SIZE * VALID_UTF_STRING.size());
                    size_t index = 0;
                    for (int i = 0; i < 5; ++i) {
                        byte c = in.readByte();
                        if (i ==
                            4) {  // The 4th utf character is missing one byte intentionally in the invalid utf string
                            ASSERT_THROW(hazelcast::util::UTFUtil::readUTF8Char(in, c, utfBuffer, index),
                                         exception::UTFDataFormatException);
                        } else {
                            hazelcast::util::UTFUtil::readUTF8Char(in, c, utfBuffer, index);
                        }
                    }
                }
            }
        }
    }
}
