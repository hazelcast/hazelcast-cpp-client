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
#include "Array.h"
class Portable;

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
    
    virtual Portable readPortable(string) throw(ios_base::failure) = 0;
    
    virtual Array<byte> readByteArray(string) throw(ios_base::failure) = 0;
    
    virtual Array<char> readCharArray(string) throw(ios_base::failure) = 0;
    
    virtual Array<int> readIntArray(string) throw(ios_base::failure) = 0;
    
    virtual Array<long> readLongArray(string) throw(ios_base::failure) = 0;
    
    virtual Array<double> readDoubleArray(string) throw(ios_base::failure) = 0;
    
    virtual Array<float> readFloatArray(string) throw(ios_base::failure) = 0;
    
    virtual Array<short> readShortArray(string) throw(ios_base::failure) = 0;
    
    virtual Array<Portable> readPortableArray(string) throw(ios_base::failure) = 0;
};

#endif
