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

template<int Idx>
struct stress_type
{
};

} // namespace compact
} // namespace test
} // namespace client
} // namespace hazelcast

namespace hazelcast {
namespace client {
namespace serialization {

template<int Idx>
struct hz_serializer<test::compact::stress_type<Idx>> : compact_serializer
{
    static std::string type_name()
    {
        return "stress_type_" + std::to_string(Idx);
    }

    static void write(const test::compact::stress_type<Idx>&,
                      compact_writer& writer)
    {
        writer.write_int32("field_" + std::to_string(Idx), Idx);
    }

    static test::compact::stress_type<Idx> read(compact_reader& reader)
    {
        return test::compact::stress_type<Idx>{};
    }
};

} // namespace serialization
} // namespace client
} // namespace hazelcast