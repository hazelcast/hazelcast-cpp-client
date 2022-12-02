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
#include <hazelcast/client/hazelcast_client.h>

int
main()
{
    using namespace hazelcast::client::sql;

    auto hz = hazelcast::new_client().get();

    // populate the map with some data
    auto map = hz.get_map("myMap").get();
    map->put("key1", 1.0);
    map->put("key2", 2.0);
    map->put("key3", 3.0);
    map->put("key4", 4.0);
    map->put("key5", 5.0);

    auto sql = hz.get_sql();
    // Create mapping for the doubles. This needs to be done only once per map.
    auto result = sql
                    .execute(R"(
                      CREATE OR REPLACE MAPPING myMap (
                          __key VARCHAR,
                          this DOUBLE
                      )
                      TYPE IMAP
                      OPTIONS (
                          'keyFormat' = 'varchar',
                          'valueFormat' = 'double'
                      )
                    )")
                    .get();

    // Retrieve 3 elements starting from the offset 1 so it will skip first
    // element
    result =
      sql
        .execute("SELECT * FROM myMap ORDER BY this ASC LIMIT ? OFFSET ?", 3, 1)
        .get();

    for (auto itr = result->iterator(); itr.has_next();) {
        auto page = itr.next().get();

        std::cout << "There are " << page->row_count()
                  << " rows returned from the cluster database" << std::endl;

        for (auto const& row : page->rows()) {
            std::cout << "(" << row.get_object<std::string>(0) << ", "
                      << row.get_object<double>(1) << ")" << std::endl;
        }
    }

    std::cout << "Finished" << std::endl;

    return 0;
}
