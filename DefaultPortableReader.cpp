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
#include "ByteArray.h"
#include "CharArray.h"
#include "DoubleArray.h"
#include "FloatArray.h"
#include "IntegerArray.h"
#include "LongArray.h"
#include "PortablePointerArray.h"
#include "ShortArray.h"

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

ByteArray* DefaultPortableReader::readByteArray(string fieldName) throw(ios_base::failure){
    int pos = getPosition(fieldName);
    input->position(pos);
    int len = input->readInt();
    ByteArray* values = new ByteArray(len);
    for (int i = 0; i < len; i++) {
        (*values)[i] = input->readByte();
    }
    return values;
};

CharArray* DefaultPortableReader::readCharArray(string fieldName) throw(ios_base::failure){
    int pos = getPosition(fieldName);
    input->position(pos);
    int len = input->readInt();
    CharArray* values = new CharArray(len);
    for (int i = 0; i < len; i++) {
        (*values)[i] = input->readByte();
    }
    return values;
};

IntegerArray* DefaultPortableReader::readIntArray(string fieldName) throw(ios_base::failure){
    int pos = getPosition(fieldName);
    input->position(pos);
    int len = input->readInt();
    IntegerArray* values = new IntegerArray(len);
    for (int i = 0; i < len; i++) {
        (*values)[i] = input->readByte();
    }
    return values;
};

LongArray* DefaultPortableReader::readLongArray(string fieldName) throw(ios_base::failure){
    int pos = getPosition(fieldName);
    input->position(pos);
    int len = input->readInt();
    LongArray* values = new LongArray(len);
    for (int i = 0; i < len; i++) {
        (*values)[i] = input->readByte();
    }
    return values;
};

DoubleArray* DefaultPortableReader::readDoubleArray(string fieldName) throw(ios_base::failure){
    int pos = getPosition(fieldName);
    input->position(pos);
    int len = input->readInt();
    DoubleArray* values = new DoubleArray(len);
    for (int i = 0; i < len; i++) {
        (*values)[i] = input->readByte();
    }
    return values;
};

FloatArray* DefaultPortableReader::readFloatArray(string fieldName) throw(ios_base::failure){
    int pos = getPosition(fieldName);
    input->position(pos);
    int len = input->readInt();
    FloatArray* values = new FloatArray(len);
    for (int i = 0; i < len; i++) {
        (*values)[i] = input->readByte();
    }
    return values;
};

ShortArray* DefaultPortableReader::readShortArray(string fieldName) throw(ios_base::failure){
    int pos = getPosition(fieldName);
    input->position(pos);
    int len = input->readInt();
    ShortArray* values = new ShortArray(len);
    for (int i = 0; i < len; i++) {
        (*values)[i] = input->readByte();
    }
    return values;
};

PortablePointerArray* DefaultPortableReader::readPortableArray(string fieldName) throw(ios_base::failure){
    FieldDefinitionImpl* fd = cd->get(fieldName);
    if(fd != NULL){
        throw "unknown field exception " + fieldName;
    }
    int pos = getPosition(fieldName);
    input->position(pos);
    int len = input->readInt();
    PortablePointerArray* portables = new PortablePointerArray(len);
    input->setDataClassId(fd->getClassId());
    for (int i = 0; i < len; i++) {
        (*portables)[i] = serializer->read(input);
    }
    return portables;
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
