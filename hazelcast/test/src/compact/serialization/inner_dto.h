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

#include "named_dto.h"
#include "../compact_helper.h"

namespace hazelcast {
namespace client {
namespace test {
namespace compact {

struct inner_dto
{
    boost::optional<std::vector<bool>> bools;
    boost::optional<std::vector<int8_t>> bytes;
    boost::optional<std::vector<int16_t>> shorts;
    boost::optional<std::vector<int32_t>> ints;
    boost::optional<std::vector<int64_t>> longs;
    boost::optional<std::vector<float>> floats;
    boost::optional<std::vector<double>> doubles;
    boost::optional<std::vector<boost::optional<std::string>>> strings;
    boost::optional<std::vector<boost::optional<named_dto>>> nn;
    boost::optional<std::vector<boost::optional<big_decimal>>> bigDecimals;
    boost::optional<std::vector<boost::optional<local_time>>> localTimes;
    boost::optional<std::vector<boost::optional<local_date>>> localDates;
    boost::optional<std::vector<boost::optional<local_date_time>>>
      localDateTimes;
    boost::optional<std::vector<boost::optional<offset_date_time>>>
      offsetDateTimes;
    boost::optional<std::vector<boost::optional<bool>>> nullableBools;
    boost::optional<std::vector<boost::optional<int8_t>>> nullableBytes;
    boost::optional<std::vector<boost::optional<int16_t>>> nullableShorts;
    boost::optional<std::vector<boost::optional<int32_t>>> nullableInts;
    boost::optional<std::vector<boost::optional<int64_t>>> nullableLongs;
    boost::optional<std::vector<boost::optional<float>>> nullableFloats;
    boost::optional<std::vector<boost::optional<double>>> nullableDoubles;
};

bool
operator==(const inner_dto& lhs, const inner_dto& rhs)
{
    return lhs.bools == rhs.bools && lhs.bytes == rhs.bytes &&
           lhs.shorts == rhs.shorts && lhs.ints == rhs.ints &&
           lhs.longs == rhs.longs && lhs.floats == rhs.floats &&
           lhs.doubles == rhs.doubles && lhs.strings == rhs.strings &&
           lhs.nn == rhs.nn && lhs.bigDecimals == rhs.bigDecimals &&
           lhs.localTimes == rhs.localTimes &&
           lhs.localDates == rhs.localDates &&
           lhs.localDateTimes == rhs.localDateTimes &&
           lhs.offsetDateTimes == rhs.offsetDateTimes &&
           lhs.nullableBools == rhs.nullableBools &&
           lhs.nullableBytes == rhs.nullableBytes &&
           lhs.nullableShorts == rhs.nullableShorts &&
           lhs.nullableInts == rhs.nullableInts &&
           lhs.nullableLongs == rhs.nullableLongs &&
           lhs.nullableFloats == rhs.nullableFloats &&
           lhs.nullableDoubles == rhs.nullableDoubles;
}

inner_dto
create_inner_dto()
{
    return inner_dto{
        boost::make_optional<std::vector<bool>>({ true, false }),
        boost::make_optional<std::vector<int8_t>>({ 0, 1, 2 }),
        boost::make_optional<std::vector<int16_t>>({ 3, 4, 5 }),
        boost::make_optional<std::vector<int32_t>>({ 9, 8, 7, 6 }),
        boost::make_optional<std::vector<int64_t>>({ 0, 1, 5, 7, 9, 11 }),
        boost::make_optional<std::vector<float>>({ 0.6543f, -3.56f, 45.67f }),
        boost::make_optional<std::vector<double>>(
          { 456.456, 789.789, 321.321 }),
        boost::make_optional<std::vector<boost::optional<std::string>>>(
          { boost::make_optional<std::string>("test"), boost::none }),
        boost::make_optional<std::vector<boost::optional<named_dto>>>(
          { boost::make_optional(
              named_dto{ boost::make_optional<std::string>("test"), 1 }),
            boost::none }),
        boost::make_optional<
          std::vector<boost::optional<hazelcast::client::big_decimal>>>(
          { boost::make_optional(hazelcast::client::big_decimal{
              boost::multiprecision::cpp_int{ "12345" }, 0 }),
            boost::make_optional(hazelcast::client::big_decimal{
              boost::multiprecision::cpp_int{ "123456" }, 0 }) }),
        boost::make_optional<
          std::vector<boost::optional<hazelcast::client::local_time>>>(
          { boost::make_optional(current_time()),
            boost::none,
            boost::make_optional(current_time()) }),
        boost::make_optional<
          std::vector<boost::optional<hazelcast::client::local_date>>>(
          { boost::make_optional(current_date()),
            boost::none,
            boost::make_optional(current_date()) }),
        boost::make_optional<
          std::vector<boost::optional<hazelcast::client::local_date_time>>>(
          { boost::make_optional(current_timestamp()), boost::none }),
        boost::make_optional<
          std::vector<boost::optional<hazelcast::client::offset_date_time>>>(
          { boost::make_optional(current_timestamp_with_timezone()) }),
        boost::make_optional<std::vector<boost::optional<bool>>>(
          { boost::make_optional(true),
            boost::make_optional(false),
            boost::none }),
        boost::make_optional<std::vector<boost::optional<int8_t>>>(
          { boost::make_optional<int8_t>(0),
            boost::make_optional<int8_t>(1),
            boost::make_optional<int8_t>(2),
            boost::none }),
        boost::make_optional<std::vector<boost::optional<int16_t>>>(
          { boost::make_optional<int16_t>(3),
            boost::make_optional<int16_t>(4),
            boost::make_optional<int16_t>(5),
            boost::none }),
        boost::make_optional<std::vector<boost::optional<int32_t>>>(
          { boost::make_optional<int32_t>(9),
            boost::make_optional<int32_t>(8),
            boost::make_optional<int32_t>(7),
            boost::make_optional<int32_t>(6),
            boost::none }),
        boost::make_optional<std::vector<boost::optional<int64_t>>>(
          { boost::make_optional<int64_t>(0),
            boost::make_optional<int64_t>(1),
            boost::make_optional<int64_t>(5),
            boost::make_optional<int64_t>(7),
            boost::none }),
        boost::make_optional<std::vector<boost::optional<float>>>(
          { boost::make_optional(0.6543f),
            boost::make_optional(-3.56f),
            boost::make_optional(45.67f),
            boost::none }),
        boost::make_optional<std::vector<boost::optional<double>>>(
          { boost::make_optional(456.456),
            boost::make_optional(789.789),
            boost::make_optional(321.321),
            boost::none }),
    };
}

} // namespace compact
} // namespace test

namespace serialization {

template<>
struct hz_serializer<test::compact::inner_dto>
  : public compact::compact_serializer
{
    static void write(const test::compact::inner_dto& object,
                      compact::compact_writer& writer)
    {
        writer.write_array_of_boolean("bools", object.bools);
        writer.write_array_of_int8("bytes", object.bytes);
        writer.write_array_of_int16("shorts", object.shorts);
        writer.write_array_of_int32("ints", object.ints);
        writer.write_array_of_int64("longs", object.longs);
        writer.write_array_of_float32("floats", object.floats);
        writer.write_array_of_float64("doubles", object.doubles);
        writer.write_array_of_string("strings", object.strings);
        writer.write_array_of_decimal("bigDecimals", object.bigDecimals);
        writer.write_array_of_time("localTimes", object.localTimes);
        writer.write_array_of_date("localDates", object.localDates);
        writer.write_array_of_timestamp("localDateTimes",
                                        object.localDateTimes);
        writer.write_array_of_timestamp_with_timezone("offsetDateTimes",
                                                      object.offsetDateTimes);
        writer.write_array_of_compact("nn", object.nn);
        writer.write_array_of_nullable_boolean("nullableBools",
                                               object.nullableBools);
        writer.write_array_of_nullable_int8("nullableBytes",
                                            object.nullableBytes);
        writer.write_array_of_nullable_int16("nullableShorts",
                                             object.nullableShorts);
        writer.write_array_of_nullable_int32("nullableInts",
                                             object.nullableInts);
        writer.write_array_of_nullable_int64("nullableLongs",
                                             object.nullableLongs);
        writer.write_array_of_nullable_float32("nullableFloats",
                                               object.nullableFloats);
        writer.write_array_of_nullable_float64("nullableDoubles",
                                               object.nullableDoubles);
    }

    static test::compact::inner_dto read(compact::compact_reader& reader)
    {
        test::compact::inner_dto object;
        object.bools = reader.read_array_of_boolean("bools");
        object.bytes = reader.read_array_of_int8("bytes");
        object.shorts = reader.read_array_of_int16("shorts");
        object.ints = reader.read_array_of_int32("ints");
        object.longs = reader.read_array_of_int64("longs");
        object.floats = reader.read_array_of_float32("floats");
        object.doubles = reader.read_array_of_float64("doubles");
        object.strings = reader.read_array_of_string("strings");
        object.bigDecimals = reader.read_array_of_decimal("bigDecimals");
        object.localTimes = reader.read_array_of_time("localTimes");
        object.localDates = reader.read_array_of_date("localDates");
        object.localDateTimes =
          reader.read_array_of_timestamp("localDateTimes");
        object.offsetDateTimes =
          reader.read_array_of_timestamp_with_timezone("offsetDateTimes");
        object.nn =
          reader.read_array_of_compact<test::compact::named_dto>("nn");
        object.nullableBools =
          reader.read_array_of_nullable_boolean("nullableBools");
        object.nullableBytes =
          reader.read_array_of_nullable_int8("nullableBytes");
        object.nullableShorts =
          reader.read_array_of_nullable_int16("nullableShorts");
        object.nullableInts =
          reader.read_array_of_nullable_int32("nullableInts");
        object.nullableLongs =
          reader.read_array_of_nullable_int64("nullableLongs");
        object.nullableFloats =
          reader.read_array_of_nullable_float32("nullableFloats");
        object.nullableDoubles =
          reader.read_array_of_nullable_float64("nullableDoubles");
        return object;
    }

    static std::string type_name() { return "inner"; }
};

} // namespace serialization
} // namespace client
} // namespace hazelcast
