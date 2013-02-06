//
//  ClassDefinitionWriter.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_CLASS_DEFINITION_WRITER
#define HAZELCAST_CLASS_DEFINITION_WRITER


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
    
    void writeByteArray(string fieldName, std::vector<byte>&);
    
    void writeCharArray(string fieldName, std::vector<char>&);
    
    void writeIntArray(string fieldName, std::vector<int>&);
    
    void writeLongArray(string fieldName, std::vector<long>&);
    
    void writeDoubleArray(string fieldName, std::vector<double>&);
    
    void writeFloatArray(string fieldName, std::vector<float>&);
    
    void writeShortArray(string fieldName, std::vector<short>&);
    
    template<typename T>
    void writePortableArray(string fieldName, std::vector<T>& portables){
        int classId = portables[0].getClassId();
        for (int i = 1; i < portables.size(); i++) {
            if (portables[i].getClassId() != classId) {
                throw "Illegal Argument Exception";
            }
        }
        FieldDefinition fd(index++, fieldName, FieldDefinition::TYPE_PORTABLE_ARRAY, classId);
        addNestedField(portables[0], fd);
    };
    
    ClassDefinition* cd;
private:
    void addNestedField(Portable& p, FieldDefinition& fd);

    int index;
    PortableSerializer* serializer;
    
};

}}}
#endif /* HAZELCAST_CLASS_DEFINITION_WRITER */
