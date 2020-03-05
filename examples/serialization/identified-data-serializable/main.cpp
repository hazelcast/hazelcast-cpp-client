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
#include <hazelcast/client/serialization/IdentifiedDataSerializable.h>
#include <hazelcast/client/serialization/ObjectDataInput.h>
#include <hazelcast/client/serialization/ObjectDataOutput.h>

class Person : public hazelcast::client::serialization::IdentifiedDataSerializable {
public:
    Person() {
    }

    Person(const char *n) : name(new std::string(n)) {
    }

    Person(const Person &rhs) {
        if (rhs.name.get() == NULL) {
            name.reset();
        } else {
            name = std::unique_ptr<std::string>(new std::string(*rhs.name));
        }
    }

    const std::string *getName() const {
        return name.get();
    }

    int getFactoryId() const {
        return 1;
    }

    int getClassId() const {
        return 1;
    }

    void writeData(hazelcast::client::serialization::ObjectDataOutput &out) const {
        out.writeUTF(name.get());
    }

    void readData(hazelcast::client::serialization::ObjectDataInput &in) {
        name = in.readUTF();
    }

private:
    std::unique_ptr<std::string> name;
};

std::ostream &operator<<(std::ostream &out, const Person &p) {
    const std::string *str = p.getName();
    out << ((NULL == str) ? "NULL" : str->c_str());
    return out;
}

int main() {
    hazelcast::client::HazelcastClient hz;

    hazelcast::client::IMap<std::string, Person> map = hz.getMap<std::string, Person>("map");
    Person testPerson("foo");
    map.put("foo", testPerson);
    std::cout << "Finished writing" << std::endl;
    std::cout << map.get("foo");
    std::cout << "Finished reading" << std::endl;


    std::cout << "Finished" << std::endl;

    return 0;
}

