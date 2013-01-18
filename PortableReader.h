//
//  PortableReader.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef Server_PortableReader_h
#define Server_PortableReader_h

#include <iostream>
#include <string>

class Portable;
class ByteArray;
class CharArray;
class IntegerArray;
class LongArray;
class DoubleArray;
class FloatArray;
class ShortArray;
class PortablePointerArray;

using namespace std;
typedef unsigned char byte;

class PortableReader{
    
public:
    virtual int readInt(string) throw(ios_base::failure) = 0;
    
    virtual long readLong(string) throw(ios_base::failure) = 0;
    
    virtual string readUTF(string) throw(ios_base::failure) = 0;
    
    virtual bool readBoolean(string) throw(ios_base::failure) = 0;
    
    virtual byte readByte(string) throw(ios_base::failure) = 0;
    
    virtual char readChar(string) throw(ios_base::failure) = 0;
    
    virtual double readDouble(string) throw(ios_base::failure) = 0;
    
    virtual float readFloat(string) throw(ios_base::failure) = 0;
    
    virtual short readShort(string) throw(ios_base::failure) = 0;
    
    virtual Portable* readPortable(string) throw(ios_base::failure) = 0;
    
    virtual ByteArray* readByteArray(string) throw(ios_base::failure) = 0;
    
    virtual CharArray* readCharArray(string) throw(ios_base::failure) = 0;
    
    virtual IntegerArray* readIntArray(string) throw(ios_base::failure) = 0;
    
    virtual LongArray* readLongArray(string) throw(ios_base::failure) = 0;
    
    virtual DoubleArray* readDoubleArray(string) throw(ios_base::failure) = 0;
    
    virtual FloatArray* readFloatArray(string) throw(ios_base::failure) = 0;
    
    virtual ShortArray* readShortArray(string) throw(ios_base::failure) = 0;
    
    virtual PortablePointerArray* readPortableArray(string) throw(ios_base::failure) = 0;
};

#endif
