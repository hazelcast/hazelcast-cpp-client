//
//  PortableWriter.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef Server_PortableWriter_h
#define Server_PortableWriter_h

#include <iostream>
#include <string>

using namespace std;
typedef unsigned char byte;
class PortableWriter{
public:
    virtual void writeInt(string, int) throw(ios_base::failure) = 0;
    
    virtual void writeLong(string, long) throw(ios_base::failure) = 0;
    
    virtual void writeUTF(string, string) throw(ios_base::failure) = 0;
    
    virtual void writeBoolean(string, const bool) throw(ios_base::failure) = 0;
    
    virtual void writeByte(string, const byte) throw(ios_base::failure) = 0;
    
    virtual void writeChar(string, const int) throw(ios_base::failure) = 0;
    
    virtual void writeDouble(string, const double) throw(ios_base::failure) = 0;
    
    virtual void writeFloat(string, const float) throw(ios_base::failure) = 0;
    
    virtual void writeShort(string, const short) throw(ios_base::failure) = 0;
    
//    virtual void writePortable(string, Portable portable) throw(ios_base::failure) = 0;
    
    virtual void writeByteArray(string, byte* bytes) throw(ios_base::failure) = 0;
    
    virtual void writeCharArray(string, char* chars) throw(ios_base::failure) = 0;
    
    virtual void writeIntArray(string, int* ints) throw(ios_base::failure) = 0;
    
    virtual void writeLongArray(string, long* longs) throw(ios_base::failure) = 0;
    
    virtual void writeDoubleArray(string, double*s) throw(ios_base::failure) = 0;
    
    virtual void writeFloatArray(string, float*s) throw(ios_base::failure) = 0;
    
    virtual void writeShortArray(string, short*s) throw(ios_base::failure) = 0;
    
//    virtual void writePortableArray(string  , Portable* portables) throw(ios_base::failure) = 0;
};

#endif
