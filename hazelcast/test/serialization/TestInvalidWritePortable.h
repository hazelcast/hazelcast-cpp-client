//
// Created by sancar koyunlu on 5/6/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
// To change the template use AppCode | Preferences | File Templates.
//




#ifndef __TestInvalidWritePortable_H_
#define __TestInvalidWritePortable_H_

#include "hazelcast/client/Portable.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace test {
            class HAZELCAST_API TestInvalidWritePortable : public Portable {
            public:
                TestInvalidWritePortable();

                TestInvalidWritePortable(long l, int i, std::string s);

                int getFactoryId() const;

                int getClassId() const;

                void writePortable(serialization::PortableWriter& writer) const;

                void readPortable(serialization::PortableReader& reader);

                long l;
                int i;
                std::string s;
            };
        }
    }
}


#endif //__TestInvalidWritePortable_H_


