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
#include <hazelcast/client/big_decimal.h>

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
    boost::optional<std::vector<bool>> booleans;
};

bool
operator==(const bits_dto& lhs, const bits_dto& rhs)
{
    return lhs.a == rhs.a && lhs.b == rhs.b && lhs.c == rhs.c &&
           lhs.d == rhs.d && lhs.e == rhs.e && lhs.f == rhs.f &&
           lhs.g == rhs.g && lhs.h == rhs.h && lhs.id == rhs.id &&
           lhs.booleans == rhs.booleans;
}

struct node_dto
{
    int id;
    std::shared_ptr<node_dto> child;
};

bool
operator==(const node_dto& lhs, const node_dto& rhs)
{
    return lhs.id == rhs.id &&
           (lhs.child == rhs.child ||
            (lhs.child && rhs.child && *lhs.child == *rhs.child));
}

struct named_dto
{
    boost::optional<std::string> name;
    int my_int;
};

bool
operator==(const named_dto& lhs, const named_dto& rhs)
{
    return lhs.name == rhs.name && lhs.my_int == rhs.my_int;
}

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
           lhs.nn == rhs.nn && lhs.nullableBools == rhs.nullableBools &&
           lhs.nullableBytes == rhs.nullableBytes &&
           lhs.nullableShorts == rhs.nullableShorts &&
           lhs.nullableInts == rhs.nullableInts &&
           lhs.nullableLongs == rhs.nullableLongs &&
           lhs.nullableFloats == rhs.nullableFloats &&
           lhs.nullableDoubles == rhs.nullableDoubles;
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
    std::string str;
    boost::optional<inner_dto> p;
    boost::optional<hazelcast::client::big_decimal> bigDecimal;
    boost::optional<boost::posix_time::time_duration> localTime;
    boost::optional<boost::gregorian::date> localDate;
    boost::optional<boost::posix_time::ptime> localDateTime;
    boost::optional<boost::local_time::local_date_time> offsetDateTime;
    boost::optional<bool> nullable_bool;
    boost::optional<int8_t> nullable_b;
    boost::optional<int16_t> nullable_s;
    boost::optional<int32_t> nullable_i;
    boost::optional<int64_t> nullable_l;
    boost::optional<float> nullable_f;
    boost::optional<double> nullable_d;
};

bool
operator==(const main_dto& lhs, const main_dto& rhs)
{
    return lhs.boolean == rhs.boolean && lhs.b == rhs.b && lhs.s == rhs.s &&
           lhs.i == rhs.i && lhs.l == rhs.l && lhs.f == rhs.f &&
           lhs.d == rhs.d && lhs.str == rhs.str && lhs.p == rhs.p &&
           lhs.bigDecimal == rhs.bigDecimal && lhs.localTime == rhs.localTime &&
           lhs.localDate == rhs.localDate &&
           lhs.localDateTime == rhs.localDateTime &&
           lhs.offsetDateTime == rhs.offsetDateTime &&
           lhs.nullable_bool == rhs.nullable_bool &&
           lhs.nullable_b == rhs.nullable_b &&
           lhs.nullable_s == rhs.nullable_s &&
           lhs.nullable_i == rhs.nullable_i &&
           lhs.nullable_l == rhs.nullable_l &&
           lhs.nullable_f == rhs.nullable_f && lhs.nullable_d == rhs.nullable_d;
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
          { boost::make_optional<named_dto>(
              named_dto{ boost::make_optional<std::string>("test"), 1 }),
            boost::none })
    };
}
main_dto
create_main_dto()
{
    inner_dto p = create_inner_dto();
    return main_dto{
        true,
        113,
        -500,
        56789,
        -50992225L,
        900.5678f,
        -897543.3678909,
        "this is main object created for testing!",
        p,
        hazelcast::client::big_decimal{
          boost::multiprecision::cpp_int{ "12312313" }, 0 },
        boost::posix_time::time_duration(1, 2, 3, 4),
        boost::gregorian::date(2015, 12, 31),
        boost::posix_time::ptime(boost::gregorian::date(2015, 12, 31),
                                 boost::posix_time::time_duration(1, 2, 3, 4)),
        boost::local_time::local_date_time(
          boost::posix_time::ptime(
            boost::gregorian::date(2015, 12, 31),
            boost::posix_time::time_duration(1, 2, 3, 4)),
          boost::local_time::time_zone_ptr(
            new boost::local_time::posix_time_zone("UTC-10:21:13"))),
        true,
        113,
        (short)-500,
        56789,
        -50992225L,
        900.5678f,
        -897543.3678909
    };
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
        writer.write_array_of_boolean("booleans", dto.booleans);
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
        dto.booleans = reader.read_array_of_boolean("booleans");
        return dto;
    }

    static std::string type_name() { return "bits_dto"; }
};

