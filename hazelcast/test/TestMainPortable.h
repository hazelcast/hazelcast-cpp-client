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
    template<typename HzWriter>
    friend void hazelcast::client::serialization::writePortable(HzWriter& writer, const TestMainPortable& data);

    template<typename HzReader>
    friend void hazelcast::client::serialization::readPortable(HzReader& reader, TestMainPortable& data);

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

namespace hazelcast {
    namespace client {
        namespace serialization {
//
//            inline int getSerializerId(const TestMainPortable& x) {
//                return SerializationConstants::CONSTANT_TYPE_PORTABLE;
//            };

            inline int getFactoryId(const TestMainPortable& t) {
                return 1;
            }

            inline int getClassId(const TestMainPortable& t) {
                return 1;
            }

            template<typename HzWriter>
            inline void writePortable(HzWriter& writer, const TestMainPortable& data) {
                writer.writeByte("b", data.b);
                writer.writeBoolean("bool", data.boolean);
                writer.writeChar("c", data.c);
                writer.writeShort("s", data.s);
                writer.writeInt("i", data.i);
                writer.writeLong("l", data.l);
                writer.writeFloat("f", data.f);
                writer.writeDouble("d", data.d);
                writer.writeUTF("str", data.str);
                writer.writePortable("p", data.p);
            };

            template<typename HzReader>
            inline void readPortable(HzReader& reader, TestMainPortable& data) {
                data.null = false;
                data.b = reader.readByte("b");
                data.boolean = reader.readBoolean("bool");
                data.c = reader.readChar("c");
                data.s = reader.readShort("s");
                data.i = reader.readInt("i");
                data.l = reader.readLong("l");
                data.f = reader.readFloat("f");
                data.d = reader.readDouble("d");
                data.str = reader.readUTF("str");
                data.p = reader.template readPortable<TestInnerPortable>("p");
            };
        }
    }
}
#endif
