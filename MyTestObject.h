//
//  MyTestObject.h
//  Server
//
//  Created by sancar koyunlu on 1/3/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef __Server__MyTestObject__
#define __Server__MyTestObject__

#include <string>
#include "DataSerializable.h"

class MyTestObject : public DataSerializable{
public:
    MyTestObject(std::string,char,int,float,int,bool,long,double);
    MyTestObject();
    void writeData(DataOutput&) const throw(std::ios_base::failure);
    void readData(DataInput&) throw(std::ios_base::failure);
private:
    friend std::ostream& operator<<(std::ostream&, const MyTestObject&);
    std::string mUtf;
    char mChar;
    int mInt;
    float mFloat;
    int mShort;
    bool mBool;
    long mLong;
    double mDouble;
};

#endif /* defined(__Server__MyTestObject__) */
