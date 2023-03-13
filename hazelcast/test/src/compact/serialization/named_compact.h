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
#include "inner_dto.h"

namespace hazelcast {
namespace client {
namespace test {
namespace compact {

// This corresponds to
// com.hazelcast.internal.serialization.impl.portable.NamedPortable It is added
// for GenericRecord, since portable is not supported yet its name is renamed to
// `named_compact`
struct named_compact
{
    std::string name;
    int myint;
};

bool
operator==(const named_compact& lhs, const named_compact& rhs)
{
    return lhs.name == rhs.name && lhs.myint == rhs.myint;
}

} // namespace compact
} // namespace test

namespace serialization {

template<>
struct hz_serializer<test::compact::named_compact>
  : public compact::compact_serializer
{
    static void write(const test::compact::named_compact& object,
                      compact::compact_writer& writer)
    {
        writer.write_string("name", object.name);
        writer.write_int32("myint", object.myint);
    }

    static test::compact::named_compact read(compact::compact_reader& reader)
    {
        return test::compact::named_compact{ *reader.read_string("name"),
                                             reader.read_int32("myint") };
    }

    static std::string type_name() { return "named_portable"; }
};

} // namespace serialization
} // namespace client
} // namespace hazelcast