/*
 * Copyright (c) 2008-2021, Hazelcast, Inc. All Rights Reserved.
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

#pragma once

#include <vector>

#include "hazelcast/util/export.h"
#include "hazelcast/client/address.h"

namespace hazelcast {
namespace client {
namespace connection {
/**
 * Provides initial addresses for client to find and connect to a node
 */
class HAZELCAST_API AddressProvider
{
public:
    /**
     * @return The possible member addresses to connect to.
     */
    virtual std::vector<address> load_addresses() = 0;

    /**
     * Translates the given address to another address specific to
     * network or service
     *
     * @param address to be translated
     * @return translated address or boost::none if no translation is found.
     */
    virtual boost::optional<address> translate(const address& addr) = 0;

    /**
     * @return true for the \DefaultAddressProvider , false otherwise.
     */
    virtual bool is_default_provider();

    virtual ~AddressProvider() = default;
};
} // namespace connection
} // namespace client
} // namespace hazelcast
