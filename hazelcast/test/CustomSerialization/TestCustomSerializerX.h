//
// Created by sancar koyunlu on 7/1/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_XmlSerializer
#define HAZELCAST_XmlSerializer

#include <assert.h>
#include "Serializer.h"
#include "BufferedDataOutput.h"
#include "BufferedDataInput.h"

template<typename T>
class TestCustomSerializerX : public hazelcast::client::serialization::Serializer<T> {
public:

    void write(hazelcast::client::serialization::BufferedDataOutput & out, const T& object) {
        out.writeInt(666);
        out.writeUTF(object.name);
        out.writeInt(666);
    }

    void read(hazelcast::client::serialization::BufferedDataInput & in, T& object) {
        int i = in.readInt();
        assert(i == 666);
        object.name = in.readUTF();
        i = in.readInt();
        assert(i == 666);
    }

    int getTypeId() const{
        return 666;
    };
};


#endif //HAZELCAST_XmlSerializer
