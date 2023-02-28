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

#include <gtest/gtest.h>
#include "hazelcast/client/serialization/pimpl/schema.h"

namespace hazelcast {
namespace client {
namespace test {
namespace compact {

class CompactSchemaTest : public ::testing::Test
{};

TEST_F(CompactSchemaTest, test_constructor)
{
    using hazelcast::client::serialization::pimpl::field_descriptor;
    using hazelcast::client::serialization::pimpl::field_kind;
    using hazelcast::client::serialization::pimpl::schema;

    schema all_types_schema{
        std::string{ "something" },
        std::unordered_map<std::string, field_descriptor>{
          { "boolean-0", BOOLEAN },
          { "boolean-1", BOOLEAN },
          { "boolean-2", BOOLEAN },
          { "boolean-3", BOOLEAN },
          { "boolean-4", BOOLEAN },
          { "boolean-5", BOOLEAN },
          { "boolean-6", BOOLEAN },
          { "boolean-7", BOOLEAN },
          { "boolean-8", BOOLEAN },
          { "boolean-9", BOOLEAN },
          { "boolean[]", ARRAY_OF_BOOLEAN },
          { "int8", INT8 },
          { "int8[]", ARRAY_OF_INT8 },
          { "int16", INT16 },
          { "int16[]", ARRAY_OF_INT16 },
          { "int32", INT32 },
          { "int32[]", ARRAY_OF_INT32 },
          { "int64", INT64 },
          { "int64[]", ARRAY_OF_INT64 },
          { "float32", FLOAT32 },
          { "float32[]", ARRAY_OF_FLOAT32 },
          { "float64", FLOAT64 },
          { "float64[]", ARRAY_OF_FLOAT64 },
          { "string", STRING },
          { "string[]", ARRAY_OF_STRING },
          { "decimal", DECIMAL },
          { "decimal[]", ARRAY_OF_DECIMAL },
          { "time", TIME },
          { "time[]", ARRAY_OF_TIME },
          { "date", DATE },
          { "date[]", ARRAY_OF_DATE },
          { "timestamp", TIMESTAMP },
          { "timestamp[]", ARRAY_OF_TIMESTAMP },
          { "timestamp_with_timezone", TIMESTAMP_WITH_TIMEZONE },
          { "timestamp_with_timezone[]", ARRAY_OF_TIMESTAMP_WITH_TIMEZONE },
          { "compact", COMPACT },
          { "compact[]", ARRAY_OF_COMPACT },
          { "nullable<boolean>", NULLABLE_BOOLEAN },
          { "nullable<boolean>[]", ARRAY_OF_NULLABLE_BOOLEAN },
          { "nullable<int8>", NULLABLE_INT8 },
          { "nullable<int8>[]", ARRAY_OF_NULLABLE_INT8 },
          { "nullable<int16>", NULLABLE_INT16 },
          { "nullable<int16>[]", ARRAY_OF_NULLABLE_INT16 },
          { "nullable<int32>", NULLABLE_INT32 },
          { "nullable<int32>[]", ARRAY_OF_NULLABLE_INT32 },
          { "nullable<int64>", NULLABLE_INT64 },
          { "nullable<int64>[]", ARRAY_OF_NULLABLE_INT64 },
          { "nullable<float32>", NULLABLE_FLOAT32 },
          { "nullable<float32>[]", ARRAY_OF_NULLABLE_INT64 },
          { "nullable<float64>", NULLABLE_FLOAT64 },
          { "nullable<float64>[]", ARRAY_OF_NULLABLE_FLOAT64 } }
    };

    auto result = all_types_schema.fields();

    // Assert num of fields
    ASSERT_EQ(all_types_schema.fixed_size_fields_length(), 29);
    ASSERT_EQ(all_types_schema.number_of_var_size_fields(), 35);

    // Assert fix-sized fields
    ASSERT_EQ(result["float64"].offset, 0);
    ASSERT_EQ(result["float64"].index, -1);
    ASSERT_EQ(result["float64"].bit_offset, -1);
    ASSERT_EQ(result["int64"].offset, 8);
    ASSERT_EQ(result["int64"].index, -1);
    ASSERT_EQ(result["int64"].bit_offset, -1);
    ASSERT_EQ(result["float32"].offset, 16);
    ASSERT_EQ(result["float32"].index, -1);
    ASSERT_EQ(result["float32"].bit_offset, -1);
    ASSERT_EQ(result["int32"].offset, 20);
    ASSERT_EQ(result["int32"].index, -1);
    ASSERT_EQ(result["int32"].bit_offset, -1);
    ASSERT_EQ(result["int16"].offset, 24);
    ASSERT_EQ(result["int16"].index, -1);
    ASSERT_EQ(result["int16"].bit_offset, -1);
    ASSERT_EQ(result["int8"].offset, 26);
    ASSERT_EQ(result["int8"].index, -1);
    ASSERT_EQ(result["int8"].bit_offset, -1);
    ASSERT_EQ(result["boolean-0"].offset, 27);
    ASSERT_EQ(result["boolean-0"].index, -1);
    ASSERT_EQ(result["boolean-0"].bit_offset, 0);
    ASSERT_EQ(result["boolean-1"].offset, 27);
    ASSERT_EQ(result["boolean-1"].index, -1);
    ASSERT_EQ(result["boolean-1"].bit_offset, 1);
    ASSERT_EQ(result["boolean-2"].offset, 27);
    ASSERT_EQ(result["boolean-2"].index, -1);
    ASSERT_EQ(result["boolean-2"].bit_offset, 2);
    ASSERT_EQ(result["boolean-3"].offset, 27);
    ASSERT_EQ(result["boolean-3"].index, -1);
    ASSERT_EQ(result["boolean-3"].bit_offset, 3);
    ASSERT_EQ(result["boolean-4"].offset, 27);
    ASSERT_EQ(result["boolean-4"].index, -1);
    ASSERT_EQ(result["boolean-4"].bit_offset, 4);
    ASSERT_EQ(result["boolean-5"].offset, 27);
    ASSERT_EQ(result["boolean-5"].index, -1);
    ASSERT_EQ(result["boolean-5"].bit_offset, 5);
    ASSERT_EQ(result["boolean-6"].offset, 27);
    ASSERT_EQ(result["boolean-6"].index, -1);
    ASSERT_EQ(result["boolean-6"].bit_offset, 6);
    ASSERT_EQ(result["boolean-7"].offset, 27);
    ASSERT_EQ(result["boolean-7"].index, -1);
    ASSERT_EQ(result["boolean-7"].bit_offset, 7);
    ASSERT_EQ(result["boolean-8"].offset, 28);
    ASSERT_EQ(result["boolean-8"].index, -1);
    ASSERT_EQ(result["boolean-8"].bit_offset, 0);
    ASSERT_EQ(result["boolean-9"].offset, 28);
    ASSERT_EQ(result["boolean-9"].index, -1);
    ASSERT_EQ(result["boolean-9"].bit_offset, 1);

    // Assert variable sized fields
    ASSERT_EQ(result["boolean[]"].offset, -1);
    ASSERT_EQ(result["boolean[]"].index, 0);
    ASSERT_EQ(result["boolean[]"].bit_offset, -1);
    ASSERT_EQ(result["compact"].offset, -1);
    ASSERT_EQ(result["compact"].index, 1);
    ASSERT_EQ(result["compact"].bit_offset, -1);
    ASSERT_EQ(result["compact[]"].offset, -1);
    ASSERT_EQ(result["compact[]"].index, 2);
    ASSERT_EQ(result["compact[]"].bit_offset, -1);
    ASSERT_EQ(result["date"].offset, -1);
    ASSERT_EQ(result["date"].index, 3);
    ASSERT_EQ(result["date"].bit_offset, -1);
    ASSERT_EQ(result["date[]"].offset, -1);
    ASSERT_EQ(result["date[]"].index, 4);
    ASSERT_EQ(result["date[]"].bit_offset, -1);
    ASSERT_EQ(result["decimal"].offset, -1);
    ASSERT_EQ(result["decimal"].index, 5);
    ASSERT_EQ(result["decimal"].bit_offset, -1);
    ASSERT_EQ(result["decimal[]"].offset, -1);
    ASSERT_EQ(result["decimal[]"].index, 6);
    ASSERT_EQ(result["decimal[]"].bit_offset, -1);
    ASSERT_EQ(result["float32[]"].offset, -1);
    ASSERT_EQ(result["float32[]"].index, 7);
    ASSERT_EQ(result["float32[]"].bit_offset, -1);
    ASSERT_EQ(result["float64[]"].offset, -1);
    ASSERT_EQ(result["float64[]"].index, 8);
    ASSERT_EQ(result["float64[]"].bit_offset, -1);
    ASSERT_EQ(result["int16[]"].offset, -1);
    ASSERT_EQ(result["int16[]"].index, 9);
    ASSERT_EQ(result["int16[]"].bit_offset, -1);
    ASSERT_EQ(result["int32[]"].offset, -1);
    ASSERT_EQ(result["int32[]"].index, 10);
    ASSERT_EQ(result["int32[]"].bit_offset, -1);
    ASSERT_EQ(result["int64[]"].offset, -1);
    ASSERT_EQ(result["int64[]"].index, 11);
    ASSERT_EQ(result["int64[]"].bit_offset, -1);
    ASSERT_EQ(result["int8[]"].offset, -1);
    ASSERT_EQ(result["int8[]"].index, 12);
    ASSERT_EQ(result["int8[]"].bit_offset, -1);
    ASSERT_EQ(result["nullable<boolean>"].offset, -1);
    ASSERT_EQ(result["nullable<boolean>"].index, 13);
    ASSERT_EQ(result["nullable<boolean>"].bit_offset, -1);
    ASSERT_EQ(result["nullable<boolean>[]"].offset, -1);
    ASSERT_EQ(result["nullable<boolean>[]"].index, 14);
    ASSERT_EQ(result["nullable<boolean>[]"].bit_offset, -1);
    ASSERT_EQ(result["nullable<float32>"].offset, -1);
    ASSERT_EQ(result["nullable<float32>"].index, 15);
    ASSERT_EQ(result["nullable<float32>"].bit_offset, -1);
    ASSERT_EQ(result["nullable<float32>[]"].offset, -1);
    ASSERT_EQ(result["nullable<float32>[]"].index, 16);
    ASSERT_EQ(result["nullable<float32>[]"].bit_offset, -1);
    ASSERT_EQ(result["nullable<float64>"].offset, -1);
    ASSERT_EQ(result["nullable<float64>"].index, 17);
    ASSERT_EQ(result["nullable<float64>"].bit_offset, -1);
    ASSERT_EQ(result["nullable<float64>[]"].offset, -1);
    ASSERT_EQ(result["nullable<float64>[]"].index, 18);
    ASSERT_EQ(result["nullable<float64>[]"].bit_offset, -1);
    ASSERT_EQ(result["nullable<int16>"].offset, -1);
    ASSERT_EQ(result["nullable<int16>"].index, 19);
    ASSERT_EQ(result["nullable<int16>"].bit_offset, -1);
    ASSERT_EQ(result["nullable<int16>[]"].offset, -1);
    ASSERT_EQ(result["nullable<int16>[]"].index, 20);
    ASSERT_EQ(result["nullable<int16>[]"].bit_offset, -1);
    ASSERT_EQ(result["nullable<int32>"].offset, -1);
    ASSERT_EQ(result["nullable<int32>"].index, 21);
    ASSERT_EQ(result["nullable<int32>"].bit_offset, -1);
    ASSERT_EQ(result["nullable<int32>[]"].offset, -1);
    ASSERT_EQ(result["nullable<int32>[]"].index, 22);
    ASSERT_EQ(result["nullable<int32>[]"].bit_offset, -1);
    ASSERT_EQ(result["nullable<int64>"].offset, -1);
    ASSERT_EQ(result["nullable<int64>"].index, 23);
    ASSERT_EQ(result["nullable<int64>"].bit_offset, -1);
    ASSERT_EQ(result["nullable<int64>[]"].offset, -1);
    ASSERT_EQ(result["nullable<int64>[]"].index, 24);
    ASSERT_EQ(result["nullable<int64>[]"].bit_offset, -1);
    ASSERT_EQ(result["nullable<int8>"].offset, -1);
    ASSERT_EQ(result["nullable<int8>"].index, 25);
    ASSERT_EQ(result["nullable<int8>"].bit_offset, -1);
    ASSERT_EQ(result["nullable<int8>[]"].offset, -1);
    ASSERT_EQ(result["nullable<int8>[]"].index, 26);
    ASSERT_EQ(result["nullable<int8>[]"].bit_offset, -1);
    ASSERT_EQ(result["string"].offset, -1);
    ASSERT_EQ(result["string"].index, 27);
    ASSERT_EQ(result["string"].bit_offset, -1);
    ASSERT_EQ(result["string[]"].offset, -1);
    ASSERT_EQ(result["string[]"].index, 28);
    ASSERT_EQ(result["string[]"].bit_offset, -1);
    ASSERT_EQ(result["time"].offset, -1);
    ASSERT_EQ(result["time"].index, 29);
    ASSERT_EQ(result["time"].bit_offset, -1);
    ASSERT_EQ(result["time[]"].offset, -1);
    ASSERT_EQ(result["time[]"].index, 30);
    ASSERT_EQ(result["time[]"].bit_offset, -1);
    ASSERT_EQ(result["timestamp"].offset, -1);
    ASSERT_EQ(result["timestamp"].index, 31);
    ASSERT_EQ(result["timestamp"].bit_offset, -1);
    ASSERT_EQ(result["timestamp[]"].offset, -1);
    ASSERT_EQ(result["timestamp[]"].index, 32);
    ASSERT_EQ(result["timestamp[]"].bit_offset, -1);
    ASSERT_EQ(result["timestamp_with_timezone"].offset, -1);
    ASSERT_EQ(result["timestamp_with_timezone"].index, 33);
    ASSERT_EQ(result["timestamp_with_timezone"].bit_offset, -1);
    ASSERT_EQ(result["timestamp_with_timezone[]"].offset, -1);
    ASSERT_EQ(result["timestamp_with_timezone[]"].index, 34);
    ASSERT_EQ(result["timestamp_with_timezone[]"].bit_offset, -1);
}

TEST_F(CompactSchemaTest, test_with_no_fields)
{
    schema no_fields_schema{
        std::string{ "something" },
        std::unordered_map<std::string, field_descriptor>{}
    };

    ASSERT_EQ(no_fields_schema.fields().size(), 0);
    ASSERT_EQ(no_fields_schema.fixed_size_fields_length(), 0);
    ASSERT_EQ(no_fields_schema.number_of_var_size_fields(), 0);
}

} // namespace compact
} // namespace test
} // namespace client
} // namespace hazelcast