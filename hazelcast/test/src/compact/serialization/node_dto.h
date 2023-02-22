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

#include "hazelcast/client/serialization/serialization.h"

namespace hazelcast {
namespace client {
namespace test {
namespace compact {

struct node_dto
{
    int id;
    std::shared_ptr<node_dto> child;
};

bool
operator==(const node_dto& lhs, const node_dto& rhs)
{
    return lhs.id == rhs.id &&
           (lhs.child == rhs.child ||
            (lhs.child && rhs.child && *lhs.child == *rhs.child));
}

} // namespace compact
} // namespace test

namespace serialization {

template<>
struct hz_serializer<test::compact::node_dto>
  : public compact::compact_serializer
{
    static void write(const test::compact::node_dto& object,
                      compact::compact_writer& writer)
    {
        writer.write_int32("id", object.id);
        writer.write_compact<test::compact::node_dto>(
          "child",
          object.child == nullptr ? boost::none
                                  : boost::make_optional(*object.child));
    }

    static test::compact::node_dto read(compact::compact_reader& reader)
    {
        auto id = reader.read_int32("id");
        auto&& child = reader.read_compact<test::compact::node_dto>("child");
        return test::compact::node_dto{
            id,
            child.has_value()
              ? std::make_shared<test::compact::node_dto>(child.value())
              : nullptr
        };
    }

    static std::string type_name() { return "node"; }
};

} // namespace serialization
} // namespace client
} // namespace hazelcast
