//
//  PortableWriter.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_PORTABLE_WRITER
#define HAZELCAST_PORTABLE_WRITER

#include "../Array.h"
#include <iostream>
#include <string>

using namespace std;

namespace hazelcast{ 
namespace client{
namespace serialization{
    
class Portable;

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
    
    virtual void writeByteArray(string, Array<byte>&) throw(ios_base::failure) = 0;
    
    virtual void writeCharArray(string, Array<char>&) throw(ios_base::failure) = 0;
    
    virtual void writeIntArray(string, Array<int>&) throw(ios_base::failure) = 0;
    
    virtual void writeLongArray(string, Array<long>&) throw(ios_base::failure) = 0;
    
    virtual void writeDoubleArray(string, Array<double>&) throw(ios_base::failure) = 0;
    
    virtual void writeFloatArray(string, Array<float>&) throw(ios_base::failure) = 0;
    
    virtual void writeShortArray(string, Array<short>&) throw(ios_base::failure) = 0;
    
    virtual void writePortableArray(string  , Array<Portable*>&) throw(ios_base::failure) = 0;
};

}}}
#endif /* HAZELCAST_PORTABLE_WRITER */
