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

#include "hazelcast/util/export.h"

namespace hazelcast {
namespace client {
namespace sql {
namespace impl {
class HAZELCAST_API query_utils
{
public:
    static void throw_public_exception(std::exception_ptr e,
                                       boost::uuids::uuid id);

    /**
     * Finds a larger same-version group of data members from a collection of
     * members and, if {@code localMember} is from that group, return that.
     * Otherwise return a random member from the group. If the same-version
     * groups have the same size, return a member from the newer group
     * (preferably the local one).
     * <p>
     * Used for SqlExecute messages.
     *
     * @param members list of all members
     * @return the chosen member or none, if no data member is found
     */
    static boost::optional<member> member_of_same_larger_version_group(
      const std::vector<member>& members);
};
} // namespace impl
} // namespace sql
} // namespace client
} // namespace hazelcast
