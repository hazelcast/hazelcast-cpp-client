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
#include <tuple>
#include <limits>

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

/**
 * It contains 9 boolean fields.
 * So it will occupy to 9 bits when it is serialized.
 * 9 bits mean 2 bytes
 * So 'i' was added for this reason to accomodate more than a byte
 * to enlarge to scope of the test.
*/
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
    bool i = false;
    int id = 0;
    boost::optional<std::vector<bool>> booleans;
};

bool
operator==(const bits_dto& lhs, const bits_dto& rhs)
{
    return lhs.a == rhs.a && lhs.b == rhs.b && lhs.c == rhs.c &&
           lhs.d == rhs.d && lhs.e == rhs.e && lhs.f == rhs.f &&
           lhs.g == rhs.g && lhs.h == rhs.h && lhs.i == rhs.i &&
           lhs.id == rhs.id && lhs.booleans == rhs.booleans;
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
    boost::optional<std::vector<boost::optional<big_decimal>>> bigDecimals;
    boost::optional<std::vector<boost::optional<local_time>>> localTimes;
    boost::optional<std::vector<boost::optional<local_date>>> localDates;
    boost::optional<std::vector<boost::optional<local_date_time>>>
      localDateTimes;
    boost::optional<std::vector<boost::optional<offset_date_time>>>
      offsetDateTimes;
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
           lhs.nn == rhs.nn && lhs.bigDecimals == rhs.bigDecimals &&
           lhs.localTimes == rhs.localTimes &&
           lhs.localDates == rhs.localDates &&
           lhs.localDateTimes == rhs.localDateTimes &&
           lhs.offsetDateTimes == rhs.offsetDateTimes &&
           lhs.nullableBools == rhs.nullableBools &&
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
    boost::optional<hazelcast::client::local_time> localTime;
    boost::optional<hazelcast::client::local_date> localDate;
    boost::optional<hazelcast::client::local_date_time> localDateTime;
    boost::optional<hazelcast::client::offset_date_time> offsetDateTime;
    boost::optional<bool> nullableBool;
    boost::optional<int8_t> nullableB;
    boost::optional<int16_t> nullableS;
    boost::optional<int32_t> nullableI;
    boost::optional<int64_t> nullableL;
    boost::optional<float> nullableF;
    boost::optional<double> nullableD;
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
           lhs.nullableBool == rhs.nullableBool &&
           lhs.nullableB == rhs.nullableB && lhs.nullableS == rhs.nullableS &&
           lhs.nullableI == rhs.nullableI && lhs.nullableL == rhs.nullableL &&
           lhs.nullableF == rhs.nullableF && lhs.nullableD == rhs.nullableD;
}

struct wrong_field_name_read_obj
{
    int value;
};

struct type_mistmatch_obj
{
    int value;
};

hazelcast::client::local_time
current_time()
{
    std::time_t t = std::time(nullptr); // get time now
    std::tm* now = std::localtime(&t);
    return hazelcast::client::local_time{ static_cast<uint8_t>(now->tm_hour),
                                          static_cast<uint8_t>(now->tm_min),
                                          static_cast<uint8_t>(now->tm_sec),
                                          0 };
}

hazelcast::client::local_date
current_date()
{
    std::time_t t = std::time(nullptr); // get time now
    std::tm* now = std::localtime(&t);
    return hazelcast::client::local_date{ now->tm_year + 1900,
                                          static_cast<uint8_t>(now->tm_mon),
                                          static_cast<uint8_t>(now->tm_mday) };
}

hazelcast::client::local_date_time
current_timestamp()
{
    return hazelcast::client::local_date_time{ current_date(), current_time() };
}

