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

#include <stdexcept>

#include <boost/uuid/uuid.hpp>
#include <boost/optional.hpp>

#include "hazelcast/util/export.h"

namespace hazelcast {
namespace client {
namespace sql {

class HAZELCAST_API hazelcast_sql_exception : public std::runtime_error
{
public:
    hazelcast_sql_exception(boost::uuids::uuid originating_member_id,
                            int32_t code,
                            const boost::optional<std::string>& message,
                            const boost::optional<std::string>& suggestion)
    : std::runtime_error(message ? message.value() : "")
    , originating_member_id_(originating_member_id)
    , code_(code)
    , suggestion_(suggestion)
    {}

    boost::uuids::uuid originating_member_id() const {
        return originating_member_id_;
    }

    int32_t code() const {
        return code_;
    }

    const boost::optional<std::string>& suggestion() const {
        return suggestion_;
    }

private:
    boost::uuids::uuid originating_member_id_;
    int32_t code_;
    boost::optional<std::string> suggestion_;
};

} // namespace sql
} // namespace client
} // namespace hazelcast
