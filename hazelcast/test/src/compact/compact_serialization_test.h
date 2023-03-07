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

#include "../TestHelperFunctions.h"

#include "compact_test_base.h"
#include "serialization/main_dto.h"
#include "serialization/node_dto.h"
#include "serialization/bits_dto.h"
#include "serialization/employee_dto.h"
#include "serialization/wrong_field_name_read_obj.h"
#include "serialization/type_mismatch_obj.h"
#include "serialization/write_to_field_twice.h"

namespace hazelcast {
namespace client {
namespace test {
namespace compact {

using namespace serialization::pimpl;

class CompactSerializationTest : public compact_test_base
{
public:
    SerializationService& serialization_service()
    {
        return spi::ClientContext{ client }.get_serialization_service();
    }

    template<typename T>
    T to_data_and_back_to_object(SerializationService& ss, T& value)
    {
        data data = ss.to_data<T>(value);
        return *(ss.to_object<T>(data));
    }
};

TEST_F(CompactSerializationTest, testAllTypes)
{
    auto expected = create_main_dto();
    auto actual = to_data_and_back_to_object(serialization_service(), expected);
    ASSERT_EQ(expected, actual);
}

TEST_F(CompactSerializationTest, testRecursive)
{
    auto n2 = std::make_shared<node_dto>(node_dto{ 2, nullptr });
    auto n1 = std::make_shared<node_dto>(node_dto{ 1, n2 });
    node_dto expected{ 0, n1 };
    auto actual = to_data_and_back_to_object(serialization_service(), expected);
    ASSERT_EQ(expected, actual);
}

TEST_F(CompactSerializationTest, testBits)
{
    auto& ss = serialization_service();

    bits_dto expected;
    expected.a = true;
    expected.b = true;
    expected.i = true;
    expected.id = 121;
    expected.booleans = boost::make_optional<std::vector<bool>>(
      { true, false, false, false, true, false, false, false });

    const data& data = ss.to_data(expected);
    // hash(4) + typeid(4) + schemaId(8) + (4 byte length) + (2 bytes for 9
    // bits) + (4 bytes for int) (4 byte length of byte array) + (1 byte for
    // booleans array of 8 bits) + (1 byte offset bytes)
    ASSERT_EQ(32, data.total_size());

    bits_dto actual = *(ss.to_object<bits_dto>(data));
    ASSERT_EQ(expected, actual);
}

void
check_schema_field(const schema& schema,
                   const std::string& field_name,
                   int offset,
                   int index,
                   int bit_offset)
{
    ASSERT_EQ(offset, schema.fields().at(field_name).offset);
    ASSERT_EQ(index, schema.fields().at(field_name).index);
    ASSERT_EQ(bit_offset, schema.fields().at(field_name).bit_offset);
}

TEST_F(CompactSerializationTest, test_field_order_fixed_size)
{
    schema_writer schema_writer("typeName");
    auto writer = serialization::pimpl::create_compact_writer(&schema_writer);
    serialization::hz_serializer<employee_dto>::write(employee_dto{}, writer);
    auto schema = std::move(schema_writer).build();

    check_schema_field(schema, "id", 0, -1, -1);
    check_schema_field(schema, "age", 8, -1, -1);
    check_schema_field(schema, "rank", 12, -1, -1);
    check_schema_field(schema, "isFired", 16, -1, 0);
    check_schema_field(schema, "isHired", 16, -1, 1);
}

TEST_F(CompactSerializationTest, test_schema_writer_counts)
{
    using serialization::field_kind;

    schema_writer schema_writer("typename");
    schema_writer.add_field("int1", field_kind::INT32);
    schema_writer.add_field("int2", field_kind::INT32);
    schema_writer.add_field("string1", field_kind::STRING);
    auto schema = std::move(schema_writer).build();

    ASSERT_EQ(8, schema.fixed_size_fields_length());
    ASSERT_EQ(1, schema.number_of_var_size_fields());
}

TEST_F(CompactSerializationTest, test_read_when_field_does_not_exist)
{
    auto& ss = serialization_service();

    wrong_field_name_read_obj obj;

    auto data = ss.to_data(obj);
    ASSERT_THROW(ss.to_object<wrong_field_name_read_obj>(data),
                 exception::hazelcast_serialization);
}

TEST_F(CompactSerializationTest, test_read_with_type_mismatch)
{
    auto& ss = serialization_service();
    type_mistmatch_obj obj;

    auto data = ss.to_data(obj);
    ASSERT_THROW(ss.to_object<type_mistmatch_obj>(data),
                 exception::hazelcast_serialization);
}

TEST_F(CompactSerializationTest, test_write_to_same_field_twice)
{
    auto& ss = serialization_service();
    write_to_field_twice obj;

    ASSERT_THROW(ss.to_data(obj), exception::hazelcast_serialization);
}

} // namespace compact
} // namespace test
} // namespace client
} // namespace hazelcast
