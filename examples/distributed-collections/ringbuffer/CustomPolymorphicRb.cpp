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
#include <hazelcast/client/HazelcastClient.h>
#include <hazelcast/client/adaptor/RawPointerMap.h>

using namespace hazelcast::client;

class BaseCustom {
public:
    BaseCustom() {
        value = 3;
    }

    BaseCustom(int value) : value(value) {}

    int getValue() const {
        return value;
    }

    void setValue(int value) {
        BaseCustom::value = value;
    }

    bool operator<(const BaseCustom &rhs) const {
        return getValue() < rhs.getValue();
    }

private:
    int value;
};

class Derived1Custom : public BaseCustom {
public:
    Derived1Custom() : BaseCustom(4) {}

    Derived1Custom(int value) : BaseCustom(value) {}
};

class Derived2Custom : public Derived1Custom {
public:
    Derived2Custom() : Derived1Custom(5) {}
};

int32_t getHazelcastTypeId(const BaseCustom *) {
    return 3;
}

int32_t getHazelcastTypeId(const Derived1Custom *) {
    return 4;
}

int32_t getHazelcastTypeId(const Derived2Custom *) {
    return 5;
}

class BaseCustomSerializer : public serialization::Serializer<BaseCustom> {
public:
    virtual int32_t getHazelcastTypeId() const {
        return 3;
    }

    virtual void write(serialization::ObjectDataOutput &out, const BaseCustom &object) {
        out.writeInt(object.getValue());
    }

    virtual void read(serialization::ObjectDataInput &in, BaseCustom &object) {
        object.setValue(in.readInt());
    }

    virtual void *create(serialization::ObjectDataInput &in) {
        return new BaseCustom;
    }
};

class Derived1CustomSerializer : public BaseCustomSerializer {
public:
    virtual int32_t getHazelcastTypeId() const {
        return 4;
    }

    virtual void *create(serialization::ObjectDataInput &in) {
        return new Derived1Custom;
    }
};

class Derived2CustomSerializer : public BaseCustomSerializer {
public:
    virtual int32_t getHazelcastTypeId() const {
        return 5;
    }

    virtual void *create(serialization::ObjectDataInput &in) {
        return new Derived2Custom;
    }
};

int main() {
    ClientConfig config;
    SerializationConfig &serializationConfig = config.getSerializationConfig();
    serializationConfig.registerSerializer(
            std::shared_ptr<serialization::SerializerBase>(new BaseCustomSerializer));

    serializationConfig.registerSerializer(
            std::shared_ptr<serialization::SerializerBase>(new Derived1CustomSerializer));

    serializationConfig.registerSerializer(
            std::shared_ptr<serialization::SerializerBase>(new Derived2CustomSerializer));

    HazelcastClient client(config);

    std::shared_ptr<Ringbuffer<BaseCustom> > ringBuffer = client.getRingbuffer<BaseCustom>("MyRingBuffer");

    BaseCustom baseCustom;
    Derived1Custom derived1Custom;
    Derived2Custom derived2Custom;
    ringBuffer->add(baseCustom);
    ringBuffer->add(derived1Custom);
    ringBuffer->add(derived2Custom);

    int64_t sequence = ringBuffer->headSequence();
    std::auto_ptr<BaseCustom> value = ringBuffer->readOne(sequence);
    std::cout << "Got the first value from the ringbuffer. The value is:" << value->getValue() << std::endl;

    value = ringBuffer->readOne(sequence + 1);
    std::cout << "Got the second value from the ringbuffer. The value is:" << value->getValue() << std::endl;

    value = ringBuffer->readOne(sequence + 2);
    std::cout << "Got the third value from the ringbuffer. The value is:" << value->getValue() << std::endl;

    std::cout << "Finished" << std::endl;

    return 0;
}

