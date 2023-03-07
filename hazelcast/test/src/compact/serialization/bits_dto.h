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

namespace hazelcast {
namespace client {
namespace test {
namespace compact {

struct bits_dto
{
    bool a = false;
    bool b = false;
    bool c = false;
    bool d = false;
    bool e = false;
    bool f = false;
    bool g = false;
    bool h = false;
    bool i = false;
    int id = 0;
    boost::optional<std::vector<bool>> booleans;
};

bool
operator==(const bits_dto& lhs, const bits_dto& rhs)
{
    return lhs.a == rhs.a && lhs.b == rhs.b && lhs.c == rhs.c &&
           lhs.d == rhs.d && lhs.e == rhs.e && lhs.f == rhs.f &&
           lhs.g == rhs.g && lhs.h == rhs.h && lhs.i == rhs.i &&
           lhs.id == rhs.id && lhs.booleans == rhs.booleans;
}

} // namespace compact
} // namespace test

namespace serialization {

template<>
struct hz_serializer<test::compact::bits_dto>
  : public compact::compact_serializer
{
    static void write(const test::compact::bits_dto& dto,
                      compact::compact_writer& writer)
    {
        writer.write_boolean("a", dto.a);
        writer.write_boolean("b", dto.b);
        writer.write_boolean("c", dto.c);
        writer.write_boolean("d", dto.d);
        writer.write_boolean("e", dto.e);
        writer.write_boolean("f", dto.f);
        writer.write_boolean("g", dto.g);
        writer.write_boolean("h", dto.h);
        writer.write_boolean("i", dto.i);
        writer.write_int32("id", dto.id);
        writer.write_array_of_boolean("booleans", dto.booleans);
    }

    static test::compact::bits_dto read(compact::compact_reader& reader)
    {
        test::compact::bits_dto dto;
        dto.a = reader.read_boolean("a");
        dto.b = reader.read_boolean("b");
        dto.c = reader.read_boolean("c");
        dto.d = reader.read_boolean("d");
        dto.e = reader.read_boolean("e");
        dto.f = reader.read_boolean("f");
        dto.g = reader.read_boolean("g");
        dto.h = reader.read_boolean("h");
        dto.i = reader.read_boolean("i");
        dto.id = reader.read_int32("id");
        dto.booleans = reader.read_array_of_boolean("booleans");
        return dto;
    }

    static std::string type_name() { return "bits_dto"; }
};

} // namespace serialization

} // namespace client
} // namespace hazelcast
