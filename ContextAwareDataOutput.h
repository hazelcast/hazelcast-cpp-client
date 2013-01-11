//
//  DataOutput.h
//  Server
//
//  Created by sancar koyunlu on 1/3/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef Server_ContextAwareDataOutput_h
#define Server_ContextAwareDataOutput_h

#include <string>
#include <iostream>
#include <sstream>
#include "BufferObjectDataOutput.h"
typedef char byte;

class ContextAwareDataOutput : public BufferObjectDataOutput{
public:
    ContextAwareDataOutput();
   
    virtual void write(byte* bytes, int offset, int length);
    
    virtual void writeBoolean(bool b);
    
    virtual void writeByte(int i);
    
    virtual void writeShort(int i);
    
    virtual void writeChar(int i);
    
    virtual void writeInt(int i);
    
    virtual void writeLong(long l);
    
    virtual void writeFloat(float v);
    
    virtual void writeDouble(double v);
    
    virtual void writeUTF(std::string s);
    
    std::string toString();
private:
    std::ostringstream buffer;
};

#endif
