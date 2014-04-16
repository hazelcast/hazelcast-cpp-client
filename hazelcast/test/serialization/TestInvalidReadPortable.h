//
// Created by sancar koyunlu on 5/6/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
// To change the template use AppCode | Preferences | File Templates.
//




#ifndef __TestInvalidReadPortable_H_
#define __TestInvalidReadPortable_H_

#include "hazelcast/client/serialization/Portable.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace test {
            class HAZELCAST_API TestInvalidReadPortable : public serialization::Portable {
            public:

                TestInvalidReadPortable();

                TestInvalidReadPortable(long l, int i, std::string s);

                int getFactoryId() const;

                int getClassId() const;

                void writePortable(serialization::PortableWriter& writer) const;

                void readPortable(serialization::PortableReader& reader);
            private:
                long l;
                int i;
                std::string s;
            };
        }
    }
}
#endif //__TestInvalidReadPortable_H_



