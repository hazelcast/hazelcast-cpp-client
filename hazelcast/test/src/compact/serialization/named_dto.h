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

struct named_dto
{
    boost::optional<std::string> name;
    int my_int;
};

bool
operator==(const named_dto& lhs, const named_dto& rhs)
{
    return lhs.name == rhs.name && lhs.my_int == rhs.my_int;
}

} // namespace compact
} // namespace test

namespace serialization {

template<>
struct hz_serializer<test::compact::named_dto> : public compact_serializer
{
    static void write(const test::compact::named_dto& dto,
                      compact_writer& writer)
    {
        writer.write_string("name", dto.name);
        writer.write_int32("my_int", dto.my_int);
    }

    static test::compact::named_dto read(compact_reader& reader)
    {
        test::compact::named_dto dto;
        dto.name = reader.read_string("name");
        dto.my_int = reader.read_int32("my_int");
        return dto;
    }

    static std::string type_name() { return "named_dto"; }
};

} // namespace serialization
} // namespace client
} // namespace hazelcast