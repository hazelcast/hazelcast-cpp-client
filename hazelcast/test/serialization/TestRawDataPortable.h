//
// Created by sancar koyunlu on 5/6/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
// To change the template use AppCode | Preferences | File Templates.
//



#ifndef __RawDataPortable_H_
#define __RawDataPortable_H_

#include "TestNamedPortable.h"
#include "TestDataSerializable.h"
#include <vector>

namespace hazelcast {
    namespace client {
        namespace test {

            class TestRawDataPortable : public Portable {
            public:
                TestRawDataPortable();

                inline int getFactoryId() const;

                inline int getClassId() const;

                inline void writePortable(serialization::PortableWriter& writer) const;

                inline void readPortable(serialization::PortableReader& reader);

                TestRawDataPortable(long l, std::vector<char> c, TestNamedPortable p, int k, std::string s, TestDataSerializable ds);

                bool operator ==(const TestRawDataPortable& m) const;

                bool operator !=(const TestRawDataPortable& m) const;

                long l;
                std::vector<char> c;
                TestNamedPortable p;
                int k;
                std::string s;
                TestDataSerializable ds;
            };
        }
    }
}


#endif //__RawDataPortable_H_

