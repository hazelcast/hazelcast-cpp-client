//
//  DefaultPortableWriter.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#include "DefaultPortableWriter.h"
#include "DataOutput.h"
#include "ClassDefinition.h"
#include "PortableSerializer.h"
#include "FieldDefinition.h"
#include "../Array.h"

namespace hazelcast{ 
namespace client{
namespace serialization{

DefaultPortableWriter::DefaultPortableWriter(PortableSerializer* serializer, DataOutput* output, ClassDefinition* cd){
    this->serializer = serializer;
    this->output = output;
    this->offset = output->position();
    this->cd = cd;
    char* zeros = new  char[offset + cd->getFieldCount() * 4]; 
    this->output->write(zeros,offset, cd->getFieldCount() * 4);
    delete [] zeros;
};

void DefaultPortableWriter::writeInt(string fieldName, int value){
    setPosition(fieldName);
    output->writeInt(value);
};

void DefaultPortableWriter::writeLong(string fieldName, long value){
    setPosition(fieldName);
    output->writeLong(value);
};

void DefaultPortableWriter::writeBoolean(string fieldName, bool value){
    setPosition(fieldName);
    output->writeBoolean(value);
};

void DefaultPortableWriter::writeByte(string fieldName, byte value){
    setPosition(fieldName);
    output->writeByte(value);
};

void DefaultPortableWriter::writeChar(string fieldName, int value){
    setPosition(fieldName);
    output->writeChar(value);
};

void DefaultPortableWriter::writeDouble(string fieldName, double value){
    setPosition(fieldName);
    output->writeDouble(value);
};

void DefaultPortableWriter::writeFloat(string fieldName, float value){
    setPosition(fieldName);
    output->writeFloat(value);
};

void DefaultPortableWriter::writeShort(string fieldName, short value){
    setPosition(fieldName);
    output->writeShort(value);
};

void DefaultPortableWriter::writeUTF(string fieldName, string str){
    setPosition(fieldName);
    output->writeUTF(str);
};

void DefaultPortableWriter::writePortable(string fieldName, Portable& portable){
    setPosition(fieldName);
    bool isNull = &portable == NULL;
    output->writeBoolean(isNull);
    if (!isNull) {
        serializer->write(output, portable);
    }
};

void DefaultPortableWriter::writeByteArray(string fieldName, Array<byte>& values){
    setPosition(fieldName);
    int len = values.length();
    output->writeInt(len);
    if (len > 0) {
        for (int i = 0; i < len; i++) {
            output->writeByte(values[i]);
        }
    }
};

void DefaultPortableWriter::writeCharArray(string fieldName, Array<char>& values){
    setPosition(fieldName);
    int len = values.length();
    output->writeInt(len);
    if (len > 0) {
        for (int i = 0; i < len; i++) {
            output->writeChar(values[i]);
        }
    }
};

void DefaultPortableWriter::writeIntArray(string fieldName, Array<int>& values){
    setPosition(fieldName);
    int len = values.length();
    output->writeInt(len);
    if (len > 0) {
        for (int i = 0; i < len; i++) {
            output->writeInt(values[i]);
        }
    }
};

void DefaultPortableWriter::writeLongArray(string fieldName, Array<long>& values){
    setPosition(fieldName);
    int len = values.length();
    output->writeInt(len);
    if (len > 0) {
        for (int i = 0; i < len; i++) {
            output->writeLong(values[i]);
        }
    }
};

void DefaultPortableWriter::writeDoubleArray(string fieldName, Array<double>& values){
    setPosition(fieldName);
    int len = values.length();
    output->writeInt(len);
    if (len > 0) {
        for (int i = 0; i < len; i++) {
            output->writeDouble(values[i]);
        }
    }
};

void DefaultPortableWriter::writeFloatArray(string fieldName, Array<float>& values){
    setPosition(fieldName);
    int len = values.length();
    output->writeInt(len);
    if (len > 0) {
        for (int i = 0; i < len; i++) {
            output->writeFloat(values[i]);
        }
    }
};

void DefaultPortableWriter::writeShortArray(string fieldName, Array<short>& values){
    setPosition(fieldName);
    int len = values.length();
    output->writeInt(len);
    if (len > 0) {
        for (int i = 0; i < len; i++) {
            output->writeShort(values[i]);
        }
    }
};

void DefaultPortableWriter::writePortableArray(string fieldName, Array<Portable*>& portables){
    setPosition(fieldName);
    int len = portables.length();
    output->writeInt(len);
    if (len > 0) {
        for (int i = 0; i < len; i++) {
            serializer->write(output, *(portables[i]) );
        }
    }
};

void DefaultPortableWriter::setPosition(string fieldName){
    if(!cd->isFieldDefinitionExists(fieldName)){
       std::string error;
       error +=  "HazelcastSerializationException( Invalid field name: '";
       error += fieldName;
       error += "' for ClassDefinition {id: "; 
       error += cd->getClassId();
       error += ", version: ";
       error += cd->getVersion();
       error += "}";
        
       throw error;
    }
    FieldDefinition fd = cd->get(fieldName);
    
    int pos = output->position();
    int index = fd.getIndex();
    // index = fieldIndex++; // if class versions are the same.
    output->writeInt(offset + index * 4, pos);
    
};

}}}

