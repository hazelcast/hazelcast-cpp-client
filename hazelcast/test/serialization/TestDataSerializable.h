//
// Created by sancar koyunlu on 5/12/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.



#ifndef __TestMobile_H_
#define __TestMobile_H_

#include "IdentifiedDataSerializable.h"

namespace hazelcast {
    namespace client {
        namespace test {
            class TestDataSerializable : public IdentifiedDataSerializable {
            public:
                TestDataSerializable();

                TestDataSerializable(int i, char c);

                bool operator ==(const TestDataSerializable & rhs) const;

                bool operator !=(const TestDataSerializable& m) const;

                int getFactoryId() const;

                int getClassId() const;

                void writeData(serialization::ObjectDataOutput& writer) const;

                void readData(serialization::ObjectDataInput& reader);

                int i;
                char c;
            };

        }
    }
}
#endif //__TestMobile_H_
