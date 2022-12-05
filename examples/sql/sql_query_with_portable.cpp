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

struct Person
{
    std::string name;
    bool male;
    int32_t age;
};

std::ostream&
operator<<(std::ostream& os, const Person& person)
{
    os << "name: " << person.name << " male: " << person.male
       << " age: " << person.age;
    return os;
}

namespace hazelcast {
namespace client {
namespace serialization {
template<>
struct hz_serializer<Person> : portable_serializer
{
    static int32_t get_factory_id() noexcept { return 1; }

    static int32_t get_class_id() noexcept { return 3; }

    static void write_portable(const Person& object, portable_writer& out)
    {
        out.write("name", object.name);
        out.write("gender", object.male);
        out.write("age", object.age);
    }

    static Person read_portable(portable_reader& in)
    {
        return Person{ in.read<std::string>("name"),
                       in.read<bool>("gender"),
                       in.read<int32_t>("age") };
    }
};
} // namespace serialization
} // namespace client
} // namespace hazelcast

void
print_row(const hazelcast::client::sql::sql_page::sql_row& row);

void
print_result(hazelcast::client::sql::sql_result& result);

/**
 * Demonstrates how to create mapping from portable type.
 * After creating mapping, fields of portable type
 * injected into columns of table.
*/
int
main()
{
    using namespace hazelcast::client::sql;

    auto hz = hazelcast::new_client().get();

    // populate the map with some data
    auto map = hz.get_map("person_table").get();
    map->put("joe", Person{ "joe", true, 40 }).get();
    map->put("sarah", Person{ "sarah", false, 35 }).get();
    map->put("john", Person{ "john", true, 50 }).get();

    auto sql = hz.get_sql();

    // Create mapping for the person_table. This needs to be done only once per
    // map.
    auto result = sql
                    .execute(R"(
                CREATE OR REPLACE MAPPING person_table (
                    __key VARCHAR,
                    name VARCHAR,
                    gender BOOLEAN,
                    age INT
                    )
                  TYPE IMap
                    OPTIONS (
                      'keyFormat' = 'varchar',
                      'valueFormat' = 'portable',
                      'valuePortableFactoryId' = '1',
                      'valuePortableClassId' = '3'
                    )
                    )")
                    .get();

    std::cout << "Mapping created. Affected row count:" << result->update_count()
              << std::endl;

    // One can select all rows of the table with *.
    result = sql.execute("SELECT * FROM person_table").get();
    std::cout << "Query(SELECT * FROM person_table) result:" << std::endl;
    print_result(*result);

    // Fetch people older than 38
    result = sql.execute("SELECT * FROM person_table WHERE age > ?", 38).get();
    std::cout << "Query(SELECT * FROM person_table WHERE age > 38) result:" << std::endl;
    print_result(*result);

    std::cout << "Finished" << std::endl;

    return 0;
}

void
print_result(hazelcast::client::sql::sql_result& result)
{
    for (auto itr = result.iterator(); itr.has_next();) {
        auto page = itr.next().get();

        for (auto const& row : page->rows()) {
            print_row(row);
        }
    }
}

void
print_row(const hazelcast::client::sql::sql_page::sql_row& row)
{
    auto name = *row.get_object<std::string>(0);
    auto male = *row.get_object<bool>("gender");
    auto age = *row.get_object<int32_t>("age");
    std::cout << "(" << name << ", " << (male ? "male" : "female")
              << ", age:" << age << ")" << std::endl;
}
