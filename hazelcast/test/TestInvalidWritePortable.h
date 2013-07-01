//
// Created by sancar koyunlu on 5/6/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
// To change the template use AppCode | Preferences | File Templates.
//



#ifndef __TestInvalidWritePortable_H_
#define __TestInvalidWritePortable_H_

#include <iostream>


using namespace hazelcast::client::serialization;

class TestInvalidWritePortable {
public:

    TestInvalidWritePortable() {

    }

    TestInvalidWritePortable(long l, int i, std::string s) {
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

            inline int getSerializerId(const TestInvalidWritePortable& x) {
                return SerializationConstants::CONSTANT_TYPE_PORTABLE;
            };

            inline int getFactoryId(const TestInvalidWritePortable& t) {
                return 1;
            }

            inline int getClassId(const TestInvalidWritePortable& t) {
                return 5;
            }

            template<typename HzWriter>
            inline void writePortable(HzWriter& writer, const TestInvalidWritePortable& data) {
                //TODO convert std::string to const char*
                writer["l"] << data.l;
                writer << data.i;
                writer["s"] << data.s;
            };

            template<typename HzReader>
            inline void readPortable(HzReader& reader, TestInvalidWritePortable& data) {
                reader["l"] >> data.l;
                reader["i"] >> data.i;
                reader["s"] >> data.s;
            };
        }
    }
}


#endif //__TestInvalidWritePortable_H_


