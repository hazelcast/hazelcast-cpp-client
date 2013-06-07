//
// Created by sancar koyunlu on 5/6/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
// To change the template use AppCode | Preferences | File Templates.
//



#ifndef __TestInvalidReadPortable_H_
#define __TestInvalidReadPortable_H_

#include <iostream>


using namespace hazelcast::client::serialization;

class TestInvalidReadPortable {
public:

    TestInvalidReadPortable() {

    }

    TestInvalidReadPortable(long l, int i, std::string s) {
        this->l = l;
        this->i = i;
        this->s = l;
    }

    long l;
    int i;
    std::string s;
};

namespace hazelcast {
    namespace client {
        namespace serialization {

            inline int getTypeSerializerId(const TestInvalidReadPortable& x) {
                return SerializationConstants::CONSTANT_TYPE_PORTABLE;
            };

            inline int getFactoryId(const TestInvalidReadPortable& t) {
                return 1;
            }

            inline int getClassId(const TestInvalidReadPortable& t) {
                return 6;
            }

            template<typename HzWriter>
            inline void writePortable(HzWriter& writer, const TestInvalidReadPortable& data) {
                writer["l"] << data.l;
                writer["i"] << data.i;
                writer["s"] << data.s;
            };

            template<typename HzReader>
            inline void readPortable(HzReader& reader, TestInvalidReadPortable& data) {
                reader["l"] >> data.l;
                reader >> data.i;
                reader["s"] >> data.s;
            };
        }
    }
}


#endif //__TestInvalidReadPortable_H_


