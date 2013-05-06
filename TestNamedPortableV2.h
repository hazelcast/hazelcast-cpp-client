#ifndef __Server__TestNamedPortableV2__
#define __Server__TestNamedPortableV2__


#include <iostream>
#include "hazelcast/client/serialization/PortableReader.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/serialization/Portable.h"
#include "TestNamedPortable.h"

using namespace hazelcast::client::serialization;

class TestNamedPortablev2 : public TestNamedPortable {
public:
    TestNamedPortablev2() {

    };

    TestNamedPortablev2(std::string name, int v) : TestNamedPortable(name, v * 10) {
        this->v = v;
    };

    void writePortable(PortableWriter & writer) {
        TestNamedPortable::writePortable(writer);
        writer.writeInt("v", v);
    };

    void readPortable(PortableReader & reader) {
        TestNamedPortable::readPortable(reader);
        v = reader.readInt("v");
    }

    int v;

};

#endif