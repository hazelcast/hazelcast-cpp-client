//
// Created by sancar koyunlu on 5/12/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.



#ifndef __TestMobile_H_
#define __TestMobile_H_

#include "hazelcast/client/serialization/SerializationConstants.h"
#include "ObjectDataOutput.h"
#include "ObjectDataInput.h"
#include "IdentifiedDataSerializable.h"


class TestDataSerializable : public hazelcast::client::IdentifiedDataSerializable {
public:
    TestDataSerializable() {

    }

    TestDataSerializable(int i, char c):i(i), c(c) {

    }

    bool operator ==(const TestDataSerializable & rhs) const {
        if (this == &rhs)
            return true;
        if (i != rhs.i) return false;
        if (c != rhs.c) return false;
        return true;
    };

    bool operator !=(const TestDataSerializable& m) const {
        return !(*this == m);
    };

    inline int getFactoryId() const {
        return 1;
    }

    inline int getClassId() const {
        return 1;
    }

    inline void writeData(ObjectDataOutput& writer) const {
        writer.writeChar(c);
        writer.writeInt(i);
    };

    inline void readData(ObjectDataInput& reader) {
        c = reader.readChar();
        i = reader.readInt();
    };

    int i;
    char c;
};

#endif //__TestMobile_H_
