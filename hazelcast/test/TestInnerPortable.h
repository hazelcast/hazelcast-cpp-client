//
//  TestInnerPortable.h
//  Server
//
//  Created by sancar koyunlu on 1/11/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef Server_TestInnerPortable_h
#define Server_TestInnerPortable_h

#include "Portable.h"
#include "TestNamedPortable.h"
#include <vector>

using namespace hazelcast::client;

class TestInnerPortable : public Portable {
    template<typename HzWriter>
    friend void hazelcast::client::serialization::writePortable(HzWriter& writer, const TestInnerPortable& data);

    template<typename HzReader>
    friend void hazelcast::client::serialization::readPortable(HzReader& reader, TestInnerPortable& data);

public:
    TestInnerPortable() {
    };

    TestInnerPortable(const TestInnerPortable& rhs) {
        *this = rhs;
    }

    TestInnerPortable(std::vector<byte> b,
            std::vector<char> c,
            std::vector<short> s,
            std::vector<int> i,
            std::vector<long> l,
            std::vector<float> f,
            std::vector<double> d,
            std::vector<TestNamedPortable> n):bb(b), cc(c), ss(s), ii(i), ll(l), ff(f), dd(d), nn(n) {
    };

    TestInnerPortable& operator = (const TestInnerPortable& rhs) {
        bb = rhs.bb;
        cc = rhs.cc;
        ss = rhs.ss;
        ii = rhs.ii;
        ll = rhs.ll;
        ff = rhs.ff;
        dd = rhs.dd;
        nn = rhs.nn;
        return (*this);
    }

    int getClassId() {
        return 2;
    };

    int getFactoryId() {
        return 1;
    };

    ~TestInnerPortable() {
    }


    bool operator ==(const TestInnerPortable& m) const {
        if (bb != m.bb) return false;
        if (cc != m.cc) return false;
        if (ss != m.ss) return false;
        if (ii != m.ii) return false;
        if (ll != m.ll) return false;
        if (ff != m.ff) return false;
        if (dd != m.dd) return false;
        for (int i = 0; i < nn.size(); i++)
            if (nn[i] != m.nn[i])
                return false;
        return true;
    };


    bool operator !=(const TestInnerPortable& m) const {
        return !(*this == m);
    };

    std::vector<int> ii;
private:
    std::vector<byte> bb;
    std::vector<char> cc;
    std::vector<short> ss;
    std::vector<long> ll;
    std::vector<float> ff;
    std::vector<double> dd;
    std::vector< TestNamedPortable > nn;

};
namespace hazelcast {
    namespace client {
        namespace serialization {

//            inline int getSerializerId(const TestInnerPortable& x) {
//                return SerializationConstants::CONSTANT_TYPE_PORTABLE;
//            };

            inline int getFactoryId(const TestInnerPortable& t) {
                return 1;
            }

            inline int getClassId(const TestInnerPortable& t) {
                return 2;
            }

            template<typename HzWriter>
            inline void writePortable(HzWriter& writer, const TestInnerPortable& data) {
                writer.writeByteArray("b", data.bb);
                writer.writeCharArray("c", data.cc);
                writer.writeShortArray("s", data.ss);
                writer.writeIntArray("i", data.ii);
                writer.writeLongArray("l", data.ll);
                writer.writeFloatArray("f", data.ff);
                writer.writeDoubleArray("d", data.dd);
                writer.writePortableArray("nn", data.nn);
            };

            template<typename HzReader>
            inline void readPortable(HzReader& reader, TestInnerPortable& data) {
                data.bb = reader.readByteArray("b");
                data.cc = reader.readCharArray("c");
                data.ss = reader.readShortArray("s");
                data.ii = reader.readIntArray("i");
                data.ll = reader.readLongArray("l");
                data.ff = reader.readFloatArray("f");
                data.dd = reader.readDoubleArray("d");
                data.nn = reader.template readPortableArray<TestNamedPortable>("nn");
            };
        }
    }
}
#endif
