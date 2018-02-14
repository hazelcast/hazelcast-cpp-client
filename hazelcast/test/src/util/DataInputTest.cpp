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

#include <gtest/gtest.h>
#include <hazelcast/client/serialization/pimpl/DataInput.h>

namespace hazelcast {
    namespace client {
        namespace test {
            namespace util {
                class DataInputTest : public ::testing::Test
                {};

                TEST_F(DataInputTest, testReadByte) {
                    std::vector<byte> bytes;
                    bytes.push_back(0x01);
                    bytes.push_back(0x12);
                    serialization::pimpl::DataInput dataInput(bytes);
                    ASSERT_EQ(0x01, dataInput.readByte());
                    ASSERT_EQ(0x12, dataInput.readByte());
                }

                TEST_F(DataInputTest, testReadBoolean) {
                    std::vector<byte> bytes;
                    bytes.push_back(0x00);
                    bytes.push_back(0x10);
                    serialization::pimpl::DataInput dataInput(bytes);
                    ASSERT_FALSE(dataInput.readBoolean());
                    ASSERT_TRUE(dataInput.readBoolean());
                }

                TEST_F(DataInputTest, testReadChar) {
                    std::vector<byte> bytes;
                    bytes.push_back('a');
                    bytes.push_back('b');
                    serialization::pimpl::DataInput dataInput(bytes);
                    ASSERT_EQ('b', dataInput.readChar());
                }

                TEST_F(DataInputTest, testReadShort) {
                    std::vector<byte> bytes;
                    bytes.push_back(0x12);
                    bytes.push_back(0x34);
                    bytes.push_back(0x56);
                    serialization::pimpl::DataInput dataInput(bytes);
                    ASSERT_EQ(0x1234, dataInput.readShort());
                }

                TEST_F(DataInputTest, testReadInteger) {
                    std::vector<byte> bytes;
                    bytes.push_back(0x12);
                    bytes.push_back(0x34);
                    bytes.push_back(0x56);
                    bytes.push_back(0x78);
                    bytes.push_back(0x90);
                    serialization::pimpl::DataInput dataInput(bytes);
                    ASSERT_EQ(INT32_C(0x12345678), dataInput.readInt());
                }

                TEST_F(DataInputTest, testReadLong) {
                    std::vector<byte> bytes;
                    bytes.push_back(0x12);
                    bytes.push_back(0x34);
                    bytes.push_back(0x56);
                    bytes.push_back(0x78);
                    bytes.push_back(0x90);
                    bytes.push_back(0x9A);
                    bytes.push_back(0x9B);
                    bytes.push_back(0x9C);
                    serialization::pimpl::DataInput dataInput(bytes);
                    ASSERT_EQ(INT64_C(0x12345678909A9B9C), dataInput.readLong());
                }

                TEST_F(DataInputTest, testReadUTF) {
                    std::vector<byte> bytes;
                    bytes.push_back(0x00);
                    bytes.push_back(0x00);
                    bytes.push_back(0x00);
                    bytes.push_back(0x04);
                    bytes.push_back('b');
                    bytes.push_back('d');
                    bytes.push_back('f');
                    bytes.push_back('h');
                    serialization::pimpl::DataInput dataInput(bytes);
                    std::auto_ptr<std::string> utf = dataInput.readUTF();
                    ASSERT_NE((std::string *) NULL, utf.get());
                    ASSERT_EQ("bdfh", *utf);
                }

                TEST_F(DataInputTest, testReadByteArray) {
                    std::vector<byte> bytes;
                    bytes.push_back(0x00);
                    bytes.push_back(0x00);
                    bytes.push_back(0x00);
                    bytes.push_back(0x02);
                    std::vector<byte> actualDataBytes;
                    actualDataBytes.push_back(0x12);
                    actualDataBytes.push_back(0x34);
                    bytes.insert(bytes.end(), actualDataBytes.begin(), actualDataBytes.end());
                    serialization::pimpl::DataInput dataInput(bytes);
                    std::auto_ptr<std::vector<byte> > readBytes = dataInput.readByteArray();
                    ASSERT_NE((std::vector<byte> *) NULL, readBytes.get());
                    ASSERT_EQ(actualDataBytes, *readBytes);
                }

