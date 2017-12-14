/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
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
#include <hazelcast/client/serialization/pimpl/DataOutput.h>

namespace hazelcast {
    namespace client {
        namespace test {
            namespace util {
                class DataOutputTest : public ::testing::Test
                {};

                TEST_F(DataOutputTest, testWriteByte) {
                    std::vector<byte> bytes;
                    bytes.push_back(0x01);
                    bytes.push_back(0x12);
                    serialization::pimpl::DataOutput dataOutput;
                    dataOutput.writeByte((byte)0x01);
                    dataOutput.writeByte(0x12);
                    ASSERT_EQ(bytes, *dataOutput.toByteArray());
                }

                TEST_F(DataOutputTest, testWriteBoolean) {
                    std::vector<byte> bytes;
                    bytes.push_back(0x00);
                    bytes.push_back(0x01);
                    serialization::pimpl::DataOutput dataOutput;
                    dataOutput.writeBoolean(false);
                    dataOutput.writeBoolean(true);
                    ASSERT_EQ(bytes, *dataOutput.toByteArray());
                }

                TEST_F(DataOutputTest, testWriteChar) {
                    std::vector<byte> bytes;
                    bytes.push_back(0);
                    bytes.push_back('b');
                    serialization::pimpl::DataOutput dataOutput;
                    dataOutput.writeChar('b');
                    ASSERT_EQ(bytes, *dataOutput.toByteArray());
                }

                TEST_F(DataOutputTest, testWriteShort) {
                    std::vector<byte> bytes;
                    bytes.push_back(0x12);
                    bytes.push_back(0x34);
                    serialization::pimpl::DataOutput dataOutput;
                    dataOutput.writeShort(0x1234);
                    ASSERT_EQ(bytes, *dataOutput.toByteArray());
                }

                TEST_F(DataOutputTest, testWriteInteger) {
                    std::vector<byte> bytes;
                    bytes.push_back(0x12);
                    bytes.push_back(0x34);
                    bytes.push_back(0x56);
                    bytes.push_back(0x78);
                    serialization::pimpl::DataOutput dataOutput;
                    dataOutput.writeInt(0x12345678);
                    ASSERT_EQ(bytes, *dataOutput.toByteArray());
                }

                TEST_F(DataOutputTest, testWriteLong) {
                    std::vector<byte> bytes;
                    bytes.push_back(0x12);
                    bytes.push_back(0x34);
                    bytes.push_back(0x56);
                    bytes.push_back(0x78);
                    bytes.push_back(0x90);
                    bytes.push_back(0x9A);
                    bytes.push_back(0x9B);
                    bytes.push_back(0x9C);
                    serialization::pimpl::DataOutput dataOutput;
                    dataOutput.writeLong(0x12345678909A9B9C);
                    ASSERT_EQ(bytes, *dataOutput.toByteArray());
                }

                TEST_F(DataOutputTest, testWriteUTF) {
                    std::vector<byte> bytes;
                    bytes.push_back(0x00);
                    bytes.push_back(0x00);
                    bytes.push_back(0x00);
                    bytes.push_back(0x04);
                    bytes.push_back('b');
                    bytes.push_back('d');
                    bytes.push_back('f');
                    bytes.push_back('h');
                    serialization::pimpl::DataOutput dataOutput;
                    std::string value("bdfh");
                    dataOutput.writeUTF(&value);
                    ASSERT_EQ(bytes, *dataOutput.toByteArray());
                }

                TEST_F(DataOutputTest, testWriteByteArray) {
                    std::vector<byte> bytes;
                    bytes.push_back(0x00);
                    bytes.push_back(0x00);
                    bytes.push_back(0x00);
                    bytes.push_back(0x02);
                    std::vector<byte> actualDataBytes;
                    actualDataBytes.push_back(0x12);
                    actualDataBytes.push_back(0x34);
                    bytes.insert(bytes.end(), actualDataBytes.begin(), actualDataBytes.end());
                    serialization::pimpl::DataOutput dataOutput;
                    dataOutput.writeByteArray(&actualDataBytes);
                    ASSERT_EQ(bytes, *dataOutput.toByteArray());
                }

