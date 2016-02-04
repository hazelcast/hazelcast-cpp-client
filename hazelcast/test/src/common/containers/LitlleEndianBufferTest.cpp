/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
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
// Created by İhsan Demir on 17/05/15.
//

#include <cmath>
#include <gtest/gtest.h>

#include "hazelcast/util/LittleEndianBufferWrapper.h"

namespace hazelcast {
    namespace client {
        namespace test {
            namespace common {
                namespace containers {
                    class LittleEndianBufferTest : public ::testing::Test,
                                                   public util::LittleEndianBufferWrapper /* Need this in order to test*/
                    {};

                    TEST_F (LittleEndianBufferTest, testBinaryFormat) {
                        #define TEST_DATA_SIZE 8
                        #define LARGE_BUFFER_SIZE 20
                        #define START_BYTE_NUMBER  5

                        uint64_t ONE = 1;
                        uint64_t oneByteFactor = ONE << 8;
                        uint64_t twoBytesFactor = ONE << 16;
                        uint64_t threeBytesFactor = ONE << 24;
                        uint64_t fourBytesFactor = ONE << 32;
                        uint64_t fiveBytesFactor = ONE << 40;
                        uint64_t sixBytesFactor = ONE << 48;
                        uint64_t sevenBytesFactor = ONE << 56;

                        byte buf[TEST_DATA_SIZE] = {0x8A, 0x9A, 0xAA, 0xBA, 0xCA, 0xDA, 0xEA, 0x8B};
                        byte largeBuffer[LARGE_BUFFER_SIZE];
                        memcpy(largeBuffer + START_BYTE_NUMBER, buf, TEST_DATA_SIZE);

                        // ----- Test unsigned get starts ---------------------------------
                        // NOTE: When the first bit of the highest byte is equal to 1, than the number is negative,
                        // and the value is (-1 * (1s complement + 1)) (i.e. this is twos complement)
                        {
                            wrapForRead(largeBuffer, LARGE_BUFFER_SIZE, START_BYTE_NUMBER);
                            uint8_t result = getUint8();
                            ASSERT_EQ(0x8A, result);
                        }

                        {
                            wrapForRead(largeBuffer, LARGE_BUFFER_SIZE, START_BYTE_NUMBER);
                            uint16_t result = getUint16();
                            ASSERT_EQ(0x8A +
                                         0x9A * oneByteFactor, result);
                        }

                        {
                            wrapForRead(largeBuffer, LARGE_BUFFER_SIZE, START_BYTE_NUMBER);
                            uint32_t result = getUint32();
                            ASSERT_EQ(0x8A +
                                         0x9A * oneByteFactor +
                                         0xAA * twoBytesFactor +
                                         0xBA * threeBytesFactor , result);
                        }

                        {
                            wrapForRead(largeBuffer, LARGE_BUFFER_SIZE, START_BYTE_NUMBER);
                            uint64_t result = getUint64();
                            ASSERT_EQ(0x8A +
                                         0x9A * oneByteFactor +
                                         0xAA * twoBytesFactor +
                                         0xBA * threeBytesFactor +
                                         0xCA * fourBytesFactor +
                                         0xDA * fiveBytesFactor +
                                         0xEA * sixBytesFactor +
                                         0x8B * sevenBytesFactor, result);
                        }
                        // ----- Test unsigned get ends ---------------------------------

                        // ----- Test signed get starts ---------------------------------

                        {
                            wrapForRead(largeBuffer, LARGE_BUFFER_SIZE, START_BYTE_NUMBER);
                            int16_t result = getInt16();
                            ASSERT_EQ(-1 * (~((int16_t)(0x8A +
                                         0x9A * oneByteFactor)) + 1), result);
                        }

                        {
                            wrapForRead(largeBuffer, LARGE_BUFFER_SIZE, START_BYTE_NUMBER);
                            int32_t result = getInt32();
                            ASSERT_EQ(-1 * (~((int32_t)(
                                    0x8A +
                                    0x9A * oneByteFactor +
                                    0xAA * twoBytesFactor +
                                    0xBA * threeBytesFactor)) + 1), result);
                        }

                        {
                            wrapForRead(largeBuffer, LARGE_BUFFER_SIZE, START_BYTE_NUMBER);
                            int64_t result = getInt64();
                            ASSERT_EQ(-1 * (~((int64_t)(
                                    0x8A +
                                    0x9A * oneByteFactor +
                                    0xAA * twoBytesFactor +
                                    0xBA * threeBytesFactor +
                                    0xCA * fourBytesFactor +
                                    0xDA * fiveBytesFactor +
                                    0xEA * sixBytesFactor +
                                    0x8B * sevenBytesFactor)) + 1), result);
                        }
                        // ----- Test signed get ends ---------------------------------

                        const byte firstChar = 'B';
                        byte strBytes[8] = {4, 0, 0, 0, /* This part is the len field which is 4 bytes */
                                            firstChar, firstChar + 1, firstChar + 2, firstChar + 3}; // This is string BCDE

                        {
                            wrapForRead(strBytes, 8, 0);
                            ASSERT_EQ("BCDE", getStringUtf8());
                        }
                        
                        // ---- Test consecutive gets starts ---------------------------
                        {
                            byte continousBuffer[45] = {0x8A, 0x9A, 0xAA, 0xBA, 0xCA, 0xDA, 0xEA, 0x8B,
                                                                       0x8A, 0x9A, 0xAA, 0xBA, 0xCA, 0xDA, 0xEA, 0x8B,
                                                                       0x8A, 0x9A, 0xAA, 0xBA, 0xCA, 0xDA, 0xEA, 0x8B,
                                                                       0x8A, 0x9A, 0xAA, 0xBA, 0xCA,
                                                                       4, 0, 0, 0, /* This part is the len field which is 4 bytes */
                                                                       firstChar, firstChar + 1, firstChar + 2, firstChar + 3,
                                                                       0x8A, 0x01, 0x00, 0xBA, 0xCA, 0xDA, 0xEA, 0x8B};
                            
                            wrapForRead(continousBuffer, 8 * 10, 0);

                            {
                                uint8_t result = getUint8();
                                ASSERT_EQ(0x8A, result);
                            }

                            {
                                uint16_t result = getUint16();
                                ASSERT_EQ(0x9A +
                                             0xAA * oneByteFactor, result);
                            }

                            {
                                uint32_t result = getUint32();
                                ASSERT_EQ(0xBA +
                                             0xCA * oneByteFactor +
                                             0xDA * twoBytesFactor +
                                             0xEA * threeBytesFactor , result);
                            }

                            {
                                uint64_t result = getUint64();
                                ASSERT_EQ(0x8B +
                                             0x8A * oneByteFactor +
                                             0x9A * twoBytesFactor +
                                             0xAA * threeBytesFactor +
                                             0xBA * fourBytesFactor +
                                             0xCA * fiveBytesFactor +
                                             0xDA * sixBytesFactor +
                                             0xEA * sevenBytesFactor, result);
                            }
                            // ----- Test unsigned get ends ---------------------------------

                            // ----- Test signed get starts ---------------------------------

                            {
                                int16_t result = getInt16();
                                ASSERT_EQ(-1 * (~((int16_t)(0x8B +
                                                               0x8A * oneByteFactor)) + 1), result);
                            }

                            {
                                int32_t result = getInt32();
                                ASSERT_EQ(-1 * (~((int32_t)(
                                        0x9A +
                                        0xAA * oneByteFactor +
                                        0xBA * twoBytesFactor +
                                        0xCA * threeBytesFactor)) + 1), result);
                            }

                            {
                                int64_t result = getInt64();
                                ASSERT_EQ(-1 * (~((int64_t)(
                                        0xDA +
                                        0xEA * oneByteFactor +
                                        0x8B * twoBytesFactor +
                                        0x8A * threeBytesFactor +
                                        0x9A * fourBytesFactor +
                                        0xAA * fiveBytesFactor +
                                        0xBA * sixBytesFactor +
                                        0xCA * sevenBytesFactor)) + 1), result);
                            }
                            // ----- Test signed get ends ---------------------------------

                            {
                                ASSERT_EQ("BCDE", getStringUtf8());
                            }

                            {
                                bool result = getBoolean();
                                ASSERT_EQ(true, result);

                                result = getBoolean();
                                ASSERT_EQ(true, result);

                                result = getBoolean();
                                ASSERT_EQ(false, result);
                            }

                        }
                        // ---- Test consecutive gets ends ---------------------------

                        // ---- Write related tests starts --------------------------
                        byte writeBuffer[30];
                        wrapForWrite(writeBuffer, 30, 0);

                        set((uint8_t) 0x8A);
                        ASSERT_EQ(0x8A, writeBuffer[0]);
                        
                        set(true);
                        ASSERT_EQ(0x01, writeBuffer[1]);

                        set(false);
                        ASSERT_EQ(0x00, writeBuffer[2]);

                        set('C');
                        ASSERT_EQ('C', writeBuffer[3]);

                        int16_t int16Val = 0x7BCD;
                        set(int16Val);
                        ASSERT_EQ(0xCD, writeBuffer[4]);
                        ASSERT_EQ(0x7B, writeBuffer[5]);

                        uint16_t uInt16Val = 0xABCD;
                        set(uInt16Val);
                        ASSERT_EQ(0xCD, writeBuffer[6]);
                        ASSERT_EQ(0xAB, writeBuffer[7]);

                        int32_t int32Val = 0xAEBCEEFF;
                        set(int32Val);
                        ASSERT_EQ(0xFF, writeBuffer[8]);
                        ASSERT_EQ(0xEE, writeBuffer[9]);
                        ASSERT_EQ(0xBC, writeBuffer[10]);
                        ASSERT_EQ(0xAE, writeBuffer[11]);


                        set(std::string("Test Data"));
                        ASSERT_EQ(0x09, (int)writeBuffer[12]);
                        ASSERT_EQ(0x0, writeBuffer[13]);
                        ASSERT_EQ(0x0, writeBuffer[14]);
                        ASSERT_EQ(0x0, writeBuffer[15]);
                        ASSERT_EQ('T', writeBuffer[16]);
                        ASSERT_EQ('e', writeBuffer[17]);
                        ASSERT_EQ('a', writeBuffer[24]);
                    }
                }
            }
        }
    }
}
