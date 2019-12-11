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

class Value : public hazelcast::client::serialization::IdentifiedDataSerializable {
public:
    int amount;

    Value() {
    }

    Value(const Value &rhs) {
        amount = rhs.amount;
    }

    bool equals(const Value &rhs) {
        return rhs.amount == amount;
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
    }

    void readData(hazelcast::client::serialization::ObjectDataInput &in) {
        amount = in.readInt();
    }
};

int main() {
    hazelcast::client::HazelcastClient hz;

    hazelcast::client::IMap<std::string, Value> map =
            hz.getMap<std::string, Value>("map");

    std::string key("1");
    Value v;
    map.put(key, v);
    std::cout << "Starting" << std::endl;
    for (int k = 0; k < 1000; k++) {
        if (k % 100 == 0) {
            std::cout << "At: " << k << std::endl;
        }

        boost::shared_ptr<Value> oldValue = map.get(key);
        hazelcast::util::sleepmillis(10);
        map.put(key, *oldValue);
    }
    std::cout << "Finished! Result = " << map.get(key)->amount << std::endl;

    std::cout << "Finished" << std::endl;

    return 0;
}
