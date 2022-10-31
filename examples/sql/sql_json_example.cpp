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

    employees->put(0 , hazelcast_json_value { R"({"name": "Alice", "age": 32})" });
    employees->put(1 , hazelcast_json_value { R"({"name": "John", "age": 42})" });
    employees->put(2 , hazelcast_json_value { R"({"name": "Jake", "age": 18})" });

    auto sql = hz.get_sql();
    // Create mapping for the integers. This needs to be done only once per map.
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

    // Fetch values in between (40, 50)
    result =
      sql.execute(R"(
            SELECT JSON_VALUE(this, '$.name') AS name
            FROM employees
            WHERE JSON_VALUE(this, '$.age' RETURNING INT) > 25
        )")
        .get();

    auto it = result->page_iterator();
    std::cout << "There are " << (*it)->row_count()
              << " rows returned from the cluster database" << std::endl;

    for (; it; (++it).get()) {
        for (auto const& row : (*it)->rows()) {
            std::cout << "Name :" << row.get_object<std::string>(0) << std::endl;
        }
    }

    return 0;
}
