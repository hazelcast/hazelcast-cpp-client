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

#include <memory>
#include <sstream>

#include <gtest/gtest.h>

#include "hazelcast/client/hazelcast_client.h"
#include "../remote_controller_client.h"
#include "../TestHelperFunctions.h"

#include "compact_test_base.h"
#include "serialization/sample_compact_type.h"

namespace hazelcast {
namespace client {
namespace test {
namespace compact {

class CompactMix : public compact_test_base
{};

TEST_F(CompactMix, schema_service_get_non_existing_schema)
{
    spi::ClientContext ctx{ client };

    serialization::pimpl::default_schema_service service{ ctx };

    ASSERT_EQ(service.get(1000), nullptr);
}

TEST_F(CompactMix, schema_equality)
{
    auto x = get_schema<sample_compact_type>();
    auto y = get_schema<sample_compact_type>();

    ASSERT_EQ(x, y);
}

TEST_F(CompactMix, schema_inequality)
{
    auto x = get_schema<sample_compact_type>();
    auto y = get_schema<nested_type>();

    ASSERT_NE(x, y);
}

TEST_F(CompactMix, field_kind_output)
{
    using field_kind = serialization::field_kind;
    auto to_string = [](field_kind kind) {
        std::ostringstream os;

        os << kind;

        return os.str();
    };

    EXPECT_EQ(to_string(field_kind::BOOLEAN), "BOOLEAN");
    EXPECT_EQ(to_string(field_kind::ARRAY_OF_BOOLEAN), "ARRAY_OF_BOOLEAN");
    EXPECT_EQ(to_string(field_kind::INT8), "INT8");
    EXPECT_EQ(to_string(field_kind::ARRAY_OF_INT8), "ARRAY_OF_INT8");
    EXPECT_EQ(to_string(field_kind::INT16), "INT16");
    EXPECT_EQ(to_string(field_kind::ARRAY_OF_INT16), "ARRAY_OF_INT16");
    EXPECT_EQ(to_string(field_kind::INT32), "INT32");
    EXPECT_EQ(to_string(field_kind::ARRAY_OF_INT32), "ARRAY_OF_INT32");
    EXPECT_EQ(to_string(field_kind::INT64), "INT64");
    EXPECT_EQ(to_string(field_kind::ARRAY_OF_INT64), "ARRAY_OF_INT64");
    EXPECT_EQ(to_string(field_kind::FLOAT32), "FLOAT32");
    EXPECT_EQ(to_string(field_kind::ARRAY_OF_FLOAT32), "ARRAY_OF_FLOAT32");
    EXPECT_EQ(to_string(field_kind::FLOAT64), "FLOAT64");
    EXPECT_EQ(to_string(field_kind::ARRAY_OF_FLOAT64), "ARRAY_OF_FLOAT64");
    EXPECT_EQ(to_string(field_kind::STRING), "STRING");
    EXPECT_EQ(to_string(field_kind::ARRAY_OF_STRING), "ARRAY_OF_STRING");
    EXPECT_EQ(to_string(field_kind::DECIMAL), "DECIMAL");
    EXPECT_EQ(to_string(field_kind::ARRAY_OF_DECIMAL), "ARRAY_OF_DECIMAL");
    EXPECT_EQ(to_string(field_kind::TIME), "TIME");
    EXPECT_EQ(to_string(field_kind::ARRAY_OF_TIME), "ARRAY_OF_TIME");
    EXPECT_EQ(to_string(field_kind::DATE), "DATE");
    EXPECT_EQ(to_string(field_kind::ARRAY_OF_DATE), "ARRAY_OF_DATE");
    EXPECT_EQ(to_string(field_kind::TIMESTAMP), "TIMESTAMP");
    EXPECT_EQ(to_string(field_kind::ARRAY_OF_TIMESTAMP), "ARRAY_OF_TIMESTAMP");
    EXPECT_EQ(to_string(field_kind::TIMESTAMP_WITH_TIMEZONE),
              "TIMESTAMP_WITH_TIMEZONE");
    EXPECT_EQ(to_string(field_kind::ARRAY_OF_TIMESTAMP_WITH_TIMEZONE),
              "ARRAY_OF_TIMESTAMP_WITH_TIMEZONE");
    EXPECT_EQ(to_string(field_kind::COMPACT), "COMPACT");
    EXPECT_EQ(to_string(field_kind::ARRAY_OF_COMPACT), "ARRAY_OF_COMPACT");
    EXPECT_EQ(to_string(field_kind::NULLABLE_BOOLEAN), "NULLABLE_BOOLEAN");
    EXPECT_EQ(to_string(field_kind::ARRAY_OF_NULLABLE_BOOLEAN),
              "ARRAY_OF_NULLABLE_BOOLEAN");
    EXPECT_EQ(to_string(field_kind::NULLABLE_INT8), "NULLABLE_INT8");
    EXPECT_EQ(to_string(field_kind::ARRAY_OF_NULLABLE_INT8),
              "ARRAY_OF_NULLABLE_INT8");
    EXPECT_EQ(to_string(field_kind::NULLABLE_INT16), "NULLABLE_INT16");
    EXPECT_EQ(to_string(field_kind::ARRAY_OF_NULLABLE_INT16),
              "ARRAY_OF_NULLABLE_INT16");
    EXPECT_EQ(to_string(field_kind::NULLABLE_INT32), "NULLABLE_INT32");
    EXPECT_EQ(to_string(field_kind::ARRAY_OF_NULLABLE_INT32),
              "ARRAY_OF_NULLABLE_INT32");
    EXPECT_EQ(to_string(field_kind::NULLABLE_INT64), "NULLABLE_INT64");
    EXPECT_EQ(to_string(field_kind::NULLABLE_FLOAT32), "NULLABLE_FLOAT32");
    EXPECT_EQ(to_string(field_kind::ARRAY_OF_NULLABLE_FLOAT32),
              "ARRAY_OF_NULLABLE_FLOAT32");
    EXPECT_EQ(to_string(field_kind::NULLABLE_FLOAT64), "NULLABLE_FLOAT64");
    EXPECT_EQ(to_string(field_kind::ARRAY_OF_NULLABLE_FLOAT64),
              "ARRAY_OF_NULLABLE_FLOAT64");
}

} // namespace compact
} // namespace test
} // namespace client
} // namespace hazelcast