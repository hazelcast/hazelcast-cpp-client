//
// Created by sancar koyunlu on 5/6/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
// To change the template use AppCode | Preferences | File Templates.
//



#ifndef __RawDataPortable_H_
#define __RawDataPortable_H_

#include "TestNamedPortable.h"
#include "TestDataSerializable.h"
#include <iostream>


class TestRawDataPortable : public hazelcast::client::Portable {
public:

    long l;
    std::vector<char> c;
    TestNamedPortable p;
    int k;
    std::string s;
    TestDataSerializable ds;

    TestRawDataPortable() {

    };

    inline int getFactoryId() const{
        return 1;
    }

    inline int getClassId()const {
        return 4;
    }

    template<typename HzWriter>
    inline void writePortable(HzWriter& writer) const{
        writer.writeLong("l", l);
        writer.writeCharArray("c", c);
        writer.writePortable("p", p);
        ObjectDataOutput *out = writer.getRawDataOutput();
        out->writeInt(k);
        out->writeUTF(s);
        ds.writeData(*out);
    };

    template<typename HzReader>
    inline void readPortable(HzReader& reader) {
        l = reader.readLong("l");
        c = reader.readCharArray("c");
        p = reader.template readPortable<TestNamedPortable>("p");
        ObjectDataInput *in = reader.getRawDataInput();
        k = in->readInt();
        s = in->readUTF();
        ds.readData(*in);
    };

    TestRawDataPortable(long l, std::vector<char> c, TestNamedPortable p, int k, std::string s, TestDataSerializable ds) {
        this->l = l;
        this->c = c;
        this->p = p;
        this->k = k;
        this->s = s;
        this->ds = ds;
    };

    bool operator ==(const TestRawDataPortable& m) const {
        if (this == &m)
            return true;
        if (l != m.l) return false;
        if (c != m.c) return false;
        if (p != m.p) return false;
        if (k != m.k) return false;
        if (ds != m.ds) return false;
        if (s.compare(m.s) != 0) return false;
        return true;
    };

    bool operator !=(const TestRawDataPortable& m) const {
        return !(*this == m);
    };
};


#endif //__RawDataPortable_H_

