//
// Created by sancar koyunlu on 7/1/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_XmlSerializer
#define HAZELCAST_XmlSerializer

#include <assert.h>
#include "Serializer.h"
#include "ObjectDataOutput.h"
#include "ObjectDataInput.h"

template<typename T>
class TestCustomSerializerX : public hazelcast::client::serialization::Serializer<T> {
public:

    void write(hazelcast::client::serialization::ObjectDataOutput & out, const T& object) {
        out.writeInt(666);
        out.writeInt(object.id);
        out.writeInt(666);
    }

    void read(hazelcast::client::serialization::ObjectDataInput & in, T& object) {
        int i = in.readInt();
        assert(i == 666);
        object.id = in.readInt();
        i = in.readInt();
        assert(i == 666);
    }

    int getTypeId() const{
        return 666;
    };
};


#endif //HAZELCAST_XmlSerializer