hazelcast::client::offset_date_time
current_timestamp_with_timezone()
{
    return hazelcast::client::offset_date_time{ current_timestamp(), 3600 };
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
          { boost::make_optional(
              named_dto{ boost::make_optional<std::string>("test"), 1 }),
            boost::none }),
        boost::make_optional<
          std::vector<boost::optional<hazelcast::client::big_decimal>>>(
          { boost::make_optional(hazelcast::client::big_decimal{
              boost::multiprecision::cpp_int{ "12345" }, 0 }),
            boost::make_optional(hazelcast::client::big_decimal{
              boost::multiprecision::cpp_int{ "123456" }, 0 }) }),
        boost::make_optional<
          std::vector<boost::optional<hazelcast::client::local_time>>>(
          { boost::make_optional(current_time()),
            boost::none,
            boost::make_optional(current_time()) }),
        boost::make_optional<
          std::vector<boost::optional<hazelcast::client::local_date>>>(
          { boost::make_optional(current_date()),
            boost::none,
            boost::make_optional(current_date()) }),
        boost::make_optional<
          std::vector<boost::optional<hazelcast::client::local_date_time>>>(
          { boost::make_optional(current_timestamp()), boost::none }),
        boost::make_optional<
          std::vector<boost::optional<hazelcast::client::offset_date_time>>>(
          { boost::make_optional(current_timestamp_with_timezone()) }),
        boost::make_optional<std::vector<boost::optional<bool>>>(
          { boost::make_optional(true),
            boost::make_optional(false),
            boost::none }),
        boost::make_optional<std::vector<boost::optional<int8_t>>>(
          { boost::make_optional<int8_t>(0),
            boost::make_optional<int8_t>(1),
            boost::make_optional<int8_t>(2),
            boost::none }),
        boost::make_optional<std::vector<boost::optional<int16_t>>>(
          { boost::make_optional<int16_t>(3),
            boost::make_optional<int16_t>(4),
            boost::make_optional<int16_t>(5),
            boost::none }),
        boost::make_optional<std::vector<boost::optional<int32_t>>>(
          { boost::make_optional<int32_t>(9),
            boost::make_optional<int32_t>(8),
            boost::make_optional<int32_t>(7),
            boost::make_optional<int32_t>(6),
            boost::none }),
        boost::make_optional<std::vector<boost::optional<int64_t>>>(
          { boost::make_optional<int64_t>(0),
            boost::make_optional<int64_t>(1),
            boost::make_optional<int64_t>(5),
            boost::make_optional<int64_t>(7),
            boost::none }),
        boost::make_optional<std::vector<boost::optional<float>>>(
          { boost::make_optional(0.6543f),
            boost::make_optional(-3.56f),
            boost::make_optional(45.67f),
            boost::none }),
        boost::make_optional<std::vector<boost::optional<double>>>(
          { boost::make_optional(456.456),
            boost::make_optional(789.789),
            boost::make_optional(321.321),
            boost::none }),
    };
}
main_dto
create_main_dto()
{
    inner_dto p = create_inner_dto();
    return main_dto{ true,
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
                     hazelcast::client::local_time{ 1, 2, 3, 4 },
                     hazelcast::client::local_date{ 2015, 12, 31 },
                     hazelcast::client::local_date_time{
                       hazelcast::client::local_date{ 2015, 12, 31 },
                       hazelcast::client::local_time{ 1, 2, 3, 4 } },
                     hazelcast::client::offset_date_time{
                       hazelcast::client::local_date_time{
                         hazelcast::client::local_date{ 2015, 12, 31 },
                         hazelcast::client::local_time{ 1, 2, 3, 4 } },
                       100 },
                     true,
                     113,
                     (short)-500,
                     56789,
                     -50992225L,
                     900.5678f,
                     -897543.3678909 };
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
        writer.write_boolean("i", dto.i);
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
        dto.i = reader.read_boolean("i");
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
        writer.write_array_of_decimal("bigDecimals", object.bigDecimals);
        writer.write_array_of_time("localTimes", object.localTimes);
        writer.write_array_of_date("localDates", object.localDates);
        writer.write_array_of_timestamp("localDateTimes",
                                        object.localDateTimes);
        writer.write_array_of_timestamp_with_timezone("offsetDateTimes",
                                                      object.offsetDateTimes);
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
        object.bigDecimals = reader.read_array_of_decimal("bigDecimals");
        object.localTimes = reader.read_array_of_time("localTimes");
        object.localDates = reader.read_array_of_date("localDates");
        object.localDateTimes =
          reader.read_array_of_timestamp("localDateTimes");
        object.offsetDateTimes =
          reader.read_array_of_timestamp_with_timezone("offsetDateTimes");
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
        writer.write_string("str", object.str);
        writer.write_decimal("bigDecimal", object.bigDecimal);
        writer.write_time("localTime", object.localTime);
        writer.write_date("localDate", object.localDate);
        writer.write_timestamp("localDateTime", object.localDateTime);
        writer.write_timestamp_with_timezone("offsetDateTime",
                                             object.offsetDateTime);
        writer.write_compact<compact::test::inner_dto>("p", object.p);
        writer.write_nullable_boolean("nullableBool", object.nullableBool);
        writer.write_nullable_int8("nullableB", object.nullableB);
        writer.write_nullable_int16("nullableS", object.nullableS);
        writer.write_nullable_int32("nullableI", object.nullableI);
        writer.write_nullable_int64("nullableL", object.nullableL);
        writer.write_nullable_float32("nullableF", object.nullableF);
        writer.write_nullable_float64("nullableD", object.nullableD);
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
        auto str = reader.read_string("str");
        auto bigDecimal = reader.read_decimal("bigDecimal");
        auto localTime = reader.read_time("localTime");
        auto localDate = reader.read_date("localDate");
        auto localDateTime = reader.read_timestamp("localDateTime");
        auto offsetDateTime =
          reader.read_timestamp_with_timezone("offsetDateTime");
        auto p = reader.read_compact<compact::test::inner_dto>("p");
        auto nullableBool = reader.read_nullable_boolean("nullableBool");
        auto nullableB = reader.read_nullable_int8("nullableB");
        auto nullableS = reader.read_nullable_int16("nullableS");
        auto nullableI = reader.read_nullable_int32("nullableI");
        auto nullableL = reader.read_nullable_int64("nullableL");
        auto nullableF = reader.read_nullable_float32("nullableF");
        auto nullableD = reader.read_nullable_float64("nullableD");
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
                                        nullableBool,
                                        nullableB,
                                        nullableS,
                                        nullableI,
                                        nullableL,
                                        nullableF,
                                        nullableD };
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
struct CompactRabinFingerprintTest : public ::testing::Test
{
    template<typename T>
    using entry_t = std::tuple<int64_t, T, int64_t>;

