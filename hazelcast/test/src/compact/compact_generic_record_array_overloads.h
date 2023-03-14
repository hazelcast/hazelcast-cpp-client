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
#include "hazelcast/client/serialization/generic_record_builder.h"

namespace hazelcast {
namespace client {
namespace test {
namespace compact {

struct CompactGenericRecordArrayOverloads : testing::Test
{
protected:
    template<typename T>
    std::vector<T> expecteds()
    {
        return std::vector<T>{ T(0), T(1) };
    }

    template<typename T>
    std::vector<boost::optional<T>> nullable_expecteds()
    {
        return std::vector<boost::optional<T>>{ boost::optional<T>{ T(0) },
                                                boost::optional<T>{ T(1) } };
    }
};

TEST_F(CompactGenericRecordArrayOverloads, array_overloads)
{
    using namespace serialization::generic_record;
    generic_record record =
      generic_record_builder{ random_string() }
        .set_array_of_boolean("aob_opt",
                              boost::optional<std::vector<bool>>{
                                std::vector<bool>{ false, true } })
        .set_array_of_boolean("aob_vec", std::vector<bool>{ false, true })
        .set_array_of_boolean("aob_init", { false, true })
        .set_array_of_int8("aoi8_opt",
                           boost::optional<std::vector<int8_t>>{
                             std::vector<int8_t>{ int8_t(0), int8_t(1) } })
        .set_array_of_int8("aoi8_vec",
                           std::vector<int8_t>{ int8_t(0), int8_t(1) })
        .set_array_of_int8("aoi8_init", { int8_t(0), int8_t(1) })
        .set_array_of_int16("aoi16_opt",
                            boost::optional<std::vector<int16_t>>{
                              std::vector<int16_t>{ int16_t(0), int16_t(1) } })
        .set_array_of_int16("aoi16_vec",
                            std::vector<int16_t>{ int16_t(0), int16_t(1) })
        .set_array_of_int16("aoi16_init", { int16_t(0), int16_t(1) })
        .set_array_of_int32(
          "aoi32_opt",
          boost::optional<std::vector<int32_t>>{ std::vector<int32_t>{ 0, 1 } })
        .set_array_of_int32("aoi32_vec", std::vector<int32_t>{ 0, 1 })
        .set_array_of_int32("aoi32_init", { 0, 1 })
        .set_array_of_int64(
          "aoi64_opt",
          boost::optional<std::vector<int64_t>>{ std::vector<int64_t>{ 0, 1 } })
        .set_array_of_int64("aoi64_vec", std::vector<int64_t>{ 0, 1 })
        .set_array_of_int64("aoi64_init", { 0, 1 })
        .set_array_of_float32(
          "aof32_opt",
          boost::optional<std::vector<float>>{ std::vector<float>{ 0, 1 } })
        .set_array_of_float32("aof32_vec", std::vector<float>{ 0, 1 })
        .set_array_of_float32("aof32_init", { 0, 1 })
        .set_array_of_float64(
          "aof64_opt",
          boost::optional<std::vector<double>>{ std::vector<double>{ 0, 1 } })
        .set_array_of_float64("aof64_vec", std::vector<double>{ 0, 1 })
        .set_array_of_float64("aof64_init", { 0, 1 })
        .build();

    EXPECT_EQ(*record.get_array_of_boolean("aob_opt"), expecteds<bool>());
    EXPECT_EQ(*record.get_array_of_boolean("aob_vec"), expecteds<bool>());
    EXPECT_EQ(*record.get_array_of_boolean("aob_init"), expecteds<bool>());
    EXPECT_EQ(*record.get_array_of_int8("aoi8_opt"), expecteds<int8_t>());
    EXPECT_EQ(*record.get_array_of_int8("aoi8_vec"), expecteds<int8_t>());
    EXPECT_EQ(*record.get_array_of_int8("aoi8_init"), expecteds<int8_t>());
    EXPECT_EQ(*record.get_array_of_int16("aoi16_opt"), expecteds<int16_t>());
    EXPECT_EQ(*record.get_array_of_int16("aoi16_vec"), expecteds<int16_t>());
    EXPECT_EQ(*record.get_array_of_int16("aoi16_init"), expecteds<int16_t>());
    EXPECT_EQ(*record.get_array_of_int32("aoi32_opt"), expecteds<int32_t>());
    EXPECT_EQ(*record.get_array_of_int32("aoi32_vec"), expecteds<int32_t>());
    EXPECT_EQ(*record.get_array_of_int32("aoi32_init"), expecteds<int32_t>());
    EXPECT_EQ(*record.get_array_of_int64("aoi64_opt"), expecteds<int64_t>());
    EXPECT_EQ(*record.get_array_of_int64("aoi64_vec"), expecteds<int64_t>());
    EXPECT_EQ(*record.get_array_of_int64("aoi64_init"), expecteds<int64_t>());
    EXPECT_EQ(*record.get_array_of_float32("aof32_opt"), expecteds<float>());
    EXPECT_EQ(*record.get_array_of_float32("aof32_vec"), expecteds<float>());
    EXPECT_EQ(*record.get_array_of_float32("aof32_init"), expecteds<float>());
    EXPECT_EQ(*record.get_array_of_float64("aof64_opt"), expecteds<double>());
    EXPECT_EQ(*record.get_array_of_float64("aof64_vec"), expecteds<double>());
    EXPECT_EQ(*record.get_array_of_float64("aof64_init"), expecteds<double>());
}

TEST_F(CompactGenericRecordArrayOverloads, array_of_nullable_overloads)
{
    using namespace serialization::generic_record;
    generic_record record =
      generic_record_builder{ random_string() }
        .set_array_of_nullable_boolean("aob_vec",
                                       std::vector<bool>{ false, true })
        .set_array_of_nullable_boolean("aob_init", { false, true })
        .set_array_of_nullable_int8("aoi8_vec",
                                    std::vector<int8_t>{ int8_t(0), int8_t(1) })
        .set_array_of_nullable_int8("aoi8_init", { int8_t(0), int8_t(1) })
        .set_array_of_nullable_int16(
          "aoi16_vec", std::vector<int16_t>{ int16_t(0), int16_t(1) })
        .set_array_of_nullable_int16("aoi16_init", { int16_t(0), int16_t(1) })
        .set_array_of_nullable_int32("aoi32_vec", std::vector<int32_t>{ 0, 1 })
        .set_array_of_nullable_int32("aoi32_init", { 0, 1 })
        .set_array_of_nullable_int64("aoi64_vec", std::vector<int64_t>{ 0, 1 })
        .set_array_of_nullable_int64("aoi64_init", { 0, 1 })
        .set_array_of_nullable_float32("aof32_vec", std::vector<float>{ 0, 1 })
        .set_array_of_nullable_float32("aof32_init", { 0, 1 })
        .set_array_of_nullable_float64("aof64_vec", std::vector<double>{ 0, 1 })
        .set_array_of_nullable_float64("aof64_init", { 0, 1 })
        .build();

    EXPECT_EQ(*record.get_array_of_nullable_boolean("aob_vec"),
              nullable_expecteds<bool>());
    EXPECT_EQ(*record.get_array_of_nullable_boolean("aob_init"),
              nullable_expecteds<bool>());
    EXPECT_EQ(*record.get_array_of_nullable_int8("aoi8_vec"),
              nullable_expecteds<int8_t>());
    EXPECT_EQ(*record.get_array_of_nullable_int8("aoi8_init"),
              nullable_expecteds<int8_t>());
    EXPECT_EQ(*record.get_array_of_nullable_int16("aoi16_vec"),
              nullable_expecteds<int16_t>());
    EXPECT_EQ(*record.get_array_of_nullable_int16("aoi16_init"),
              nullable_expecteds<int16_t>());
    EXPECT_EQ(*record.get_array_of_nullable_int32("aoi32_vec"),
              nullable_expecteds<int32_t>());
    EXPECT_EQ(*record.get_array_of_nullable_int32("aoi32_init"),
              nullable_expecteds<int32_t>());
    EXPECT_EQ(*record.get_array_of_nullable_int64("aoi64_vec"),
              nullable_expecteds<int64_t>());
    EXPECT_EQ(*record.get_array_of_nullable_int64("aoi64_init"),
              nullable_expecteds<int64_t>());
    EXPECT_EQ(*record.get_array_of_nullable_float32("aof32_vec"),
              nullable_expecteds<float>());
    EXPECT_EQ(*record.get_array_of_nullable_float32("aof32_init"),
              nullable_expecteds<float>());
    EXPECT_EQ(*record.get_array_of_nullable_float64("aof64_vec"),
              nullable_expecteds<double>());
    EXPECT_EQ(*record.get_array_of_nullable_float64("aof64_init"),
              nullable_expecteds<double>());
}

} // namespace compact
} // namespace test
} // namespace client
} // namespace hazelcast