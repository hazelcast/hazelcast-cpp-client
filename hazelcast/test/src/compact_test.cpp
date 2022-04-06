/*
 * Copyright (c) 2008-2021, Hazelcast, Inc. All Rights Reserved.
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
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#ifdef HZ_BUILD_WITH_SSL
#include <openssl/crypto.h>
#endif

#include <hazelcast/client/serialization/serialization.h>

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4996) // for unsafe getenv
#endif
using namespace hazelcast::client::serialization::pimpl;
namespace hazelcast {
namespace client {
namespace compact {
namespace test {

struct node_dto
{
    int id;
    std::shared_ptr<node_dto> child;
};

bool
operator==(const node_dto& lhs, const node_dto& rhs)
{
    return lhs.id == rhs.id && lhs.child == rhs.child;
}

struct inner_dto
{
    std::string str;
};

bool
operator==(const inner_dto& lhs, const inner_dto& rhs)
{
    return lhs.str == rhs.str;
}
struct main_dto
{
    int i;
    boost::optional<inner_dto> p;
    std::string str;
};

bool
operator==(const main_dto& lhs, const main_dto& rhs)
{
    return lhs.i == rhs.i && lhs.p == rhs.p && lhs.str == rhs.str;
}

main_dto
create_main_dto()
{
    inner_dto p{ "Johny" };
    return main_dto{ 30, p, "John" };
}

std::ostream&
operator<<(std::ostream& out, const main_dto& main_dto)
{
    out << "i " << main_dto.i << ", str " << main_dto.str;
    return out;
}

std::ostream&
operator<<(std::ostream& out, const inner_dto& inner_dto)
{
    out << "str " << inner_dto.str;
    return out;
}

std::ostream&
operator<<(std::ostream& out, const node_dto& node_dto)
{
    out << "id " << node_dto.id;
    if (node_dto.child == nullptr) {
        out << ", child null";
    } else {
        out << ", child " << *node_dto.child;
    }
    return out;
}

/**
 * This class is to simulate versioning.
 * We will provide this struct with serializer returning type name of the
 * original main dto. This way we can use the serialized data of this class
 * to test to_object of the original main_dto.
 */
struct empty_main_dto
{};

} // namespace test
} // namespace compact

namespace serialization {

template<>
struct hz_serializer<compact::test::inner_dto> : public compact_serializer
{
    static void write(const compact::test::inner_dto& object,
                      compact_writer& writer)
    {
        writer.write_string("name", object.str);
    }

    static compact::test::inner_dto read(compact_reader& reader)
    {
        auto str = reader.read_string("name");
        return compact::test::inner_dto{ str.value() };
    }

    static std::string type_name() { return "main"; }
};

template<>
struct hz_serializer<compact::test::main_dto> : public compact_serializer
{
    static void write(const compact::test::main_dto& object,
                      compact_writer& writer)
    {
        writer.write_int32("i", object.i);
        writer.write_compact<compact::test::inner_dto>("p", object.p);
        writer.write_string("name", object.str);
    }

    static compact::test::main_dto read(compact_reader& reader)
    {
        auto i = reader.read_int32("i", 1);
        auto p =
          reader.read_compact<compact::test::inner_dto>("p", boost::none);
        auto str =
          reader.read_string("name", boost::make_optional<std::string>("NA"));
        return compact::test::main_dto{ i, p, *str };
    }

    static std::string type_name() { return "main"; }
};

template<>
struct hz_serializer<compact::test::node_dto> : public compact_serializer
{
    static void write(const compact::test::node_dto& object,
                      compact_writer& writer)
    {
        writer.write_int32("id", object.id);
        writer.write_compact<compact::test::node_dto>(
          "child",
          object.child == nullptr ? boost::none
                                  : boost::make_optional(*object.child));
    }

    static compact::test::node_dto read(compact_reader& reader)
    {
        auto id = reader.read_int32("id", 1);
        auto&& child = reader.read_compact<compact::test::node_dto>("child");
        return compact::test::node_dto{
            id,
            child.has_value()
              ? std::make_shared<compact::test::node_dto>(child.value())
              : nullptr
        };
    }

