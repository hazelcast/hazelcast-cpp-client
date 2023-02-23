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

struct nullable_primitive_object
{
    boost::optional<bool> nullableBoolean;
    boost::optional<int8_t> nullableByte;
    boost::optional<int16_t> nullableShort;
    boost::optional<int32_t> nullableInt;
    boost::optional<int64_t> nullableLong;
    boost::optional<float> nullableFloat;
    boost::optional<double> nullableDouble;
    boost::optional<std::vector<boost::optional<bool>>> nullableBooleans;
    boost::optional<std::vector<boost::optional<int8_t>>> nullableBytes;
    boost::optional<std::vector<boost::optional<int16_t>>> nullableShorts;
    boost::optional<std::vector<boost::optional<int32_t>>> nullableInts;
    boost::optional<std::vector<boost::optional<int64_t>>> nullableLongs;
    boost::optional<std::vector<boost::optional<float>>> nullableFloats;
    boost::optional<std::vector<boost::optional<double>>> nullableDoubles;
};

} // namespace compact
} // namespace test

namespace serialization {

template<>
struct hz_serializer<test::compact::nullable_primitive_object>
  : public compact::compact_serializer
{
    static void write(const test::compact::nullable_primitive_object& object,
                      compact::compact_writer& writer)
    {
        writer.write_nullable_boolean("boolean", object.nullableBoolean);
        writer.write_nullable_int8("byte", object.nullableByte);
        writer.write_nullable_int16("short", object.nullableShort);
        writer.write_nullable_int32("int", object.nullableInt);
        writer.write_nullable_int64("long", object.nullableLong);
        writer.write_nullable_float32("float", object.nullableFloat);
        writer.write_nullable_float64("double", object.nullableDouble);
        writer.write_array_of_nullable_boolean("booleans",
                                               object.nullableBooleans);
        writer.write_array_of_nullable_int8("bytes", object.nullableBytes);
        writer.write_array_of_nullable_int16("shorts", object.nullableShorts);
        writer.write_array_of_nullable_int32("ints", object.nullableInts);
        writer.write_array_of_nullable_int64("longs", object.nullableLongs);
        writer.write_array_of_nullable_float32("floats", object.nullableFloats);
        writer.write_array_of_nullable_float64("doubles",
                                               object.nullableDoubles);
    }

    static test::compact::nullable_primitive_object read(
      compact::compact_reader& reader)
    {
        test::compact::nullable_primitive_object object;

        object.nullableBoolean = reader.read_nullable_boolean("boolean");
        object.nullableByte = reader.read_nullable_int8("byte");
        object.nullableShort = reader.read_nullable_int16("short");
        object.nullableInt = reader.read_nullable_int32("int");
        object.nullableLong = reader.read_nullable_int64("long");
        object.nullableFloat = reader.read_nullable_float32("float");
        object.nullableDouble = reader.read_nullable_float64("double");
        object.nullableBooleans =
          reader.read_array_of_nullable_boolean("booleans");
        object.nullableBytes = reader.read_array_of_nullable_int8("bytes");
        object.nullableShorts = reader.read_array_of_nullable_int16("shorts");
        object.nullableInts = reader.read_array_of_nullable_int32("ints");
        object.nullableLongs = reader.read_array_of_nullable_int64("longs");
        object.nullableFloats = reader.read_array_of_nullable_float32("floats");
        object.nullableDoubles =
          reader.read_array_of_nullable_float64("doubles");

        return object;
    }

    // typename is same as the primitive_object on purpose.
    // This is to simulate two different applications implementing the same
    // class with different serializers.
    static std::string type_name() { return "primitive_object"; }
};
} // namespace serialization

} // namespace client
} // namespace hazelcast