//
//  ClassDefinitionWriter.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_CLASS_DEFINITION_WRITER
#define HAZELCAST_CLASS_DEFINITION_WRITER

#include "../Array.h"
#include "PortableWriter.h"
#include "ClassDefinition.h"
#include <iostream>
#include <string>

using namespace std;
namespace hazelcast{ 
namespace client{
namespace serialization{
    
class FieldDefinition;
class Portable;
class PortableSerializer;

class ClassDefinitionWriter : public PortableWriter{
public:
    ClassDefinitionWriter(int classId,int version, PortableSerializer* serializer);
    
    void writeInt(string fieldName, int value);
    
    void writeLong(string fieldName, long value);
    
    void writeUTF(string fieldName, string str);
    
    void writeBoolean(string fieldName, bool value);
    
    void writeByte(string fieldName, byte value);
    
    void writeChar(string fieldName, int value);
    
    void writeDouble(string fieldName, double value);
    
    void writeFloat(string fieldName, float value);
    
    void writeShort(string fieldName, short value);
    
    void writePortable(string fieldName, Portable& portable);
    
    void writeByteArray(string fieldName, Array<byte>&);
    
    void writeCharArray(string fieldName, Array<char>&);
    
    void writeIntArray(string fieldName, Array<int>&);
    
    void writeLongArray(string fieldName, Array<long>&);
    
    void writeDoubleArray(string fieldName, Array<double>&);
    
    void writeFloatArray(string fieldName, Array<float>&);
    
    void writeShortArray(string fieldName, Array<short>&);
    
    
    void writePortableArray(string fieldName, Array<Portable*>& portables);
    
    ClassDefinition* cd;
private:
    void addNestedField(Portable& p, FieldDefinition& fd);

    int index;
    PortableSerializer* serializer;
    
};

}}}
#endif /* HAZELCAST_CLASS_DEFINITION_WRITER */
