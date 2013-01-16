//
//  ClassDefinitionWriter.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#include "ClassDefinitionWriter.h"
#include "FieldDefinitionImpl.h"
ClassDefinitionWriter::ClassDefinitionWriter(int classId, int version){
    cd.classId = classId;
    cd.version = version;
};

void ClassDefinitionWriter::writeInt(string fieldName, int value) throw(ios_base::failure){
    cd.add(new FieldDefinitionImpl(index++, fieldName, FieldDefinitionImpl::TYPE_INT));
};

void ClassDefinitionWriter::writeLong(string fieldName, long value) throw(ios_base::failure){
    cd.add(new FieldDefinitionImpl(index++, fieldName, FieldDefinitionImpl::TYPE_LONG));
};

void ClassDefinitionWriter::writeUTF(string fieldName, string str) throw(ios_base::failure){
    cd.add(new FieldDefinitionImpl(index++, fieldName, FieldDefinitionImpl::TYPE_UTF));
};

void ClassDefinitionWriter::writeBoolean(string fieldName, bool value) throw(ios_base::failure) {
    cd.add(new FieldDefinitionImpl(index++, fieldName, FieldDefinitionImpl::TYPE_BOOLEAN));
};

void ClassDefinitionWriter::writeByte(string fieldName, byte value) throw(ios_base::failure) {
    cd.add(new FieldDefinitionImpl(index++, fieldName, FieldDefinitionImpl::TYPE_BYTE));
};

void ClassDefinitionWriter::writeChar(string fieldName, int value) throw(ios_base::failure) {
    cd.add(new FieldDefinitionImpl(index++, fieldName, FieldDefinitionImpl::TYPE_CHAR));
};

void ClassDefinitionWriter::writeDouble(string fieldName, double value) throw(ios_base::failure) {
    cd.add(new FieldDefinitionImpl(index++, fieldName, FieldDefinitionImpl::TYPE_DOUBLE));
};

void ClassDefinitionWriter::writeFloat(string fieldName, float value) throw(ios_base::failure) {
    cd.add(new FieldDefinitionImpl(index++, fieldName, FieldDefinitionImpl::TYPE_FLOAT));
};

void ClassDefinitionWriter::writeShort(string fieldName, short value) throw(ios_base::failure) {
    cd.add(new FieldDefinitionImpl(index++, fieldName, FieldDefinitionImpl::TYPE_SHORT));
};
/*
void ClassDefinitionWriter::writePortable(string fieldName, Portable portable) throw(ios_base::failure) {
    FieldDefinitionImpl* fd = new FieldDefinitionImpl(index++, fieldName, FieldDefinitionImpl::TYPE_PORTABLE, portable.getClassId());
    addNestedField(&portable, fd);
};
*/
void ClassDefinitionWriter::writeByteArray(string fieldName, byte* bytes) throw(ios_base::failure) {
    cd.add(new FieldDefinitionImpl(index++, fieldName, FieldDefinitionImpl::TYPE_BYTE_ARRAY));
};
    
void ClassDefinitionWriter::writeCharArray(string fieldName, char* chars) throw(ios_base::failure) {
    cd.add(new FieldDefinitionImpl(index++, fieldName, FieldDefinitionImpl::TYPE_CHAR_ARRAY));
};
    
void ClassDefinitionWriter::writeIntArray(string fieldName, int* ints) throw(ios_base::failure) {
    cd.add(new FieldDefinitionImpl(index++, fieldName, FieldDefinitionImpl::TYPE_INT_ARRAY));
};
    
void ClassDefinitionWriter::writeLongArray(string fieldName, long* longs) throw(ios_base::failure) {
    cd.add(new FieldDefinitionImpl(index++, fieldName, FieldDefinitionImpl::TYPE_LONG_ARRAY));
};
    
void ClassDefinitionWriter::writeDoubleArray(string fieldName, double* values) throw(ios_base::failure) {
    cd.add(new FieldDefinitionImpl(index++, fieldName, FieldDefinitionImpl::TYPE_DOUBLE_ARRAY));
};

void ClassDefinitionWriter::writeFloatArray(string fieldName, float* values) throw(ios_base::failure) {
    cd.add(new FieldDefinitionImpl(index++, fieldName, FieldDefinitionImpl::TYPE_FLOAT_ARRAY));
};
    
void ClassDefinitionWriter::writeShortArray(string fieldName, short* values) throw(ios_base::failure) {
    cd.add(new FieldDefinitionImpl(index++, fieldName, FieldDefinitionImpl::TYPE_SHORT_ARRAY));
};

/*
void ClassDefinitionWriter::writePortableArray(string fieldName, Portable* portables) throw(ios_base::failure) {
    if (portables == NULL) {
        throw "Illegal Argument Exception";
    }
    Portable* p = portables;
    int classId = p->getClassId();
//    for (int i = 1; i < portables.length; i++) {//TODO
    for (int i = 1; i < 2; i++) {
        if (portables[i].getClassId() != classId) {
            throw "Illegal Argument Exception";
        }
    }
    FieldDefinitionImpl* fd = new FieldDefinitionImpl(index++, fieldName,
                                                      FieldDefinitionImpl::TYPE_PORTABLE_ARRAY, classId);
    addNestedField(p, fd);
};
*/

void ClassDefinitionWriter::addNestedField(Portable* p, FieldDefinitionImpl* fd) throw(ios_base::failure) {
    cd.add(fd);
//    ClassDefinitionImpl* nestedCd = getClassDefinition(p);
    ClassDefinitionImpl nestedCd;
    cd.add(nestedCd);
};

