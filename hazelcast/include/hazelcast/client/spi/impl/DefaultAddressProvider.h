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

#ifndef HAZELCAST_CLIENT_SPI_IMPL_DEFAULTADDRESSPROVIDER_H
#define HAZELCAST_CLIENT_SPI_IMPL_DEFAULTADDRESSPROVIDER_H

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/client/connection/AddressProvider.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace config {
            class ClientNetworkConfig;
        }
        namespace spi {
            namespace impl {
                class HAZELCAST_API DefaultAddressProvider : public connection::AddressProvider {
                public:
                    DefaultAddressProvider(config::ClientNetworkConfig &networkConfig,
                                           bool noOtherAddressProviderExist);

                    virtual std::vector<Address> loadAddresses();

                private:
                    config::ClientNetworkConfig &networkConfig;
                    bool noOtherAddressProviderExist;

                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_CLIENT_SPI_IMPL_DEFAULTADDRESSPROVIDER_H
