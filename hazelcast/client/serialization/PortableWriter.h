 //
//  PortableWriter.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_PORTABLE_WRITER
#define HAZELCAST_PORTABLE_WRITER

#include <vector>
#include <iostream>
#include <string>
using namespace std;

namespace hazelcast{ 
namespace client{
namespace serialization{
    
typedef unsigned char byte;
class Portable;

class PortableWriter{
public:
    virtual void writeInt(string fieldName, int value)= 0;
    
    virtual void writeLong(string fieldName, long value)= 0;
    
    virtual void writeBoolean(string fieldName, bool value)= 0;
    
    virtual void writeByte(string fieldName, byte value)= 0;
    
    virtual void writeChar(string fieldName, int value)= 0;
    
    virtual void writeDouble(string fieldName, double value)= 0;
    
    virtual void writeFloat(string fieldName, float value)= 0;
    
    virtual void writeShort(string fieldName, short value)= 0;
    
    virtual void writeUTF(string fieldName, string str)= 0;
    
    virtual void writePortable(string fieldName, Portable& portable)= 0;
    
    virtual void writeByteArray(string fieldName, std::vector<byte>&)= 0;
    
    virtual void writeCharArray(string fieldName, std::vector<char>&)= 0;
    
    virtual void writeIntArray(string fieldName, std::vector<int>&)= 0;
    
    virtual void writeLongArray(string fieldName, std::vector<long>&)= 0;
    
    virtual void writeDoubleArray(string fieldName, std::vector<double>&)= 0;
    
    virtual void writeFloatArray(string fieldName, std::vector<float>&)= 0;
    
    virtual void writeShortArray(string fieldName, std::vector<short>&) = 0;
    
    virtual void writePortableArray(string fieldName, std::vector<Portable*>& portables) = 0;
    
};

}}}
#endif /* HAZELCAST_PORTABLE_WRITER */
