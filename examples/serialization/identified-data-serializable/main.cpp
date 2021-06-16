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

struct Person
{
    std::string name;
    bool male;
    int32_t age;
};

std::ostream&
operator<<(std::ostream& os, const Person& person)
{
    os << "name: " << person.name << " male: " << person.male << " age: " << person.age;
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

    static void write_data(const Person& object, object_data_output& out)
    {
        out.write(object.name);
        out.write(object.male);
        out.write(object.age);
    }

    static Person read_data(object_data_input& in)
    {
        return Person{ in.read<std::string>(), in.read<bool>(), in.read<int32_t>() };
    }
};
} // namespace serialization
} // namespace client
} // namespace hazelcast

int
main()
{
    auto hz = hazelcast::new_client().get();

    auto map = hz.get_map("map").get();
    map->put("foo", Person{ "bar", true, 40 }).get();
    std::cout << *(map->get<std::string, Person>("foo").get()) << std::endl;

    std::cout << "Finished" << std::endl;

    return 0;
}
