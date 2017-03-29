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

class Value : public hazelcast::client::serialization::IdentifiedDataSerializable {
public:
    int amount;
    int version;

    Value() {
    }

    Value(const Value &rhs) {
        amount = rhs.amount;
        version = rhs.version;
    }

    bool equals(const Value &rhs) {
        return rhs.amount == amount && rhs.version == version;
    }

    bool operator==(const Value &rhs) {
        return equals(rhs);
    }

    int getFactoryId() const {
        return 1;
    }

    int getClassId() const {
        return 6;
    }

    void writeData(hazelcast::client::serialization::ObjectDataOutput &out) const {
        out.writeInt(amount);
        out.writeInt(version);
    }

    void readData(hazelcast::client::serialization::ObjectDataInput &in) {
        amount = in.readInt();
        version = in.readInt();
    }
};

int main() {
    hazelcast::client::ClientConfig config;
    hazelcast::client::HazelcastClient hz(config);

    hazelcast::client::IMap<std::string, Value> map =
            hz.getMap<std::string, Value>("map");

    std::string key("1");
    Value v;
    map.put(key, v);
    std::cout << "Starting" << std::endl;
    for (int k = 0; k < 1000; k++) {
        for (; ;) {
            hazelcast::util::SharedPtr<Value> oldValue = map.get(key);
            Value newValue(*oldValue);
            hazelcast::util::sleepmillis(10);
            newValue.amount++;
            newValue.version++;
            if (map.replace(key, *oldValue, newValue))
                break;
        }
    }
    std::cout << "Finished! Result = " << map.get(key)->amount << std::endl;

    std::cout << "Finished" << std::endl;

    return 0;
}
