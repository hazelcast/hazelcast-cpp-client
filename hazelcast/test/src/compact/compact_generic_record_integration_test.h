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

#include "compact_test_base.h"
#include "serialization/named_compact.h"
#include "serialization/generic_record_factory.h"

#include "../TestHelperFunctions.h"

namespace hazelcast {
namespace client {
namespace test {
namespace compact {

class CompactGenericRecordIntegrationTest : public compact_test_base
{
protected:
    using generic_record = serialization::generic_record::generic_record;

    void get_put_and_check_with_rc(const std::string& map_name,
                                   const named_compact& compact)
    {
        Response response;

        remote_controller_client().executeOnController(
          response,
          factory_.get_cluster_id(),
          (boost::format(
             R"(
                        var clusterMap = instance_0.getMap("%1%");
                        var record = clusterMap.get(1);

                        clusterMap.put(2, record);

                        var actualRecord = clusterMap.get(2);

                        if (actualRecord.getFieldKind("name") === com.hazelcast.nio.serialization.FieldKind.STRING &&
                            actualRecord.getFieldKind("myint") === com.hazelcast.nio.serialization.FieldKind.INT32 &&
                            actualRecord.getString("name") === "%2%" &&
                            actualRecord.getInt32("myint") === %3%) {
                            result = "" + true;
                        } else {
                            result = "" + false;
                        }
                    )") %
           map_name % compact.name % compact.myint)
            .str(),
          Lang::JAVASCRIPT);

        ASSERT_EQ(response.result, "true");
    }

