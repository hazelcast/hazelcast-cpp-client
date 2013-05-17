//
// Created by sancar koyunlu on 5/12/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.



#ifndef __TestMobile_H_
#define __TestMobile_H_

//#include "Portable.h"
#include "DataSerializable.h"


class TestMobile : public hazelcast::client::serialization::DataSerializable {
public:
    TestMobile();

    TestMobile(int i, char c);

    bool operator ==(TestMobile& rhs);

    int i;
    char c;
};

namespace hazelcast {
    namespace client {
        namespace serialization {

            inline int getFactoryId(const TestMobile& t) {
                return 1;
            }

            inline int getClassId(const TestMobile& t) {
                return 1;
            }

            template<typename HzWriter>
            inline void writePortable(HzWriter& writer, const TestMobile& data) {
                writer << data.c;
                writer << data.i;
            };

            template<typename HzReader>
            inline void readPortable(HzReader& reader, TestMobile& data) {
                reader >> data.c;
                reader >> data.i;
            };
        }
    }
}

#endif //__TestMobile_H_
