/*
 * Copyright (c) 2008-2023, Hazelcast, Inc. All Rights Reserved.
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

#include "hazelcast/client/exception/protocol_exceptions.h"

namespace hazelcast {
namespace client {
namespace serialization {
namespace pimpl {

struct compact_util
{
    compact_util() = delete;

    static exception::hazelcast_serialization
    exception_for_unexpected_null_value(const std::string& field_name,
                                        const std::string& method_prefix,
                                        const std::string& method_suffix);

    static exception::hazelcast_serialization
    exception_for_unexpected_null_value_in_array(
      const std::string& field_name,
      const std::string& method_prefix,
      const std::string& method_suffix);
};

} // namespace pimpl
} // namespace serialization
} // namespace client
} // namespace hazelcast