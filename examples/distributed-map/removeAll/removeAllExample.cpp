/*
 * Copyright (c) 2008-2021, Hazelcast, Inc. All Rights Reserved.
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

using namespace hazelcast::client;

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

int
main()
{
    auto hz = hazelcast::new_client().get();

    auto personMap = hz.get_map("personMap").get();
    personMap
      ->put_all<std::string, Person>({ { "1", Person{ "Peter", true, 36 } },
                                       { "2", Person{ "John", true, 50 } },
                                       { "3", Person{ "Marry", false, 20 } },
                                       { "4", Person{ "Mike", true, 35 } },
                                       { "5", Person{ "Rob", true, 60 } },
                                       { "6", Person{ "Jane", false, 43 } } })
      .get();

    // Remove entries that whose name start with 'M'
    personMap->remove_all(query::like_predicate(hz, "name", "M%")).get();

    if (personMap->get<std::string, Person>("3").get()) {
        std::cerr << "Entry 3 is not deleted. This is unexpected!!!"
                  << std::endl;
    } else {
        std::cout << "Entry 3 is deleted." << std::endl;
    }

    if (personMap->get<std::string, Person>("4").get()) {
        std::cerr << "Entry 4 is not deleted. This is unexpected!!!"
                  << std::endl;
    } else {
        std::cout << "Entry 4 is deleted." << std::endl;
    }

    auto mapSize = personMap->size().get();
    if (4 == mapSize) {
        std::cout << "There are only 4 entries as expected." << std::endl;
    } else {
        std::cerr << "There are " << mapSize
                  << "entries in the map-> This is unexpected!!!" << std::endl;
    }

    std::cout << "Finished" << std::endl;

    return 0;
}
