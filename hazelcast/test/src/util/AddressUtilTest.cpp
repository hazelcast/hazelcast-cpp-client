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
#include <hazelcast/util/AddressUtil.h>

namespace hazelcast {
    namespace client {
        namespace test {
            class AddressUtilTest : public ::testing::Test {
            };

            TEST_F(AddressUtilTest, testParsingHostAndPort) {
                util::AddressHolder addressHolder = util::AddressUtil::getAddressHolder(
                        "[fe80::62c5:*:fe05:480a%en0]:8080");
                ASSERT_EQ("fe80::62c5:*:fe05:480a", addressHolder.getAddress());
                ASSERT_EQ(8080, addressHolder.getPort());
                ASSERT_EQ("en0", addressHolder.getScopeId());

                addressHolder = util::AddressUtil::getAddressHolder("[::ffff:192.0.2.128]:5700");
                ASSERT_EQ("::ffff:192.0.2.128", addressHolder.getAddress());
                ASSERT_EQ(5700, addressHolder.getPort());

                addressHolder = util::AddressUtil::getAddressHolder("192.168.1.1:5700");
                ASSERT_EQ("192.168.1.1", addressHolder.getAddress());
                ASSERT_EQ(5700, addressHolder.getPort());

                addressHolder = util::AddressUtil::getAddressHolder("hazelcast.com:80");
                ASSERT_EQ("hazelcast.com", addressHolder.getAddress());
                ASSERT_EQ(80, addressHolder.getPort());
            }
            
            TEST_F(AddressUtilTest, testGetByNameIpV6) {
                std::string addrString("::1");
                asio::ip::address address = util::AddressUtil::getByName(addrString);
                ASSERT_TRUE(address.is_v6());
                ASSERT_FALSE(address.is_v4());
                ASSERT_EQ(addrString, address.to_string());
            }

            TEST_F(AddressUtilTest, testGetByNameIpV4) {
                std::string addrString("127.0.0.1");
                asio::ip::address address = util::AddressUtil::getByName(addrString);
                ASSERT_TRUE(address.is_v4());
                ASSERT_FALSE(address.is_v6());
                ASSERT_EQ(addrString, address.to_string());
            }
        }
    }
}
