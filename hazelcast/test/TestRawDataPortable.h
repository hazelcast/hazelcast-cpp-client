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

namespace hazelcast {
    namespace client {
        namespace serialization {

//            inline int getSerializerId(const TestRawDataPortable& x) {
//                return SerializationConstants::CONSTANT_TYPE_PORTABLE;
//            };

            inline int getFactoryId(const TestRawDataPortable& t) {
                return 1;
            }

            inline int getClassId(const TestRawDataPortable& t) {
                return 4;
            }

            template<typename HzWriter>
            inline void writePortable(HzWriter& writer, const TestRawDataPortable& data) {
                writer.writeLong("l", data.l);
                writer.writeCharArray("c", data.c);
                writer.writePortable("p", data.p);
                BufferedDataOutput *out = writer.getRawDataOutput();
                out->writeInt(data.k);
                out->writeUTF(data.s);
                writeData(*out, data.ds);
            };

            template<typename HzReader>
            inline void readPortable(HzReader& reader, TestRawDataPortable& data) {
                data.l = reader.readLong("l");
                data.c = reader.readCharArray("c");
                data.p = reader.template readPortable<TestNamedPortable>("p");
                BufferedDataInput *in = reader.getRawDataInput();
                data.k = in->readInt();
                data.s = in->readUTF();
                readData(*in, data.ds);
            };
        }
    }
}


#endif //__RawDataPortable_H_