                TEST_F(DataInputTest, testReadBooleanArray) {
                    std::vector<byte> bytes;
                    bytes.push_back(0x00);
                    bytes.push_back(0x00);
                    bytes.push_back(0x00);
                    bytes.push_back(0x02);
                    bytes.push_back(0x00);
                    bytes.push_back(0x01);
                    serialization::pimpl::DataInput dataInput(bytes);
                    std::auto_ptr<std::vector<bool> > booleanArray = dataInput.readBooleanArray();
                    ASSERT_NE((std::vector<bool> *) NULL, booleanArray.get());
                    ASSERT_EQ(2U, booleanArray->size());
                    ASSERT_FALSE((*booleanArray)[0]);
                    ASSERT_TRUE((*booleanArray)[1]);
                }

                TEST_F(DataInputTest, testReadCharArray) {
                    std::vector<byte> bytes;
                    bytes.push_back(0x00);
                    bytes.push_back(0x00);
                    bytes.push_back(0x00);
                    bytes.push_back(0x02);
                    bytes.push_back(0);
                    bytes.push_back('f');
                    bytes.push_back(0);
                    bytes.push_back('h');
                    serialization::pimpl::DataInput dataInput(bytes);
                    std::auto_ptr<std::vector<char> > charArray = dataInput.readCharArray();
                    ASSERT_NE((std::vector<char> *) NULL, charArray.get());
                    ASSERT_EQ(2U, charArray->size());
                    ASSERT_EQ('f', (*charArray)[0]);
                    ASSERT_EQ('h', (*charArray)[1]);
            }

                TEST_F(DataInputTest, testReadShortArray) {
                    std::vector<byte> bytes;
                    bytes.push_back(0x00);
                    bytes.push_back(0x00);
                    bytes.push_back(0x00);
                    bytes.push_back(0x02);
                    bytes.push_back(0x12);
                    bytes.push_back(0x34);
                    bytes.push_back(0x56);
                    bytes.push_back(0x78);
                    serialization::pimpl::DataInput dataInput(bytes);
                    std::auto_ptr<std::vector<int16_t> > array = dataInput.readShortArray();
                    ASSERT_NE((std::vector<int16_t> *) NULL, array.get());
                    ASSERT_EQ(2U, array->size());
                    ASSERT_EQ(0x1234, (*array)[0]);
                    ASSERT_EQ(0x5678, (*array)[1]);
                }

                TEST_F(DataInputTest, testReadIntegerArray) {
                    std::vector<byte> bytes;
                    bytes.push_back(0x00);
                    bytes.push_back(0x00);
                    bytes.push_back(0x00);
                    bytes.push_back(0x02);
                    bytes.push_back(0x12);
                    bytes.push_back(0x34);
                    bytes.push_back(0x56);
                    bytes.push_back(0x78);
                    bytes.push_back(0x1A);
                    bytes.push_back(0xBC);
                    bytes.push_back(0xDE);
                    bytes.push_back(0xEF);
                    serialization::pimpl::DataInput dataInput(bytes);
                    std::auto_ptr<std::vector<int32_t> > array = dataInput.readIntArray();
                    ASSERT_NE((std::vector<int32_t> *) NULL, array.get());
                    ASSERT_EQ(2U, array->size());
                    ASSERT_EQ(INT32_C(0x12345678), (*array)[0]);
                    ASSERT_EQ(INT32_C(0x1ABCDEEF), (*array)[1]);
                }

                TEST_F(DataInputTest, testReadLongArray) {
                    std::vector<byte> bytes;
                    bytes.push_back(0x00);
                    bytes.push_back(0x00);
                    bytes.push_back(0x00);
                    bytes.push_back(0x02);
                    bytes.push_back(0x12);
                    bytes.push_back(0x34);
                    bytes.push_back(0x56);
                    bytes.push_back(0x78);
                    bytes.push_back(0x9A);
                    bytes.push_back(0xBC);
                    bytes.push_back(0xDE);
                    bytes.push_back(0xEF);
                    bytes.push_back(0x11);
                    bytes.push_back(0xA2);
                    bytes.push_back(0xA3);
                    bytes.push_back(0xA4);
                    bytes.push_back(0xA5);
                    bytes.push_back(0xA6);
                    bytes.push_back(0xA7);
                    bytes.push_back(0xA8);
                    serialization::pimpl::DataInput dataInput(bytes);
                    std::auto_ptr<std::vector<int64_t> > array = dataInput.readLongArray();
                    ASSERT_NE((std::vector<int64_t> *) NULL, array.get());
                    ASSERT_EQ(2U, array->size());
                    ASSERT_EQ(INT64_C(0x123456789ABCDEEF), (*array)[0]);
                    ASSERT_EQ(INT64_C(0x11A2A3A4A5A6A7A8), (*array)[1]);
                }
            }
        }
    }
}
