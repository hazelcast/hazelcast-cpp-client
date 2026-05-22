/*
 * Copyright (c) 2008-2026, Hazelcast, Inc. All Rights Reserved.
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

#include <sstream>
#include <vector>
#include <string>

#include <gtest/gtest.h>

#include <boost/format.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include "hazelcast/client/serialization/serialization.h"
#include "hazelcast/client/serialization/pimpl/compact/schema.h"
#include "hazelcast/client/spi/ClientContext.h"

#include "compact_test_base.h"
#include "../TestHelperFunctions.h"

namespace hazelcast {
namespace client {
namespace test {
namespace compact {

// A compact-serializable type whose fixed-size fields include several pairs
// of fields sharing the same kind/size, with names whose insertion order is
// intentionally non-alphabetical. Used to exercise the stable_sort behavior
// in schema construction.
struct same_size_record
{
    int32_t i32_a = 0;
    int32_t i32_b = 0;
    int32_t i32_c = 0;
    int64_t i64_a = 0;
    int64_t i64_b = 0;
    int16_t i16_a = 0;
    int16_t i16_b = 0;
    int8_t i8_a = 0;
    int8_t i8_b = 0;
};

inline bool
operator==(const same_size_record& lhs, const same_size_record& rhs)
{
    return lhs.i32_a == rhs.i32_a && lhs.i32_b == rhs.i32_b &&
           lhs.i32_c == rhs.i32_c && lhs.i64_a == rhs.i64_a &&
           lhs.i64_b == rhs.i64_b && lhs.i16_a == rhs.i16_a &&
           lhs.i16_b == rhs.i16_b && lhs.i8_a == rhs.i8_a &&
           lhs.i8_b == rhs.i8_b;
}

} // namespace compact
} // namespace test

namespace serialization {

template<>
struct hz_serializer<test::compact::same_size_record>
  : public compact::compact_serializer
{
    static void write(const test::compact::same_size_record& v,
                      compact::compact_writer& writer)
    {
        // Field write order is intentionally not alphabetical. The schema's
        // canonical ordering must be derived from field names, not from the
        // order they are written in.
        writer.write_int32("i32_c", v.i32_c);
        writer.write_int64("i64_b", v.i64_b);
        writer.write_int32("i32_a", v.i32_a);
        writer.write_int64("i64_a", v.i64_a);
        writer.write_int32("i32_b", v.i32_b);
        writer.write_int16("i16_b", v.i16_b);
        writer.write_int16("i16_a", v.i16_a);
        writer.write_int8("i8_b", v.i8_b);
        writer.write_int8("i8_a", v.i8_a);
    }

    static test::compact::same_size_record read(compact::compact_reader& reader)
    {
        test::compact::same_size_record v;
        v.i32_a = reader.read_int32("i32_a");
        v.i32_b = reader.read_int32("i32_b");
        v.i32_c = reader.read_int32("i32_c");
        v.i64_a = reader.read_int64("i64_a");
        v.i64_b = reader.read_int64("i64_b");
        v.i16_a = reader.read_int16("i16_a");
        v.i16_b = reader.read_int16("i16_b");
        v.i8_a = reader.read_int8("i8_a");
        v.i8_b = reader.read_int8("i8_b");
        return v;
    }

    static std::string type_name() { return "same_size_record"; }
};

} // namespace serialization

namespace test {
namespace compact {

// Pure unit test for the schema constructor: no remote controller required.
class CompactSchemaStableSortTest : public ::testing::Test
{};

TEST_F(CompactSchemaStableSortTest,
       fixed_size_fields_with_same_kind_keep_alphabetical_order)
{
    using serialization::field_kind;
    using serialization::pimpl::field_descriptor;
    using serialization::pimpl::schema;

    // Insertion order is intentionally non-alphabetical; multiple fields
    // share the same fixed size to exercise sort stability.
    schema s{ std::string{ "same_size_record" },
              std::unordered_map<std::string, field_descriptor>{
                { "i32_c", field_descriptor{ field_kind::INT32 } },
                { "i64_b", field_descriptor{ field_kind::INT64 } },
                { "i32_a", field_descriptor{ field_kind::INT32 } },
                { "i64_a", field_descriptor{ field_kind::INT64 } },
                { "i32_b", field_descriptor{ field_kind::INT32 } },
                { "i16_b", field_descriptor{ field_kind::INT16 } },
                { "i16_a", field_descriptor{ field_kind::INT16 } },
                { "i8_b", field_descriptor{ field_kind::INT8 } },
                { "i8_a", field_descriptor{ field_kind::INT8 } } } };

    const auto& f = s.fields();

    // INT64 (8 bytes) come first, alphabetical within group.
    EXPECT_EQ(f.at("i64_a").offset, 0);
    EXPECT_EQ(f.at("i64_b").offset, 8);
    // INT32 (4 bytes) next, alphabetical within group.
    EXPECT_EQ(f.at("i32_a").offset, 16);
    EXPECT_EQ(f.at("i32_b").offset, 20);
    EXPECT_EQ(f.at("i32_c").offset, 24);
    // INT16 (2 bytes) next.
    EXPECT_EQ(f.at("i16_a").offset, 28);
    EXPECT_EQ(f.at("i16_b").offset, 30);
    // INT8 (1 byte) last.
    EXPECT_EQ(f.at("i8_a").offset, 32);
    EXPECT_EQ(f.at("i8_b").offset, 33);

    // No variable-size or boolean fields here.
    EXPECT_EQ(s.number_of_var_size_fields(), 0u);
    EXPECT_EQ(s.fixed_size_fields_length(), 34u);
}

// Integration tests: compare C++ schema layout against the Java reference
// implementation, and validate cross-language compact value round-trip.
class CompactSchemaStableSortIntegrationTest : public compact_test_base
{
protected:
    struct java_field
    {
        int kind = 0;
        int index = -1;
        int offset = -1;
        int bit_offset = -1;
    };

    struct java_schema_view
    {
        std::string type_name;
        std::map<std::string, java_field> fields;
    };

    java_schema_view read_schema_from_server(
      const serialization::pimpl::schema& schema)
    {
        Response response;

        remote_controller_client().executeOnController(
          response,
          factory_->get_cluster_id(),
          (boost::format(
             R"(
                var schemas = instance_0.getOriginal().node.getSchemaService().getAllSchemas();
                var iterator = schemas.iterator();
                var schema;
                while (iterator.hasNext()) {
                    var s = iterator.next();
                    if (s.getSchemaId() == "%1%") {
                        schema = s;
                        break;
                    }
                }

                var obj = {};
                obj.type_name = schema.getTypeName();
                obj.fields = [];

                var fit = schema.getFields().iterator();
                while (fit.hasNext()) {
                    var d = fit.next();
                    obj.fields.push({
                        name: d.getFieldName(),
                        kind: d.getKind().getId(),
                        index: d.getIndex(),
                        offset: d.getOffset(),
                        bitOffset: d.getBitOffset()
                    });
                }

                result = "" + JSON.stringify(obj);
             )") %
           schema.schema_id())
            .str(),
          Lang::JAVASCRIPT);

        return parse_schema_view(response.result);
    }

private:
    java_schema_view parse_schema_view(const std::string& text)
    {
        using namespace boost::property_tree;

        std::stringstream input;
        input << text;

        ptree root;
        read_json(input, root);

        java_schema_view view;
        view.type_name = root.get<std::string>("type_name");

        for (ptree::value_type& f : root.get_child("fields")) {
            java_field jf;
            jf.kind = f.second.get<int>("kind");
            jf.index = f.second.get<int>("index");
            jf.offset = f.second.get<int>("offset");
            jf.bit_offset = f.second.get<int>("bitOffset");
            view.fields.emplace(f.second.get<std::string>("name"), jf);
        }

        return view;
    }
};

// Verifies that the layout (offset / bit_offset / index) computed by the
// patched C++ client matches the Java reference implementation for a schema
// containing multiple same-size fixed fields. Without std::stable_sort, the
// per-field offsets may differ from Java's even though schema_id matches.
TEST_F(CompactSchemaStableSortIntegrationTest,
       cpp_schema_layout_matches_java_for_same_size_fields)
{
    auto schema = get_schema<same_size_record>();

    // Send the schema (name + kinds) to the cluster so Java can compute its
    // own layout from the same field set.
    replicate_schema<same_size_record>();

    auto java = read_schema_from_server(schema);

    EXPECT_EQ(schema.type_name(), java.type_name);
    ASSERT_EQ(schema.fields().size(), java.fields.size());

    for (const auto& entry : schema.fields()) {
        const std::string& name = entry.first;
        const auto& cpp_desc = entry.second;

        auto it = java.fields.find(name);
        ASSERT_NE(it, java.fields.end()) << "missing field on server: " << name;

        const auto& jf = it->second;
        EXPECT_EQ(static_cast<int>(cpp_desc.kind), jf.kind)
          << "kind mismatch for " << name;
        EXPECT_EQ(cpp_desc.offset, jf.offset) << "offset mismatch for " << name;
        EXPECT_EQ(cpp_desc.index, jf.index) << "index mismatch for " << name;
        EXPECT_EQ(static_cast<int>(cpp_desc.bit_offset), jf.bit_offset)
          << "bit_offset mismatch for " << name;
    }
}

// End-to-end round-trip: the C++ client puts a compact value with multiple
// same-size fields; the Java server reads it back as a GenericRecord and we
// verify each field value. If C++ wrote with offsets that disagree with what
// Java computes from the schema, fields would alias and the values would not
// match.
TEST_F(CompactSchemaStableSortIntegrationTest,
       cpp_write_java_read_round_trip_with_same_size_fields)
{
    same_size_record value;
    value.i32_a = 101;
    value.i32_b = 102;
    value.i32_c = 103;
    value.i64_a = 201;
    value.i64_b = 202;
    value.i16_a = 31;
    value.i16_b = 32;
    value.i8_a = 11;
    value.i8_b = 12;

    auto map_name = random_string();
    client->get_map(map_name).get()->put<int, same_size_record>(1, value).get();

    Response response;
    remote_controller_client().executeOnController(response,
                                                   factory_->get_cluster_id(),
                                                   (boost::format(
                                                      R"(
            var map = instance_0.getMap("%1%");
            var record = map.get(1);
            if (record.getInt32("i32_a") === 101 &&
                record.getInt32("i32_b") === 102 &&
                record.getInt32("i32_c") === 103 &&
                record.getInt64("i64_a") == 201 &&
                record.getInt64("i64_b") == 202 &&
                record.getInt16("i16_a") === 31 &&
                record.getInt16("i16_b") === 32 &&
                record.getInt8("i8_a") === 11 &&
                record.getInt8("i8_b") === 12) {
                result = "" + true;
            } else {
                result = "" + false;
            }
         )") % map_name)
                                                     .str(),
                                                   Lang::JAVASCRIPT);

    ASSERT_EQ(response.result, "true");
}

} // namespace compact
} // namespace test
} // namespace client
} // namespace hazelcast
