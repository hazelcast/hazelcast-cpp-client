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
#include "hazelcast/util/ByteBuffer.h"
#include <vector>

namespace hazelcast {
    namespace client {
        namespace test {

            class TestRawDataPortable : public serialization::Portable {
            public:
                TestRawDataPortable();

                int getFactoryId() const;

                int getClassId() const;

                void writePortable(serialization::PortableWriter &writer) const;

                void readPortable(serialization::PortableReader &reader);

                TestRawDataPortable(long l, hazelcast::util::CharVector_ptr c, TestNamedPortable p, int k, std::string s, TestDataSerializable ds);

                bool operator ==(const TestRawDataPortable &m) const;

                bool operator !=(const TestRawDataPortable &m) const;

                long l;
                hazelcast::util::CharVector_ptr c;
                TestNamedPortable p;
                int k;
                std::string s;
                TestDataSerializable ds;
            };
        }
    }
}


#endif //__RawDataPortable_H_


