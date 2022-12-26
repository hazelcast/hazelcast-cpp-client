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

#include "hazelcast/client/serialization/serialization.h"

namespace hazelcast {
namespace client {
namespace test {
namespace compact {

struct nested_type
{
    int y;
};

}
}

namespace serialization {

template<>
struct hz_serializer<test::compact::nested_type> : compact_serializer
{
    static void write(const test::compact::nested_type& object,
                      compact_writer& writer)
    {
        writer.write_int32("x", object.y);
    }

    static test::compact::nested_type read(compact_reader& reader)
    {
        test::compact::nested_type object;

        object.y = reader.read_int32("y");

        return object;
    }

    static std::string type_name() { return "nested_type"; }
};

}
}
}