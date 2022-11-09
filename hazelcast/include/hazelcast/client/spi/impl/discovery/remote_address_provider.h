/*
 * Copyright (c) 2008-2022, Hazelcast, Inc. All Rights Reserved.
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

#include <functional>
#include <unordered_map>
#include <mutex>

#include "hazelcast/util/export.h"
#include "hazelcast/client/connection/AddressProvider.h"

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4251) // for dll export
#endif

namespace hazelcast {
namespace client {
namespace spi {
namespace impl {
namespace discovery {
class HAZELCAST_API remote_address_provider : public connection::AddressProvider
{
public:
    remote_address_provider(
      std::function<std::unordered_map<address, address>()> addr_map_method,
      bool use_public);

    std::vector<address> load_addresses() override;

    util::optional<address> translate(const address& addr) override;

private:
    std::function<std::unordered_map<address, address>()> refresh_address_map_;
    const bool use_public_;
    std::mutex lock_;
    std::unordered_map<address, address> private_to_public_;
};
} // namespace discovery
} // namespace impl
} // namespace spi
} // namespace client
} // namespace hazelcast

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
