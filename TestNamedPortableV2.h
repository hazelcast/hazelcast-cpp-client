#ifndef __Server__TestNamedPortableV2__
#define __Server__TestNamedPortableV2__


#include "TestNamedPortable.h"

using namespace hazelcast::client::serialization;

class TestNamedPortableV2 {
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


    std::string name;
    int k;
    int v;

};

namespace hazelcast {
    namespace client {
        namespace serialization {

            inline int getFactoryId(const TestNamedPortableV2& t) {
                return 1;
            }

            inline int getClassId(const TestNamedPortableV2& t) {
                return 3;
            }

            template<typename HzWriter>
            inline void writePortable(HzWriter& writer, const TestNamedPortableV2& data) {
                writer["v"] << data.v;
                writer["name"] << data.name;
                writer["myint"] << data.k;
            };

            template<typename HzReader>
            inline void readPortable(HzReader& reader, TestNamedPortableV2& data) {
                reader["v"] >> data.v;
                reader["name"] >> data.name;
                reader["myint"] >> data.k;
            };
        }
    }
}
#endif
