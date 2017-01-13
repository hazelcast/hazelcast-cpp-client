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
#include <hazelcast/client/serialization/IdentifiedDataSerializable.h>
#include <memory>
#include <map>
#include <hazelcast/client/query/SqlPredicate.h>

class Car : public hazelcast::client::serialization::IdentifiedDataSerializable {
public:
    Car() { }

    Car(const char *name) {
        attributes["name"] = name;
        attributes["tripStart"] = "0";
        attributes["tripStop"] = "0";
    }

    Car(const char *name, int breakHorsePower, int mileage) {
        attributes["name"] = name;
        attributes["tripStart"] = "0";
        attributes["tripStop"] = "0";
        char buf[50];
        hazelcast::util::snprintf(buf, 50, "%d", breakHorsePower);
        attributes["bhp"] = buf;
        hazelcast::util::snprintf(buf, 50, "%d", mileage);
        attributes["mileage"] = buf;
    }

    const std::string &getAttribute(const char *name) {
        return attributes[name];
    }

    int getFactoryId() const {
        return 1;
    }

    int getClassId() const {
        return 4;
    }

    void writeData(hazelcast::client::serialization::ObjectDataOutput &out) const {
        out.writeInt((int) attributes.size());
        for (std::map<std::string, std::string>::const_iterator it = attributes.begin();
             it != attributes.end(); ++it) {
            out.writeUTF(&it->first);
            out.writeUTF(&it->second);
        }
    }

    void readData(hazelcast::client::serialization::ObjectDataInput &in) {
        int size = in.readInt();
        if (size > 0) {
            for (int i = 0; i < size; ++i) {
                std::auto_ptr<std::string> key = in.readUTF();
                std::auto_ptr<std::string> value = in.readUTF();
                attributes[*key] = *value;
            }
        } else {
            attributes.clear();
        }
    }

    const std::map<std::string, std::string> &getAttributes() const {
        return attributes;
    }

private:
    std::map<std::string, std::string> attributes;
};

std::ostream &operator<<(std::ostream &out, const Car &c) {
    out << "Car{"
    << "attributes={";

    const std::map<std::string, std::string> &attrs = c.getAttributes();
    for (std::map<std::string, std::string>::const_iterator it = attrs.begin();
         it != attrs.end();) {
        out << "(" << it->first << ", " << it->second << ")";
        ++it;
        if (it != attrs.end()) {
            out << ", ";
        }
    }

    out << "}";

    return out;
}

int main() {
    hazelcast::client::ClientConfig config;
    hazelcast::client::HazelcastClient hz(config);

    hazelcast::client::IMap<int, Car> map = hz.getMap<int, Car>("cars");

    map.put(1, Car("Audi Q7", 250, 22000));
    map.put(2, Car("BMW X5", 312, 34000));
    map.put(3, Car("Porsche Cayenne", 408, 57000));

    // we're using a custom attribute called 'attribute' which is provided by the 'CarAttributeExtractor'
    // we are also passing an argument 'mileage' to the extractor
    hazelcast::client::query::SqlPredicate criteria("attribute[mileage] < 30000");
    std::vector<Car> cars = map.values(criteria);

    for (std::vector<Car>::const_iterator it = cars.begin(); it != cars.end(); ++it) {
        std::cout << (*it) << std::endl;
    }

    std::cout << "Finished" << std::endl;

    return 0;
}

