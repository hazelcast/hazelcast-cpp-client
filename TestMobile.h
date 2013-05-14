//
// Created by sancar koyunlu on 5/12/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.



#ifndef __TestMobile_H_
#define __TestMobile_H_


class TestMobile {
public:
    TestMobile();

    TestMobile(int i, char c);

    bool operator ==(TestMobile& rhs);

    int i;
    char c;
};

//int getFactoryId(const TestMobile& t);
//
//int getClassId(const TestMobile& t);
//
//template<typename HzWriter>
//void operator <<(HzWriter& writer, const TestMobile& data);
//
//template<typename HzReader>
//void operator >>(HzReader& reader, TestMobile& data);

/*
template <>
int getFactoryId<TestMobile>(){
    return 1;
};
*/

inline int getFactoryId(const TestMobile& t) {
    return 1;
}

inline int getClassId(const TestMobile& t) {
    return 1;
}

template<typename HzWriter>
inline void writePortable(HzWriter& writer, const TestMobile& data) {
//    writer["sadada"] << data.c;
    writer << data.i;
    writer << data.c;
};

template<typename HzReader>
inline void readPortable(HzReader& reader, TestMobile& data) {
//  reader["wedsa"] >> data.c;
    reader >> data.i;
    reader >> data.c;
};


#endif //__TestMobile_H_
