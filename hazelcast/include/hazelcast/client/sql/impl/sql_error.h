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

#include <cstdint>
#include <string>

#include <boost/optional.hpp>
#include <boost/uuid/uuid.hpp>

#include "hazelcast/util/export.h"

namespace hazelcast {
namespace client {
namespace sql {
namespace impl {

struct HAZELCAST_API sql_error
{
    int32_t code;
    boost::optional<std::string> message;
    boost::uuids::uuid originating_member_id;
    boost::optional<std::string> suggestion;
};

} // namespace impl
} // namespace sql
} // namespace client
} // namespace hazelcast
