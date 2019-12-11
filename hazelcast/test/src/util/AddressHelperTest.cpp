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

#include <ClientTestSupport.h>
#include <hazelcast/util/AddressHelper.h>

namespace hazelcast {
    namespace client {
        namespace test {
            class AddressHelperTest : public ClientTestSupport {
            };

            TEST_F(AddressHelperTest, testGetPossibleSocketAddresses) {
                std::string address("10.2.3.1");
                std::vector<Address> addresses = util::AddressHelper::getSocketAddresses(address, getLogger());
                ASSERT_EQ(3U, addresses.size());
                std::set<Address> socketAddresses;
                socketAddresses.insert(addresses.begin(), addresses.end());
                ASSERT_NE(socketAddresses.end(), socketAddresses.find(Address(address, 5701)));
                ASSERT_NE(socketAddresses.end(), socketAddresses.find(Address(address, 5702)));
                ASSERT_NE(socketAddresses.end(), socketAddresses.find(Address(address, 5703)));
            }

            TEST_F(AddressHelperTest, testAddressHolder) {
                util::AddressHolder holder("127.0.0.1", "en0", 8000);
                ASSERT_EQ("127.0.0.1", holder.getAddress());
                ASSERT_EQ(8000, holder.getPort());
                ASSERT_EQ("en0", holder.getScopeId());
            }
        }
    }
}

