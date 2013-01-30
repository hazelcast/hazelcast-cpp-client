//
//  PortableReader.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
#include "Portable.h"
#include "PortableReader.h"
#include "DataInput.h"
#include "PortableSerializer.h"
#include "FieldDefinition.h"
#include "../Array.h"
#include <string>

namespace hazelcast{ 
namespace client{
namespace serialization{

PortableReader::PortableReader(PortableSerializer* serializer, DataInput& input, ClassDefinition* cd){
    this->serializer = serializer;
    this->input = &input;
    this->cd = cd;
    this->offset = input.position();
};

int PortableReader::readInt(string fieldName){
    int pos = getPosition(fieldName);
    return input->readInt(pos);
};

long PortableReader::readLong(string fieldName){
    int pos = getPosition(fieldName);
    return input->readLong(pos);
};

bool PortableReader::readBoolean(string fieldName){
    int pos = getPosition(fieldName);
    return input->readBoolean(pos);
};

byte PortableReader::readByte(string fieldName){
    int pos = getPosition(fieldName);
    return input->readByte(pos);
};

char PortableReader::readChar(string fieldName){
    int pos = getPosition(fieldName);
    return input->readChar(pos);
};

double PortableReader::readDouble(string fieldName){
    int pos = getPosition(fieldName);
    return input->readDouble(pos);
};

float PortableReader::readFloat(string fieldName){
    int pos = getPosition(fieldName);
    return input->readFloat(pos);
};


short PortableReader::readShort(string fieldName){
    int pos = getPosition(fieldName);
    return input->readShort(pos);
};

string PortableReader::readUTF(string fieldName){
    int pos = getPosition(fieldName);
    input->position(pos);
    return input->readUTF();
};

auto_ptr<Portable> PortableReader::readPortable(string fieldName) {
    if(!cd->isFieldDefinitionExists(fieldName))
       throw "throwUnknownFieldException" + fieldName;

    FieldDefinition fd = cd->get(fieldName);

    int pos = getPosition(&fd);
    input->position(pos);
    bool isNull = input->readBoolean();
    if (isNull) {//TODO search for return NULL
        auto_ptr<Portable> x;
        return x;
    }
    input->setDataClassId(fd.getClassId());
    auto_ptr<Portable> p (serializer->read(*input) );

    input->setDataClassId(cd->getClassId());
    return p;
};

Array<byte> PortableReader::readByteArray(string fieldName){
    int pos = getPosition(fieldName);
    input->position(pos);
    int len = input->readInt();
    Array<byte> values(len);
    for (int i = 0; i < len; i++) {
        values[i] = input->readByte();
    }
    return values;
};

Array<char> PortableReader::readCharArray(string fieldName){
    int pos = getPosition(fieldName);
    input->position(pos);
    int len = input->readInt();
    Array<char> values(len);
    for (int i = 0; i < len; i++) {
        values[i] = input->readChar();
    }
    return values;
};

Array<int> PortableReader::readIntArray(string fieldName){
    int pos = getPosition(fieldName);
    input->position(pos);
    int len = input->readInt();
    Array<int> values(len);
    for (int i = 0; i < len; i++) {
        values[i] = input->readInt();
    }
    return values;
};

Array<long> PortableReader::readLongArray(string fieldName){
    int pos = getPosition(fieldName);
    input->position(pos);
    int len = input->readInt();
    Array<long> values(len);
    for (int i = 0; i < len; i++) {
        values[i] = input->readLong();
    }
    return values;
};

Array<double> PortableReader::readDoubleArray(string fieldName){
    int pos = getPosition(fieldName);
    input->position(pos);
    int len = input->readInt();
    Array<double> values(len);
    for (int i = 0; i < len; i++) {
        values[i] = input->readDouble();
    }
    return values;
};

Array<float> PortableReader::readFloatArray(string fieldName){
    int pos = getPosition(fieldName);
    input->position(pos);
    int len = input->readInt();
    Array<float> values(len);
    for (int i = 0; i < len; i++) {
        values[i] = input->readFloat();
    }
    return values;
};

Array<short> PortableReader::readShortArray(string fieldName){
    int pos = getPosition(fieldName);
    input->position(pos);
    int len = input->readInt();
    Array<short> values(len);
    for (int i = 0; i < len; i++) {
        values[i] = input->readShort();
    }
    return values;
};

Array< auto_ptr<Portable> > PortableReader::readPortableArray(string fieldName){
    if(!cd->isFieldDefinitionExists(fieldName))
          throw "throwUnknownFieldException" + fieldName;
      FieldDefinition fd = cd->get(fieldName);

      int pos = getPosition(fieldName);
      input->position(pos);
      int len = input->readInt();
      Array< auto_ptr<Portable> > portables(len);
      input->setDataClassId(fd.getClassId());
      for (int i = 0; i < len; i++) {
          portables[i] = serializer->read(*input);
      }
      return portables;

};

int PortableReader::getPosition(string fieldName){
    if(!cd->isFieldDefinitionExists(fieldName))
        throw "throwUnknownFieldException" + fieldName;
    FieldDefinition fd = cd->get(fieldName);
    return getPosition(&fd);
};

int PortableReader::getPosition(FieldDefinition* fd){
    return input->readInt(offset + fd->getIndex() * 4);
};

}}}
