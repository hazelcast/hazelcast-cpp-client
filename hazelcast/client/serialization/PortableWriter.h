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
    virtual void writeInt(string, int) = 0;
    
    virtual void writeLong(string, long) = 0;
    
    virtual void writeUTF(string, string) = 0;
    
    virtual void writeBoolean(string, const bool) = 0;
    
    virtual void writeByte(string, const byte) = 0;
    
    virtual void writeChar(string, const int) = 0;
    
    virtual void writeDouble(string, const double) = 0;
    
    virtual void writeFloat(string, const float) = 0;
    
    virtual void writeShort(string, const short) = 0;
    
    virtual void writePortable(string, Portable& portable) = 0;
    
    virtual void writeByteArray(string, Array<byte>&) = 0;
    
    virtual void writeCharArray(string, Array<char>&) = 0;
    
    virtual void writeIntArray(string, Array<int>&) = 0;
    
    virtual void writeLongArray(string, Array<long>&) = 0;
    
    virtual void writeDoubleArray(string, Array<double>&) = 0;
    
    virtual void writeFloatArray(string, Array<float>&) = 0;
    
    virtual void writeShortArray(string, Array<short>&) = 0;
    
    virtual void writePortableArray(string  , Array<Portable*>&) = 0;
};

}}}
#endif /* HAZELCAST_PORTABLE_WRITER */
