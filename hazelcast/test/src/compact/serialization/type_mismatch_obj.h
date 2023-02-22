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

struct type_mistmatch_obj
{
    int value;
};

} // namespace compact
} // namespace test

namespace serialization {

template<>
struct hz_serializer<test::compact::type_mistmatch_obj>
  : compact::compact_serializer
{
    static void write(const test::compact::type_mistmatch_obj& obj,
                      compact::compact_writer& writer)
    {
        writer.write_int32("field_1", obj.value);
    }

    static test::compact::type_mistmatch_obj read(
      compact::compact_reader& reader)
    {
        test::compact::type_mistmatch_obj obj;

        obj.value = static_cast<int>(reader.read_float32("field_1"));

        return obj;
    }

    static std::string type_name() { return "type_mistmatch_obj"; }
};

} // namespace serialization
} // namespace client
} // namespace hazelcast