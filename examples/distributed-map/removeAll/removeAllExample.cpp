/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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
//
// Created by İhsan Demir on 21/12/15.
//
#include <hazelcast/client/HazelcastClient.h>
#include <hazelcast/client/query/LikePredicate.h>

using namespace hazelcast::client;

class Person : public hazelcast::client::serialization::IdentifiedDataSerializable {
public:
    Person() { }

    Person(const Person &p) : male(p.male), age(p.age) {
        if (NULL != p.name.get()) {
            name = std::auto_ptr<std::string>(new std::string(*p.name));
        }
    }

    Person &operator=(const Person &p) {
        if (NULL != p.name.get()) {
            name = std::auto_ptr<std::string>(new std::string(*p.name));
        }
        male = p.male;
        age = p.age;

        return *this;
    }

    Person(const char *n, bool male, int age)
            : name(std::auto_ptr<std::string>(new std::string(n))), male(male), age(age) { }

    int getFactoryId() const {
        return 666;
    }

    int getClassId() const {
        return 10;
    }

    void writeData(hazelcast::client::serialization::ObjectDataOutput &out) const {
        out.writeUTF(name.get());
        out.writeBoolean(male);
        out.writeInt(age);
    }

    void readData(hazelcast::client::serialization::ObjectDataInput &in) {
        name = in.readUTF();
        male = in.readBoolean();
        age = in.readInt();
    }


    const std::string *getName() const {
        return name.get();
    }

    bool isMale() const {
        return male;
    }

    int getAge() const {
        return age;
    }

private:
    std::auto_ptr<std::string> name;
    bool male;
    int age;
};

std::ostream &operator<<(std::ostream &out, const Person &p) {
    const std::string *n = p.getName();
    out << "Person{"
    << "male=" << p.isMale()
    << ", name='" << (NULL == n ? "No name" : n->c_str()) << "\'"
    << ", age=" << p.getAge()
    << '}';
    return out;
}

int main() {
    hazelcast::client::ClientConfig config;
    hazelcast::client::HazelcastClient hz(config);

    hazelcast::client::IMap<std::string, Person> personMap = hz.getMap<std::string, Person>("personMap");

    Person p1("Peter", true, 36);
    Person p2("John", true, 50);
    Person p3("Marry", false, 20);
    Person p4("Mike", true, 35);
    Person p5("Rob", true, 60);
    Person p6("Jane", false, 43);
    personMap.put("1", p1);
    personMap.put("2", p2);
    personMap.put("3", p3);
    personMap.put("4", p4);
    personMap.put("5", p5);
    personMap.put("6", p6);

    // Remove entries that whose name start with 'M'
    personMap.removeAll(query::LikePredicate("name", "M%"));

    if (personMap.get("3").get() != NULL) {
        std::cerr << "Entry 3 is not deleted. This is unexpected!!!" << std::endl;
    } else {
        std::cout << "Entry 3 is deleted." << std::endl;
    }
    
    if (personMap.get("4").get() != NULL) {
        std::cerr << "Entry 4 is not deleted. This is unexpected!!!" << std::endl;
    } else {
        std::cout << "Entry 4 is deleted." << std::endl;
    }

    int mapSize = personMap.size();
    if (4 == mapSize) {
        std::cout << "There are only 4 entries as expected." << std::endl;
    } else {
        std::cerr << "There are " << mapSize << "entries in the map. This is unexpected!!!" << std::endl;
    }
    
    std::cout << "Finished" << std::endl;

    return 0;
}
