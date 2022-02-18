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

#include <string>
#include <unordered_map>

#include "hazelcast/util/export.h"
#include "hazelcast/client/address.h"

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4251) // for dll export
#endif

namespace hazelcast {
class logger;

namespace client {
class client_properties;
namespace config {
class client_aws_config;
}
namespace aws {
class HAZELCAST_API aws_client
{
public:
    aws_client(std::chrono::steady_clock::duration timeout,
               config::client_aws_config& aws_config,
               const client_properties& client_properties,
               logger& lg);

    std::unordered_map<address, address> get_addresses();

private:
#ifdef HZ_BUILD_WITH_SSL
    std::chrono::steady_clock::duration timeout_;
    config::client_aws_config& aws_config_;
    std::string endpoint_;
    logger& logger_;
    int aws_member_port_;
#endif // HZ_BUILD_WITH_SSL
};
} // namespace aws
} // namespace client
} // namespace hazelcast

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
