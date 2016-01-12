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
#include <hazelcast/client/serialization/PortableWriter.h>
#include <hazelcast/client/serialization/PortableReader.h>
#include <hazelcast/client/query/SqlPredicate.h>

class Limb : public hazelcast::client::serialization::Portable {
public:
    Limb() { }

    Limb(const std::string &n) : name(n) { }

    int getFactoryId() const {
        return 1;
    }

    int getClassId() const {
        return 9;
    }

    void writePortable(hazelcast::client::serialization::PortableWriter &out) const {
        out.writeUTF("n", &name);
    }

    void readPortable(hazelcast::client::serialization::PortableReader &in) {
        name = *in.readUTF("n");
    }

    const std::string &getName() const {
        return name;
    }

private:
    std::string name;
};

class Person : public hazelcast::client::serialization::IdentifiedDataSerializable {
public:
    Person() { }

    Person(const std::string &name, const std::vector<Limb> &limbs) : name(name), limbs(limbs) { }

    int getFactoryId() const {
        return 1;
    }

    int getClassId() const {
        return 8;
    }

    void writeData(hazelcast::client::serialization::ObjectDataOutput &out) const {
        out.writeUTF(&name);
        int size = (int) limbs.size();
        out.writeInt(size);
        for (int i = 0; i < size; ++i) {
            out.writeObject<Limb>(&limbs[i]);
        }
    }

    void readData(hazelcast::client::serialization::ObjectDataInput &in) {
        name = *in.readUTF();
        int size = in.readInt();
        limbs = std::vector<Limb>((size_t) size);
        for (int i = 0; i < size; ++i) {
            limbs[i] = *in.readObject<Limb>();
        }
    }


    const std::string &getName() const {
        return name;
    }

    const std::vector<Limb> &getLimbs() const {
        return limbs;
    }

private:
    std::string name;
    std::vector<Limb> limbs;
};

int main() {
    hazelcast::client::ClientConfig config;
    hazelcast::client::HazelcastClient hz(config);

    hazelcast::client::IMap<int, Person> map =
            hz.getMap<int, Person>("map");

    std::vector<Limb> legs;
    legs.push_back(Limb("left-leg"));
    legs.push_back(Limb("right-leg"));

    std::vector<Limb> hands;
    hands.push_back(Limb("left-hand"));
    hands.push_back(Limb("right-hand"));

    std::vector<Limb> arms;
    hands.push_back(Limb("left-arm"));
    hands.push_back(Limb("right-arm"));

    map.put(1, Person("Georg", legs));
    map.put(2, Person("Peter", hands));
    map.put(3, Person("Hans", legs));
    map.put(4, Person("Stefanie", arms));

    hazelcast::client::query::SqlPredicate predicate("limbs[any].name == right-leg");
    std::vector<Person> employees = map.values(predicate);

    std::cout << "People:" << std::endl;
    for (std::vector<Person>::const_iterator it = employees.begin(); it != employees.end(); ++it) {
        std::cout << (*it).getName() << ", Limbs are:{";
        const std::vector<Limb> &values = it->getLimbs();
        for (std::vector<Limb>::const_iterator valIt = values.begin(); valIt != values.end();) {
            std::cout << valIt->getName();
            ++valIt;
            if (valIt != values.end()) {
                std::cout << ", ";
            }
        }
        std::cout << "}, ";
    }
    std::cout << std::endl;

    std::cout << "Finished" << std::endl;

    return 0;
}
