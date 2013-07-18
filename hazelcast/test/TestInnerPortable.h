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

    int getClassId() const {
        return 2;
    };

    int getFactoryId() const {
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

    template<typename HzWriter>
    void writePortable(HzWriter& writer) const {
        writer.writeByteArray("b", bb);
        writer.writeCharArray("c", cc);
        writer.writeShortArray("s", ss);
        writer.writeIntArray("i", ii);
        writer.writeLongArray("l", ll);
        writer.writeFloatArray("f", ff);
        writer.writeDoubleArray("d", dd);
        writer.writePortableArray("nn", nn);
    };

    template<typename HzReader>
    void readPortable(HzReader& reader) {
        bb = reader.readByteArray("b");
        cc = reader.readCharArray("c");
        ss = reader.readShortArray("s");
        ii = reader.readIntArray("i");
        ll = reader.readLongArray("l");
        ff = reader.readFloatArray("f");
        dd = reader.readDoubleArray("d");
        nn = reader.template readPortableArray<TestNamedPortable>("nn");
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

#endif
