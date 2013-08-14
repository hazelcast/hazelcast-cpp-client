#ifndef __Server__TestNamedPortableV2__
#define __Server__TestNamedPortableV2__


#include "TestNamedPortable.h"

using namespace hazelcast::client::serialization;

class TestNamedPortableV2 : public hazelcast::client::Portable {
public:
    TestNamedPortableV2() {

    };

    TestNamedPortableV2(std::string name, int v) : name(name), k(v * 10), v(v) {
    };

    bool operator ==(TestNamedPortableV2 & m) {
        if (this == &m)
            return true;
        if (k != m.k)
            return false;
        if (name.compare(m.name))
            return false;
        if (v != m.v) return false;
        return true;
    };

    bool operator !=(TestNamedPortableV2 & m) {
        return !(*this == m);
    };



    inline int getFactoryId() const{
        return 1;
    }

    inline int getClassId() const{
        return 3;
    }


    inline void writePortable(serialization::PortableWriter& writer) const{
        writer.writeInt("v", v);
        writer.writeUTF("name", name);
        writer.writeInt("myint", k);
    };


    inline void readPortable(serialization::PortableReader& reader) {
        v = reader.readInt("v");
        name = reader.readUTF("name");
        k = reader.readInt("myint");
    };
    
    std::string name;
    int k;
    int v;

};


#endif
