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

/**
 * At this example, there are some integer key/values at a map and
 * this example demonstrates how to use advanced query options
 * such as cursor_buffer_size, timeout, expected result type.
 */
int
main()
{
    using namespace hazelcast::client::sql;

    auto hz = hazelcast::new_client().get();

    // populate the map with some data
    auto map = hz.get_map("mymap").get();
    for (int i = 0; i < 100; ++i) {
        map->put(int64_t(i), "key-" + std::to_string(i)).get();
    }

    hz.get_sql()
      .execute(
        R"(
            CREATE OR REPLACE MAPPING mymap
            TYPE IMAP
            OPTIONS (
                'keyFormat' = 'bigint',
                'valueFormat' = 'varchar'
            )
        )")
      .get();

    // we can do the same query with an sql_statement that we compose
    // and pass to the execute method
    sql_statement statement(hz, "SELECT * FROM mymap ORDER BY __key");

    statement
      // Specifies `timeout` for the query
      // If it is exceeded, operations will be cancelled
      // and `hazelcast_sql_exception` will be thrown.
      .timeout(std::chrono::seconds{ 5 })
      // SELECT query results are fetched page by page.
      // Every page contains N rows.
      // `cursor_buffer_size` specifies this property.
      .cursor_buffer_size(10)
      // If result of SQL query doesn't contain any rows
      // then throws an exception. For example, if query
      // is an `UPDATE` query or `DELETE FROM` it will
      // throw an exception.
      .expected_result_type(sql_expected_result_type::rows);

    auto result = hz.get_sql().execute(statement).get();

    for (auto it = result->iterator(); it.has_next();) {
        auto page = it.next().get();

        for (const sql_page::sql_row& row : page->rows()) {
            // Note that it returns boost::optional<T>.
            // If the column doesn't exist returns
            // boost::optional<T> is empty.
            auto key = row.get_object<int64_t>("__key");
            auto value = row.get_object<std::string>("this");

            std::cout << "Key : " << key << " Value : " << value << std::endl;
        }
    }
}