/*
 * Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
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
#include <hazelcast/client/HazelcastAll.h>

using namespace hazelcast::client;

class SampleDataSerializableFactory;

class Employee : public serialization::IdentifiedDataSerializable {
public:
    static const int TYPE_ID = 100;

    virtual int getFactoryId() const {
        return 1000;
    }

    virtual int getClassId() const {
        return TYPE_ID;
    }

    virtual void writeData(serialization::ObjectDataOutput &writer) const {
        writer.writeInt(id);
        writer.writeUTF(&name);
    }

    virtual void readData(serialization::ObjectDataInput &reader) {
        id = reader.readInt();
        name = *reader.readUTF();
    }

private:
    int id;
    std::string name;
};

class SampleDataSerializableFactory : public serialization::DataSerializableFactory {
public:
    static const int FACTORY_ID = 1000;

    virtual std::unique_ptr<serialization::IdentifiedDataSerializable> create(int32_t classId) {
        switch (classId) {
            case 100:
                return std::unique_ptr<serialization::IdentifiedDataSerializable>(new Employee());
            default:
                return std::unique_ptr<serialization::IdentifiedDataSerializable>();
        }

    }
};

int main() {
    ClientConfig clientConfig;
    clientConfig.getSerializationConfig().addDataSerializableFactory(SampleDataSerializableFactory::FACTORY_ID,
                                                                     std::shared_ptr<serialization::DataSerializableFactory>(
                                                                             new SampleDataSerializableFactory()));

    HazelcastClient hz(clientConfig);
    //Employee can be used here
    hz.shutdown();

    return 0;
}
