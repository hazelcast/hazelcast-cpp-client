//
//  ClassDefinitionWriter.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#include "ClassDefinitionWriter.h"
#include "FieldDefinition.h"
#include "Portable.h"
#include "PortableSerializer.h"
#include "ClassDefinition.h"

namespace hazelcast{ 
namespace client{
namespace serialization{

ClassDefinitionWriter::ClassDefinitionWriter(int classId, int version, PortableSerializer* serializer):index(0){
    cd = new ClassDefinition;
    cd->classId = classId;
    cd->version = version;
    this->serializer = serializer;
};

void ClassDefinitionWriter::writeInt(string fieldName, int value){
    FieldDefinition fd(index++, fieldName, FieldDefinition::TYPE_INT);
    cd->add(fd);
};

void ClassDefinitionWriter::writeLong(string fieldName, long value){
    FieldDefinition fd(index++, fieldName,FieldDefinition::TYPE_LONG);
    cd->add(fd);
};

void ClassDefinitionWriter::writeUTF(string fieldName, string str){
    FieldDefinition fd(index++, fieldName,FieldDefinition::TYPE_UTF);
    cd->add(fd);
};

void ClassDefinitionWriter::writeBoolean(string fieldName, bool value){
    FieldDefinition fd(index++, fieldName,FieldDefinition::TYPE_BOOLEAN);
    cd->add(fd);
};

void ClassDefinitionWriter::writeByte(string fieldName, byte value){
    FieldDefinition fd(index++, fieldName,FieldDefinition::TYPE_BYTE);
    cd->add(fd);
};

void ClassDefinitionWriter::writeChar(string fieldName, int value){
    FieldDefinition fd(index++, fieldName, FieldDefinition::TYPE_CHAR);
    cd->add(fd);
};

void ClassDefinitionWriter::writeDouble(string fieldName, double value){
    FieldDefinition fd(index++, fieldName, FieldDefinition::TYPE_DOUBLE);
    cd->add(fd);
};

void ClassDefinitionWriter::writeFloat(string fieldName, float value){
    FieldDefinition fd(index++, fieldName, FieldDefinition::TYPE_FLOAT);
    cd->add(fd);
};

void ClassDefinitionWriter::writeShort(string fieldName, short value){
    FieldDefinition fd(index++, fieldName,FieldDefinition::TYPE_SHORT);
    cd->add(fd);
};

void ClassDefinitionWriter::writePortable(string fieldName, Portable& portable){
    FieldDefinition fd = FieldDefinition(index++, fieldName, FieldDefinition::TYPE_PORTABLE, portable.getClassId() );
    addNestedField(portable, fd);
};

void ClassDefinitionWriter::writeByteArray(string fieldName, std::vector<byte>& v){
    FieldDefinition fd(index++, fieldName, FieldDefinition::TYPE_BYTE_ARRAY);
    cd->add(fd);
};
    
void ClassDefinitionWriter::writeCharArray(string fieldName, std::vector<char>& v){
    FieldDefinition fd(index++, fieldName, FieldDefinition::TYPE_CHAR_ARRAY);
    cd->add(fd);
};
    
void ClassDefinitionWriter::writeIntArray(string fieldName, std::vector<int>& v){
    FieldDefinition fd(index++, fieldName, FieldDefinition::TYPE_INT_ARRAY);
    cd->add(fd);
};
    
void ClassDefinitionWriter::writeLongArray(string fieldName, std::vector<long>& v){
    FieldDefinition fd(index++, fieldName, FieldDefinition::TYPE_LONG_ARRAY);
    cd->add(fd);
};
    
void ClassDefinitionWriter::writeDoubleArray(string fieldName, std::vector<double>& v){
    FieldDefinition fd(index++, fieldName, FieldDefinition::TYPE_DOUBLE_ARRAY);
    cd->add(fd);
};

void ClassDefinitionWriter::writeFloatArray(string fieldName, std::vector<float>& v){
    FieldDefinition fd(index++, fieldName, FieldDefinition::TYPE_FLOAT_ARRAY);
    cd->add(fd);
};
    
void ClassDefinitionWriter::writeShortArray(string fieldName, std::vector<short>& v){
    FieldDefinition fd(index++, fieldName, FieldDefinition::TYPE_SHORT_ARRAY);
    cd->add(fd);
};

void ClassDefinitionWriter::writePortableArray(string fieldName, std::vector<Portable*>& portables){
    int classId = portables[0]->getClassId();
    for (int i = 1; i < portables.size(); i++) {
        if (portables[i]->getClassId() != classId) {
            throw "Illegal Argument Exception";
        }
    }
    FieldDefinition fd(index++, fieldName, FieldDefinition::TYPE_PORTABLE_ARRAY, classId);
    addNestedField(*(portables[0]), fd);
};

void ClassDefinitionWriter::addNestedField(Portable& p, FieldDefinition& fd){
    cd->add(fd);
    ClassDefinition* nestedCd =  serializer->getClassDefinition(p);

    cd->add(nestedCd);
};

}}}