    template<typename T>
    void check_each(std::vector<entry_t<T>> entries)
    {
        for (const entry_t<T>& e : entries) {
            using namespace hazelcast::client::serialization::pimpl;

            auto fp_before = std::get<0>(e);
            auto value = std::get<1>(e);
            auto expected = std::get<2>(e);
            auto fp_after = rabin_finger_print::fingerprint64(fp_before, value);

            EXPECT_EQ(fp_after, expected);
        }
    }
};

TEST_F(CompactRabinFingerprintTest, test_i8_fingerprint)
{
    check_each(std::vector<entry_t<byte>>{
      // Before               Val   After(Expected)
      { 100, -5, -6165936963810616235 },
      { INT64_MIN, 0, 36028797018963968 },
      { 9223372036854775807, 113, -3588673659009074035 },
      { -13, -13, 72057594037927935 },
      { 42, 42, 0 },
      { 42, -42, -1212835703325587522 },
      { 0, 0, 0 },
      { -123456789, 0, 7049212178818848951 },
      { 123456789, 127, -8322440716502314713 },
      { 127, -128, -7333697815154264656 },
    });
}

TEST_F(CompactRabinFingerprintTest, test_i32_fingerprint)
{
    check_each(std::vector<entry_t<int>>{
      // Before               Val            After(Expected)
      { INT64_MIN, 2147483647, 6066553457199370002 },
      { 9223372036854775807, INT32_MIN, 6066553459773452525 },
      { 9223372036854707, 42, -961937498224213201 },
      { -42, -42, 4294967295 },
      { 42, 42, 0 },
      { 42, -442, 7797744281030715531 },
      { 0, 0, 0 },
      { -123456789, 0, -565582369564281851 },
      { 123456786669, 42127, 7157681543413310373 },
      { 2147483647, INT32_MIN, -7679311364898232185 } });
}

