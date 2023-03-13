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
#include "serialization/main_dto.h"

namespace hazelcast {
namespace client {
namespace test {
namespace compact {

class CompactReadWriteIntegrationTest : public compact_test_base
{};

TEST_F(CompactReadWriteIntegrationTest, map_put_get)
{
    auto dto = create_main_dto();
    auto map_name = random_string();
    auto key = random_string();

    auto map = client->get_map(map_name).get();

    map->put(key, dto).get();
    auto actual = map->get<std::string, main_dto>(key).get();

    ASSERT_TRUE(actual.has_value());
    EXPECT_EQ(dto, *actual);
}

TEST_F(CompactReadWriteIntegrationTest, write_read_sql)
{
    sample_compact_type value{ 100 };
    auto map_name = random_string();
    auto key = random_string();
    auto type_name = serialization::hz_serializer<sample_compact_type>::type_name();

    auto map = client->get_map(map_name).get();

    map->put(key, value).get();

    std::string query = (boost::format("CREATE MAPPING %1% ("
                                       "__key VARCHAR,"
                                       "x INT"
                                       ")"
                                       "TYPE IMap "
                                       "OPTIONS ("
                                       "'keyFormat' = 'varchar', "
                                       "'valueFormat' = 'compact', "
                                       "'valueCompactTypeName' = '%2%' "
                                       ")") %
                         map_name % type_name)
                          .str();

    (void)client->get_sql().execute(query).get();

    auto result = client->get_sql()
                    .execute(str(boost::format("SELECT * FROM %1%") % map_name))
                    .get();

    for (auto it = result->iterator(); it.has_next();) {
        auto page = it.next().get();

        for (const sql::sql_page::sql_row& row : page->rows()) {
            auto x_opt = row.get_object<int>("x");
            ASSERT_TRUE(x_opt.has_value());
            EXPECT_EQ(x_opt.value(), value.x);
        }
    }
}

} // namespace compact
} // namespace test
} // namespace client
} // namespace hazelcast