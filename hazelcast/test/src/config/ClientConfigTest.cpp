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

#include <ClientTestSupport.h>
#include <hazelcast/client/ClientConfig.h>

namespace hazelcast {
    namespace client {

        class HazelcastClient;

        namespace test {
            class ClientConfigTest : public ClientTestSupport
            {};

            TEST_F(ClientConfigTest, testGetAddresses) {
                ClientConfig clientConfig;
                Address address("localhost", 5555);
                clientConfig.getNetworkConfig().addAddress(address);

                std::set<Address, addressComparator> addresses = clientConfig.getAddresses();
                ASSERT_EQ(1U, addresses.size());
                ASSERT_EQ(address, *addresses.begin());
            }

            TEST_F(ClientConfigTest, testAddresseses) {
                ClientConfig clientConfig;
                std::vector<Address> addresses;
                addresses.push_back(Address("localhost", 5555));
                addresses.push_back(Address("localhost", 6666));
                clientConfig.getNetworkConfig().addAddresses(addresses);

                std::set<Address, addressComparator> configuredAddresses = clientConfig.getAddresses();
                ASSERT_EQ(2U, addresses.size());
                std::vector<Address> configuredAddressVector(configuredAddresses.begin(), configuredAddresses.end());
                ASSERT_EQ(addresses, configuredAddressVector);
            }
        }
    }
}

