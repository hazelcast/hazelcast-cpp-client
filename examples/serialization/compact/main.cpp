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
#include <hazelcast/client/serialization/serialization.h>

struct PersonDTO
{
    std::string name;
    std::string surname;
    int32_t age;
};

std::ostream&
operator<<(std::ostream& os, const PersonDTO& person)
{
    os << "name: " << person.name << " surname: " << person.surname
       << " age: " << person.age;

    return os;
}

namespace hazelcast {
namespace client {
namespace serialization {

template<>
struct hz_serializer<PersonDTO> : compact::compact_serializer
{
    static void write(const PersonDTO& object, compact::compact_writer& out)
    {
        out.write_int32("age", object.age);
        out.write_string("name", object.name);
        out.write_string("surname", object.surname);
    }

    static PersonDTO read(compact::compact_reader& in)
    {
        PersonDTO person;

        person.age = in.read_int32("age");
        boost::optional<std::string> name = in.read_string("name");

        if (name) {
        }

        boost::optional<std::string> surname = in.read_string("surname");

        if (surname) {
            person.surname = *surname;
        }
    static std::string type_name() { return "person"; }
};

} // namespace serialization
} // namespace client
} // namespace hazelcast

/**
 * This example demonstrates how to use a type with compact serialization.
 */
int
main()
{
    auto hz = hazelcast::new_client().get();
    auto map = hz.get_map("map").get();

    map->put("Peter", PersonDTO{ "Peter", "Stone", 45 }).get();
    auto person =
      map->get<std::string, PersonDTO>(std::string{ "Peter" }).get();

    std::cout << person << std::endl;

    std::cout << "Finished" << std::endl;

    return 0;
}