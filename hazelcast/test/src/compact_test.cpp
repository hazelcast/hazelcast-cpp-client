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

struct bits_dto
{
    bool a = false;
    bool b = false;
    bool c = false;
    bool d = false;
    bool e = false;
    bool f = false;
    bool g = false;
    bool h = false;
    int id = 0;
};

bool
operator==(const bits_dto& lhs, const bits_dto& rhs)
{
    return lhs.a == rhs.a && lhs.b == rhs.b && lhs.c == rhs.c &&
           lhs.d == rhs.d && lhs.e == rhs.e && lhs.f == rhs.f &&
           lhs.g == rhs.g && lhs.h == rhs.h && lhs.id == rhs.id;
}

struct node_dto
{
    int id;
    std::shared_ptr<node_dto> child;
};

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

bool
operator==(const node_dto& lhs, const node_dto& rhs)
{
    if (lhs.id != rhs.id)
        return false;
    if (lhs.child == rhs.child)
        return true;
    if (lhs.child == nullptr && rhs.child != nullptr)
        return false;
    if (lhs.child != nullptr && rhs.child == nullptr)
        return false;
    return *lhs.child == *rhs.child;
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

std::ostream&
operator<<(std::ostream& out, const inner_dto& inner_dto)
{
    out << "str " << inner_dto.str;
    return out;
}

struct main_dto
{
    bool boolean;
    int8_t b;
    int16_t s;
    int32_t i;
    int64_t l;
    float f;
    double d;
    boost::optional<inner_dto> p;
    std::string str;
};

bool
operator==(const main_dto& lhs, const main_dto& rhs)
{
    return lhs.b == rhs.b && lhs.boolean == rhs.boolean && lhs.s == rhs.s &&
           lhs.i == rhs.i && lhs.l == rhs.l && lhs.f == rhs.f &&
           lhs.d == rhs.d && lhs.p == rhs.p && lhs.str == rhs.str;
}

main_dto
create_main_dto()
{
    inner_dto p{ "Johny" };
    return main_dto{
        true, 113,   -500, 56789, -50992225L, 900.5678f, -897543.3678909,
        p,    "John"
    };
}

std::ostream&
operator<<(std::ostream& out, const main_dto& main_dto)
{
    out << "{" << main_dto.b << ", " << main_dto.boolean << ", " << main_dto.s
        << ", " << main_dto.i << ", " << main_dto.l << ", " << main_dto.f
        << ", " << main_dto.d << ", " << main_dto.p << ", " << main_dto.str
        << "}";
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

struct employee_dto
{
    int32_t age;
    int32_t rank;
    int64_t id;
    bool isHired;
    bool isFired;
};

bool
operator==(const employee_dto& lhs, const employee_dto& rhs)
{
    return lhs.age == rhs.age && lhs.rank == rhs.rank && lhs.id == rhs.id &&
           lhs.isHired == rhs.isHired && lhs.isFired == rhs.isFired;
}

std::ostream&
operator<<(std::ostream& out, const employee_dto& employee_dto)
{
    out << "{" << employee_dto.age << ", " << employee_dto.rank << ", "
        << employee_dto.id << ", " << employee_dto.isHired << ", "
        << employee_dto.isFired << "}";
    return out;
}

} // namespace test
} // namespace compact

namespace serialization {

template<>
struct hz_serializer<compact::test::bits_dto> : public compact_serializer
{
    static void write(const compact::test::bits_dto& dto,
                      compact_writer& writer)
    {
        writer.write_boolean("a", dto.a);
        writer.write_boolean("b", dto.b);
        writer.write_boolean("c", dto.c);
        writer.write_boolean("d", dto.d);
        writer.write_boolean("e", dto.e);
        writer.write_boolean("f", dto.f);
        writer.write_boolean("g", dto.g);
        writer.write_boolean("h", dto.h);
        writer.write_int32("id", dto.id);
    }

    static compact::test::bits_dto read(compact_reader& reader)
    {
        compact::test::bits_dto dto;
        dto.a = reader.read_boolean("a");
        dto.b = reader.read_boolean("b");
        dto.c = reader.read_boolean("c");
        dto.d = reader.read_boolean("d");
        dto.e = reader.read_boolean("e");
        dto.f = reader.read_boolean("f");
        dto.g = reader.read_boolean("g");
        dto.h = reader.read_boolean("h");
        dto.id = reader.read_int32("id");
        return dto;
    }

    static std::string type_name() { return "bits_dto"; }
};
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
        writer.write_boolean("bool", object.boolean);
        writer.write_int8("b", object.b);
        writer.write_int16("s", object.s);
        writer.write_int32("i", object.i);
        writer.write_int64("l", object.l);
        writer.write_float32("f", object.f);
        writer.write_float64("d", object.d);
        writer.write_compact<compact::test::inner_dto>("p", object.p);
        writer.write_string("name", object.str);
    }

    static compact::test::main_dto read(compact_reader& reader)
    {
        auto boolean = reader.read_boolean("bool");
        auto b = reader.read_int8("b");
        auto s = reader.read_int16("s");
        auto i = reader.read_int32("i");
        auto l = reader.read_int64("l");
        auto f = reader.read_float32("f");
        auto d = reader.read_float64("d");
        auto p = reader.read_compact<compact::test::inner_dto>("p");
        auto str = reader.read_string("name");
        return compact::test::main_dto{ boolean, b, s, i, l, f, d, p, *str };
    }

    static std::string type_name() { return "inner"; }
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
        auto id = reader.read_int32("id");
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

template<>
struct hz_serializer<compact::test::employee_dto> : public compact_serializer
{

    static void write(const compact::test::employee_dto& object,
                      compact_writer& writer)
    {
        writer.write_int32("age", object.age);
        writer.write_int32("rank", object.rank);
        writer.write_int64("id", object.id);
        writer.write_boolean("isHired", object.isHired);
        writer.write_boolean("isFired", object.isFired);
    }

    static compact::test::employee_dto read(compact_reader& reader)
    {
        auto age = reader.read_int32("age");
        auto rank = reader.read_int32("rank");
        auto id = reader.read_int64("id");
        auto isHired = reader.read_boolean("isHired");
        auto isFired = reader.read_boolean("isFired");
        return compact::test::employee_dto{ age, rank, id, isHired, isFired };
    }
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

TEST_F(CompactSerializationTest, testBits)
{
    serialization_config config;
    SerializationService ss(config);

    bits_dto expected;
    expected.a = true;
    expected.b = true;
    expected.id = 121;

    const data& data = ss.to_data(expected);
    // hash(4) + typeid(4) + schemaId(8) + (1 bytes for 8 bits)
    // + (4 bytes for int)
    ASSERT_EQ(21, data.total_size());

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