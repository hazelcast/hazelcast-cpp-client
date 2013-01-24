//
//  ClassDefinitionWriter.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#include "ClassDefinitionWriter.h"
#include "FieldDefinitionImpl.h"
#include "Portable.h"
#include "PortableSerializer.h"

ClassDefinitionWriter::ClassDefinitionWriter(int classId, int version, PortableSerializer* serializer):index(0){
    cd.classId = classId;
    cd.version = version;
    this->serializer = serializer;
};

void ClassDefinitionWriter::writeInt(string fieldName, int value) throw(ios_base::failure){
    FieldDefinitionImpl fd(index++, fieldName, FieldDefinitionImpl::TYPE_INT);
    cd.add(fd);
};

void ClassDefinitionWriter::writeLong(string fieldName, long value) throw(ios_base::failure){
    FieldDefinitionImpl fd(index++, fieldName,FieldDefinitionImpl::TYPE_LONG);
    cd.add(fd);
};

void ClassDefinitionWriter::writeUTF(string fieldName, string str) throw(ios_base::failure){
    FieldDefinitionImpl fd(index++, fieldName,FieldDefinitionImpl::TYPE_UTF);
    cd.add(fd);
};

void ClassDefinitionWriter::writeBoolean(string fieldName, bool value) throw(ios_base::failure) {
    FieldDefinitionImpl fd(index++, fieldName,FieldDefinitionImpl::TYPE_BOOLEAN);
    cd.add(fd);
};

void ClassDefinitionWriter::writeByte(string fieldName, byte value) throw(ios_base::failure) {
    FieldDefinitionImpl fd(index++, fieldName,FieldDefinitionImpl::TYPE_BYTE);
    cd.add(fd);
};

void ClassDefinitionWriter::writeChar(string fieldName, int value) throw(ios_base::failure) {
    FieldDefinitionImpl fd(index++, fieldName, FieldDefinitionImpl::TYPE_CHAR);
    cd.add(fd);
};

void ClassDefinitionWriter::writeDouble(string fieldName, double value) throw(ios_base::failure) {
    FieldDefinitionImpl fd(index++, fieldName, FieldDefinitionImpl::TYPE_DOUBLE);
    cd.add(fd);
};

void ClassDefinitionWriter::writeFloat(string fieldName, float value) throw(ios_base::failure) {
    FieldDefinitionImpl fd(index++, fieldName, FieldDefinitionImpl::TYPE_FLOAT);
    cd.add(fd);
};

void ClassDefinitionWriter::writeShort(string fieldName, short value) throw(ios_base::failure) {
    FieldDefinitionImpl fd(index++, fieldName,FieldDefinitionImpl::TYPE_SHORT);
    cd.add(fd);
};

void ClassDefinitionWriter::writePortable(string fieldName, Portable& portable) throw(ios_base::failure) {
    FieldDefinitionImpl fd = FieldDefinitionImpl(index++, fieldName, FieldDefinitionImpl::TYPE_PORTABLE, portable.getClassId() );
    addNestedField(portable, fd);
};

void ClassDefinitionWriter::writeByteArray(string fieldName, Array<byte>& v) throw(ios_base::failure) {
    FieldDefinitionImpl fd(index++, fieldName, FieldDefinitionImpl::TYPE_BYTE_ARRAY);
    cd.add(fd);
};
    
void ClassDefinitionWriter::writeCharArray(string fieldName, Array<char>& v) throw(ios_base::failure) {
    FieldDefinitionImpl fd(index++, fieldName, FieldDefinitionImpl::TYPE_CHAR_ARRAY);
    cd.add(fd);
};
    
void ClassDefinitionWriter::writeIntArray(string fieldName, Array<int>& v) throw(ios_base::failure) {
    FieldDefinitionImpl fd(index++, fieldName, FieldDefinitionImpl::TYPE_INT_ARRAY);
    cd.add(fd);
};
    
void ClassDefinitionWriter::writeLongArray(string fieldName, Array<long>& v) throw(ios_base::failure) {
    FieldDefinitionImpl fd(index++, fieldName, FieldDefinitionImpl::TYPE_LONG_ARRAY);
    cd.add(fd);
};
    
void ClassDefinitionWriter::writeDoubleArray(string fieldName, Array<double>& v) throw(ios_base::failure) {
    FieldDefinitionImpl fd(index++, fieldName, FieldDefinitionImpl::TYPE_DOUBLE_ARRAY);
    cd.add(fd);
};

void ClassDefinitionWriter::writeFloatArray(string fieldName, Array<float>& v) throw(ios_base::failure) {
    FieldDefinitionImpl fd(index++, fieldName, FieldDefinitionImpl::TYPE_FLOAT_ARRAY);
    cd.add(fd);
};
    
void ClassDefinitionWriter::writeShortArray(string fieldName, Array<short>& v) throw(ios_base::failure) {
    FieldDefinitionImpl fd(index++, fieldName, FieldDefinitionImpl::TYPE_SHORT_ARRAY);
    cd.add(fd);
};


void ClassDefinitionWriter::writePortableArray(string fieldName, Array<Portable*>& portables) throw(ios_base::failure) {
    
    int classId = portables[0]->getClassId();
    for (int i = 1; i < portables.length(); i++) {
        if (portables[i]->getClassId() != classId) {
            throw "Illegal Argument Exception";
        }
    }
    FieldDefinitionImpl fd(index++, fieldName, FieldDefinitionImpl::TYPE_PORTABLE_ARRAY, classId);
    addNestedField(*(portables[0]), fd);
};


void ClassDefinitionWriter::addNestedField(Portable& p, FieldDefinitionImpl& fd) throw(ios_base::failure) {
    cd.add(fd);
    ClassDefinitionImpl nestedCd =  serializer->getClassDefinition(p);

    cd.add(nestedCd);
};

