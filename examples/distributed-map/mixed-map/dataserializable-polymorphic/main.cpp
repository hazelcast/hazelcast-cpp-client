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
#include <hazelcast/client/adaptor/RawPointerMap.h>

using namespace hazelcast::client;

class BaseDataSerializable : public serialization::IdentifiedDataSerializable {
public:
    virtual ~BaseDataSerializable() {}

    virtual int getFactoryId() const {
        return 666;
    }

    virtual int getClassId() const {
        return 10;
    }

    virtual void writeData(serialization::ObjectDataOutput &writer) const {
    }

    virtual void readData(serialization::ObjectDataInput &reader) {
    }

    virtual bool operator<(const BaseDataSerializable &rhs) const {
        return getClassId() < rhs.getClassId();
    }
};

class Derived1DataSerializable : public BaseDataSerializable {
public:
    virtual int getClassId() const {
        return 11;
    }
};

class Derived2DataSerializable : public Derived1DataSerializable {
public:
    virtual int getClassId() const {
        return 12;
    }
};

class PolymorphicDataSerializableFactory : public serialization::DataSerializableFactory {
public:
    virtual std::auto_ptr<serialization::IdentifiedDataSerializable> create(int32_t typeId) {
        switch (typeId) {
            case 10:
                return std::auto_ptr<serialization::IdentifiedDataSerializable>(new BaseDataSerializable);
            case 11:
                return std::auto_ptr<serialization::IdentifiedDataSerializable>(new Derived1DataSerializable);
            case 12:
                return std::auto_ptr<serialization::IdentifiedDataSerializable>(new Derived2DataSerializable);
            default:
                return std::auto_ptr<serialization::IdentifiedDataSerializable>();
        }
    }
};

int main() {
    ClientConfig config;
    SerializationConfig &serializationConfig = config.getSerializationConfig();
    serializationConfig.addDataSerializableFactory(666,
                                                   boost::shared_ptr<serialization::DataSerializableFactory>(
                                                           new PolymorphicDataSerializableFactory()));

    HazelcastClient client(config);

    IMap<int, BaseDataSerializable> imapCustom(client.getMap<int, BaseDataSerializable>("MyMap"));
    adaptor::RawPointerMap<int, BaseDataSerializable> rawPointerMap(imapCustom);

    BaseDataSerializable base;
    Derived1DataSerializable derived1;
    Derived2DataSerializable derived2;
    rawPointerMap.put(1, base);
    rawPointerMap.put(2, derived1);
    rawPointerMap.put(3, derived2);

    std::auto_ptr<BaseDataSerializable> value = rawPointerMap.get(3);
    std::cout << "Got the value for key 3. The value class id is:" << value->getClassId() << std::endl;

    std::set<int> keys;
    keys.insert(1);
    keys.insert(2);
    keys.insert(3);

    std::auto_ptr<EntryArray<int, BaseDataSerializable> > entries = rawPointerMap.getAll(keys);
    size_t numberOfEntries = entries->size();
    std::cout << "Got " << numberOfEntries << " entries from the map." << std::endl;
    for (size_t i = 0; i < numberOfEntries; ++i) {
        std::pair<const int *, const BaseDataSerializable *> entry = (*entries)[i];
        std::cout << "Entry " << i << ": (" << *entry.first << ", " << entry.second->getClassId() << ")" << std::endl;
    }

    std::cout << "Finished" << std::endl;

    return 0;
}

