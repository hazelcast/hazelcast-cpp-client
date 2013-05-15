#ifndef __Server__TestNamedPortableV2__
#define __Server__TestNamedPortableV2__


#include <iostream>
#include "hazelcast/client/serialization/PortableReader.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/serialization/Portable.h"
#include "TestNamedPortable.h"

using namespace hazelcast::client::serialization;

class TestNamedPortableV2 : public TestNamedPortable {
public:
    TestNamedPortableV2() {

    };

    TestNamedPortableV2(std::string name, int v) : TestNamedPortable(name, v * 10) {
        this->v = v;
    };

//    void writePortable(PortableWriter & writer) {
//        TestNamedPortable::writePortable(writer);
//        writer.writeInt("v", v);
//    };
//
//    void readPortable(PortableReader & reader) {
//        TestNamedPortable::readPortable(reader);
//        v = reader.readInt("v");
//    }

    bool TestNamedPortableV2::operator ==(TestNamedPortableV2 & m) {
        if (TestNamedPortable::operator!=(m)) return false;
        if (v != m.v) return false;
        return true;
    };

    bool TestNamedPortableV2::operator !=(TestNamedPortableV2 & m) {
        return !(*this == m);
    };


    int v;

};

inline int getFactoryId(const TestNamedPortableV2& t) {
    return 1;
}

inline int getClassId(const TestNamedPortableV2& t) {
    return 3;
}

template<typename HzWriter>
inline void writePortable(HzWriter& writer, const TestNamedPortableV2& data) {
    writer << (TestNamedPortable) data;
    writer["v"] << data.v;
};

template<typename HzReader>
inline void readPortable(HzReader& reader, TestNamedPortableV2& data) {
    reader << (TestNamedPortable) data;
    reader["v"] >> data.v;
};
#endif