    static std::string type_name() { return "node"; }
};

template<>
struct hz_serializer<compact::test::empty_main_dto> : public compact_serializer
{
    static void write(const compact::test::empty_main_dto& object,
                      compact_writer& writer)
    {}

    static compact::test::empty_main_dto read(compact_reader& reader)
    {
        return compact::test::empty_main_dto{};
    }

    static std::string type_name() { return "main"; }
};

} // namespace serialization

namespace compact {
namespace test {
class CompactSerializationTest : public ::testing::Test
{
public:
    template<typename T>
    T to_data_and_back_to_object(SerializationService& ss, T& value)
    {
        data data = ss.to_data<T>(value);
        return *(ss.to_object<T>(data));
    }
};

TEST_F(CompactSerializationTest, testAllTypes)
{
    serialization_config config;
    SerializationService ss(config);

    main_dto expected = create_main_dto();
    auto actual = to_data_and_back_to_object(ss, expected);
    ASSERT_EQ(expected, actual);
}

TEST_F(CompactSerializationTest, testRecursive)
{
    serialization_config config;
    SerializationService ss(config);

    auto n2 = std::make_shared<node_dto>(node_dto{ 2, nullptr });
    auto n1 = std::make_shared<node_dto>(node_dto{ 1, n2 });
    node_dto expected{ 0, n1 };
    auto actual = to_data_and_back_to_object(ss, expected);
    ASSERT_EQ(expected, actual);
}

TEST_F(CompactSerializationTest,
       testReaderReturnsDefaultValues_whenDataIsMissing)
{
    serialization_config config;
    SerializationService ss(config);

    empty_main_dto empty;
    data data = ss.to_data(empty);
    main_dto actual = *(ss.to_object<main_dto>(data));
    ASSERT_EQ(1, actual.i);
    ASSERT_EQ(boost::none, actual.p);
    ASSERT_EQ("NA", actual.str);
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

TEST_F(CompactSerializationTest, test_schema_field_order)
{
    schema_writer schema_writer("typename");
    schema_writer.add_field("int2", field_kind::INT32);
    schema_writer.add_field("int1", field_kind::INT32);
    schema_writer.add_field("string1", field_kind::STRING);
    schema_writer.add_field("string2", field_kind::STRING);
    auto schema = std::move(schema_writer).build();

    check_schema_field(schema, "int1", 0, -1, -1);
    check_schema_field(schema, "int2", 4, -1, -1);
    check_schema_field(schema, "string1", -1, 0, -1);
    check_schema_field(schema, "string2", -1, 1, -1);
}

TEST_F(CompactSerializationTest, test_schema_writer_counts)
{
    schema_writer schema_writer("typename");
    schema_writer.add_field("int1", field_kind::INT32);
    schema_writer.add_field("int2", field_kind::INT32);
    schema_writer.add_field("string1", field_kind::STRING);
    auto schema = std::move(schema_writer).build();

    ASSERT_EQ(8, schema.fixed_size_fields_length());
    ASSERT_EQ(1, schema.number_of_var_size_fields());
}

TEST_F(CompactSerializationTest, test_rabin_fingerprint_consistent_with_server)
{
    //    // TODO sancar rewrite as following when all types are implemented
    //    schema_writer schema_writer("typename");
    //    serialization::compact_writer writer =
    //    create_compact_writer(&schema_writer);
    //    serialization::hz_serializer<main_dto>::write(create_main_dto(),
    //    writer); auto schema = std::move(schema_writer).build();
    //    This magic number is generated via Java code for exact same class.
    //    ASSERT_EQ(814479248787788739L, schema.schema_id());

    schema_writer schema_writer("typeName");
    schema_writer.add_field("a", field_kind::BOOLEAN);
    schema_writer.add_field("b", field_kind::ARRAY_OF_BOOLEAN);
    schema_writer.add_field("c", field_kind::TIMESTAMP_WITH_TIMEZONE);
    auto schema = std::move(schema_writer).build();
    // This magic number is generated via Java code for exact same class.
    ASSERT_EQ(-2132873845851116364, schema.schema_id());
}

} // namespace test
} // namespace compact
} // namespace client
} // namespace hazelcast