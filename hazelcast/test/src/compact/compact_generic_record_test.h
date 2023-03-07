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

#include <boost/property_tree/json_parser.hpp>

#include "compact_test_base.h"
#include "serialization/generic_record_factory.h"

#include "../TestHelperFunctions.h"

namespace hazelcast {
namespace client {
namespace test {
namespace compact {

class CompactGenericRecordTest : public compact_test_base
{
protected:
    using generic_record_builder =
      serialization::generic_record::generic_record_builder;
    using generic_record = serialization::generic_record::generic_record;

    SerializationService& serialization_service()
    {
        return spi::ClientContext{ client }.get_serialization_service();
    }

    void assert_setter_throws(generic_record_builder& builder,
                              const std::string& field_name,
                              int value,
                              const std::string& error_message)
    {
        auto fn = [&]() { builder.set_int32(field_name, value); };
        auto handler =
          [&](const hazelcast::client::exception::hazelcast_serialization& e) {
              std::string message = e.what();

              EXPECT_NE(message.find(error_message), std::string::npos);
          };

        EXPECT_THROW_FN(fn(), exception::hazelcast_serialization, handler);
    }

    void verify_new_builder_with_clone(generic_record& record)
    {
        generic_record_builder clone_builder = record.new_builder_with_clone();

        clone_builder.set_int32("foo", 2);

        assert_setter_throws(
          clone_builder, "foo", 5, "Field can only be written once");
        assert_setter_throws(
          clone_builder, "notExisting", 3, "Invalid field name");

        generic_record clone = clone_builder.build();

        EXPECT_EQ(2, clone.get_int32("foo"));
        EXPECT_EQ(1231L, clone.get_int64("bar"));
    }
};

TEST_F(CompactGenericRecordTest, test_generic_record_to_string_valid_json)
{
    auto record = create_generic_record();

    std::stringstream ss;

    ss << record;

    boost::property_tree::ptree pt;

    ASSERT_NO_THROW(boost::property_tree::read_json(ss, pt));
}

TEST_F(CompactGenericRecordTest, test_clone_deserialized_generic_record)
{
    generic_record_builder builder{ "fooBarTypeName" };
    builder.set_int32("foo", 1);
    assert_setter_throws(builder, "foo", 5, "Field can only be written once");

    builder.set_int64("bar", 1231L);
    generic_record record = builder.build();

    verify_new_builder_with_clone(record);
}

TEST_F(CompactGenericRecordTest, test_get_field_kind)
{
    generic_record record =
      generic_record_builder{ "test" }.set_string("s", "s").build();

    EXPECT_EQ(serialization::field_kind::STRING, record.get_field_kind("s"));
    EXPECT_EQ(serialization::field_kind::NOT_AVAILABLE,
              record.get_field_kind("ss"));

    auto& service = serialization_service();

    auto data = service.to_data(record);

    // Ensure that schema is distributed
    client.get_map(random_string()).get()->put(random_string(), record).get();
    auto internal_generic_record = service.to_object<generic_record>(data);

    ASSERT_TRUE(internal_generic_record.has_value());

    EXPECT_EQ(serialization::field_kind::STRING,
              internal_generic_record->get_field_kind("s"));
    EXPECT_EQ(serialization::field_kind::NOT_AVAILABLE,
              internal_generic_record->get_field_kind("ss"));
}

TEST_F(CompactGenericRecordTest,
       test_get_field_throws_exception_when_field_does_not_exist)
{
    generic_record record = generic_record_builder{ "test" }.build();

    assert_that_thrown_by<exception::hazelcast_serialization>(
      [&]() { record.get_int32("doesNotExist"); },
      [](const exception::hazelcast_serialization& e) {
          std::string message = e.what();

          EXPECT_NE(message.find("Invalid field name"), std::string::npos);
      });

    auto& ss = serialization_service();
    auto data = ss.to_data(record);

    // Ensure that schema is distributed
    client.get_map(random_string()).get()->put(random_string(), record).get();
    boost::optional<generic_record> internal_generic_record =
      ss.to_object<generic_record>(data);

    ASSERT_TRUE(internal_generic_record.has_value());

    assert_that_thrown_by<exception::hazelcast_serialization>(
      [&] { internal_generic_record->get_int32("doesNotExist"); },
      [&](const exception::hazelcast_serialization& e) {
          std::string message = e.what();

          EXPECT_NE(message.find("Invalid field name"), std::string::npos);
      });
}

TEST_F(CompactGenericRecordTest,
       test_get_field_throws_exception_when_field_type_does_not_match)
{
    generic_record record =
      generic_record_builder{ "test" }.set_int32("foo", 123).build();

    assert_that_thrown_by<exception::hazelcast_serialization>(
      [&] { record.get_int64("foo"); },
      [&](const exception::hazelcast_serialization& e) {
          std::string message = e.what();

          EXPECT_NE(message.find("Invalid field kind"), std::string::npos);
      });

    auto& ss = serialization_service();
    auto data = ss.to_data(record);

    // Ensure that schema is distributed
    client.get_map(random_string()).get()->put(random_string(), record).get();
    boost::optional<generic_record> internal_generic_record =
      ss.to_object<generic_record>(data);

    ASSERT_TRUE(internal_generic_record.has_value());

    assert_that_thrown_by<exception::hazelcast_serialization>(
      [&] { internal_generic_record->get_int64("foo"); },
      [&](const exception::hazelcast_serialization& e) {
          std::string message = e.what();

          EXPECT_NE(message.find("Invalid field kind"), std::string::npos);
      });
}

TEST_F(CompactGenericRecordTest, test_equality)
{
    auto type_name = random_string();
    auto x = create_generic_record(type_name);
    auto y = create_generic_record(type_name);

    EXPECT_EQ(x, y);
}

TEST_F(CompactGenericRecordTest, test_inequality)
{
    // Different typenames
    auto x = create_generic_record(random_string());
    auto y = create_generic_record(random_string());
    auto z = x.new_builder_with_clone().set_int32("id", 1000).build();

    EXPECT_NE(x, y);
    EXPECT_NE(y, z);
}

TEST_F(CompactGenericRecordTest,
       test_read_nullable_primitive_as_primitive_if_it_is_null)
{
    using namespace serialization::generic_record;

    auto record = generic_record_builder{ random_string() }
                    .set_nullable_boolean("nullable_boolean", boost::none)
                    .build();

    EXPECT_THROW(record.get_boolean("nullable_boolean"),
                 exception::hazelcast_serialization);
}

TEST_F(CompactGenericRecordTest,
       test_read_array_of_primitive_as_array_of_nullable_primitive)
{
    using namespace serialization::generic_record;

    auto record =
      generic_record_builder{ random_string() }
        .set_array_of_boolean("array_of_boolean",
                              std::vector<bool>{ true, false })
        .set_array_of_int8("array_of_int8", std::vector<int8_t>{ 8, 9 })
        .set_array_of_int16("array_of_int16", std::vector<int16_t>{ 16, 17 })
        .set_array_of_int32("array_of_int32", std::vector<int32_t>{ 32, 33 })
        .set_array_of_int64("array_of_int64", std::vector<int64_t>{ 64, 65 })
        .set_array_of_float32("array_of_float32",
                              std::vector<float>{ 32.32f, 33.33f })
        .set_array_of_float64("array_of_float64",
                              std::vector<double>{ 64.64, 65.65 })
        .build();

    {
        auto values = record.get_array_of_nullable_boolean("array_of_boolean");
        EXPECT_TRUE(values.has_value());
        EXPECT_EQ(*values->at(0), true);
        EXPECT_EQ(*values->at(1), false);
    }

    {
        auto values = record.get_array_of_nullable_int8("array_of_int8");
        EXPECT_TRUE(values.has_value());
        EXPECT_EQ(*values->at(0), 8);
        EXPECT_EQ(*values->at(1), 9);
    }

    {
        auto values = record.get_array_of_nullable_int16("array_of_int16");
        EXPECT_TRUE(values.has_value());
        EXPECT_EQ(*values->at(0), 16);
        EXPECT_EQ(*values->at(1), 17);
    }

    {
        auto values = record.get_array_of_nullable_int32("array_of_int32");
        EXPECT_TRUE(values.has_value());
        EXPECT_EQ(*values->at(0), 32);
        EXPECT_EQ(*values->at(1), 33);
    }

    {
        auto values = record.get_array_of_nullable_int64("array_of_int64");
        EXPECT_TRUE(values.has_value());
        EXPECT_EQ(*values->at(0), 64);
        EXPECT_EQ(*values->at(1), 65);
    }

    {
        auto values = record.get_array_of_nullable_float32("array_of_float32");
        EXPECT_TRUE(values.has_value());
        EXPECT_EQ(*values->at(0), 32.32f);
        EXPECT_EQ(*values->at(1), 33.33f);
    }

    {
        auto values = record.get_array_of_nullable_float64("array_of_float64");
        EXPECT_TRUE(values.has_value());
        EXPECT_EQ(*values->at(0), 64.64);
        EXPECT_EQ(*values->at(1), 65.65);
    }
}

TEST_F(CompactGenericRecordTest,
       test_read_array_of_nullable_primitive_as_array_of_primitive)
{
    using namespace serialization::generic_record;

    auto record =
      generic_record_builder{ random_string() }
        .set_array_of_nullable_boolean(
          "array_of_nullable_boolean",
          std::vector<boost::optional<bool>>{ boost::optional<bool>{ true },
                                              boost::optional<bool>{ false } })
        .set_array_of_nullable_int8(
          "array_of_nullable_int8",
          std::vector<boost::optional<int8_t>>{ boost::optional<int8_t>{ 88 },
                                                boost::optional<int8_t>{ 89 } })
        .set_array_of_nullable_int16("array_of_nullable_int16",
                                     std::vector<boost::optional<int16_t>>{
                                       boost::optional<int16_t>{ 1616 },
                                       boost::optional<int16_t>{ 1717 } })
        .set_array_of_nullable_int32("array_of_nullable_int32",
                                     std::vector<boost::optional<int32_t>>{
                                       boost::optional<int32_t>{ 3232 },
                                       boost::optional<int32_t>{ 3333 } })
        .set_array_of_nullable_int64("array_of_nullable_int64",
                                     std::vector<boost::optional<int64_t>>{
                                       boost::optional<int64_t>{ 6464 },
                                       boost::optional<int64_t>{ 6565 } })
        .set_array_of_nullable_float32("array_of_nullable_float32",
                                       std::vector<boost::optional<float>>{
                                         boost::optional<float>{ 3232.3232f },
                                         boost::optional<float>{ 3333.3333f } })
        .set_array_of_nullable_float64("array_of_nullable_float64",
                                       std::vector<boost::optional<double>>{
                                         boost::optional<double>{ 6464.6464 },
                                         boost::optional<double>{ 6565.6565 } })
        .build();

    {
        auto values = record.get_array_of_boolean("array_of_nullable_boolean");
        EXPECT_TRUE(values.has_value());
        EXPECT_EQ(values->at(0), true);
        EXPECT_EQ(values->at(1), false);
    }

    {
        auto values = record.get_array_of_int8("array_of_nullable_int8");
        EXPECT_TRUE(values.has_value());
        EXPECT_EQ(values->at(0), 88);
        EXPECT_EQ(values->at(1), 89);
    }

    {
        auto values = record.get_array_of_int16("array_of_nullable_int16");
        EXPECT_TRUE(values.has_value());
        EXPECT_EQ(values->at(0), 1616);
        EXPECT_EQ(values->at(1), 1717);
    }

    {
        auto values = record.get_array_of_int32("array_of_nullable_int32");
        EXPECT_TRUE(values.has_value());
        EXPECT_EQ(values->at(0), 3232);
        EXPECT_EQ(values->at(1), 3333);
    }

    {
        auto values = record.get_array_of_int64("array_of_nullable_int64");
        EXPECT_TRUE(values.has_value());
        EXPECT_EQ(values->at(0), 6464);
        EXPECT_EQ(values->at(1), 6565);
    }

    {
        auto values =
          record.get_array_of_nullable_float32("array_of_nullable_float32");
        EXPECT_TRUE(values.has_value());
        EXPECT_EQ(values->at(0), 3232.3232f);
        EXPECT_EQ(values->at(1), 3333.3333f);
    }

    {
        auto values =
          record.get_array_of_nullable_float64("array_of_nullable_float64");
        EXPECT_TRUE(values.has_value());
        EXPECT_EQ(values->at(0), 6464.6464);
        EXPECT_EQ(values->at(1), 6565.6565);
    }
}

TEST_F(
  CompactGenericRecordTest,
  test_read_array_of_nullable_primitive_as_array_of_primitive_if_it_contains_null)
{
    using namespace serialization::generic_record;

    auto record = generic_record_builder{ random_string() }
                    .set_array_of_nullable_boolean(
                      "array_of_nullable_boolean",
                      std::vector<boost::optional<bool>>{
                        boost::optional<bool>{ true }, boost::none })
                    .build();

    EXPECT_THROW(record.get_array_of_boolean("array_of_nullable_boolean"),
                 exception::hazelcast_serialization);
}

TEST_F(CompactGenericRecordTest, test_get_field_names)
{
    generic_record record = create_generic_record();

    EXPECT_EQ(record.get_field_names().size(), 78);
}

TEST_F(CompactGenericRecordTest,
       test_generic_builder_and_generic_record_integrity)
{
    generic_record record = create_generic_record();

    EXPECT_EQ(record.get_int32("id"), 1);
    EXPECT_EQ(record.get_boolean("boolean"), true);
    EXPECT_EQ(record.get_int8("int8"), 8);
    EXPECT_EQ(record.get_int16("int16"), 16);
    EXPECT_EQ(record.get_int32("int32"), 32);
    EXPECT_EQ(record.get_int64("int64"), 64);
    EXPECT_EQ(record.get_float32("float32"), 32.32f);
    EXPECT_EQ(record.get_float64("float64"), 64.64);
    EXPECT_EQ(record.get_nullable_boolean("nullable_boolean"), true);
    EXPECT_EQ(record.get_nullable_boolean("nullable_boolean_null"),
              boost::none);
    EXPECT_EQ(record.get_nullable_int8("nullable_int8"),
              boost::optional<int8_t>(88));
    EXPECT_EQ(record.get_nullable_int8("nullable_int8_null"), boost::none);
    EXPECT_EQ(record.get_nullable_int16("nullable_int16"),
              boost::optional<int16_t>(1616));
    EXPECT_EQ(record.get_nullable_int16("nullable_int16_null"), boost::none);
    EXPECT_EQ(record.get_nullable_int32("nullable_int32"),
              boost::optional<int32_t>(3232));
    EXPECT_EQ(record.get_nullable_int32("nullable_int32_null"), boost::none);
    EXPECT_EQ(record.get_nullable_int64("nullable_int64"),
              boost::optional<int64_t>(6464));
    EXPECT_EQ(record.get_nullable_int64("nullable_int64_null"), boost::none);
    EXPECT_EQ(record.get_nullable_float32("nullable_float32"),
              boost::optional<float>(3232.3232f));
    EXPECT_EQ(record.get_nullable_float32("nullable_float32_null"),
              boost::none);
    EXPECT_EQ(record.get_nullable_float64("nullable_float64"),
              boost::optional<double>(6464.6464));
    EXPECT_EQ(record.get_nullable_float64("nullable_float64_null"),
              boost::none);
    EXPECT_EQ(record.get_string("string"),
              boost::optional<std::string>("a_text"));
    EXPECT_EQ(record.get_string("string_null"), boost::none);
    EXPECT_EQ(record.get_generic_record("generic_record")
                ->get_boolean("nested_boolean"),
              true);
    EXPECT_EQ(record.get_generic_record("generic_record_null"), boost::none);

    {
        big_decimal expected{ 14, 10 };
        EXPECT_TRUE(record.get_decimal("decimal") == expected);
        EXPECT_TRUE(record.get_decimal("decimal_null") == boost::none);
    }

    {
        local_time expected{ 19, 52, 10, 123456789 };
        EXPECT_TRUE(record.get_time("time") == expected);
        EXPECT_TRUE(record.get_time("time_null") == boost::none);
    }

    {
        local_date expected{ 2023, 2, 6 };
        EXPECT_TRUE(record.get_date("date") == expected);
        EXPECT_TRUE(record.get_date("date_null") == boost::none);
    }

    {
        local_date_time expected{ { 2023, 2, 6 }, { 9, 2, 8, 123456789 } };
        EXPECT_TRUE(record.get_timestamp("timestamp") == expected);
        EXPECT_TRUE(record.get_timestamp("timestamp_null") == boost::none);
    }

    {
        offset_date_time expected{
            { { 2023, 2, 6 }, { 19, 52, 10, 123456789 } }, 5000
        };
        EXPECT_TRUE(record.get_timestamp_with_timezone(
                      "timestamp_with_timezone") == expected);
        EXPECT_TRUE(record.get_timestamp_with_timezone(
                      "timestamp_with_timezone_null") == boost::none);
    }

    {
        std::vector<bool> expected{ true, false };
        EXPECT_TRUE(record.get_array_of_boolean("array_of_boolean") ==
                    expected);
        EXPECT_TRUE(record.get_array_of_boolean("array_of_boolean_null") ==
                    boost::none);
    }

    {
        std::vector<int8_t> expected{ 8, 9 };
        EXPECT_TRUE(record.get_array_of_int8("array_of_int8") == expected);
        EXPECT_TRUE(record.get_array_of_int8("array_of_int8_null") ==
                    boost::none);
    }

    {
        std::vector<int16_t> expected{ 16, 17 };
        EXPECT_TRUE(record.get_array_of_int16("array_of_int16") == expected);
        EXPECT_TRUE(record.get_array_of_int16("array_of_int16_null") ==
                    boost::none);
    }

    {
        std::vector<int32_t> expected{ 32, 33 };
        EXPECT_TRUE(record.get_array_of_int32("array_of_int32") == expected);
        EXPECT_TRUE(record.get_array_of_int32("array_of_int32_null") ==
                    boost::none);
    }

    {
        std::vector<int64_t> expected{ 64, 65 };
        EXPECT_TRUE(record.get_array_of_int64("array_of_int64") == expected);
        EXPECT_TRUE(record.get_array_of_int64("array_of_int64_null") ==
                    boost::none);
    }

    {
        std::vector<float> expected{ 32.32f, 33.33f };
        EXPECT_TRUE(record.get_array_of_float32("array_of_float32") ==
                    expected);
        EXPECT_TRUE(record.get_array_of_float32("array_of_float32_null") ==
                    boost::none);
    }

    {
        std::vector<double> expected{ 64.64, 65.65 };
        EXPECT_TRUE(record.get_array_of_float64("array_of_float64") ==
                    expected);
        EXPECT_TRUE(record.get_array_of_float64("array_of_float64_null") ==
                    boost::none);
    }

    {
        std::vector<boost::optional<bool>> expected{
            boost::optional<bool>{ true },
            boost::optional<bool>{ false },
            boost::none
        };
        EXPECT_TRUE(record.get_array_of_nullable_boolean(
                      "array_of_nullable_boolean") == expected);
        EXPECT_TRUE(record.get_array_of_nullable_boolean(
                      "array_of_nullable_boolean_null") == boost::none);
    }

    {
        std::vector<boost::optional<int8_t>> expected{
            boost::optional<int8_t>{ 88 },
            boost::optional<int8_t>{ 89 },
            boost::none
        };
        EXPECT_TRUE(record.get_array_of_nullable_int8(
                      "array_of_nullable_int8") == expected);
        EXPECT_TRUE(record.get_array_of_nullable_int8(
                      "array_of_nullable_int8_null") == boost::none);
    }

    {
        std::vector<boost::optional<int16_t>> expected{
            boost::optional<int16_t>{ 1616 },
            boost::optional<int16_t>{ 1717 },
            boost::none
        };
        EXPECT_TRUE(record.get_array_of_nullable_int16(
                      "array_of_nullable_int16") == expected);
        EXPECT_TRUE(record.get_array_of_nullable_int16(
                      "array_of_nullable_int16_null") == boost::none);
    }

    {
        std::vector<boost::optional<int32_t>> expected{
            boost::optional<int32_t>{ 3232 },
            boost::optional<int32_t>{ 3333 },
            boost::none
        };
        EXPECT_TRUE(record.get_array_of_nullable_int32(
                      "array_of_nullable_int32") == expected);
        EXPECT_TRUE(record.get_array_of_nullable_int32(
                      "array_of_nullable_int32_null") == boost::none);
    }

    {
        std::vector<boost::optional<int64_t>> expected{
            boost::optional<int64_t>{ 6464 },
            boost::optional<int64_t>{ 6565 },
            boost::none
        };
        EXPECT_TRUE(record.get_array_of_nullable_int64(
                      "array_of_nullable_int64") == expected);
        EXPECT_TRUE(record.get_array_of_nullable_int64(
                      "array_of_nullable_int64_null") == boost::none);
    }

    {
        std::vector<boost::optional<float>> expected{
            boost::optional<float>{ 3232.3232f },
            boost::optional<float>{ 3333.3333f },
            boost::none
        };
        EXPECT_TRUE(record.get_array_of_nullable_float32(
                      "array_of_nullable_float32") == expected);
        EXPECT_TRUE(record.get_array_of_nullable_float32(
                      "array_of_nullable_float32_null") == boost::none);
    }

    {
        std::vector<boost::optional<double>> expected{
            boost::optional<double>{ 6464.6464 },
            boost::optional<double>{ 6565.6565 },
            boost::none
        };
        EXPECT_TRUE(record.get_array_of_nullable_float64(
                      "array_of_nullable_float64") == expected);
        EXPECT_TRUE(record.get_array_of_nullable_float64(
                      "array_of_nullable_float64_null") == boost::none);
    }

    {
        std::vector<boost::optional<std::string>> expected{
            boost::optional<std::string>{ "str1" },
            boost::optional<std::string>{ "str2" },
            boost::none
        };
        EXPECT_TRUE(record.get_array_of_string("array_of_string") == expected);
        EXPECT_TRUE(record.get_array_of_string("array_of_string_null") ==
                    boost::none);
    }

    {
        std::vector<boost::optional<big_decimal>> expected{
            boost::optional<big_decimal>{ big_decimal{ 14, 10 } }, boost::none
        };
        EXPECT_TRUE(record.get_array_of_decimal("array_of_decimal") ==
                    expected);
        EXPECT_TRUE(record.get_array_of_decimal("array_of_decimal_null") ==
                    boost::none);
    }

    {
        std::vector<boost::optional<local_time>> expected{
            boost::optional<local_time>{ local_time{ 19, 52, 10, 123456789 } },
            boost::none
        };
        EXPECT_TRUE(record.get_array_of_time("array_of_time") == expected);
        EXPECT_TRUE(record.get_array_of_time("array_of_time_null") ==
                    boost::none);
    }

    {
        std::vector<boost::optional<local_date>> expected{
            boost::optional<local_date>{ local_date{ 2023, 2, 6 } }, boost::none
        };
        EXPECT_TRUE(record.get_array_of_date("array_of_date") == expected);
        EXPECT_TRUE(record.get_array_of_date("array_of_date_null") ==
                    boost::none);
    }

    {
        std::vector<boost::optional<local_date_time>> expected{
            boost::optional<local_date_time>{
              local_date_time{ { 2023, 2, 6 }, { 19, 52, 10, 123456789 } } },
            boost::none
        };
        EXPECT_TRUE(record.get_array_of_timestamp("array_of_timestamp") ==
                    expected);
        EXPECT_TRUE(record.get_array_of_timestamp("array_of_timestamp_null") ==
                    boost::none);
    }

    {
        std::vector<boost::optional<offset_date_time>> expected{
            boost::optional<offset_date_time>{ offset_date_time{
              { { 2023, 2, 6 }, { 19, 52, 10, 123456789 } }, 5000 } },
            boost::none
        };
        EXPECT_TRUE(record.get_array_of_timestamp_with_timezone(
                      "array_of_timestamp_with_timezone") == expected);
        EXPECT_TRUE(record.get_array_of_timestamp_with_timezone(
                      "array_of_timestamp_with_timezone_null") == boost::none);
    }

    {
        EXPECT_TRUE(
          record.get_array_of_generic_record("array_of_generic_record")
            ->at(0)
            ->get_boolean("nested_boolean") == true);
        EXPECT_TRUE(
          record.get_array_of_generic_record("array_of_generic_record")
            ->at(1) == boost::none);
        EXPECT_TRUE(record.get_array_of_generic_record(
                      "array_of_generic_record_null") == boost::none);
    }
}

} // namespace compact
} // namespace test
} // namespace client
} // namespace hazelcast