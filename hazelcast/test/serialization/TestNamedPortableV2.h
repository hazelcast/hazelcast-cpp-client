#ifndef __Server__TestNamedPortableV2__
#define __Server__TestNamedPortableV2__


#include "TestNamedPortable.h"

namespace hazelcast {
    namespace client {
        namespace test {
            class HAZELCAST_API TestNamedPortableV2 : public Portable {
            public:
                TestNamedPortableV2();

                TestNamedPortableV2(std::string name, int v);

                int getFactoryId() const;

                int getClassId() const;

                void writePortable(serialization::PortableWriter& writer) const;

                void readPortable(serialization::PortableReader& reader);

                std::string name;
                int k;
                int v;

            };

        }
    }
}


#endif
