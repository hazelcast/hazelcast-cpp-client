//
// Created by sancar koyunlu on 5/6/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
// To change the template use AppCode | Preferences | File Templates.
//



#ifndef __RawDataPortable_H_
#define __RawDataPortable_H_

#include <iostream>
#include "TestNamedPortable.h"
#include "TestDataSerializable.h"


class TestRawDataPortable : public Portable {
public:

    long l;
    std::vector<char> c;
    TestNamedPortable p;
    int k;
    std::string s;
    TestDataSerializable ds;

    TestRawDataPortable() {

    }

    TestRawDataPortable(long l, std::vector<char> c, TestNamedPortable p, int k, std::string s, TestDataSerializable ds) {
        this->l = l;
        this->c = c;
        this->p = p;
        this->k = k;
        this->s = s;
        this->ds = ds;
    }

    bool operator ==(TestRawDataPortable& m) {
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

    bool operator !=(TestRawDataPortable& m) {
        return !(*this == m);
    };
};

namespace hazelcast {
    namespace client {
        namespace serialization {

            inline int getFactoryId(const TestRawDataPortable& t) {
                return 1;
            }

            inline int getClassId(const TestRawDataPortable& t) {
                return 4;
            }

            template<typename HzWriter>
            inline void writePortable(HzWriter& writer, const TestRawDataPortable& data) {
                writer["l"] << data.l;
                writer["c"] << data.c;
                writer["p"] << data.p;
                writer << data.k;
                writer << data.s;
                writer << data.ds;
            };

            template<typename HzReader>
            inline void readPortable(HzReader& reader, TestRawDataPortable& data) {
                reader["l"] >> data.l;
                reader["c"] >> data.c;
                reader["p"] >> data.p;
                reader >> data.k;
                reader >> data.s;
                reader >> data.ds;
            };
        }
    }
}


#endif //__RawDataPortable_H_

