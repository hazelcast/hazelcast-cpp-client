/*
 * Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
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
#include <hazelcast/client/HazelcastAll.h>

using namespace hazelcast::client;

struct Person {
    friend std::ostream &operator<<(std::ostream &os, const Person &person);

    std::string name;
    bool male;
    int32_t age;
};

std::ostream &operator<<(std::ostream &os, const Person &person) {
    os << "name: " << person.name << " male: " << person.male << " age: " << person.age;
    return os;
}

class MyGlobalSerializer : public hazelcast::client::serialization::global_serializer {
public:
    void write(const boost::any &obj, hazelcast::client::serialization::ObjectDataOutput &out) override {
        auto const &object = boost::any_cast<Person>(obj);
        out.write(object.name);
        out.write(object.male);
        out.write(object.age);
    }

    boost::any read(hazelcast::client::serialization::ObjectDataInput &in) override {
        return boost::any(Person{in.read<std::string>(), in.read<bool>(), in.read<int32_t>()});
    }
};

int main() {
    // Start the Hazelcast Client and connect to an already running Hazelcast Cluster on 127.0.0.1
    hazelcast::client::ClientConfig config;
    hazelcast::client::SerializationConfig serializationConfig;
    serializationConfig.setGlobalSerializer(std::make_shared<MyGlobalSerializer>());
    config.setSerializationConfig(serializationConfig);

    hazelcast::client::HazelcastClient hz(std::move(config));

    return 0;
}
