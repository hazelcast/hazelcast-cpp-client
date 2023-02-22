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

struct primitive_object
{
    bool boolean_;
    int8_t byte_;
    int16_t short_;
    int32_t int_;
    int64_t long_;
    float float_;
    double double_;
    boost::optional<std::vector<bool>> booleans;
    boost::optional<std::vector<int8_t>> bytes;
    boost::optional<std::vector<int16_t>> shorts;
    boost::optional<std::vector<int32_t>> ints;
    boost::optional<std::vector<int64_t>> longs;
    boost::optional<std::vector<float>> floats;
    boost::optional<std::vector<double>> doubles;
};

} // namespace compact
} // namespace test

namespace serialization {
template<>
struct hz_serializer<test::compact::primitive_object>
  : public compact::compact_serializer
{
    static void write(const test::compact::primitive_object& object,
                      compact::compact_writer& writer)
    {
        writer.write_boolean("boolean", object.boolean_);
        writer.write_int8("byte", object.byte_);
        writer.write_int16("short", object.short_);
        writer.write_int32("int", object.int_);
        writer.write_int64("long", object.long_);
        writer.write_float32("float", object.float_);
        writer.write_float64("double", object.double_);
        writer.write_array_of_boolean("booleans", object.booleans);
        writer.write_array_of_int8("bytes", object.bytes);
        writer.write_array_of_int16("shorts", object.shorts);
        writer.write_array_of_int32("ints", object.ints);
        writer.write_array_of_int64("longs", object.longs);
        writer.write_array_of_float32("floats", object.floats);
        writer.write_array_of_float64("doubles", object.doubles);
    }

    static test::compact::primitive_object read(compact::compact_reader& reader)
    {
        test::compact::primitive_object object;

        object.boolean_ = reader.read_boolean("boolean");
        object.byte_ = reader.read_int8("byte");
        object.short_ = reader.read_int16("short");
        object.int_ = reader.read_int32("int");
        object.long_ = reader.read_int64("long");
        object.float_ = reader.read_float32("float");
        object.double_ = reader.read_float64("double");
        object.booleans = reader.read_array_of_boolean("booleans");
        object.bytes = reader.read_array_of_int8("bytes");
        object.shorts = reader.read_array_of_int16("shorts");
        object.ints = reader.read_array_of_int32("ints");
        object.longs = reader.read_array_of_int64("longs");
        object.floats = reader.read_array_of_float32("floats");
        object.doubles = reader.read_array_of_float64("doubles");

        return object;
    }

    static std::string type_name() { return "primitive_object"; }
};

} // namespace serialization
} // namespace client
} // namespace hazelcast