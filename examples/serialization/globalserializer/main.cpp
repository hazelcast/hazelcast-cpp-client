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

class MyGlobalSerializer
  : public hazelcast::client::serialization::global_serializer
{
public:
    void write(
      const boost::any& obj,
      hazelcast::client::serialization::object_data_output& out) override
    {
        auto const& object = boost::any_cast<Person>(obj);
        out.write(object.name);
        out.write(object.male);
        out.write(object.age);
    }

    boost::any read(
      hazelcast::client::serialization::object_data_input& in) override
    {
        return boost::any(Person{
          in.read<std::string>(), in.read<bool>(), in.read<int32_t>() });
    }
};

int
main()
{
    hazelcast::client::client_config config;
    config.get_serialization_config().set_global_serializer(
      std::make_shared<MyGlobalSerializer>());
    auto hz = hazelcast::new_client(std::move(config)).get();

    auto map = hz.get_map("map").get();
    map->put("foo", Person{ "first last name", false, 19 }).get();
    std::cout << "Got value \"" << *(map->get<std::string, Person>("foo").get())
              << "\" from the map->" << std::endl;
    std::cout << "Finished" << std::endl;

    return 0;
}
