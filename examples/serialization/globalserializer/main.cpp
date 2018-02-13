/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
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
#include <hazelcast/client/HazelcastClient.h>
#include <hazelcast/client/serialization/ObjectDataInput.h>
#include <hazelcast/client/serialization/ObjectDataOutput.h>

class Person {
public:
    Person() {
    }

    Person(const std::string& n) : name(n) {
    }

    void setName(const std::string& n) {
        name = n;
    }

    const std::string& getName() const {
        return name;
    }

private:
    std::string name;
};

class GlobalSerializer : public hazelcast::client::serialization::StreamSerializer {
public:

    virtual void write(hazelcast::client::serialization::ObjectDataOutput &out, const void *object) {
        const Person *p = static_cast<const Person *>(object);
        out.writeUTF(&p->getName());
    }

    virtual void *read(hazelcast::client::serialization::ObjectDataInput &in) {
        return new Person(*(in.readUTF()));
    }

    int getHazelcastTypeId() const {
        // type id for the global serializer
        return 345;
    };
};

std::ostream &operator<<(std::ostream &out, const Person &p) {
    const std::string & str = p.getName();
    out << str;
    return out;
}

int main() {
    hazelcast::client::ClientConfig config;
    hazelcast::client::SerializationConfig serializationConfig;
    serializationConfig.setGlobalSerializer(boost::shared_ptr<hazelcast::client::serialization::StreamSerializer>(
            new GlobalSerializer()));
    config.setSerializationConfig(serializationConfig);
    hazelcast::client::HazelcastClient hz(config);

    hazelcast::client::IMap<std::string, Person> map = hz.getMap<std::string, Person>("map");
    Person testPerson("bar");
    map.put("foo", testPerson);
    std::cout << "Got value \"" << *(map.get("foo")) << "\" from the map." << std::endl;
    std::cout << "Finished" << std::endl;

    return 0;
}
