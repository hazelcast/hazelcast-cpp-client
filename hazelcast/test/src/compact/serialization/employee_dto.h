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

namespace hazelcast {
namespace client {
namespace test {
namespace compact {

struct employee_dto
{
    int32_t age;
    int32_t rank;
    int64_t id;
    bool isHired;
    bool isFired;
};

bool
operator==(const employee_dto& lhs, const employee_dto& rhs)
{
    return lhs.age == rhs.age && lhs.rank == rhs.rank && lhs.id == rhs.id &&
           lhs.isHired == rhs.isHired && lhs.isFired == rhs.isFired;
}

}
}

namespace serialization {

template<>
struct hz_serializer<test::compact::employee_dto> : public compact_serializer
{
    static void write(const test::compact::employee_dto& object,
                      compact_writer& writer)
    {
        writer.write_int32("age", object.age);
        writer.write_int32("rank", object.rank);
        writer.write_int64("id", object.id);
        writer.write_boolean("isHired", object.isHired);
        writer.write_boolean("isFired", object.isFired);
    }

    static test::compact::employee_dto read(compact_reader& reader)
    {
        auto age = reader.read_int32("age");
        auto rank = reader.read_int32("rank");
        auto id = reader.read_int64("id");
        auto isHired = reader.read_boolean("isHired");
        auto isFired = reader.read_boolean("isFired");

        return test::compact::employee_dto{ age, rank, id, isHired, isFired };
    }
};

}
}
}