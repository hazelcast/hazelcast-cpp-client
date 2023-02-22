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

#include "compact_test_base.h"
#include "serialization/primitive_object.h"
#include "serialization/nullable_primitive_object.h"

namespace hazelcast {
namespace client {
namespace test {
namespace compact {

template<typename T>
void
ASSERT_SAME_CONTENT(
  const boost::optional<std::vector<boost::optional<T>>>& expected,
  const boost::optional<std::vector<T>>& actual)
{
    ASSERT_EQ(expected->size(), actual->size());
    for (size_t i = 0; i < expected->size(); ++i) {
        ASSERT_EQ(expected->at(i).value(), actual->at(i));
    }
}

template<typename T>
void
ASSERT_SAME_CONTENT(
  const boost::optional<std::vector<T>>& expected,
  const boost::optional<std::vector<boost::optional<T>>>& actual)
{
    ASSERT_EQ(expected->size(), actual->size());
    for (size_t i = 0; i < expected->size(); ++i) {
        ASSERT_EQ(expected->at(i), actual->at(i).value());
    }
}

class CompactNullablePrimitiveInteroperabilityTest : public compact_test_base
{
protected:
    SerializationService& serialization_service()
    {
        return spi::ClientContext{ client }.get_serialization_service();
    }
};

TEST_F(CompactNullablePrimitiveInteroperabilityTest,
       testWritePrimitiveReadNullable)
{
    primitive_object expected{
        true,
        2,
        4,
        8,
        4444,
        8321.321F,
        41231.32,
        boost::make_optional(std::vector<bool>{ true, false }),
        boost::make_optional(std::vector<int8_t>{ 1, 2 }),
        boost::make_optional(std::vector<int16_t>{ 1, 4 }),
        boost::make_optional(std::vector<int32_t>{ 1, 8 }),
        boost::make_optional(std::vector<int64_t>{ 1, 4444 }),
        boost::make_optional(std::vector<float>{ 1.0F, 8321.321F }),
        boost::make_optional(std::vector<double>{ 41231.32, 2 })
    };

    replicate_schema<primitive_object>();
    replicate_schema<nullable_primitive_object>();

    SerializationService& ss = serialization_service();

    const data& data = ss.to_data(expected);
    auto actual = ss.to_object<nullable_primitive_object>(data).value();
    ASSERT_EQ(expected.boolean_, actual.nullableBoolean.value());
    ASSERT_EQ(expected.byte_, actual.nullableByte.value());
    ASSERT_EQ(expected.short_, actual.nullableShort.value());
    ASSERT_EQ(expected.int_, actual.nullableInt.value());
    ASSERT_EQ(expected.long_, actual.nullableLong.value());
    ASSERT_EQ(expected.float_, actual.nullableFloat.value());
    ASSERT_EQ(expected.double_, actual.nullableDouble.value());
    ASSERT_SAME_CONTENT(expected.booleans, actual.nullableBooleans);
    ASSERT_SAME_CONTENT(expected.bytes, actual.nullableBytes);
    ASSERT_SAME_CONTENT(expected.shorts, actual.nullableShorts);
    ASSERT_SAME_CONTENT(expected.ints, actual.nullableInts);
    ASSERT_SAME_CONTENT(expected.longs, actual.nullableLongs);
    ASSERT_SAME_CONTENT(expected.floats, actual.nullableFloats);
    ASSERT_SAME_CONTENT(expected.doubles, actual.nullableDoubles);
}

TEST_F(CompactNullablePrimitiveInteroperabilityTest,
       testWriteNullableReadPrimitive)
{
    nullable_primitive_object expected{
        boost::make_optional<bool>(true),
        boost::make_optional<int8_t>(4),
        boost::make_optional<int16_t>(6),
        boost::make_optional<int32_t>(8),
        boost::make_optional<int64_t>(4444),
        boost::make_optional<float>(8321.321F),
        boost::make_optional<double>(41231.32),
        boost::make_optional<std::vector<boost::optional<bool>>>(
          std::vector<boost::optional<bool>>{
            boost::make_optional<bool>(true),
            boost::make_optional<bool>(false) }),
        boost::make_optional<std::vector<boost::optional<int8_t>>>(
          std::vector<boost::optional<int8_t>>{
            boost::make_optional<int8_t>(1), boost::make_optional<int8_t>(2) }),
        boost::make_optional<std::vector<boost::optional<int16_t>>>(
          std::vector<boost::optional<int16_t>>{
            boost::make_optional<int16_t>(1),
            boost::make_optional<int16_t>(4) }),
        boost::make_optional<std::vector<boost::optional<int32_t>>>(
          std::vector<boost::optional<int32_t>>{
            boost::make_optional<int32_t>(1),
            boost::make_optional<int32_t>(8) }),
        boost::make_optional<std::vector<boost::optional<int64_t>>>(
          std::vector<boost::optional<int64_t>>{
            boost::make_optional<int64_t>(1),
            boost::make_optional<int64_t>(4444) }),
        boost::make_optional<std::vector<boost::optional<float>>>(
          std::vector<boost::optional<float>>{
            boost::make_optional<float>(1.0F),
            boost::make_optional<float>(8321.321F) }),
        boost::make_optional<std::vector<boost::optional<double>>>(
          std::vector<boost::optional<double>>{
            boost::make_optional<double>(41231.32),
            boost::make_optional<double>(2) })
    };

    replicate_schema<primitive_object>();
    replicate_schema<nullable_primitive_object>();

    SerializationService& ss = serialization_service();

    const data& data = ss.to_data(expected);
    auto actual = ss.to_object<primitive_object>(data).value();
    ASSERT_EQ(expected.nullableBoolean.value(), actual.boolean_);
    ASSERT_EQ(expected.nullableByte.value(), actual.byte_);
    ASSERT_EQ(expected.nullableShort.value(), actual.short_);
    ASSERT_EQ(expected.nullableInt.value(), actual.int_);
    ASSERT_EQ(expected.nullableLong.value(), actual.long_);
    ASSERT_EQ(expected.nullableFloat.value(), actual.float_);
    ASSERT_EQ(expected.nullableDouble.value(), actual.double_);
    ASSERT_SAME_CONTENT(expected.nullableBooleans, actual.booleans);
    ASSERT_SAME_CONTENT(expected.nullableBytes, actual.bytes);
    ASSERT_SAME_CONTENT(expected.nullableShorts, actual.shorts);
    ASSERT_SAME_CONTENT(expected.nullableInts, actual.ints);
    ASSERT_SAME_CONTENT(expected.nullableLongs, actual.longs);
    ASSERT_SAME_CONTENT(expected.nullableFloats, actual.floats);
    ASSERT_SAME_CONTENT(expected.nullableDoubles, actual.doubles);
}

TEST_F(CompactNullablePrimitiveInteroperabilityTest,
       testWriteNullReadPrimitiveThrowsException)
{
    nullable_primitive_object expected;

    replicate_schema<primitive_object>();
    replicate_schema<nullable_primitive_object>();

    SerializationService& ss = serialization_service();

    const data& data = ss.to_data(expected);
    ASSERT_THROW(ss.to_object<primitive_object>(data),
                 exception::hazelcast_serialization);
}

} // namespace compact
} // namespace test
} // namespace client
} // namespace hazelcast
