//
// Created by sancar koyunlu on 7/1/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_TestCustomSerializableWithBase
#define HAZELCAST_TestCustomSerializableWithBase

#include "BufferedDataOutput.h"
#include "Serializer.h"
#include "BufferedDataInput.h"
#include "TestCustomXSerializable.h"
#include <assert.h>
#include <iostream>


class TestCustomPersonSerializer : public hazelcast::client::serialization::Serializer<TestCustomPerson> {
public:

    template<typename T>
    void write(hazelcast::client::serialization::BufferedDataOutput & out, const T& object) {
//        std::cerr << ">" << typeid(object).name() << std::endl;
        out.writeInt(999);
        out.writeUTF(object.getName());
        privateWrite(out, object);
        out.writeInt(999);
    };

    template<typename T>
    void read(hazelcast::client::serialization::BufferedDataInput & in, T& object) {
//        std::cerr << "<" << typeid(object).name() << std::endl;
        int i = in.readInt();
        assert(i == 999);
        object.setName(in.readUTF());
        i = in.readInt();
        assert(i == 999);
    };

    void privateWrite(hazelcast::client::serialization::BufferedDataOutput & out, const TestCustomStudent& object) {
        out.writeInt(object.id);
    };

    void privateWrite(hazelcast::client::serialization::BufferedDataOutput & out, const TestCustomTwoNamedPerson& object) {
        out.writeUTF(object.surname);

    };

    void privateRead(hazelcast::client::serialization::BufferedDataInput & in, TestCustomStudent& object) {
        object.id = in.readInt();
    };

    void privateRead(hazelcast::client::serialization::BufferedDataInput & in, TestCustomTwoNamedPerson& object) {
        object.surname = in.readUTF();
    };

    int getTypeId() const {
        return 999;
    };

};


#endif //HAZELCAST_TestCustomSerializableWithBase

