//
//  TestNamedPortable.h
//  Server
//
//  Created by sancar koyunlu on 1/11/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef Server_TestNamedPortable_h
#define Server_TestNamedPortable_h

#include "hazelcast/client/serialization/Portable.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace test {
            class HAZELCAST_API TestNamedPortable : public serialization::Portable {
            public:
                TestNamedPortable();

                TestNamedPortable(std::string name, int k);

                int getFactoryId() const;

                int getClassId() const;

                void writePortable(serialization::PortableWriter& writer) const;

                void readPortable(serialization::PortableReader& reader);

                virtual bool operator ==(const TestNamedPortable& m) const;

                virtual bool operator !=(const TestNamedPortable& m) const;

                std::string name;
                int k;
            };
        }
    }
}


#endif