template<>
struct hz_serializer<compact::test::named_dto> : public compact_serializer
{
    static void write(const compact::test::named_dto& dto,
                      compact_writer& writer)
    {
        writer.write_string("name", dto.name);
        writer.write_int32("my_int", dto.my_int);
    }

    static compact::test::named_dto read(compact_reader& reader)
    {
        compact::test::named_dto dto;
        dto.name = reader.read_string("name");
        dto.my_int = reader.read_int32("my_int");
        return dto;
    }

    static std::string type_name() { return "named_dto"; }
};

template<>
struct hz_serializer<compact::test::inner_dto> : public compact_serializer
{
    static void write(const compact::test::inner_dto& object,
                      compact_writer& writer)
    {
        writer.write_array_of_boolean("bools", object.bools);
        writer.write_array_of_int8("bytes", object.bytes);
        writer.write_array_of_int16("shorts", object.shorts);
        writer.write_array_of_int32("ints", object.ints);
        writer.write_array_of_int64("longs", object.longs);
        writer.write_array_of_float32("floats", object.floats);
        writer.write_array_of_float64("doubles", object.doubles);
        writer.write_array_of_string("strings", object.strings);
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

    static compact::test::inner_dto read(compact_reader& reader)
    {
        compact::test::inner_dto object;
        object.bools = reader.read_array_of_boolean("bools");
        object.bytes = reader.read_array_of_int8("bytes");
        object.shorts = reader.read_array_of_int16("shorts");
        object.ints = reader.read_array_of_int32("ints");
        object.longs = reader.read_array_of_int64("longs");
        object.floats = reader.read_array_of_float32("floats");
        object.doubles = reader.read_array_of_float64("doubles");
        object.strings = reader.read_array_of_string("strings");
        object.nn =
          reader.read_array_of_compact<compact::test::named_dto>("nn");
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
        writer.write_string("name", object.str);
        writer.write_decimal("bigDecimal", object.bigDecimal);
        writer.write_time("localTime", object.localTime);
        writer.write_date("localDate", object.localDate);
        writer.write_timestamp("localDateTime", object.localDateTime);
        writer.write_timestamp_with_timezone("offsetDateTime",
                                             object.offsetDateTime);
        writer.write_compact<compact::test::inner_dto>("p", object.p);
        writer.write_nullable_boolean("nullable_bool", object.nullable_bool);
        writer.write_nullable_int8("nullable_b", object.nullable_b);
        writer.write_nullable_int16("nullable_s", object.nullable_s);
        writer.write_nullable_int32("nullable_i", object.nullable_i);
        writer.write_nullable_int64("nullable_l", object.nullable_l);
        writer.write_nullable_float32("nullable_f", object.nullable_f);
        writer.write_nullable_float64("nullable_d", object.nullable_d);
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
        auto str = reader.read_string("name");
        auto bigDecimal = reader.read_decimal("bigDecimal");
        auto localTime = reader.read_time("localTime");
        auto localDate = reader.read_date("localDate");
        auto localDateTime = reader.read_timestamp("localDateTime");
        auto offsetDateTime =
          reader.read_timestamp_with_timezone("offsetDateTime");
        auto p = reader.read_compact<compact::test::inner_dto>("p");
        auto nullable_bool = reader.read_nullable_boolean("nullable_bool");
        auto nullable_b = reader.read_nullable_int8("nullable_b");
        auto nullable_s = reader.read_nullable_int16("nullable_s");
        auto nullable_i = reader.read_nullable_int32("nullable_i");
        auto nullable_l = reader.read_nullable_int64("nullable_l");
        auto nullable_f = reader.read_nullable_float32("nullable_f");
        auto nullable_d = reader.read_nullable_float64("nullable_d");
        return compact::test::main_dto{ boolean,
                                        b,
                                        s,
                                        i,
                                        l,
                                        f,
                                        d,
                                        *str,
                                        p,
                                        bigDecimal,
                                        localTime,
                                        localDate,
                                        localDateTime,
                                        offsetDateTime,
                                        nullable_bool,
                                        nullable_b,
                                        nullable_s,
                                        nullable_i,
                                        nullable_l,
                                        nullable_f,
                                        nullable_d };
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

    auto expected = create_main_dto();
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
    expected.booleans = boost::make_optional<std::vector<bool>>(
      { true, false, false, false, true, false, false, false });

    const data& data = ss.to_data(expected);
    // hash(4) + typeid(4) + schemaId(8) + (4 byte length) + (1 bytes for 8
    // bits) + (4 bytes for int) (4 byte length of byte array) + (1 byte for
    // booleans array of 8 bits) + (1 byte offset bytes)
    ASSERT_EQ(31, data.total_size());

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

struct primitive_object
{
    bool boolean_;
    int8_t byte_;
    int16_t short_;
    int32_t int_;
    int64_t long_;
    float float_;
    double double_;
    boost::optional<std::vector<bool>> booleans;
    boost::optional<std::vector<int8_t>> bytes;
    boost::optional<std::vector<int16_t>> shorts;
    boost::optional<std::vector<int32_t>> ints;
    boost::optional<std::vector<int64_t>> longs;
    boost::optional<std::vector<float>> floats;
    boost::optional<std::vector<double>> doubles;
};

struct nullable_primitive_object
{
    boost::optional<bool> nullableBoolean;
    boost::optional<int8_t> nullableByte;
    boost::optional<int16_t> nullableShort;
    boost::optional<int32_t> nullableInt;
    boost::optional<int64_t> nullableLong;
    boost::optional<float> nullableFloat;
    boost::optional<double> nullableDouble;
    boost::optional<std::vector<boost::optional<bool>>> nullableBooleans;
    boost::optional<std::vector<boost::optional<int8_t>>> nullableBytes;
    boost::optional<std::vector<boost::optional<int16_t>>> nullableShorts;
    boost::optional<std::vector<boost::optional<int32_t>>> nullableInts;
    boost::optional<std::vector<boost::optional<int64_t>>> nullableLongs;
    boost::optional<std::vector<boost::optional<float>>> nullableFloats;
    boost::optional<std::vector<boost::optional<double>>> nullableDoubles;
};
} // namespace test
} // namespace compact

namespace serialization {
template<>
struct hz_serializer<compact::test::primitive_object>
  : public compact_serializer
{
    static void write(const compact::test::primitive_object& object,
                      compact_writer& writer)
    {
        writer.write_boolean("boolean", object.boolean_);
        writer.write_int8("byte", object.byte_);
        writer.write_int16("short", object.short_);
        writer.write_int32("int", object.int_);
        writer.write_int64("long", object.long_);
        writer.write_float32("float", object.float_);
        writer.write_float64("double", object.double_);
        writer.write_array_of_boolean("booleans", object.booleans);
        writer.write_array_of_int8("bytes", object.bytes);
        writer.write_array_of_int16("shorts", object.shorts);
        writer.write_array_of_int32("ints", object.ints);
        writer.write_array_of_int64("longs", object.longs);
        writer.write_array_of_float32("floats", object.floats);
        writer.write_array_of_float64("doubles", object.doubles);
    }

    static compact::test::primitive_object read(compact_reader& reader)
    {
        compact::test::primitive_object object;
        object.boolean_ = reader.read_boolean("boolean");
        object.byte_ = reader.read_int8("byte");
        object.short_ = reader.read_int16("short");
        object.int_ = reader.read_int32("int");
        object.long_ = reader.read_int64("long");
        object.float_ = reader.read_float32("float");
        object.double_ = reader.read_float64("double");
        object.booleans = reader.read_array_of_boolean("booleans");
        object.bytes = reader.read_array_of_int8("bytes");
        object.shorts = reader.read_array_of_int16("shorts");
        object.ints = reader.read_array_of_int32("ints");
        object.longs = reader.read_array_of_int64("longs");
        object.floats = reader.read_array_of_float32("floats");
        object.doubles = reader.read_array_of_float64("doubles");
        return object;
    }

    static std::string type_name() { return "primitive_object"; }
};

template<>
struct hz_serializer<compact::test::nullable_primitive_object>
  : public compact_serializer
{
    static void write(const compact::test::nullable_primitive_object& object,
                      compact_writer& writer)
    {
        writer.write_nullable_boolean("boolean", object.nullableBoolean);
        writer.write_nullable_int8("byte", object.nullableByte);
        writer.write_nullable_int16("short", object.nullableShort);
        writer.write_nullable_int32("int", object.nullableInt);
        writer.write_nullable_int64("long", object.nullableLong);
        writer.write_nullable_float32("float", object.nullableFloat);
        writer.write_nullable_float64("double", object.nullableDouble);
        writer.write_array_of_nullable_boolean("booleans",
                                               object.nullableBooleans);
        writer.write_array_of_nullable_int8("bytes", object.nullableBytes);
        writer.write_array_of_nullable_int16("shorts", object.nullableShorts);
        writer.write_array_of_nullable_int32("ints", object.nullableInts);
        writer.write_array_of_nullable_int64("longs", object.nullableLongs);
        writer.write_array_of_nullable_float32("floats", object.nullableFloats);
        writer.write_array_of_nullable_float64("doubles",
                                               object.nullableDoubles);
    }

    static compact::test::nullable_primitive_object read(compact_reader& reader)
    {
        compact::test::nullable_primitive_object object;
        object.nullableBoolean = reader.read_nullable_boolean("boolean");
        object.nullableByte = reader.read_nullable_int8("byte");
        object.nullableShort = reader.read_nullable_int16("short");
        object.nullableInt = reader.read_nullable_int32("int");
        object.nullableLong = reader.read_nullable_int64("long");
        object.nullableFloat = reader.read_nullable_float32("float");
        object.nullableDouble = reader.read_nullable_float64("double");
        object.nullableBooleans =
          reader.read_array_of_nullable_boolean("booleans");
        object.nullableBytes = reader.read_array_of_nullable_int8("bytes");
        object.nullableShorts = reader.read_array_of_nullable_int16("shorts");
        object.nullableInts = reader.read_array_of_nullable_int32("ints");
        object.nullableLongs = reader.read_array_of_nullable_int64("longs");
        object.nullableFloats = reader.read_array_of_nullable_float32("floats");
        object.nullableDoubles =
          reader.read_array_of_nullable_float64("doubles");
        return object;
    }

    // typename is same as the primitive_object on purpose.
    // This is to simulate two different applications implementing the same
    // class with different serializers.
    static std::string type_name() { return "primitive_object"; }
};
} // namespace serialization
namespace compact {
namespace test {
class CompactNullablePrimitiveInteroperabilityTest : public ::testing::Test
{};

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
    serialization_config config;
    SerializationService ss(config);

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
    serialization_config config;
    SerializationService ss(config);

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
    serialization_config config;
    SerializationService ss(config);

    const data& data = ss.to_data(expected);
    ASSERT_THROW(ss.to_object<primitive_object>(data),
                 exception::hazelcast_serialization);
}
} // namespace test
} // namespace compact
} // namespace client
} // namespace hazelcast