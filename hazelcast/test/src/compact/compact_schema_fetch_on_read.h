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

#include <memory>
#include <sstream>
#include <functional>

#include <gtest/gtest.h>

#include "hazelcast/client/hazelcast_client.h"
#include "../remote_controller_client.h"
#include "../TestHelperFunctions.h"

#include "compact_test_base.h"
#include "serialization/a_type.h"

namespace hazelcast {
namespace client {
namespace test {
namespace compact {

class CompactSchemaFetchOnRead : public compact_test_base
{
protected:
    std::string map_name_{ random_string() };
    std::string key_{ random_string() };

    void put_record_with_rc()
    {
        Response response;

        remote_controller_client().executeOnController(
          response,
          factory_.get_cluster_id(),
          (boost::format(
             R"(
                    var nested_type = com.hazelcast.nio.serialization.genericrecord.GenericRecordBuilder.compact("%1%").setInt32("y", 101).build();
                    var a_type = com.hazelcast.nio.serialization.genericrecord.GenericRecordBuilder.compact("%2%").setInt32("x", 100).setGenericRecord("nested", nested_type).build();

                    var map_name = "%3%";
                    var key = "%4%";
                    var map = instance_0.getMap(map_name);

                    map.put(key, a_type);

                    result = "true";
                        )") %
           serialization::hz_serializer<nested_type>::type_name() %
           serialization::hz_serializer<a_type>::type_name() % map_name_ % key_)
            .str(),
          Lang::JAVASCRIPT);

        if (response.result != "true")
            GTEST_FAIL();
    }

    void put_record_with_different_name()
    {
        Response response;

        remote_controller_client().executeOnController(
          response,
          factory_.get_cluster_id(),
          (boost::format(
             R"(
                    var value = com.hazelcast.nio.serialization.genericrecord.GenericRecordBuilder.compact("wrong_type_name").build();

                    var map_name = "%1%";
                    var key = "%2%";
                    var map = instance_0.getMap(map_name);

                    map.put(key, value);

                    result = "true";
                        )") %
           map_name_ % key_)
            .str(),
          Lang::JAVASCRIPT);

        if (response.result != "true")
            GTEST_FAIL();
    }
};

TEST_F(CompactSchemaFetchOnRead, imap_get)
{
    put_record_with_rc();

    auto map = client.get_map(map_name_).get();
    auto a_t = map->get<std::string, a_type>(key_).get();

    EXPECT_EQ(a_t->x, 100);
    EXPECT_EQ(a_t->nested.y, 101);
}

TEST_F(CompactSchemaFetchOnRead, throw_exception_on_typename_mismatch)
{
    put_record_with_different_name();

    auto map = client.get_map(map_name_).get();

    auto fn = [=]() { map->get<std::string, a_type>(key_).get(); };

    EXPECT_THROW(fn(), exception::hazelcast_serialization);
}

TEST_F(CompactSchemaFetchOnRead, sql_read)
{
    put_record_with_rc();

    std::string query =
      (boost::format("CREATE MAPPING %1% ("
                     "__key VARCHAR,"
                     "x INT"
                     ")"
                     "TYPE IMap "
                     "OPTIONS ("
                     "'keyFormat' = 'varchar', "
                     "'valueFormat' = 'compact', "
                     "'valueCompactTypeName' = '%2%' "
                     ")") %
       map_name_ % serialization::hz_serializer<a_type>::type_name())
        .str();

    (void)client.get_sql().execute(query).get();

    auto result =
      client.get_sql()
        .execute(str(boost::format("SELECT * FROM %1%") % map_name_))
        .get();

    auto page = result->iterator().next().get();

    auto actual = page->rows().front().get_object<int>("x");
    ASSERT_TRUE(actual.has_value());
    EXPECT_EQ(actual.value(), 100);
}

} // namespace compact
} // namespace test
} // namespace client
} // namespace hazelcast