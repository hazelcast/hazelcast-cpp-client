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
//
// Created by İhsan Demir on 18/05/15.
//

#include <gtest/gtest.h>
#include "hazelcast/util/Bits.h"

namespace hazelcast {
    namespace client {
        namespace test {
            namespace util {
                class BitsTest : public ::testing::Test
                {};

                TEST_F(BitsTest, testLittleEndian) {
                    uint64_t ONE = 1;
                    uint64_t oneByteFactor = ONE << 8;
                    uint64_t twoBytesFactor = ONE << 16;
                    uint64_t threeBytesFactor = ONE << 24;
                    uint64_t fourBytesFactor = ONE << 32;
                    uint64_t fiveBytesFactor = ONE << 40;
                    uint64_t sixBytesFactor = ONE << 48;
                    uint64_t sevenBytesFactor = ONE << 56;

                    {
                        int16_t expected = 0x7A * 256 + 0xBC;
                        int16_t actual;
                        byte *resBytes = (byte *)&actual;
                        hazelcast::util::Bits::nativeToLittleEndian2(&expected, &actual);
                        ASSERT_EQ(0xBC, resBytes[0]);
                        ASSERT_EQ(0x7A, resBytes[1]);
                    }

                    {
						int32_t expected = 0x1A * (int32_t)threeBytesFactor +
							0x9A * (int32_t)twoBytesFactor + 0xAA * (int32_t)oneByteFactor + 0xBA;
                        int32_t actual;
                        byte *resBytes = (byte *)&actual;
                        hazelcast::util::Bits::nativeToLittleEndian4(&expected, &actual);
                        ASSERT_EQ(0xBA, resBytes[0]);
                        ASSERT_EQ(0xAA, resBytes[1]);
                        ASSERT_EQ(0x9A, resBytes[2]);
                        ASSERT_EQ(0x1A, resBytes[3]);
                    }

                    {
                        int64_t expected =
                                0x1A * sevenBytesFactor +
                                0x2A * sixBytesFactor +
                                0x3A * fiveBytesFactor +
                                0x4A * fourBytesFactor +
                                0x5A * threeBytesFactor +
                                0x6A * twoBytesFactor +
                                0x7A * oneByteFactor +
                                0x8A;

                        int64_t actual;
                        byte *resBytes = (byte *)&actual;
                        hazelcast::util::Bits::nativeToLittleEndian8(&expected, &actual);
                        ASSERT_EQ(0x8A, resBytes[0]);
                        ASSERT_EQ(0x7A, resBytes[1]);
                        ASSERT_EQ(0x6A, resBytes[2]);
                        ASSERT_EQ(0x5A, resBytes[3]);
                        ASSERT_EQ(0x4A, resBytes[4]);
                        ASSERT_EQ(0x3A, resBytes[5]);
                        ASSERT_EQ(0x2A, resBytes[6]);
                        ASSERT_EQ(0x1A, resBytes[7]);
                    }

                    // Little to Native tests
                    {
                        byte source[2] = {0xAB, 0xBC};
                        uint16_t actual;
                        hazelcast::util::Bits::littleEndianToNative2(&source, &actual);
                        ASSERT_EQ(0xBC * oneByteFactor + 0xAB, actual);
                    }

                    {
                        byte source[4] = {0xAB, 0xBC, 0xDE, 0xA1};
                        uint32_t actual;
                        hazelcast::util::Bits::littleEndianToNative4(&source, &actual);
                        ASSERT_EQ(0xA1 * threeBytesFactor +
                                     0xDE * twoBytesFactor +
                                     0xBC * oneByteFactor +
                                     0xAB, actual);
                    }

                    {
                        byte source[8] = {0xAB, 0x9B, 0x8B, 0x7B, 0x6B, 0x5B, 0x4B, 0xA1};
                        uint64_t actual;
                        hazelcast::util::Bits::littleEndianToNative8(&source, &actual);
                        ASSERT_EQ(0xA1 * sevenBytesFactor +
                                     0x4B * sixBytesFactor +
                                     0x5B * fiveBytesFactor +
                                     0x6B * fourBytesFactor +
                                     0x7B * threeBytesFactor +
                                     0x8B * twoBytesFactor +
                                     0x9B * oneByteFactor +
                                     0xAB, actual);
                    }
                }
            }
        }
    }
}
