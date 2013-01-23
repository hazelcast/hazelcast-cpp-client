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
#include "Array.h"

DefaultPortableReader::DefaultPortableReader(PortableSerializer* serializer, ContextAwareDataInput& input, ClassDefinitionImpl* cd){
    this->serializer = serializer;
    this->input = &input;
    this->cd = cd;
    this->offset = input.position();
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
    return input->readShort(pos);
};

string DefaultPortableReader::readUTF(string fieldName) throw(ios_base::failure){
    int pos = getPosition(fieldName);
    input->position(pos);
    return input->readUTF();
};

auto_ptr<Portable> DefaultPortableReader::readPortable(string fieldName) throw(ios_base::failure) {
    if(!cd->isFieldDefinitionExists(fieldName))
       throw "throwUnknownFieldException" + fieldName;
     
    FieldDefinitionImpl fd = cd->get(fieldName);

    int pos = getPosition(&fd);
    input->position(pos);
    bool isNull = input->readBoolean();
    if (isNull) {//TODO search for return NULL
        auto_ptr<Portable> x;
        return x;
    }
    input->setDataClassId(fd.getClassId());
    auto_ptr<Portable> p =  serializer->read(*input);
    
    input->setDataClassId(cd->classId);
    return p;
};

Array<byte> DefaultPortableReader::readByteArray(string fieldName) throw(ios_base::failure){
    int pos = getPosition(fieldName);
    input->position(pos);
    int len = input->readInt();
    Array<byte> values(len);
    for (int i = 0; i < len; i++) {
        values[i] = input->readByte();
    }
    return values;
};

Array<char> DefaultPortableReader::readCharArray(string fieldName) throw(ios_base::failure){
    int pos = getPosition(fieldName);
    input->position(pos);
    int len = input->readInt();
    Array<char> values(len);
    for (int i = 0; i < len; i++) {
        values[i] = input->readChar();
    }
    return values;
};

Array<int> DefaultPortableReader::readIntArray(string fieldName) throw(ios_base::failure){
    int pos = getPosition(fieldName);
    input->position(pos);
    int len = input->readInt();
    Array<int> values(len);
    for (int i = 0; i < len; i++) {
        values[i] = input->readInt();
    }
    return values;
};

Array<long> DefaultPortableReader::readLongArray(string fieldName) throw(ios_base::failure){
    int pos = getPosition(fieldName);
    input->position(pos);
    int len = input->readInt();
    Array<long> values(len);
    for (int i = 0; i < len; i++) {
        values[i] = input->readLong();
    }
    return values;
};

Array<double> DefaultPortableReader::readDoubleArray(string fieldName) throw(ios_base::failure){
    int pos = getPosition(fieldName);
    input->position(pos);
    int len = input->readInt();
    Array<double> values(len);
    for (int i = 0; i < len; i++) {
        values[i] = input->readDouble();
    }
    return values;
};

Array<float> DefaultPortableReader::readFloatArray(string fieldName) throw(ios_base::failure){
    int pos = getPosition(fieldName);
    input->position(pos);
    int len = input->readInt();
    Array<float> values(len);
    for (int i = 0; i < len; i++) {
        values[i] = input->readFloat();
    }
    return values;
};

Array<short> DefaultPortableReader::readShortArray(string fieldName) throw(ios_base::failure){
    int pos = getPosition(fieldName);
    input->position(pos);
    int len = input->readInt();
    Array<short> values(len);
    for (int i = 0; i < len; i++) {
        values[i] = input->readShort();
    }
    return values;
};

Array< auto_ptr<Portable> > DefaultPortableReader::readPortableArray(string fieldName) throw(ios_base::failure){//TODO
    if(!cd->isFieldDefinitionExists(fieldName))
        throw "throwUnknownFieldException" + fieldName;
    FieldDefinitionImpl fd = cd->get(fieldName);
    
    int pos = getPosition(fieldName);
    input->position(pos);
    int len = input->readInt();
    Array<auto_ptr<Portable> > portables(len);
    input->setDataClassId(fd.getClassId());
    for (int i = 0; i < len; i++) {
        portables[i] = serializer->read(*input);
    }
    return portables;
};

int DefaultPortableReader::getPosition(string fieldName) throw(ios_base::failure){
    if(!cd->isFieldDefinitionExists(fieldName))
        throw "throwUnknownFieldException" + fieldName;
    FieldDefinitionImpl fd = cd->get(fieldName);
    return getPosition(&fd);
};

int DefaultPortableReader::getPosition(FieldDefinitionImpl* fd) throw(ios_base::failure){
    return input->readInt(offset + fd->getIndex() * 4);
};
