//
//  TestMainPortable.h
//  Server
//
//  Created by sancar koyunlu on 1/11/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef Server_TestMainPortable_h
#define Server_TestMainPortable_h


#include <iostream>
#include "hazelcast/client/serialization/PortableReader.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/serialization/Portable.h"
#include "TestInnerPortable.h"

using namespace hazelcast::client::serialization;

class TestMainPortable : public Portable {
public:

    TestMainPortable() {
    };

    TestMainPortable(const TestMainPortable& rhs) {
        *this = rhs;
    }

    ~TestMainPortable() {
    }

    TestMainPortable(byte b, bool boolean, char c, short s, int i, long l, float f, double d, string str, TestInnerPortable p) {
        this->b = b;
        this->boolean = boolean;
        this->c = c;
        this->s = s;
        this->i = i;
        this->l = l;
        this->f = f;
        this->d = d;
        this->str = str;
        this->p = p;
    };

    const TestMainPortable& operator = (const TestMainPortable& rhs) {
        b = rhs.b;
        boolean = rhs.boolean;
        c = rhs.c;
        s = rhs.s;
        i = rhs.i;
        l = rhs.l;
        f = rhs.f;
        d = rhs.d;
        str = rhs.str;
        p = rhs.p;
        return (*this);
    };


    int getClassId() {
        return 1;
    };

    int getFactoryId() {
        return 1;
    };

    void writePortable(PortableWriter& writer) {
        writer.writeByte("b", b);
        writer.writeBoolean("bool", boolean);
        writer.writeChar("c", c);
        writer.writeShort("s", s);
        writer.writeInt("i", i);
        writer.writeLong("l", l);
        writer.writeFloat("f", f);
        writer.writeDouble("d", d);
        writer.writeUTF("str", str);
        writer.writePortable("p", p);

    };

    void readPortable(PortableReader& reader) {
        b = reader.readByte("b");
        boolean = reader.readBoolean("bool");
        c = reader.readChar("c");
        s = reader.readShort("s");
        i = reader.readInt("i");
        l = reader.readLong("l");
        f = reader.readFloat("f");
        d = reader.readDouble("d");
        str = reader.readUTF("str");
        p = reader.readPortable<TestInnerPortable>("p");

    };

    bool operator ==(TestMainPortable& m) {
        if (this == &m) return true;

        if (b != m.b) return false;
        if (boolean != m.boolean) return false;
        if (c != m.c) return false;
        if (s != m.s) return false;
        if (i != m.i) return false;
        if (l != m.l) return false;
        if (f != m.f) return false;
        if (d != m.d) return false;
        if (str.compare(m.str)) return false;
        if (p != m.p) return false;
        return true;
    };

    bool operator !=(TestMainPortable& m) {
        return !(*this == m);
    };
    TestInnerPortable p;
    int i;
private:
    byte b;
    bool boolean;
    char c;
    short s;
    long l;
    float f;
    double d;
    string str;
};

#endif
