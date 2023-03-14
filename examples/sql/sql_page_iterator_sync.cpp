
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
#include <algorithm>
#include <iterator>

#include <boost/algorithm/string.hpp>

#include <hazelcast/client/hazelcast_client.h>

using hazelcast::client::hazelcast_client;

void
populate_map(hazelcast_client&);
void
create_mapping(hazelcast_client&);
void
for_loop(hazelcast_client&);
void
algorithm_copy(hazelcast_client&);
void
algorithm_filter(hazelcast_client&);
void
timeout(hazelcast_client&);

/**
 * Normally, cpp-client provides an async api for every features.
 * But for sake of convenience and similar usages with native C++ iterators
 * cpp-client provides sync page iterator. So `page_iterator_sync` is a blocking
 * iterator. It wraps the `page_iterator` and allows to be used in sync
 * manner. This example demonstrates how to use `page_iterator_sync` and what
 * the use cases are.
 */
int
main()
{
    auto hz = hazelcast::new_client().get();

    // Preparation
    populate_map(hz);
    create_mapping(hz);

    // Use cases, examples
    for_loop(hz);
    algorithm_copy(hz);
    algorithm_filter(hz);
    timeout(hz);

    return 0;
}

void
populate_map(hazelcast_client& client)
{
    // Populate a map before using it in sql.
    auto map = client.get_map("integers").get();

    for (int i = 0; i < 100; ++i) {
        map->put(i, i).get();
    }
}

void
create_mapping(hazelcast_client& client)
{
    // Create mapping for the integers.
    // This needs to be done only once per map.
    // It is required to use a map in SQL query.
    auto result = client.get_sql()
                    .execute(R"(
                CREATE OR REPLACE MAPPING integers
                  TYPE IMap
                    OPTIONS (
                      'keyFormat' = 'int',
                      'valueFormat' = 'int'
                      )
                    )")
                    .get();
}

std::shared_ptr<hazelcast::client::sql::sql_result>
select_numbers(hazelcast::client::hazelcast_client& client)
{
    using namespace hazelcast::client::sql;

    sql_statement statement(client, "SELECT * FROM integers");

    // Set cursor buffer size to 5
    // So there will be 20 pages(100 / 5 = 20)
    statement.cursor_buffer_size(5);

    return client.get_sql().execute(statement).get();
}

void
seperator(const std::string& text = std::string{})
{
    std::string output(60, '=');
    boost::replace_first(output, std::string(text.size(), '='), text);

    output = std::string(20, '=') + output;

    std::cout << output << std::endl;
}

void
for_loop(hazelcast_client& client)
{
    seperator("for_loop() - BEGIN");

    auto result = select_numbers(client);

    for (auto it = result->pbegin(); it != result->pend(); ++it) {
        seperator();
        for (const auto& row : it->rows()) {
            std::cout << *row.get_object<int>(0);
        }

        std::cout << std::endl;
    }

    seperator("for_loop() - END");
    seperator();
}

void
algorithm_copy(hazelcast_client& client)
{
    using hazelcast::client::sql::sql_page;

    seperator("algorithm_copy() - BEGIN");

    auto result = select_numbers(client);

    std::vector<std::shared_ptr<sql_page>> pages;

    copy(result->pbegin(), result->pend(), back_inserter(pages));

    std::vector<int> numbers;

    for (const auto& page : pages) {
        transform(
          begin(page->rows()),
          end(page->rows()),
          back_inserter(numbers),
          [](const sql_page::sql_row& row) { return *row.get_object<int>(0); });
    }

    sort(begin(numbers), end(numbers));
    copy(begin(numbers),
         end(numbers),
         std::ostream_iterator<int>(std::cout, "\n"));

    seperator("algorithm_copy - END");
}

void
algorithm_filter(hazelcast_client& client)
{
    using hazelcast::client::sql::sql_page;

    seperator("algorithm_filter - BEGIN");

    auto result = select_numbers(client);

    std::vector<std::shared_ptr<sql_page>> pages;

    copy_if(result->pbegin(),
            result->pend(),
            back_inserter(pages),
            [](const std::shared_ptr<sql_page>& p) {
                // Filter out the pages which contains a number which is
                // divisable by 20
                return any_of(begin(p->rows()),
                              end(p->rows()),
                              [](const sql_page::sql_row& row) {
                                  return *row.get_object<int>(0) % 20 == 0;
                              });
            });

    for (const auto& page : pages) {
        for (const sql_page::sql_row& row : page->rows()) {
            std::cout << row.get_object<int>(0) << " ";
        }

        std::cout << std::endl;
    }

    seperator("algorithm_filter - END");
}

void
timeout(hazelcast_client& client)
{
    seperator("timeout - BEGIN");

    // `generate_stream(1)` generates a row per seconds, so it will guaranteed
    // that it will timeout
    auto result =
      client.get_sql().execute("SELECT * FROM TABLE(generate_stream(1))").get();

    auto it = result->pbegin(std::chrono::milliseconds{ 1 });

    try {
        ++it;
        ++it;
    } catch (hazelcast::client::exception::no_such_element& e) {
        std::cout << "Timedout" << std::endl;
    }

    seperator("timeout - END");
}