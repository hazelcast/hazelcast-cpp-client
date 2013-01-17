//
//  DefaultPortableReader.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
#include <string>
#include "Portable.h"
#include "DefaultPortableReader.h"
#include "ContextAwareDataInput.h"
#include "PortableSerializer.h"

DefaultPortableReader::DefaultPortableReader(PortableSerializer* serializer, ContextAwareDataInput* input, ClassDefinitionImpl* cd){
    this->serializer = serializer;
    this->input = input;
    this->cd = cd;
    this->offset = input->position();
};

int DefaultPortableReader::readInt(string fieldName) throw(ios_base::failure){
    int pos = getPosition(fieldName);
    return input->readInt(pos);
};

long DefaultPortableReader::readLong(string fieldName) throw(ios_base::failure){
    int pos = getPosition(fieldName);
    return input->readLong(pos);
};

bool DefaultPortableReader::readBoolean(string fieldName) throw(ios_base::failure){
    int pos = getPosition(fieldName);
    return input->readBoolean(pos);
};

byte DefaultPortableReader::readByte(string fieldName) throw(ios_base::failure){
    int pos = getPosition(fieldName);
    return input->readByte(pos);
};

char DefaultPortableReader::readChar(string fieldName) throw(ios_base::failure){
    int pos = getPosition(fieldName);
    return input->readChar(pos);
};

double DefaultPortableReader::readDouble(string fieldName) throw(ios_base::failure){
    int pos = getPosition(fieldName);
    return input->readDouble(pos);
};

float DefaultPortableReader::readFloat(string fieldName) throw(ios_base::failure){
    int pos = getPosition(fieldName);
    return input->readFloat(pos);
};


short DefaultPortableReader::readShort(string fieldName) throw(ios_base::failure){
    int pos = getPosition(fieldName);
    return input->readInt(pos);
};

string DefaultPortableReader::readUTF(string fieldName) throw(ios_base::failure){
    int pos = getPosition(fieldName);
    return input->readUTF();
};

Portable* DefaultPortableReader::readPortable(string fieldName) throw(ios_base::failure) {
    FieldDefinitionImpl* fd = cd->get(fieldName);
    if (fd == NULL) {
        throw "UnknownFieldException" + fieldName;
    }
    int pos = getPosition(fd);
    input->position(pos);
    bool isNull = input->readBoolean();
    if (!isNull) {
 
        input->setDataClassId(fd->getClassId());
        input->setDataClassId(cd->classId);
        return serializer->read(input);
        
    }
    return NULL;
};

ByteArray& DefaultPortableReader::readByteArray(string fieldName) throw(ios_base::failure){
    int pos = getPosition(fieldName);
    input->position(pos);
    int len = input->readInt();
    ByteArray* values = new ByteArray(len);
    for (int i = 0; i < len; i++) {
        (*values)[i] = input->readByte();
    }
    return *values;
};

char* DefaultPortableReader::readCharArray(string fieldName) throw(ios_base::failure){

};

int* DefaultPortableReader::readIntArray(string fieldName) throw(ios_base::failure){

};

long* DefaultPortableReader::readLongArray(string fieldName) throw(ios_base::failure){

};

double* DefaultPortableReader::readDoubleArray(string fieldName) throw(ios_base::failure){

};

float* DefaultPortableReader::readFloatArray(string fieldName) throw(ios_base::failure){

};

short* DefaultPortableReader::readShortArray(string fieldName) throw(ios_base::failure){

};

Portable* DefaultPortableReader::readPortableArray(string fieldName) throw(ios_base::failure){

};

int DefaultPortableReader::getPosition(string fieldName) throw(ios_base::failure){
     FieldDefinitionImpl* fd = cd->get(fieldName);
     if (fd == NULL) {
         std::string error = "UnknownFieldException" ;
         error += fieldName;  
     }
    return getPosition(fd);
};

int DefaultPortableReader::getPosition(FieldDefinitionImpl* fd) throw(ios_base::failure){
    return input->readInt(offset + fd->getIndex() * 4);
};

string DefaultPortableReader::readNullableString(DataInput* input) throw(ios_base::failure) {

};