/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
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

int main() {
    hazelcast::client::ClientConfig config;
    hazelcast::client::HazelcastClient hz(config);

    hazelcast::client::IMap<std::string, Person> map =
            hz.getMap<std::string, Person>("personsWithIndex");

    map.addIndex("name", true);

    const int mapSize = 200000;

    char buf[30];
    char name[30];
    time_t start = time(NULL);
    for (int i = 0; i < mapSize; ++i) {
        hazelcast::util::hz_snprintf(buf, 30, "person-%d", i);
        hazelcast::util::hz_snprintf(name, 50, "myname-%d", i % 1000);
        Person p(name, (i % 2 == 0), (i % 100));
        map.put(buf, p);
    }
    time_t end = time(NULL);
    std::cout << "Put " << mapSize << " entries into the map in " << end - start << " seconds" << std::endl;

    start = time(NULL);
    hazelcast::client::query::SqlPredicate predicate("name == 'myname-30'");
    std::vector<std::pair<std::string, Person> > entries = map.entrySet(predicate);
    end = time(NULL);
    std::cout << "The query resulted in " << entries.size() << " entries in " << end - start << " seconds" << std::endl;

    std::cout << "Finished" << std::endl;

    return 0;
}
