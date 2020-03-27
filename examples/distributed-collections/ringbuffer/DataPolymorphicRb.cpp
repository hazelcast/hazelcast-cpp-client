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
    virtual std::unique_ptr<serialization::IdentifiedDataSerializable> create(int32_t typeId) {
        switch (typeId) {
            case 10:
                return std::unique_ptr<serialization::IdentifiedDataSerializable>(new BaseDataSerializable);
            case 11:
                return std::unique_ptr<serialization::IdentifiedDataSerializable>(new Derived1DataSerializable);
            case 12:
                return std::unique_ptr<serialization::IdentifiedDataSerializable>(new Derived2DataSerializable);
            default:
                return std::unique_ptr<serialization::IdentifiedDataSerializable>();
        }
    }
};

int main() {
    ClientConfig config;
    SerializationConfig &serializationConfig = config.getSerializationConfig();
    serializationConfig.addDataSerializableFactory(666,
                                                   std::shared_ptr<serialization::DataSerializableFactory>(
                                                           new PolymorphicDataSerializableFactory()));

    HazelcastClient client(config);

    std::shared_ptr<Ringbuffer<BaseDataSerializable> > ringBuffer =
            client.getRingbuffer<BaseDataSerializable>("MyRingBuffer");

    BaseDataSerializable base;
    Derived1DataSerializable derived1;
    Derived2DataSerializable derived2;
    ringBuffer->add(base);
    ringBuffer->add(derived1);
    ringBuffer->add(derived2);

    int64_t sequence = ringBuffer->headSequence();
    std::unique_ptr<BaseDataSerializable> value = ringBuffer->readOne(sequence);
    std::cout << "Got the first value from the ringbuffer. The class id is:" << value->getClassId() << std::endl;

    value = ringBuffer->readOne(sequence + 1);
    std::cout << "Got the second value from the ringbuffer. The class id is:" << value->getClassId() << std::endl;

    value = ringBuffer->readOne(sequence + 2);
    std::cout << "Got the third value from the ringbuffer. The class id is:" << value->getClassId() << std::endl;

    std::cout << "Finished" << std::endl;

    return 0;
}

