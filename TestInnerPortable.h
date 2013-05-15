//
//  TestInnerPortable.h
//  Server
//
//  Created by sancar koyunlu on 1/11/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef Server_TestInnerPortable_h
#define Server_TestInnerPortable_h

#include "hazelcast/client/serialization/Portable.h"
#include "TestNamedPortable.h"
#include <vector>

using namespace hazelcast::client;

class TestInnerPortable : public Portable {
    template<typename HzWriter>
    friend void writePortable(HzWriter& writer, const TestInnerPortable& data);

    template<typename HzReader>
    friend void readPortable(HzReader& reader, TestInnerPortable& data);

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


    bool operator ==(TestInnerPortable& m) {
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


    bool operator !=(TestInnerPortable& m) {
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

inline int getFactoryId(const TestInnerPortable& t) {
    return 1;
}

inline int getClassId(const TestInnerPortable& t) {
    return 2;
}

template<typename HzWriter>
inline void writePortable(HzWriter& writer, const TestInnerPortable& data) {
    writer["b"] << data.bb;
    writer["c"] << data.cc;
    writer["s"] << data.ss;
    writer["i"] << data.ii;
    writer["l"] << data.ll;
    writer["f"] << data.ff;
    writer["d"] << data.dd;
    writer["nn"] << data.nn;
};

template<typename HzReader>
inline void readPortable(HzReader& reader, TestInnerPortable& data) {
    reader["b"] >> data.bb;
    reader["c"] >> data.cc;
    reader["s"] >> data.ss;
    reader["i"] >> data.ii;
    reader["l"] >> data.ll;
    reader["f"] >> data.ff;
    reader["d"] >> data.dd;
    reader["nn"] >> data.nn;
};

#endif
