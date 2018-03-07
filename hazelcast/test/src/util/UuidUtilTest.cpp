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
#include <hazelcast/util/UuidUtil.h>

namespace hazelcast {
    namespace client {
        namespace test {
            class UuidUtilTest : public ::testing::Test {
            };

            TEST_F(UuidUtilTest, testUnsecureUuid) {
                util::UUID uuid1 = util::UuidUtil::newUnsecureUUID();
                util::UUID uuid2 = util::UuidUtil::newUnsecureUUID();
                ASSERT_NE(uuid1, uuid2);

                std::string uuid1String = uuid1.toString();
                std::string uuid2String = uuid2.toString();
                ASSERT_NE(uuid1String, uuid2String);
                ASSERT_EQ(36, uuid1String.length());
                ASSERT_EQ(36, uuid2String.length());

                std::stringstream ss(uuid1String);
                std::string token;
                ASSERT_TRUE(std::getline(ss, token, '-'));
                ASSERT_EQ(8, token.length());
                ASSERT_TRUE(std::getline(ss, token, '-'));
                ASSERT_EQ(4, token.length());
                ASSERT_TRUE(std::getline(ss, token, '-'));
                ASSERT_EQ(4, token.length());
                ASSERT_TRUE(std::getline(ss, token, '-'));
                ASSERT_EQ(4, token.length());
                ASSERT_TRUE(std::getline(ss, token, '-'));
                ASSERT_EQ(12, token.length());
                ASSERT_FALSE(std::getline(ss, token, '-'));
            }
        }
    }
}
