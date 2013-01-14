//
//  DataInput.h
//  Server
//
//  Created by sancar koyunlu on 1/3/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef __Server__DataInput__
#define __Server__DataInput__

#include <string>

typedef unsigned char byte;

class DataInput{
public:
    
    virtual void readFully(byte* bytes, int off, int len)= 0;
    
    virtual int skipBytes(int i)= 0;
    
    virtual bool readBoolean()= 0;
    
    virtual byte readByte()= 0;
    
    virtual short readShort()= 0;
    
    virtual char readChar()= 0;
    
    virtual int readInt()= 0;
    
    virtual long readLong()= 0;
    
    virtual float readFloat()= 0;
    
    virtual double readDouble()= 0;
    
    virtual std::string readUTF() throw(std::string) = 0;
    
    
private:
};

#endif /* defined(__Server__DataInput__) */