TEST_F(CompactRabinFingerprintTest, test_str_fingerprint)
{
    check_each(std::vector<entry_t<std::string>>{
      { 0, "hazelcast", 8164249978089638648 },
      { -31231241235, "üğişçö", 6128923854942458838 },
      { 41231542121235, "😀 😃 😄", -6875080751809013377 },
      { rabin_finger_print::INIT, "STUdent", 1896492170246289820 },
      { rabin_finger_print::INIT, "aü😄", -2084249746924383631 },
      { rabin_finger_print::INIT, "", -2316162475121075004 },
      { -123321, "xyz", 2601391163390439688 },
      { 132132123132132, "    ç", -7699875372487088773 },
      { 42, "42", 7764866287864698590 },
      { -42, "-42", -3434092993477103253 } });
}

// hazelcast.internal.serialization.impl.compact.RabinFingerPrintTest::testRabinFingerprint()
TEST_F(CompactRabinFingerprintTest, test_schema)
{
    using hazelcast::client::serialization::compact_writer;
    using hazelcast::client::serialization::pimpl::field_kind;
    using hazelcast::client::serialization::pimpl::schema_writer;

    schema_writer s_writer{ "SomeType" };
    auto writer =
      hazelcast::client::serialization::pimpl::create_compact_writer(&s_writer);

    writer.write_int32("id", 0);
    writer.write_string("name", boost::none);
    writer.write_int8("age", 0);
    writer.write_array_of_timestamp("times", boost::none);

    auto schema_id = std::move(s_writer).build().schema_id();
    ASSERT_EQ(-5445839760245891300, schema_id);
}

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
    const auto& schema = schema_of<main_dto>::schema_v;
    // This magic number is generated via Java code for exact same class.
    ASSERT_EQ(814479248787788739L, schema.schema_id());
}

TEST_F(CompactSerializationTest, test_read_when_field_does_not_exist)
{
    using hazelcast::client::compact::test::wrong_field_name_read_obj;

    serialization_config config;
    SerializationService ss(config);

    wrong_field_name_read_obj obj;

    auto data = ss.to_data(obj);
    ASSERT_THROW(ss.to_object<wrong_field_name_read_obj>(data),
                 exception::hazelcast_serialization);
}

TEST_F(CompactSerializationTest, test_read_with_type_mismatch)
{
    using hazelcast::client::compact::test::type_mistmatch_obj;

    serialization_config config;
    SerializationService ss(config);
    type_mistmatch_obj obj;

    auto data = ss.to_data(obj);
    ASSERT_THROW(ss.to_object<type_mistmatch_obj>(data),
                 exception::hazelcast_serialization);
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
struct hz_serializer<compact::test::primitive_object> : compact_serializer
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
  : compact_serializer
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

template<>
struct hz_serializer<compact::test::wrong_field_name_read_obj>
  : compact_serializer
{
    static void write(const compact::test::wrong_field_name_read_obj& obj,
                      compact_writer& writer)
    {
        writer.write_int32("field_1", obj.value);
    }

    static compact::test::wrong_field_name_read_obj read(compact_reader& reader)
    {
        compact::test::wrong_field_name_read_obj obj;

        obj.value = reader.read_int32("wrong_field");

        return obj;
    }

    static std::string type_name() { return "wrong_field_name_read_obj"; }
};

template<>
struct hz_serializer<compact::test::type_mistmatch_obj> : compact_serializer
{
    static void write(const compact::test::type_mistmatch_obj& obj,
                      compact_writer& writer)
    {
        writer.write_int32("field_1", obj.value);
    }

    static compact::test::type_mistmatch_obj read(compact_reader& reader)
    {
        compact::test::type_mistmatch_obj obj;

        obj.value = static_cast<int>(reader.read_float32("field_1"));

        return obj;
    }

    static std::string type_name() { return "type_mistmatch_obj"; }
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