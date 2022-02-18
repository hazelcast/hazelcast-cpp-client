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
#include <hazelcast/client/query/predicates.h>
#include "employee.h"

using namespace hazelcast::client;
using namespace examples;

struct Person
{
    friend std::ostream& operator<<(std::ostream& os, const Person& person);

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
struct hz_serializer<Person> : identified_data_serializer
{
    static int32_t get_factory_id() noexcept { return 1; }

    static int32_t get_class_id() noexcept { return 3; }

    static void write_data(
      const Person& object,
      hazelcast::client::serialization::object_data_output& out)
    {
        out.write(object.name);
        out.write(object.male);
        out.write(object.age);
    }

    static Person read_data(
      hazelcast::client::serialization::object_data_input& in)
    {
        return Person{ in.read<std::string>(),
                       in.read<bool>(),
                       in.read<int32_t>() };
    }
};
} // namespace serialization
} // namespace client
} // namespace hazelcast

class PredicateMember
{
public:
    std::vector<Person> get_with_name(hazelcast_client& hz,
                                      const std::string& name,
                                      hazelcast::client::imap& person_map)
    {
        return person_map
          .values<Person>(
            query::sql_predicate(hz, std::string("name==") + name))
          .get();
    }

    std::vector<Person> get_not_with_name(hazelcast_client& hz,
                                          const std::string& name,
                                          hazelcast::client::imap& person_map)
    {
        return person_map
          .values<Person>(
            query::sql_predicate(hz, std::string("name!=") + name))
          .get();
    }

    std::vector<Person> get_with_name_and_age(
      hazelcast_client& hz,
      const std::string& name,
      int32_t age,
      hazelcast::client::imap& person_map)
    {
        return person_map
          .values<Person>(query::sql_predicate(
            hz,
            (boost::format("name == %1% AND age == %2%") % name % age).str()))
          .get();
    }

    void run()
    {
        auto hz = hazelcast::new_client().get();

        auto personMap = hz.get_map("personMap").get();

        personMap
          ->put_all<std::string, Person>(
            { { "1", Person{ "Peter", true, 36 } },
              { "2", Person{ "John", true, 50 } },
              { "3", Person{ "Marry", false, 20 } },
              { "4", Person{ "Mike", true, 35 } },
              { "5", Person{ "Rob", true, 60 } },
              { "6", Person{ "Jane", false, 43 } } })
          .get();

        auto s = hz.get_set("foo").get();
        s->add(Person{ "Peter", true, 37 });
        auto personsInSet = s->to_array<Person>().get();

        std::cout << "Get with name Peter" << std::endl;
        for (auto& p : get_with_name(hz, "Peter", *personMap)) {
            std::cout << p << std::endl;
        }

        std::cout << "Get not with name Peter" << std::endl;
        for (auto& p : get_not_with_name(hz, "Peter", *personMap)) {
            std::cout << p << std::endl;
        }

        std::cout << "Find name Peter and age 36" << std::endl;
        for (auto& p : get_with_name_and_age(hz, "Peter", 36, *personMap)) {
            std::cout << p << std::endl;
        }

        std::cout << "Find name Peter and age 37" << std::endl;
        for (auto& p : get_with_name_and_age(hz, "Peter", 37, *personMap)) {
            std::cout << p << std::endl;
        }
    }
};

void
query_map_using_paging_predicate()
{
    auto client = hazelcast::new_client().get();

    auto intMap = client.get_map("testIntMapValuesWithpaging_predicate").get();

    int predSize = 5;
    const int totalEntries = 25;

    for (int i = 0; i < totalEntries; ++i) {
        intMap->put(i, i).get();
    }

    auto predicate = intMap->new_paging_predicate<int, int>((size_t)predSize);

    auto values = intMap->values<int>(predicate).get();
    std::sort(values.begin(), values.end());

    predicate.next_page();
    values = intMap->values<int>(predicate).get();

    predicate.set_page(4);

    values = intMap->values<int>(predicate).get();

    predicate.previous_page();
    values = intMap->values<int>(predicate).get();

    // paging_predicate with inner predicate (value < 10)
    auto predicate2 = intMap->new_paging_predicate<int, int>(
      5,
      query::greater_less_predicate(
        client, query::query_constants::THIS_ATTRIBUTE_NAME, 9, false, true));
    values = intMap->values<int>(predicate2).get();

    predicate2.next_page();
    // match values 5,6, 7, 8
    values = intMap->values<int>(predicate2).get();

    predicate2.next_page();
    values = intMap->values<int>(predicate2).get();

    // test paging predicate with comparator
    auto employees =
      client.get_map("testComplexObjectWithpaging_predicate").get();
    employees
      ->put_all<int32_t, employee>({ { 3, employee("ahmet", 35) },
                                     { 4, employee("mehmet", 21) },
                                     { 5, employee("deniz", 25) },
                                     { 6, employee("ali", 33) },
                                     { 7, employee("veli", 44) },
                                     { 8, employee("aylin", 5) } })
      .get();

    predSize = 2;
    std::unique_ptr<query::entry_comparator<int, employee>> comparator(
      new employee_entry_comparator());
    auto predicate3 = employees->new_paging_predicate<int, employee>(
      employee_entry_comparator(), (size_t)predSize);
    auto result = employees->values(predicate3).get();

    predicate3.next_page();
    result = employees->values(predicate3).get();
}

