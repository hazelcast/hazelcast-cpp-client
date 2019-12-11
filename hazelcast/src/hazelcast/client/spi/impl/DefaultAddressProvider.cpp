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

#include "hazelcast/client/spi/impl/DefaultAddressProvider.h"
#include "hazelcast/client/config/ClientNetworkConfig.h"

namespace hazelcast {
    namespace client {
        namespace spi {
            namespace impl {

                DefaultAddressProvider::DefaultAddressProvider(config::ClientNetworkConfig &networkConfig,
                                                               bool noOtherAddressProviderExist) : networkConfig(
                        networkConfig), noOtherAddressProviderExist(noOtherAddressProviderExist) {
                }

                std::vector<Address> DefaultAddressProvider::loadAddresses() {
                    std::vector<Address> addresses = networkConfig.getAddresses();
                    if (addresses.empty() && noOtherAddressProviderExist) {
                        addresses.push_back(Address("127.0.0.1", 5701));
                    }

                    // TODO Implement AddressHelper to add alternative ports for the same address

                    return addresses;
                }
            }
        }
    }
}
