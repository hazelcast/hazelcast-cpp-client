//
//  DataOutput.h
//  Server
//
//  Created by sancar koyunlu on 1/3/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef Server_DataOutput_h
#define Server_DataOutput_h

#include <string>

typedef char byte;

class DataOutput{
public:
    virtual void write(byte* bytes, int offset, int length) = 0;
    
    virtual void writeBoolean(bool b) = 0;
    
    virtual void writeByte(int i) = 0;
    
    virtual void writeShort(int i) = 0;
    
    virtual void writeChar(int i) = 0;
    
    virtual void writeInt(int i) = 0;
    
    virtual void writeLong(long l) = 0;
    
    virtual void writeFloat(float v) = 0;
    
    virtual void writeDouble(double v) = 0;
    
    virtual void writeUTF(std::string s) = 0;
    
};

#endif
