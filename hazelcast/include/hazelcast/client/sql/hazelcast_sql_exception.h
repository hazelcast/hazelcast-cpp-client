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

#include <boost/uuid/uuid.hpp>
#include <boost/optional.hpp>

#include "hazelcast/util/export.h"
#include "hazelcast/client/exception/protocol_exceptions.h"

namespace hazelcast {
namespace client {
namespace sql {

class HAZELCAST_API hazelcast_sql_exception : public exception::hazelcast_
{
public:
    hazelcast_sql_exception(std::string source,
                            boost::uuids::uuid originating_member_id,
                            int32_t code,
                            boost::optional<std::string> message,
                            boost::optional<std::string> suggestion,
                            std::exception_ptr cause = nullptr);

    /**
     * Gets ID of the member that caused or initiated an error condition.
     */
    const boost::uuids::uuid& originating_member_id() const;

    /**
     * Gets the internal error code associated with the exception.
     */
    int32_t code() const;

    /**
     * Gets the suggested SQL statement to remediate experienced error
     */
    const boost::optional<std::string>& suggestion() const;

private:
    boost::uuids::uuid originating_member_id_;
    int32_t code_;
    boost::optional<std::string> suggestion_;
};

} // namespace sql
} // namespace client
} // namespace hazelcast
