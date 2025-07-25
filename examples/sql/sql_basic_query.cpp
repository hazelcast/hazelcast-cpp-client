/*
 * Copyright (c) 2008-2025, Hazelcast, Inc. All Rights Reserved.
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

/**
 * At this example, there are some integer key/values at a map and
 * they are fetched by SQL queries. Also demonstrates sql_statement usage.
 */
int
main()
{
    using namespace hazelcast::client::sql;

    auto hz = hazelcast::new_client().get();

    // populate the map with some data
    auto map = hz.get_map("integers").get();
    for (int i = 0; i < 100; ++i) {
        map->put(i, i).get();
    }

    auto sql = hz.get_sql();
    // Create mapping for the integers. This needs to be done only once per map.
    auto result = sql
                    .execute(R"(
                CREATE OR REPLACE MAPPING integers
                  TYPE IMap
                    OPTIONS (
                      'keyFormat' = 'int',
                      'valueFormat' = 'int'
                      )
                    )")
                    .get();

    // Fetch values in between (40, 50)
    result =
      sql.execute("SELECT * FROM integers WHERE this > ? AND this < ?", 40, 50)
        .get();

    for (auto itr = result->iterator(); itr.has_next();) {
        auto page = itr.next().get();

        std::cout << "There are " << page->row_count() << " rows the page."
                  << std::endl;

        for (auto const& row : page->rows()) {
            std::cout << "(" << row.get_object<int>(0) << ", "
                      << row.get_object<int>(1) << ")" << std::endl;
        }
    }

    // we can do the same query with an sql_statement that we compose
    // and pass to the execute method
    sql_statement statement(
      hz, "SELECT * FROM integers WHERE this > ? AND this < ?");
    statement.set_parameters(40, 50);
    result = sql.execute(statement).get();

    auto first_page = result->iterator().next().get();
    std::cout << "There are " << first_page->row_count()
              << " rows at the first page" << std::endl;

    std::cout << "Finished" << std::endl;

    return 0;
}