    void validate_record(const std::string& map_name)
    {
        Response response;

        remote_controller_client().executeOnController(
          response,
          factory_.get_cluster_id(),
          (boost::format(
             R"(
                var map = instance_0.getMap("%1%");
                var record = map.get(1);
                var precision = 0.0001;

                if (record.getFieldKind("boolean") === com.hazelcast.nio.serialization.FieldKind.BOOLEAN &&
                    record.getBoolean("boolean") === true &&
                    record.getFieldKind("int8") === com.hazelcast.nio.serialization.FieldKind.INT8 &&
                    record.getInt8("int8") === 8 &&
                    record.getFieldKind("int16") === com.hazelcast.nio.serialization.FieldKind.INT16 &&
                    record.getInt16("int16") === 16 &&
                    record.getFieldKind("int32") === com.hazelcast.nio.serialization.FieldKind.INT32 &&
                    record.getInt32("int32") === 32 &&
                    record.getFieldKind("int64") === com.hazelcast.nio.serialization.FieldKind.INT64 &&
                    record.getInt64("int64") == 64 &&
                    record.getFieldKind("float32") === com.hazelcast.nio.serialization.FieldKind.FLOAT32 &&
                    Math.abs(record.getFloat32("float32") - 32.32) <= precision &&
                    record.getFieldKind("float64") === com.hazelcast.nio.serialization.FieldKind.FLOAT64 &&
                    Math.abs(record.getFloat64("float64") - 64.64) <= precision &&
                    record.getFieldKind("nullable_boolean") === com.hazelcast.nio.serialization.FieldKind.NULLABLE_BOOLEAN &&
                    record.getNullableBoolean("nullable_boolean") === true &&
                    record.getFieldKind("nullable_boolean_null") === com.hazelcast.nio.serialization.FieldKind.NULLABLE_BOOLEAN &&
                    record.getNullableBoolean("nullable_boolean_null") === null &&
                    record.getFieldKind("nullable_int8") === com.hazelcast.nio.serialization.FieldKind.NULLABLE_INT8 &&
                    record.getNullableInt8("nullable_int8") === 88 &&
                    record.getFieldKind("nullable_int8_null") === com.hazelcast.nio.serialization.FieldKind.NULLABLE_INT8 &&
                    record.getNullableInt8("nullable_int8_null") === null &&
                    record.getFieldKind("nullable_int16") === com.hazelcast.nio.serialization.FieldKind.NULLABLE_INT16 &&
                    record.getNullableInt16("nullable_int16") === 1616 &&
                    record.getFieldKind("nullable_int16_null") === com.hazelcast.nio.serialization.FieldKind.NULLABLE_INT16 &&
                    record.getNullableInt16("nullable_int16_null") === null &&
                    record.getFieldKind("nullable_int32") === com.hazelcast.nio.serialization.FieldKind.NULLABLE_INT32 &&
                    record.getNullableInt32("nullable_int32") === 3232 &&
                    record.getFieldKind("nullable_int32_null") === com.hazelcast.nio.serialization.FieldKind.NULLABLE_INT32 &&
                    record.getNullableInt32("nullable_int32_null") === null &&
                    record.getFieldKind("nullable_int64") === com.hazelcast.nio.serialization.FieldKind.NULLABLE_INT64 &&
                    record.getNullableInt64("nullable_int64") == 6464 &&
                    record.getFieldKind("nullable_int64_null") === com.hazelcast.nio.serialization.FieldKind.NULLABLE_INT64 &&
                    record.getNullableInt64("nullable_int64_null") === null &&
                    record.getFieldKind("nullable_float32") === com.hazelcast.nio.serialization.FieldKind.NULLABLE_FLOAT32 &&
                    Math.abs(record.getNullableFloat32("nullable_float32") - 3232.3232) <= precision &&
                    record.getFieldKind("nullable_float32_null") === com.hazelcast.nio.serialization.FieldKind.NULLABLE_FLOAT32 &&
                    record.getNullableFloat32("nullable_float32_null") === null &&
                    record.getFieldKind("nullable_float64") === com.hazelcast.nio.serialization.FieldKind.NULLABLE_FLOAT64 &&
                    Math.abs(record.getNullableFloat64("nullable_float64") - 6464.6464) <= precision &&
                    record.getFieldKind("nullable_float64_null") === com.hazelcast.nio.serialization.FieldKind.NULLABLE_FLOAT64 &&
                    record.getNullableFloat64("nullable_float64_null") === null &&
                    record.getFieldKind("string") === com.hazelcast.nio.serialization.FieldKind.STRING &&
                    record.getString("string") == "a_text" &&
                    record.getFieldKind("string_null") === com.hazelcast.nio.serialization.FieldKind.STRING &&
                    record.getString("string_null") === null &&
                    record.getFieldKind("generic_record") === com.hazelcast.nio.serialization.FieldKind.COMPACT &&
                    record.getGenericRecord("generic_record").getBoolean("nested_boolean") == true &&
                    record.getFieldKind("generic_record_null") === com.hazelcast.nio.serialization.FieldKind.COMPACT &&
                    record.getGenericRecord("generic_record_null") === null &&
                    record.getFieldKind("decimal") === com.hazelcast.nio.serialization.FieldKind.DECIMAL &&
                    record.getDecimal("decimal").scale() == 10 &&
                    record.getDecimal("decimal").unscaledValue() == 14 &&
                    record.getFieldKind("decimal_null") === com.hazelcast.nio.serialization.FieldKind.DECIMAL &&
                    record.getDecimal("decimal_null") === null &&
                    record.getFieldKind("time") === com.hazelcast.nio.serialization.FieldKind.TIME &&
                    record.getTime("time").getHour() == 19 &&
                    record.getTime("time").getMinute() == 52 &&
                    record.getTime("time").getSecond() == 10 &&
                    record.getTime("time").getNano() == 123456789 &&
                    record.getFieldKind("time_null") === com.hazelcast.nio.serialization.FieldKind.TIME &&
                    record.getTime("time_null") === null &&
                    record.getFieldKind("date") === com.hazelcast.nio.serialization.FieldKind.DATE &&
                    record.getDate("date").getYear() == 2023 &&
                    record.getDate("date").getMonthValue() == 2 &&)"
            // Splitted into two chunks
            // Because MSVC complains about it !!!
            R"(
                    record.getDate("date").getDayOfMonth() == 6 &&
                    record.getFieldKind("date_null") === com.hazelcast.nio.serialization.FieldKind.DATE &&
                    record.getDate("date_null") === null &&
                    record.getFieldKind("timestamp") === com.hazelcast.nio.serialization.FieldKind.TIMESTAMP &&
                    record.getTimestamp("timestamp").getYear() == 2023 &&
                    record.getTimestamp("timestamp").getMonthValue() == 2 &&
                    record.getTimestamp("timestamp").getDayOfMonth() == 6 &&
                    record.getTimestamp("timestamp").getHour() == 9 &&
                    record.getTimestamp("timestamp").getMinute() == 2 &&
                    record.getTimestamp("timestamp").getSecond() == 8 &&
                    record.getTimestamp("timestamp").getNano() == 123456789 &&
                    record.getFieldKind("timestamp_null") === com.hazelcast.nio.serialization.FieldKind.TIMESTAMP &&
                    record.getTimestamp("timestamp_null") === null &&
                    record.getFieldKind("timestamp_with_timezone") === com.hazelcast.nio.serialization.FieldKind.TIMESTAMP_WITH_TIMEZONE &&
                    record.getTimestampWithTimezone("timestamp_with_timezone").getYear() == 2023 &&
                    record.getTimestampWithTimezone("timestamp_with_timezone").getMonthValue() == 2 &&
                    record.getTimestampWithTimezone("timestamp_with_timezone").getDayOfMonth() == 6 &&
                    record.getTimestampWithTimezone("timestamp_with_timezone").getHour() == 19 &&
                    record.getTimestampWithTimezone("timestamp_with_timezone").getMinute() == 52 &&
                    record.getTimestampWithTimezone("timestamp_with_timezone").getSecond() == 10 &&
                    record.getTimestampWithTimezone("timestamp_with_timezone").getNano() == 123456789 &&
                    record.getTimestampWithTimezone("timestamp_with_timezone").getOffset().getTotalSeconds() == 5000 &&
                    record.getFieldKind("timestamp_with_timezone_null") === com.hazelcast.nio.serialization.FieldKind.TIMESTAMP_WITH_TIMEZONE &&
                    record.getTimestampWithTimezone("timestamp_with_timezone_null") === null &&
                    record.getFieldKind("array_of_boolean") === com.hazelcast.nio.serialization.FieldKind.ARRAY_OF_BOOLEAN &&
                    record.getArrayOfBoolean("array_of_boolean")[0] == true &&
                    record.getArrayOfBoolean("array_of_boolean")[1] == false &&
                    record.getFieldKind("array_of_boolean_null") === com.hazelcast.nio.serialization.FieldKind.ARRAY_OF_BOOLEAN &&
                    record.getArrayOfBoolean("array_of_boolean_null") == null &&
                    record.getFieldKind("array_of_int8") === com.hazelcast.nio.serialization.FieldKind.ARRAY_OF_INT8 &&
                    record.getArrayOfInt8("array_of_int8")[0] == 8 &&
                    record.getArrayOfInt8("array_of_int8")[1] == 9 &&
                    record.getFieldKind("array_of_int8_null") === com.hazelcast.nio.serialization.FieldKind.ARRAY_OF_INT8 &&
                    record.getArrayOfInt8("array_of_int8_null") == null &&
                    record.getFieldKind("array_of_int16") === com.hazelcast.nio.serialization.FieldKind.ARRAY_OF_INT16 &&
                    record.getArrayOfInt16("array_of_int16")[0] == 16 &&
                    record.getArrayOfInt16("array_of_int16")[1] == 17 &&
                    record.getFieldKind("array_of_int16_null") === com.hazelcast.nio.serialization.FieldKind.ARRAY_OF_INT16 &&
                    record.getArrayOfInt16("array_of_int16_null") == null &&
                    record.getFieldKind("array_of_int32") === com.hazelcast.nio.serialization.FieldKind.ARRAY_OF_INT32 &&
                    record.getArrayOfInt32("array_of_int32")[0] == 32 &&
                    record.getArrayOfInt32("array_of_int32")[1] == 33 &&
                    record.getFieldKind("array_of_int32_null") === com.hazelcast.nio.serialization.FieldKind.ARRAY_OF_INT32 &&
                    record.getArrayOfInt32("array_of_int32_null") == null &&
                    record.getFieldKind("array_of_int64") === com.hazelcast.nio.serialization.FieldKind.ARRAY_OF_INT64 &&
                    record.getArrayOfInt64("array_of_int64")[0] == 64 &&
                    record.getArrayOfInt64("array_of_int64")[1] == 65 &&
                    record.getFieldKind("array_of_int64_null") === com.hazelcast.nio.serialization.FieldKind.ARRAY_OF_INT64 &&
                    record.getArrayOfInt64("array_of_int64_null") == null &&
                    record.getFieldKind("array_of_float32") === com.hazelcast.nio.serialization.FieldKind.ARRAY_OF_FLOAT32 &&
                    Math.abs(record.getArrayOfFloat32("array_of_float32")[0] - 32.32 <= precision) &&
                    Math.abs(record.getArrayOfFloat32("array_of_float32")[1] - 33.33 <= precision) &&
                    record.getFieldKind("array_of_float32_null") === com.hazelcast.nio.serialization.FieldKind.ARRAY_OF_FLOAT32 &&
                    record.getArrayOfFloat32("array_of_float32_null") == null &&
                    record.getFieldKind("array_of_float64") === com.hazelcast.nio.serialization.FieldKind.ARRAY_OF_FLOAT64 &&
                    Math.abs(record.getArrayOfFloat64("array_of_float64")[0] - 64.64 <= precision) &&
                    Math.abs(record.getArrayOfFloat64("array_of_float64")[1] - 65.65 <= precision) &&
                    record.getFieldKind("array_of_float64_null") === com.hazelcast.nio.serialization.FieldKind.ARRAY_OF_FLOAT64 &&
                    record.getArrayOfFloat64("array_of_float64_null") == null &&
                    record.getFieldKind("array_of_nullable_boolean") === com.hazelcast.nio.serialization.FieldKind.ARRAY_OF_NULLABLE_BOOLEAN &&
                    record.getArrayOfNullableBoolean("array_of_nullable_boolean")[0] == true &&
                    record.getArrayOfNullableBoolean("array_of_nullable_boolean")[1] == false &&
                    record.getArrayOfNullableBoolean("array_of_nullable_boolean")[2] == null &&
                    record.getFieldKind("array_of_nullable_boolean_null") === com.hazelcast.nio.serialization.FieldKind.ARRAY_OF_NULLABLE_BOOLEAN &&
                    record.getArrayOfNullableBoolean("array_of_nullable_boolean_null") == null &&
                    record.getFieldKind("array_of_nullable_int8") === com.hazelcast.nio.serialization.FieldKind.ARRAY_OF_NULLABLE_INT8 &&
                    record.getArrayOfNullableInt8("array_of_nullable_int8")[0] == 88 &&
                    record.getArrayOfNullableInt8("array_of_nullable_int8")[1] == 89 &&
                    record.getArrayOfNullableInt8("array_of_nullable_int8")[2] == null &&
                    record.getFieldKind("array_of_nullable_int8_null") === com.hazelcast.nio.serialization.FieldKind.ARRAY_OF_NULLABLE_INT8 &&
                    record.getArrayOfNullableInt8("array_of_nullable_int8_null") == null &&
                    record.getFieldKind("array_of_nullable_int16") === com.hazelcast.nio.serialization.FieldKind.ARRAY_OF_NULLABLE_INT16 &&
                    record.getArrayOfNullableInt16("array_of_nullable_int16")[0] == 1616 &&
                    record.getArrayOfNullableInt16("array_of_nullable_int16")[1] == 1717 &&
                    record.getArrayOfNullableInt16("array_of_nullable_int16")[2] == null &&
                    record.getFieldKind("array_of_nullable_int16_null") === com.hazelcast.nio.serialization.FieldKind.ARRAY_OF_NULLABLE_INT16 &&
                    record.getArrayOfNullableInt16("array_of_nullable_int16_null") == null &&
                    record.getFieldKind("array_of_nullable_int32") === com.hazelcast.nio.serialization.FieldKind.ARRAY_OF_NULLABLE_INT32 &&
                    record.getArrayOfNullableInt32("array_of_nullable_int32")[0] == 3232 &&
                    record.getArrayOfNullableInt32("array_of_nullable_int32")[1] == 3333 &&
                    record.getArrayOfNullableInt32("array_of_nullable_int32")[2] == null &&
                    record.getFieldKind("array_of_nullable_int32_null") === com.hazelcast.nio.serialization.FieldKind.ARRAY_OF_NULLABLE_INT32 &&
                    record.getArrayOfNullableInt32("array_of_nullable_int32_null") == null &&
                    record.getFieldKind("array_of_nullable_int64") === com.hazelcast.nio.serialization.FieldKind.ARRAY_OF_NULLABLE_INT64 &&
                    record.getArrayOfNullableInt64("array_of_nullable_int64")[0] == 6464 &&
                    record.getArrayOfNullableInt64("array_of_nullable_int64")[1] == 6565 &&
                    record.getArrayOfNullableInt64("array_of_nullable_int64")[2] == null &&
                    record.getFieldKind("array_of_nullable_int64_null") === com.hazelcast.nio.serialization.FieldKind.ARRAY_OF_NULLABLE_INT64 &&
                    record.getArrayOfNullableInt64("array_of_nullable_int64_null") == null &&
                    record.getFieldKind("array_of_nullable_float32") === com.hazelcast.nio.serialization.FieldKind.ARRAY_OF_NULLABLE_FLOAT32 &&
                    Math.abs(record.getArrayOfNullableFloat32("array_of_nullable_float32")[0] - 3232.3232 <= precision) &&
                    Math.abs(record.getArrayOfNullableFloat32("array_of_nullable_float32")[1] - 3333.3333 <= precision) &&
                    record.getArrayOfNullableFloat32("array_of_nullable_float32")[2] == null &&
                    record.getFieldKind("array_of_nullable_float64_null") === com.hazelcast.nio.serialization.FieldKind.ARRAY_OF_NULLABLE_FLOAT64 &&
                    record.getArrayOfNullableFloat64("array_of_nullable_float64_null") == null &&
                    Math.abs(record.getArrayOfNullableFloat64("array_of_nullable_float64")[0] - 6464.6464 <= precision) &&
                    Math.abs(record.getArrayOfNullableFloat64("array_of_nullable_float64")[1] - 6565.6565 <= precision) &&
                    record.getArrayOfNullableFloat64("array_of_nullable_float64")[2] == null &&
                    record.getFieldKind("array_of_nullable_float64_null") === com.hazelcast.nio.serialization.FieldKind.ARRAY_OF_NULLABLE_FLOAT64 &&
                    record.getArrayOfNullableFloat64("array_of_nullable_float64_null") == null &&
                    record.getFieldKind("array_of_string") === com.hazelcast.nio.serialization.FieldKind.ARRAY_OF_STRING &&
                    record.getArrayOfString("array_of_string")[0] == "str1" &&
                    record.getArrayOfString("array_of_string")[1] == "str2" &&
                    record.getArrayOfString("array_of_string")[2] == null &&
                    record.getFieldKind("array_of_string_null") === com.hazelcast.nio.serialization.FieldKind.ARRAY_OF_STRING &&
                    record.getArrayOfString("array_of_string_null") == null &&
                    record.getFieldKind("array_of_decimal") === com.hazelcast.nio.serialization.FieldKind.ARRAY_OF_DECIMAL &&
                    record.getArrayOfDecimal("array_of_decimal")[0].scale() == 10 &&
                    record.getArrayOfDecimal("array_of_decimal")[0].unscaledValue() == 14 &&
                    record.getArrayOfDecimal("array_of_decimal")[1] === null &&
                    record.getFieldKind("array_of_decimal_null") === com.hazelcast.nio.serialization.FieldKind.ARRAY_OF_DECIMAL &&
                    record.getArrayOfDecimal("array_of_decimal_null") == null &&
                    record.getFieldKind("array_of_time") === com.hazelcast.nio.serialization.FieldKind.ARRAY_OF_TIME &&
                    record.getArrayOfTime("array_of_time")[0].getHour() == 19 &&
                    record.getArrayOfTime("array_of_time")[0].getMinute() == 52 &&
                    record.getArrayOfTime("array_of_time")[0].getSecond() == 10 &&
                    record.getArrayOfTime("array_of_time")[0].getNano() == 123456789 &&
                    record.getArrayOfTime("array_of_time")[1] == null &&
                    record.getFieldKind("array_of_time_null") === com.hazelcast.nio.serialization.FieldKind.ARRAY_OF_TIME &&
                    record.getArrayOfTime("array_of_time_null") === null &&
                    record.getFieldKind("array_of_timestamp_with_timezone") === com.hazelcast.nio.serialization.FieldKind.ARRAY_OF_TIMESTAMP_WITH_TIMEZONE &&
                    record.getArrayOfTimestampWithTimezone("array_of_timestamp_with_timezone")[0].getYear() == 2023 &&
                    record.getArrayOfTimestampWithTimezone("array_of_timestamp_with_timezone")[0].getMonthValue() == 2 &&
                    record.getArrayOfTimestampWithTimezone("array_of_timestamp_with_timezone")[0].getDayOfMonth() == 6 &&
                    record.getArrayOfTimestampWithTimezone("array_of_timestamp_with_timezone")[0].getHour() == 19 &&
                    record.getArrayOfTimestampWithTimezone("array_of_timestamp_with_timezone")[0].getMinute() == 52 &&
                    record.getArrayOfTimestampWithTimezone("array_of_timestamp_with_timezone")[0].getSecond() == 10 &&
                    record.getArrayOfTimestampWithTimezone("array_of_timestamp_with_timezone")[0].getNano() == 123456789 &&
                    record.getArrayOfTimestampWithTimezone("array_of_timestamp_with_timezone")[0].getOffset().getTotalSeconds() == 5000 &&
                    record.getArrayOfTimestampWithTimezone("array_of_timestamp_with_timezone")[1] == null &&
                    record.getFieldKind("array_of_timestamp_with_timezone_null") === com.hazelcast.nio.serialization.FieldKind.ARRAY_OF_TIMESTAMP_WITH_TIMEZONE &&
                    record.getArrayOfTimestampWithTimezone("array_of_timestamp_with_timezone_null") == null &&
                    record.getFieldKind("array_of_generic_record") === com.hazelcast.nio.serialization.FieldKind.ARRAY_OF_COMPACT &&
                    record.getArrayOfGenericRecord("array_of_generic_record")[0].getBoolean("nested_boolean") == true &&
                    record.getArrayOfGenericRecord("array_of_generic_record")[1] == null &&
                    record.getFieldKind("array_of_generic_record_null") === com.hazelcast.nio.serialization.FieldKind.ARRAY_OF_COMPACT &&
                    record.getArrayOfGenericRecord("array_of_generic_record_null") == null
                ) {
                    result = "" + true;
                } else {
                    result = "" + false;
                }
            )") %
           map_name)
            .str(),
          Lang::JAVASCRIPT);

