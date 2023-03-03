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

#include "hazelcast/client/serialization/serialization.h"
#include "nested_type.h"

namespace hazelcast {
namespace client {
namespace test {
namespace compact {

struct a_type
{
    int x;
    nested_type nested;
};

} // namespace compact
} // namespace test

namespace serialization {

template<>
struct hz_serializer<test::compact::a_type> : compact::compact_serializer
{
    static void write(const test::compact::a_type& object,
                      compact::compact_writer& writer)
    {
        writer.write_int32("x", object.x);
        writer.write_compact<test::compact::nested_type>("nested",
                                                         object.nested);
    }

    static test::compact::a_type read(compact::compact_reader& reader)
    {
        test::compact::a_type object;

        object.x = reader.read_int32("x");
        object.nested =
          *reader.read_compact<test::compact::nested_type>("nested");

        return object;
    }

    static std::string type_name() { return "a_type"; }
};

} // namespace serialization
} // namespace client
} // namespace hazelcast
