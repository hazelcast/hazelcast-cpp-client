//
//  TestMainPortable.h
//  Server
//
//  Created by sancar koyunlu on 1/11/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef Server_TestMainPortable_h
#define Server_TestMainPortable_h

#include "TestInnerPortable.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace test {
            class HAZELCAST_API TestMainPortable : public Portable {
            public:

                TestMainPortable();

                TestMainPortable(byte b, bool boolean, char c, short s, int i, long l, float f, double d, std::string str, TestInnerPortable p);

                bool operator ==(const TestMainPortable &m) const;

                bool operator !=(const TestMainPortable &m) const;

                int getFactoryId() const;

                int getClassId() const;

                void writePortable(serialization::PortableWriter &writer) const;

                void readPortable(serialization::PortableReader &reader);

                TestInnerPortable p;
                int i;
            private:
                bool null;
                byte b;
                bool boolean;
                char c;
                short s;
                long l;
                float f;
                double d;
                std::string str;
            };

        }
    }
}
#endif
