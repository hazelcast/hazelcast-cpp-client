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

#include "hazelcast/client/serialization/serialization.h"
#include "inner_dto.h"

namespace hazelcast{
namespace client {
namespace test {
namespace compact {

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

}
}

namespace serialization {

template<>
struct hz_serializer<test::compact::main_dto> : public compact_serializer
{
    static void write(const test::compact::main_dto& object,
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
        writer.write_compact<test::compact::inner_dto>("p", object.p);
        writer.write_nullable_boolean("nullableBool", object.nullableBool);
        writer.write_nullable_int8("nullableB", object.nullableB);
        writer.write_nullable_int16("nullableS", object.nullableS);
        writer.write_nullable_int32("nullableI", object.nullableI);
        writer.write_nullable_int64("nullableL", object.nullableL);
        writer.write_nullable_float32("nullableF", object.nullableF);
        writer.write_nullable_float64("nullableD", object.nullableD);
    }

    static test::compact::main_dto read(compact_reader& reader)
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
        auto p = reader.read_compact<test::compact::inner_dto>("p");
        auto nullableBool = reader.read_nullable_boolean("nullableBool");
        auto nullableB = reader.read_nullable_int8("nullableB");
        auto nullableS = reader.read_nullable_int16("nullableS");
        auto nullableI = reader.read_nullable_int32("nullableI");
        auto nullableL = reader.read_nullable_int64("nullableL");
        auto nullableF = reader.read_nullable_float32("nullableF");
        auto nullableD = reader.read_nullable_float64("nullableD");

        return test::compact::main_dto{ boolean,
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

}
}
}