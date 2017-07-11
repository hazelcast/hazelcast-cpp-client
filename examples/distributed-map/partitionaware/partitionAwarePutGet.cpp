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
#include <hazelcast/client/PartitionAware.h>

class PartitionAwareString
        : public hazelcast::client::PartitionAware<std::string>,
          public hazelcast::client::serialization::IdentifiedDataSerializable {
public:
    static const std::string desiredPartitionString;

    PartitionAwareString(const std::string &actualKey);

    virtual const std::string *getPartitionKey() const {
        return &desiredPartitionString;
    }

    virtual int getFactoryId() const {
        return 1;
    }

    virtual int getClassId() const {
        return 10;
    }

    virtual void writeData(hazelcast::client::serialization::ObjectDataOutput &writer) const {
        writer.writeUTF(&actualKey);
    }

    virtual void readData(hazelcast::client::serialization::ObjectDataInput &reader) {
        actualKey = *reader.readUTF();
    }

    const std::string &getActualKey() const {
        return actualKey;
    }

private:
    std::string actualKey;
};

const std::string PartitionAwareString::desiredPartitionString = "desiredKeyString";

PartitionAwareString::PartitionAwareString(const std::string &actualKey) : actualKey(actualKey) {}

int main() {
    hazelcast::client::ClientConfig config;
    hazelcast::client::HazelcastClient hz(config);

    PartitionAwareString partitionKey("MyString");

    hazelcast::client::IMap<PartitionAwareString, std::string> map = hz.getMap<PartitionAwareString, std::string>(
            "paritionawaremap");

    // Puts the key, value tokyo at the partition for "desiredKeyString" rather than the partition for "MyString"
    map.put(partitionKey, "Tokyo");
    boost::shared_ptr<std::string> value = map.get(partitionKey);

    std::cout << "Got the value for key " << partitionKey.getActualKey() << ". Value is:" << *value << std::endl;

    std::cout << "Finished" << std::endl;

    return 0;
}