                TEST_F(DataOutputTest, testWriteBooleanArray) {
                    std::vector<byte> bytes;
                    bytes.push_back(0x00);
                    bytes.push_back(0x00);
                    bytes.push_back(0x00);
                    bytes.push_back(0x02);
                    bytes.push_back(0x00);
                    bytes.push_back(0x01);
                    std::vector<bool> actualValues;
                    actualValues.push_back(false);
                    actualValues.push_back(true);
                    serialization::pimpl::DataOutput dataOutput;
                    dataOutput.writeBooleanArray(&actualValues);
                    ASSERT_EQ(bytes, *dataOutput.toByteArray());
                }

                TEST_F(DataOutputTest, testWriteCharArray) {
                    std::vector<byte> bytes;
                    bytes.push_back(0x00);
                    bytes.push_back(0x00);
                    bytes.push_back(0x00);
                    bytes.push_back(0x02);
                    std::vector<char> actualChars;
                    bytes.push_back(0);
                    bytes.push_back('f');
                    actualChars.push_back('f');
                    bytes.push_back(0);
                    bytes.push_back('h');
                    actualChars.push_back('h');
                    serialization::pimpl::DataOutput dataOutput;
                    dataOutput.writeCharArray(&actualChars);
                    ASSERT_EQ(bytes, *dataOutput.toByteArray());
                }

                TEST_F(DataOutputTest, testWriteShortArray) {
                    std::vector<byte> bytes;
                    bytes.push_back(0x00);
                    bytes.push_back(0x00);
                    bytes.push_back(0x00);
                    bytes.push_back(0x02);
                    std::vector<int16_t> actualValues;
                    bytes.push_back(0x12);
                    bytes.push_back(0x34);
                    actualValues.push_back(0x1234);
                    bytes.push_back(0x56);
                    bytes.push_back(0x78);
                    actualValues.push_back(0x5678);
                    serialization::pimpl::DataOutput dataOutput;
                    dataOutput.writeShortArray(&actualValues);
                    ASSERT_EQ(bytes, *dataOutput.toByteArray());
                }

                TEST_F(DataOutputTest, testWriteIntegerArray) {
                    std::vector<byte> bytes;
                    bytes.push_back(0x00);
                    bytes.push_back(0x00);
                    bytes.push_back(0x00);
                    bytes.push_back(0x02);
                    std::vector<int32_t> actualValues;
                    bytes.push_back(0x12);
                    bytes.push_back(0x34);
                    bytes.push_back(0x56);
                    bytes.push_back(0x78);
                    actualValues.push_back(0x12345678);
                    bytes.push_back(0x9A);
                    bytes.push_back(0xBC);
                    bytes.push_back(0xDE);
                    bytes.push_back(0xEF);
                    actualValues.push_back(0x9ABCDEEF);
                    serialization::pimpl::DataOutput dataOutput;
                    dataOutput.writeIntArray(&actualValues);
                    ASSERT_EQ(bytes, *dataOutput.toByteArray());
                }

                TEST_F(DataOutputTest, testWriteLongArray) {
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
                    bytes.push_back(0xA1);
                    bytes.push_back(0xA2);
                    bytes.push_back(0xA3);
                    bytes.push_back(0xA4);
                    bytes.push_back(0xA5);
                    bytes.push_back(0xA6);
                    bytes.push_back(0xA7);
                    bytes.push_back(0xA8);
                    std::vector<int64_t> actualValues;
                    actualValues.push_back(0x123456789ABCDEEF);
                    actualValues.push_back(0xA1A2A3A4A5A6A7A8);
                    serialization::pimpl::DataOutput dataOutput;
                    dataOutput.writeLongArray(&actualValues);
                    ASSERT_EQ(bytes, *dataOutput.toByteArray());
                }
            }
        }
    }
}