void
query_map_using_different_predicates()
{
    auto client = hazelcast::new_client().get();

    auto intMap = client.get_map("testValuesWithPredicateIntMap").get();

    const int numItems = 20;
    for (int i = 0; i < numItems; ++i) {
        intMap->put(i, 2 * i);
    }

    auto values = intMap->values<int>().get();

    // equal_predicate
    // key == 5
    values = intMap
               ->values<int, query::equal_predicate>(query::equal_predicate(
                 client, query::query_constants::KEY_ATTRIBUTE_NAME, 5))
               .get();

    size_t numberOfValues = values.size();
    if (numberOfValues > 0) {
        std::cout << "First value:" << values[0] << std::endl;
    }

    // value == 8
    values = intMap
               ->values<int, query::equal_predicate>(query::equal_predicate(
                 client, query::query_constants::THIS_ATTRIBUTE_NAME, 8))
               .get();

    // key == numItems
    values = intMap
               ->values<int, query::equal_predicate>(query::equal_predicate(
                 client, query::query_constants::KEY_ATTRIBUTE_NAME, numItems))
               .get();

    // NotEqual Predicate
    // key != 5
    values =
      intMap
        ->values<int, query::not_equal_predicate>(query::not_equal_predicate(
          client, query::query_constants::KEY_ATTRIBUTE_NAME, 5))
        .get();

    // this(value) != 8
    values =
      intMap
        ->values<int, query::not_equal_predicate>(query::not_equal_predicate(
          client, query::query_constants::THIS_ATTRIBUTE_NAME, 8))
        .get();

    // true_predicate
    values =
      intMap->values<int, query::true_predicate>(query::true_predicate(client))
        .get();

    // false_predicate
    values =
      intMap
        ->values<int, query::false_predicate>(query::false_predicate(client))
        .get();

    // between_predicate
    // 5 <= key <= 10
    values = intMap
               ->values<int, query::between_predicate>(query::between_predicate(
                 client, query::query_constants::KEY_ATTRIBUTE_NAME, 5, 10))
               .get();

    // greater_less_predicate
    // value <= 10
    values =
      intMap
        ->values<int,
                 query::greater_less_predicate>(query::greater_less_predicate(
          client, query::query_constants::THIS_ATTRIBUTE_NAME, 10, true, true))
        .get();

    // key < 7
    values =
      intMap
        ->values<int, query::greater_less_predicate>(
          query::greater_less_predicate(
            client, query::query_constants::KEY_ATTRIBUTE_NAME, 7, false, true))
        .get();

    // in_predicate
    // key in {4, 10, 19}
    std::vector<int> inVals{ 4, 10, 19 };
    values = intMap
               ->values<int, query::in_predicate>(query::in_predicate(
                 client, query::query_constants::KEY_ATTRIBUTE_NAME, inVals))
               .get();

    // value in {4, 10, 19}
    values = intMap
               ->values<int, query::in_predicate>(query::in_predicate(
                 client, query::query_constants::THIS_ATTRIBUTE_NAME, inVals))
               .get();

    // instance_of_predicate
    // value instanceof Integer
    values =
      intMap
        ->values<int>(query::instance_of_predicate(client, "java.lang.Integer"))
        .get();

    // not_predicate
    // !(5 <= key <= 10)
    values = intMap
               ->values<int>(query::not_predicate(
                 client,
                 query::between_predicate(
                   client, query::query_constants::KEY_ATTRIBUTE_NAME, 5, 10)))
               .get();

    // and_predicate
    // 5 <= key <= 10 AND Values in {4, 10, 19} = values {4, 10}
    values =
      intMap
        ->values<int>(query::and_predicate(
          client,
          query::between_predicate(
            client, query::query_constants::KEY_ATTRIBUTE_NAME, 5, 10),
          query::in_predicate(
            client, query::query_constants::THIS_ATTRIBUTE_NAME, inVals)))
        .get();

    // or_predicate
    // 5 <= key <= 10 OR Values in {4, 10, 19} = values {4, 10, 12, 14, 16, 18,
    // 20}
    values =
      intMap
        ->values<int>(query::or_predicate(
          client,
          query::between_predicate(
            client, query::query_constants::KEY_ATTRIBUTE_NAME, 5, 10),
          query::in_predicate(
            client, query::query_constants::THIS_ATTRIBUTE_NAME, inVals)))
        .get();

    auto imap = client.get_map("StringMap").get();

    // like_predicate
    // value LIKE "value1" : {"value1"}
    auto strValues =
      imap
        ->values<std::string>(query::like_predicate(
          client, query::query_constants::THIS_ATTRIBUTE_NAME, "value1"))
        .get();

    // ilike_predicate
    // value ILIKE "%VALue%1%" : {"myvalue_111_test", "value1", "value10",
    // "value11"}
    strValues =
      imap
        ->values<std::string>(query::ilike_predicate(
          client, query::query_constants::THIS_ATTRIBUTE_NAME, "%VALue%1%"))
        .get();

    // value ILIKE "%VAL%2%" : {"myvalue_22_test", "value2"}
    strValues =
      imap
        ->values<std::string>(query::ilike_predicate(
          client, query::query_constants::THIS_ATTRIBUTE_NAME, "%VAL%2%"))
        .get();

    // sql_predicate
    // __key BETWEEN 4 and 7 : {4, 5, 6, 7} -> {8, 10, 12, 14}
    auto sql = (boost::format("%1% BETWEEN 4 and 7") %
                query::query_constants::KEY_ATTRIBUTE_NAME)
                 .str();
    values = intMap->values<int>(query::sql_predicate(client, sql)).get();

    // regex_predicate
    // value matches the regex ".*value.*2.*" : {myvalue_22_test, value2}
    strValues =
      imap
        ->values<std::string>(query::regex_predicate(
          client, query::query_constants::THIS_ATTRIBUTE_NAME, ".*value.*2.*"))
        .get();
}

int
main()
{
    PredicateMember m;
    m.run();

    query_map_using_different_predicates();

    query_map_using_paging_predicate();

    std::cout << "Finished" << std::endl;

    return 0;
}
