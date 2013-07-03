//
//  TestMainPortable.h
//  Server
//
//  Created by sancar koyunlu on 1/11/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef Server_TestMainPortable_h
#define Server_TestMainPortable_h

#include "Portable.h"
#include "TestInnerPortable.h"
#include <string>

using namespace hazelcast::client::serialization;

class TestMainPortable : public hazelcast::client::Portable {
public:

    TestMainPortable():null(true) {
    };

    TestMainPortable(const TestMainPortable& rhs) {
        *this = rhs;
    }

    ~TestMainPortable() {
    }

    TestMainPortable(byte b, bool boolean, char c, short s, int i, long l, float f, double d, std::string str, TestInnerPortable p) {
        null = false;
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
        null = rhs.null;
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


    bool operator ==(const TestMainPortable& m) const {
        if (this == &m) return true;
        if (null == true && m.null == true)
            return true;
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

    bool operator !=(const TestMainPortable& m) const {
        return !(*this == m);
    };

    inline int getFactoryId() const{
        return 1;
    }

    inline int getClassId()const {
        return 1;
    }

    template<typename HzWriter>
    inline void writePortable(HzWriter& writer) const{
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

    template<typename HzReader>
    inline void readPortable(HzReader& reader) {
        null = false;
        b = reader.readByte("b");
        boolean = reader.readBoolean("bool");
        c = reader.readChar("c");
        s = reader.readShort("s");
        i = reader.readInt("i");
        l = reader.readLong("l");
        f = reader.readFloat("f");
        d = reader.readDouble("d");
        str = reader.readUTF("str");
        p = reader.template readPortable<TestInnerPortable>("p");
    };

    TestInnerPortable p;
    int i;
private:
    bool null;
    byte b;
    bool boolean;
    char c;
    short s;
    long l;
    float f;
    double d;
    std::string str;
};

#endif
