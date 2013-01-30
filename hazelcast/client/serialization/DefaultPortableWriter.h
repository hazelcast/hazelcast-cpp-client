 //
//  DefaultPortableWriter.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_DEFAULT_PORTABLE_WRITER
#define HAZELCAST_DEFAULT_PORTABLE_WRITER

#include "PortableWriter.h"
#include "../Array.h"
#include <iostream>
#include <string>
using namespace std;

namespace hazelcast{ 
namespace client{
namespace serialization{
    
class ClassDefinition;
class PortableSerializer;
class DataInput;
class DataOutput;
class Portable;

class DefaultPortableWriter : public PortableWriter{
public:
    DefaultPortableWriter(PortableSerializer* serializer, DataOutput* output, ClassDefinition* cd);
    
    
    void writeInt(string fieldName, int value);
    
    void writeLong(string fieldName, long value);
    
    void writeBoolean(string fieldName, bool value);
    
    void writeByte(string fieldName, byte value);
    
    void writeChar(string fieldName, int value);
    
    void writeDouble(string fieldName, double value);
    
    void writeFloat(string fieldName, float value);
    
    void writeShort(string fieldName, short value);
    
    void writeUTF(string fieldName, string str);
    
    void writePortable(string fieldName, Portable& portable);
    
    void writeByteArray(string fieldName, Array<byte>&);
    
    void writeCharArray(string fieldName, Array<char>&);
    
    void writeIntArray(string fieldName, Array<int>&);
    
    void writeLongArray(string fieldName, Array<long>&);
    
    void writeDoubleArray(string fieldName, Array<double>&);
    
    void writeFloatArray(string fieldName, Array<float>&);
    
    void writeShortArray(string fieldName, Array<short>&);
    
    void writePortableArray(string fieldName, Array<Portable*>& portables);
    
    void setPosition(string fieldName);
    
private:
    PortableSerializer* serializer;

    ClassDefinition* cd;
    DataOutput* output;
    int offset;
};

}}}
#endif /* HAZELCAST_DEFAULT_PORTABLE_WRITER */
