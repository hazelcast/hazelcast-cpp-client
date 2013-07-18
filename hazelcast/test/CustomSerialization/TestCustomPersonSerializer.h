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

    void write(hazelcast::client::serialization::BufferedDataOutput & out, const TestCustomPerson& object) {
        out.writeInt(999);
        out.writeUTF(object.getName());
        out.writeInt(999);
    };

    void read(hazelcast::client::serialization::BufferedDataInput & in, TestCustomPerson& object) {
        int i = in.readInt();
        assert(i == 999);
        object.setName(in.readUTF());
        i = in.readInt();
        assert(i == 999);
    };

    int getTypeId() const {
        return 999;
    };

};


#endif //HAZELCAST_TestCustomSerializableWithBase

