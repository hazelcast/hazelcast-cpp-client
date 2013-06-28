//
// Created by sancar koyunlu on 5/12/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.



#ifndef __TestMobile_H_
#define __TestMobile_H_

#include "hazelcast/client/serialization/SerializationConstants.h"


class TestDataSerializable {
public:
    TestDataSerializable() {

    }

    TestDataSerializable(int i, char c):i(i), c(c) {

    }

    bool operator ==(const TestDataSerializable & rhs) const {
        if (this == &rhs)
            return true;
        if (i != rhs.i) return false;
        if (c != rhs.c) return false;
        return true;
    };

    bool operator !=(const TestDataSerializable& m) const {
        return !(*this == m);
    };

    int i;
    char c;
};

namespace hazelcast {
    namespace client {
        namespace serialization {

            inline int getTypeSerializerId(const TestDataSerializable& x) {
                return SerializationConstants::CONSTANT_TYPE_DATA;
            };

            inline int getFactoryId(const TestDataSerializable& t) {
                return 1;
            }

            inline int getClassId(const TestDataSerializable& t) {
                return 1;
            }

            template<typename HzWriter>
            inline void writePortable(HzWriter& writer, const TestDataSerializable& data) {
                writer << data.c;
                writer << data.i;
            };

            template<typename HzReader>
            inline void readPortable(HzReader& reader, TestDataSerializable& data) {
                reader >> data.c;
                reader >> data.i;
            };
        }
    }
}

#endif //__TestMobile_H_
