//
//  TestMainPortable.h
//  Server
//
//  Created by sancar koyunlu on 1/11/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef Server_TestMainPortable_h
#define Server_TestMainPortable_h

#include "TestInnerPortable.h"

#include <string>

using namespace hazelcast::client::serialization;

class TestMainPortable {
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

    bool operator ==(TestMainPortable& m) {
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

    bool operator !=(TestMainPortable& m) {
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
            inline int getFactoryId(const TestMainPortable& t) {
                return 1;
            }

            inline int getClassId(const TestMainPortable& t) {
                return 1;
            }

            template<typename HzWriter>
            inline void writePortable(HzWriter& writer, const TestMainPortable& data) {
                writer["b"] << data.b;
                writer["bool"] << data.boolean;
                writer["c"] << data.c;
                writer["s"] << data.s;
                writer["i"] << data.i;
                writer["l"] << data.l;
                writer["f"] << data.f;
                writer["d"] << data.d;
                writer["str"] << data.str;
                writer["p"] << data.p;
            };

            template<typename HzReader>
            inline void readPortable(HzReader& reader, TestMainPortable& data) {
                data.null = false;
                reader["b"] >> data.b;
                reader["bool"] >> data.boolean;
                reader["c"] >> data.c;
                reader["s"] >> data.s;
                reader["i"] >> data.i;
                reader["l"] >> data.l;
                reader["f"] >> data.f;
                reader["d"] >> data.d;
                reader["str"] >> data.str;
                reader["p"] >> data.p;
            };
        }
    }
}
#endif
