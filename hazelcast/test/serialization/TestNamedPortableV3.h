//
// Created by sancar koyunlu on 05/04/15.
//


#ifndef HAZELCAST_TestNamedPortableV3
#define HAZELCAST_TestNamedPortableV3

#include "hazelcast/client/serialization/Portable.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace test {
            class TestNamedPortableV3 : public serialization::Portable {
            public:
                TestNamedPortableV3();

                TestNamedPortableV3(std::string name, short k);

                int getFactoryId() const;

                int getClassId() const;

                void writePortable(serialization::PortableWriter& writer) const;

                void readPortable(serialization::PortableReader& reader);

                virtual bool operator ==(const TestNamedPortableV3& m) const;

                virtual bool operator !=(const TestNamedPortableV3& m) const;

                std::string name;
                short k;
            };
        }
    }
}


#endif //HAZELCAST_TestNamedPortableV3
