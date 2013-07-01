//
// Created by sancar koyunlu on 5/12/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.



#ifndef __TestMobile_H_
#define __TestMobile_H_

#include "hazelcast/client/serialization/SerializationConstants.h"
#include "BufferedDataOutput.h"
#include "BufferedDataInput.h"
#include "DataSerializable.h"


class TestDataSerializable : public hazelcast::client::DataSerializable{
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

//            inline int getSerializerId(const TestDataSerializable& x) {
//                return SerializationConstants::CONSTANT_TYPE_DATA;
//            };

            inline int getFactoryId(const TestDataSerializable& t) {
                return 1;
            }

            inline int getClassId(const TestDataSerializable& t) {
                return 1;
            }

            inline void writeData(BufferedDataOutput& writer, const TestDataSerializable& data) {
                writer.writeChar(data.c);
                writer.writeInt(data.i);
            };

            inline void readData(BufferedDataInput& reader, TestDataSerializable& data) {
                data.c = reader.readChar();
                data.i = reader.readInt();
            };
        }
    }
}

#endif //__TestMobile_H_