        ASSERT_EQ(response.result, "true");
    }
};

TEST_F(CompactGenericRecordIntegrationTest, test_put_generic_record_back)
{
    // Counterpart is 'test' but using random string is
    // better to prevent name clashes.
    auto map_name = random_string();
    auto map = client.get_map(map_name).get();

    named_compact expected{ "foo", 900 };

    map->put(1, expected).get();

    get_put_and_check_with_rc(map_name, expected);

    boost::optional<named_compact> actualCompact =
      map->get<int, named_compact>(2).get();

    ASSERT_TRUE(actualCompact.has_value());
    EXPECT_EQ(expected, *actualCompact);
}

TEST_F(CompactGenericRecordIntegrationTest, test_put_get)
{
    auto map = client.get_map(random_string()).get();

    map->put(1, create_generic_record()).get();
    boost::optional<generic_record> record =
      map->get<int, generic_record>(1).get();

    ASSERT_TRUE(record.has_value());
    EXPECT_EQ(record->get_int32("id"), 1);
    EXPECT_EQ(record->get_boolean("boolean"), true);
    EXPECT_EQ(record->get_int8("int8"), 8);
    EXPECT_EQ(record->get_int16("int16"), 16);
    EXPECT_EQ(record->get_int32("int32"), 32);
    EXPECT_EQ(record->get_int64("int64"), 64);
    EXPECT_EQ(record->get_float32("float32"), 32.32f);
    EXPECT_EQ(record->get_float64("float64"), 64.64);
    EXPECT_EQ(record->get_nullable_boolean("nullable_boolean"), true);
    EXPECT_EQ(record->get_nullable_boolean("nullable_boolean_null"),
              boost::none);
    EXPECT_EQ(record->get_nullable_int8("nullable_int8"),
              boost::optional<int8_t>(88));
    EXPECT_EQ(record->get_nullable_int8("nullable_int8_null"), boost::none);
    EXPECT_EQ(record->get_nullable_int16("nullable_int16"),
              boost::optional<int16_t>(1616));
    EXPECT_EQ(record->get_nullable_int16("nullable_int16_null"), boost::none);
    EXPECT_EQ(record->get_nullable_int32("nullable_int32"),
              boost::optional<int32_t>(3232));
    EXPECT_EQ(record->get_nullable_int32("nullable_int32_null"), boost::none);
    EXPECT_EQ(record->get_nullable_int64("nullable_int64"),
              boost::optional<int64_t>(6464));
    EXPECT_EQ(record->get_nullable_int64("nullable_int64_null"), boost::none);
    EXPECT_EQ(record->get_nullable_float32("nullable_float32"),
              boost::optional<float>(3232.3232f));
    EXPECT_EQ(record->get_nullable_float32("nullable_float32_null"),
              boost::none);
    EXPECT_EQ(record->get_nullable_float64("nullable_float64"),
              boost::optional<double>(6464.6464));
    EXPECT_EQ(record->get_nullable_float64("nullable_float64_null"),
              boost::none);
    EXPECT_EQ(record->get_string("string"),
              boost::optional<std::string>("a_text"));
    EXPECT_EQ(record->get_string("string_null"), boost::none);
    EXPECT_EQ(record->get_generic_record("generic_record")
                ->get_boolean("nested_boolean"),
              true);
    EXPECT_EQ(record->get_generic_record("generic_record_null"), boost::none);

    {
        big_decimal expected{ 14, 10 };
        EXPECT_TRUE(record->get_decimal("decimal") == expected);
        EXPECT_TRUE(record->get_decimal("decimal_null") == boost::none);
    }

    {
        local_time expected{ 19, 52, 10, 123456789 };
        EXPECT_TRUE(record->get_time("time") == expected);
        EXPECT_TRUE(record->get_time("time_null") == boost::none);
    }

    {
        local_date expected{ 2023, 2, 6 };
        EXPECT_TRUE(record->get_date("date") == expected);
        EXPECT_TRUE(record->get_date("date_null") == boost::none);
    }

    {
        local_date_time expected{ { 2023, 2, 6 }, { 9, 2, 8, 123456789 } };
        EXPECT_TRUE(record->get_timestamp("timestamp") == expected);
        EXPECT_TRUE(record->get_timestamp("timestamp_null") == boost::none);
    }

    {
        offset_date_time expected{
            { { 2023, 2, 6 }, { 19, 52, 10, 123456789 } }, 5000
        };
        EXPECT_TRUE(record->get_timestamp_with_timezone(
                      "timestamp_with_timezone") == expected);
        EXPECT_TRUE(record->get_timestamp_with_timezone(
                      "timestamp_with_timezone_null") == boost::none);
    }

    {
        std::vector<bool> expected{ true, false };
        EXPECT_TRUE(record->get_array_of_boolean("array_of_boolean") ==
                    expected);
        EXPECT_TRUE(record->get_array_of_boolean("array_of_boolean_null") ==
                    boost::none);
    }

    {
        std::vector<int8_t> expected{ 8, 9 };
        EXPECT_TRUE(record->get_array_of_int8("array_of_int8") == expected);
        EXPECT_TRUE(record->get_array_of_int8("array_of_int8_null") ==
                    boost::none);
    }

    {
        std::vector<int16_t> expected{ 16, 17 };
        EXPECT_TRUE(record->get_array_of_int16("array_of_int16") == expected);
        EXPECT_TRUE(record->get_array_of_int16("array_of_int16_null") ==
                    boost::none);
    }

    {
        std::vector<int32_t> expected{ 32, 33 };
        EXPECT_TRUE(record->get_array_of_int32("array_of_int32") == expected);
        EXPECT_TRUE(record->get_array_of_int32("array_of_int32_null") ==
                    boost::none);
    }

    {
        std::vector<int64_t> expected{ 64, 65 };
        EXPECT_TRUE(record->get_array_of_int64("array_of_int64") == expected);
        EXPECT_TRUE(record->get_array_of_int64("array_of_int64_null") ==
                    boost::none);
    }

    {
        std::vector<float> expected{ 32.32f, 33.33f };
        EXPECT_TRUE(record->get_array_of_float32("array_of_float32") ==
                    expected);
        EXPECT_TRUE(record->get_array_of_float32("array_of_float32_null") ==
                    boost::none);
    }

    {
        std::vector<double> expected{ 64.64, 65.65 };
        EXPECT_TRUE(record->get_array_of_float64("array_of_float64") ==
                    expected);
        EXPECT_TRUE(record->get_array_of_float64("array_of_float64_null") ==
                    boost::none);
    }

    {
        std::vector<boost::optional<bool>> expected{
            boost::optional<bool>{ true },
            boost::optional<bool>{ false },
            boost::none
        };
        EXPECT_TRUE(record->get_array_of_nullable_boolean(
                      "array_of_nullable_boolean") == expected);
        EXPECT_TRUE(record->get_array_of_nullable_boolean(
                      "array_of_nullable_boolean_null") == boost::none);
    }

    {
        std::vector<boost::optional<int8_t>> expected{
            boost::optional<int8_t>{ 88 },
            boost::optional<int8_t>{ 89 },
            boost::none
        };
        EXPECT_TRUE(record->get_array_of_nullable_int8(
                      "array_of_nullable_int8") == expected);
        EXPECT_TRUE(record->get_array_of_nullable_int8(
                      "array_of_nullable_int8_null") == boost::none);
    }

    {
        std::vector<boost::optional<int16_t>> expected{
            boost::optional<int16_t>{ 1616 },
            boost::optional<int16_t>{ 1717 },
            boost::none
        };
        EXPECT_TRUE(record->get_array_of_nullable_int16(
                      "array_of_nullable_int16") == expected);
        EXPECT_TRUE(record->get_array_of_nullable_int16(
                      "array_of_nullable_int16_null") == boost::none);
    }

    {
        std::vector<boost::optional<int32_t>> expected{
            boost::optional<int32_t>{ 3232 },
            boost::optional<int32_t>{ 3333 },
            boost::none
        };
        EXPECT_TRUE(record->get_array_of_nullable_int32(
                      "array_of_nullable_int32") == expected);
        EXPECT_TRUE(record->get_array_of_nullable_int32(
                      "array_of_nullable_int32_null") == boost::none);
    }

    {
        std::vector<boost::optional<int64_t>> expected{
            boost::optional<int64_t>{ 6464 },
            boost::optional<int64_t>{ 6565 },
            boost::none
        };
        EXPECT_TRUE(record->get_array_of_nullable_int64(
                      "array_of_nullable_int64") == expected);
        EXPECT_TRUE(record->get_array_of_nullable_int64(
                      "array_of_nullable_int64_null") == boost::none);
    }

    {
        std::vector<boost::optional<float>> expected{
            boost::optional<float>{ 3232.3232f },
            boost::optional<float>{ 3333.3333f },
            boost::none
        };
        EXPECT_TRUE(record->get_array_of_nullable_float32(
                      "array_of_nullable_float32") == expected);
        EXPECT_TRUE(record->get_array_of_nullable_float32(
                      "array_of_nullable_float32_null") == boost::none);
    }

    {
        std::vector<boost::optional<double>> expected{
            boost::optional<double>{ 6464.6464 },
            boost::optional<double>{ 6565.6565 },
            boost::none
        };
        EXPECT_TRUE(record->get_array_of_nullable_float64(
                      "array_of_nullable_float64") == expected);
        EXPECT_TRUE(record->get_array_of_nullable_float64(
                      "array_of_nullable_float64_null") == boost::none);
    }

    {
        std::vector<boost::optional<std::string>> expected{
            boost::optional<std::string>{ "str1" },
            boost::optional<std::string>{ "str2" },
            boost::none
        };
        EXPECT_TRUE(record->get_array_of_string("array_of_string") == expected);
        EXPECT_TRUE(record->get_array_of_string("array_of_string_null") ==
                    boost::none);
    }

    {
        std::vector<boost::optional<big_decimal>> expected{
            boost::optional<big_decimal>{ big_decimal{ 14, 10 } }, boost::none
        };
        EXPECT_TRUE(record->get_array_of_decimal("array_of_decimal") ==
                    expected);
        EXPECT_TRUE(record->get_array_of_decimal("array_of_decimal_null") ==
                    boost::none);
    }

    {
        std::vector<boost::optional<local_time>> expected{
            boost::optional<local_time>{ local_time{ 19, 52, 10, 123456789 } },
            boost::none
        };
        EXPECT_TRUE(record->get_array_of_time("array_of_time") == expected);
        EXPECT_TRUE(record->get_array_of_time("array_of_time_null") ==
                    boost::none);
    }

    {
        std::vector<boost::optional<local_date>> expected{
            boost::optional<local_date>{ local_date{ 2023, 2, 6 } }, boost::none
        };
        EXPECT_TRUE(record->get_array_of_date("array_of_date") == expected);
        EXPECT_TRUE(record->get_array_of_date("array_of_date_null") ==
                    boost::none);
    }

    {
        std::vector<boost::optional<local_date_time>> expected{
            boost::optional<local_date_time>{
              local_date_time{ { 2023, 2, 6 }, { 19, 52, 10, 123456789 } } },
            boost::none
        };
        EXPECT_TRUE(record->get_array_of_timestamp("array_of_timestamp") ==
                    expected);
        EXPECT_TRUE(record->get_array_of_timestamp("array_of_timestamp_null") ==
                    boost::none);
    }

    {
        std::vector<boost::optional<offset_date_time>> expected{
            boost::optional<offset_date_time>{ offset_date_time{
              { { 2023, 2, 6 }, { 19, 52, 10, 123456789 } }, 5000 } },
            boost::none
        };
        EXPECT_TRUE(record->get_array_of_timestamp_with_timezone(
                      "array_of_timestamp_with_timezone") == expected);
        EXPECT_TRUE(record->get_array_of_timestamp_with_timezone(
                      "array_of_timestamp_with_timezone_null") == boost::none);
    }

    {
        EXPECT_TRUE(
          record->get_array_of_generic_record("array_of_generic_record")
            ->at(0)
            ->get_boolean("nested_boolean") == true);
        EXPECT_TRUE(
          record->get_array_of_generic_record("array_of_generic_record")
            ->at(1) == boost::none);
        EXPECT_TRUE(record->get_array_of_generic_record(
                      "array_of_generic_record_null") == boost::none);
    }
}

TEST_F(CompactGenericRecordIntegrationTest,
       test_put_record_from_cpp_and_read_at_server_side)
{
    auto record = create_generic_record();

    auto map_name = random_string();
    client.get_map(map_name).get()->put(1, record).get();

    validate_record(map_name);
}

TEST_F(CompactGenericRecordIntegrationTest, test_put_and_read_with_sql)
{
    auto type_name = random_string();
    auto record = create_generic_record(type_name);

    auto map_name = random_string();
    client.get_map(map_name).get()->put(1, record).get();

    (void)client.get_sql()
      .execute(boost::str(boost::format(
                            R"(
                    CREATE MAPPING "%1%" (
                        id INT,
                        int32 INT
                    )
                    TYPE IMap
                    OPTIONS (
                        'keyFormat' = 'int',
                        'valueFormat' = 'compact',
                        'valueCompactTypeName' = '%2%'
                    )
                )") % map_name %
                          type_name))
      .get();

    auto result =
      client.get_sql()
        .execute(boost::str(boost::format("SELECT * FROM %1%") % map_name))
        .get();

    for (auto iter = result->iterator(); iter.has_next();) {
        auto page = iter.next().get();

        for (const auto& row : page->rows()) {
            auto result = row.get_object<int>("int32");

            ASSERT_TRUE(result.has_value());
            EXPECT_EQ(*result, record.get_int32("int32"));
        }
    }
}

} // namespace compact
} // namespace test
} // namespace client
} // namespace hazelcast