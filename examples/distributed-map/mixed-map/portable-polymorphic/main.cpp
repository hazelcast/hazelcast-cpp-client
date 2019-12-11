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
#include <hazelcast/client/serialization/PortableWriter.h>
#include <hazelcast/client/serialization/PortableReader.h>
#include <hazelcast/client/adaptor/RawPointerMap.h>

using namespace hazelcast::client;

class BasePortable : public serialization::Portable {
public:
    virtual ~BasePortable() {}

    virtual int getFactoryId() const {
        return 666;
    }

    virtual int getClassId() const {
        return 3;
    }

    virtual void writePortable(serialization::PortableWriter &writer) const {
    }

    virtual void readPortable(serialization::PortableReader &reader) {
    }

    bool operator<(const BasePortable &rhs) const {
        return getClassId() < rhs.getClassId();
    }
};

class Derived1Portable : public BasePortable {
public:
    virtual int getClassId() const {
        return 4;
    }
};

class Derived2Portable : public BasePortable {
public:
    virtual int getClassId() const {
        return 5;
    }
};

class PolymorphicPortableFactory : public serialization::PortableFactory {
public:
    virtual std::auto_ptr<serialization::Portable> create(int32_t classId) const {
        switch (classId) {
            case 3:
                return std::auto_ptr<serialization::Portable>(new BasePortable);
            case 4:
                return std::auto_ptr<serialization::Portable>(new Derived1Portable);
            case 5:
                return std::auto_ptr<serialization::Portable>(new Derived2Portable);
            default:
                return std::auto_ptr<serialization::Portable>();
        }
    }
};

int main() {
    ClientConfig config;
    SerializationConfig &serializationConfig = config.getSerializationConfig();
    serializationConfig.addPortableFactory(666, boost::shared_ptr<serialization::PortableFactory>(
                                                           new PolymorphicPortableFactory));

    HazelcastClient client(config);

    IMap<int, BasePortable> map(client.getMap<int, BasePortable>("MyMap"));
    adaptor::RawPointerMap<int, BasePortable> rawPointerMap(map);

    BasePortable baseCustom;
    Derived1Portable derived1Portable;
    Derived2Portable derived2Portable;
    map.put(1, baseCustom);
    map.put(2, derived1Portable);
    map.put(3, derived2Portable);

    std::auto_ptr<BasePortable> value = rawPointerMap.get(3);
    std::cout << "Got value for key 3. Class Id:" << value->getClassId() << std::endl;

    std::cout << "Finished" << std::endl;
}

