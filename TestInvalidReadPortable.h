//
// Created by sancar koyunlu on 5/6/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
// To change the template use AppCode | Preferences | File Templates.
//



#ifndef __TestInvalidReadPortable_H_
#define __TestInvalidReadPortable_H_

#include <iostream>
#include "Portable.h"
#include "PortableWriter.h"
#include "PortableReader.h"

using namespace hazelcast::client::serialization;

class TestInvalidReadPortable : public Portable {
public:

    TestInvalidReadPortable() {

    }

    TestInvalidReadPortable(long l, int i, std::string s) {
        this->l = l;
        this->i = i;
        this->s = l;
    }


    int getFactoryId() {
        return 1;
    }

    int getClassId() {
        return 6;
    }

    void writePortable(PortableWriter & writer) {
        writer.writeLong("l", l);
        writer.writeLong("i", i);
        writer.writeUTF("s", s);
    }

    void readPortable(PortableReader & reader) {
        l = reader.readLong("l");
        DataInput *input = reader.getRawDataInput();
        i = reader.readInt("i");
        s = input->readUTF();
    }

    long l;
    int i;
    std::string s;
};


#endif //__TestInvalidReadPortable_H_


