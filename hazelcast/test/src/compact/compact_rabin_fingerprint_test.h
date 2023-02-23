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

#include <gtest/gtest.h>

namespace hazelcast {
namespace client {
namespace test {
namespace compact {

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
    using hazelcast::client::serialization::pimpl::schema_writer;

    schema_writer s_writer{ "SomeType" };
    auto writer =
      hazelcast::client::serialization::pimpl::create_compact_writer(&s_writer);

    writer.write_int32("id", 0);
    writer.write_string("name", boost::none);
    writer.write_int8("age", 0);
    writer.write_array_of_timestamp("times", boost::none);

    auto schema_id = std::move(s_writer).build().schema_id();
    ASSERT_EQ(3662264393229655598, schema_id);
}

} // namespace compact
} // namespace test
} // namespace client
} // namespace hazelcast