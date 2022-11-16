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
    using hazelcast::client::hazelcast_json_value;

    auto hz = hazelcast::new_client().get();

    auto employees = hz.get_map("employees").get();

    // Populate some data
    employees->put(0,
                   hazelcast_json_value{ R"({"name": "Alice", "age": 32})" });
    employees->put(1, hazelcast_json_value{ R"({"name": "John", "age": 42})" });
    employees->put(2, hazelcast_json_value{ R"({"name": "Jake", "age": 18})" });

    auto sql = hz.get_sql();
    // Create mapping for the employees map. This needs to be done only once per
    // map.
    auto result = sql
                    .execute(R"(
                        CREATE OR REPLACE MAPPING employees
                        TYPE IMap
                        OPTIONS (
                            'keyFormat' = 'int',
                            'valueFormat' = 'json'
                        )
                    )")
                    .get();

    // Select the names of employees older than 25
    result = sql
               .execute(R"(
            SELECT this
            FROM employees
            WHERE JSON_VALUE(this, '$.age' RETURNING INT) > 25
        )")
               .get();

    for (auto itr = result->iterator(); itr.has_next();) {
        auto page = itr.next().get();

        std::cout << "There are " << page->row_count()
                  << " rows returned from the cluster database" << std::endl;

        for (auto const& row : page->rows()) {
            std::cout << "Name:" << row.get_object<hazelcast_json_value>("this")
                      << std::endl;
        }
    }

    return 0;
}
