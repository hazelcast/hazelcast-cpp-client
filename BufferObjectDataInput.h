//
//  BufferObjectDataInput.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef __Server__BufferObjectDataInput__
#define __Server__BufferObjectDataInput__
#include <iostream>
#include "DataInput.h"
#include "Closeable.h"
typedef unsigned char byte;

class BufferObjectDataInput : public DataInput , Closeable{
    virtual int read(int index) throw (std::ios_base::failure)= 0 ;
    
    virtual int read(int index, byte* b, int off, int len) throw (std::ios_base::failure)= 0 ;
    
    virtual int readInt(int index) throw (std::ios_base::failure)= 0 ;
    
    virtual long readLong(int index) throw (std::ios_base::failure)= 0 ;
    
    virtual bool readBoolean(int index) throw (std::ios_base::failure)= 0 ;
    
    virtual byte readByte(int index) throw (std::ios_base::failure)= 0 ;
    
    virtual char readChar(int index) throw (std::ios_base::failure)= 0 ;
    
    virtual double readDouble(int index) throw (std::ios_base::failure)= 0 ;
    
    virtual float readFloat(int index) throw (std::ios_base::failure)= 0 ;
    
    virtual short readShort(int index) throw (std::ios_base::failure)= 0 ;
    
    virtual int position()= 0 ;
    
    virtual void position(int newPos)= 0 ;
    
    virtual void reset()= 0 ;
    
    virtual BufferObjectDataInput* duplicate()= 0;
    
    virtual BufferObjectDataInput* slice()= 0;
};

#endif /* defined(__Server__BufferObjectDataInput__) */
