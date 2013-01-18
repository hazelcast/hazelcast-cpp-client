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
class Portable;
class ByteArray;
class PortablePointerArray;

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
    
    virtual void writePortable(string, Portable& portable) throw(ios_base::failure) = 0;
    
    virtual void writeByteArray(string, ByteArray&) throw(ios_base::failure) = 0;
    
    virtual void writeCharArray(string, char* chars, int len) throw(ios_base::failure) = 0;
    
    virtual void writeIntArray(string, int* ints, int len) throw(ios_base::failure) = 0;
    
    virtual void writeLongArray(string, long* longs, int len) throw(ios_base::failure) = 0;
    
    virtual void writeDoubleArray(string, double*s, int len) throw(ios_base::failure) = 0;
    
    virtual void writeFloatArray(string, float*s, int len) throw(ios_base::failure) = 0;
    
    virtual void writeShortArray(string, short*s, int len) throw(ios_base::failure) = 0;
    
    virtual void writePortableArray(string  , PortablePointerArray&) throw(ios_base::failure) = 0;
};

#endif
