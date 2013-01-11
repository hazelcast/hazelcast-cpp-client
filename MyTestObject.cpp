//
//  MyTestObject.cpp
//  Server
//
//  Created by sancar koyunlu on 1/3/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
#include <iostream>
#include "MyTestObject.h"

MyTestObject::MyTestObject(){
    
};

MyTestObject::MyTestObject(std::string mUtf,
                           char mChar,
                           int mInt,
                           float mFloat,
                           int mShort,
                           bool mBool,
                           long mLong,
                           double mDouble){
    this->mChar = mChar;
    this->mInt = mInt;
    this->mFloat = mFloat;
    this->mShort = mShort;
    this->mUtf = mUtf;
    this->mBool = mBool;
    this->mLong = mLong;
    this->mDouble = mDouble;
};

std::ostream& operator<<(std::ostream &strm, const MyTestObject& a) {
    return strm << "MyTestObject("
                << "utf = " << a.mUtf
                << ",char = " << a.mChar
                << ",int = " << a.mInt
                << ",float = " << a.mFloat
                << ",short = " << a.mShort
                << ",bool = " << a.mBool
                << ",long = " << a.mLong
                << ",double = " << a.mDouble
                << ")";
};
//Override
void MyTestObject::writeData(DataOutput& out) const throw(std::ios_base::failure){
    out.writeUTF(mUtf);
    out.writeChar(mChar);
    out.writeInt(mInt);
    out.writeFloat(mFloat);
    out.writeShort(mShort);
    out.writeBoolean(mBool);
    out.writeLong(mLong);
    out.writeDouble(mDouble);
};
//Override
void MyTestObject::readData(DataInput& in) throw(std::ios_base::failure){
    mUtf = in.readUTF();
    mChar = in.readChar();
    mInt = in.readInt();
    mFloat = in.readFloat();
    mShort = in.readShort();
    mBool = in.readBoolean();
    mLong = in.readLong();
    mDouble = in.readDouble();
};