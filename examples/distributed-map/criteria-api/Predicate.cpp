/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
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
#include <hazelcast/client/serialization/IdentifiedDataSerializable.h>
#include <memory>
#include <hazelcast/client/query/SqlPredicate.h>

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
        return 1;
    }

    int getClassId() const {
        return 3;
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

class PredicateMember {
public:
    std::vector<Person> getWithName(const char *name, hazelcast::client::IMap<std::string, Person> &personMap) {
        char buf[200];
        hazelcast::util::snprintf(buf, 200, "name == %s", name);
        hazelcast::client::query::SqlPredicate predicate(buf);

        return personMap.values(predicate);
    }

    std::vector<Person> getNotWithName(const char *name, hazelcast::client::IMap<std::string, Person> &personMap) {
        char buf[200];
        hazelcast::util::snprintf(buf, 200, "name != %s", name);
        hazelcast::client::query::SqlPredicate predicate(buf);

        return personMap.values(predicate);
    }

    std::vector<Person> getWithNameAndAge(const char *name, int age,
                                          hazelcast::client::IMap<std::string, Person> &personMap) {
        char buf[300];
        hazelcast::util::snprintf(buf, 300, "name == %s AND age == %d", name, age);
        hazelcast::client::query::SqlPredicate predicate(buf);

        return personMap.values(predicate);
    }

    void run() {
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

        hazelcast::client::ISet<Person> s = hz.getSet<Person>("foo");
        Person p("Peter", true, 37);
        s.add(p);
        std::vector<Person> personsInSet = s.toArray();

        std::cout << "Get with name Peter" << std::endl;
        std::vector<Person> values = getWithName("Peter", personMap);
        for (std::vector<Person>::iterator it = values.begin(); it != values.end(); ++it) {
            std::cout << *it << std::endl;
        }

        std::cout << "Get not with name Peter" << std::endl;
        values = getNotWithName("Peter", personMap);
        for (std::vector<Person>::const_iterator it = values.begin(); it != values.end(); ++it) {
            std::cout << *it << std::endl;
        }

        std::cout << "Find name Peter and age 36" << std::endl;
        values = getWithNameAndAge("Peter", 36, personMap);
        for (std::vector<Person>::const_iterator it = values.begin(); it != values.end(); ++it) {
            std::cout << *it << std::endl;
        }

        std::cout << "Find name Peter and age 37" << std::endl;
        values = getWithNameAndAge("Peter", 37, personMap);
        for (std::vector<Person>::const_iterator it = values.begin(); it != values.end(); ++it) {
            std::cout << *it << std::endl;
        }
    }
};

int main() {
    PredicateMember m;
    m.run();

    std::cout << "Finished" << std::endl;

    return 0;
}

