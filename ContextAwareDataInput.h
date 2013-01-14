//
//  DataInput.h
//  Server
//
//  Created by sancar koyunlu on 1/3/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef __Server__ContextAwareDataInput__
#define __Server__ContextAwareDataInput__

#include <string>
#include "BufferObjectDataInput.h"
typedef unsigned char byte;

class ContextAwareDataInput : public BufferObjectDataInput{
public:
    ContextAwareDataInput(byte*);
    virtual void readFully(byte* bytes, int off, int len);
    
    virtual int skipBytes(int i) ;
    
    virtual bool readBoolean();
    
    virtual byte readByte();
  
    virtual short readShort();
    
    virtual char readChar();
    
    virtual int readInt();
    
    virtual long readLong();
    
    virtual float readFloat();
    
    virtual double readDouble();
    
    virtual std::string readUTF() throw(std::string);
    
    
private:
    byte* ptr;
    int size;
};

#endif /* defined(__Server__DataInput__) */
