//
//  DefaultPortableWriter.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#include "DefaultPortableWriter.h"


DefaultPortableWriter::DefaultPortableWriter(PortableSerializer* serializer, ContextAwareDataOutput* output, ClassDefinitionImpl* cd){
    this->serializer = serializer;
    this->output = output;
    this->offset = output->position();
    this->cd = cd;
    char* zeros = new  char[offset + cd->getFieldCount() * 4]; 
    this->output->write(zeros,0,offset + cd->getFieldCount() * 4);
    delete [] zeros;
};

void DefaultPortableWriter::writeInt(string fieldName, int value) throw(ios_base::failure){
    setPosition(fieldName);
    output->writeInt(value);
};

void DefaultPortableWriter::writeLong(string fieldName, long value) throw(ios_base::failure){
    setPosition(fieldName);
    output->writeLong(value);
};

void DefaultPortableWriter::writeBoolean(string fieldName, bool value) throw(ios_base::failure){
    setPosition(fieldName);
    output->writeBoolean(value);
};

void DefaultPortableWriter::writeByte(string fieldName, byte value) throw(ios_base::failure){
    setPosition(fieldName);
    output->writeByte(value);
};

void DefaultPortableWriter::writeChar(string fieldName, int value) throw(ios_base::failure){
    setPosition(fieldName);
    output->writeChar(value);
};

void DefaultPortableWriter::writeDouble(string fieldName, double value) throw(ios_base::failure){
    setPosition(fieldName);
    output->writeDouble(value);
};

void DefaultPortableWriter::writeFloat(string fieldName, float value) throw(ios_base::failure){
    setPosition(fieldName);
    output->writeFloat(value);
};

void DefaultPortableWriter::writeShort(string fieldName, short value) throw(ios_base::failure){
    setPosition(fieldName);
    output->writeShort(value);
};

void DefaultPortableWriter::writeUTF(string fieldName, string str) throw(ios_base::failure){
    setPosition(fieldName);
    output->writeUTF(str);
};

void DefaultPortableWriter::writePortable(string fieldName, Portable* portable) throw(ios_base::failure){
    setPosition(fieldName);
    bool isNull = portable == NULL;
    output->writeBoolean(isNull);
    if (!isNull) {
        serializer->write(output, portable);
    }
};

void DefaultPortableWriter::writeByteArray(string fieldName, ByteArray& values) throw(ios_base::failure){
    setPosition(fieldName);
    int len = values.length();
    output->writeInt(len);
    if (len > 0) {
        for (int i = 0; i < len; i++) {
            output->writeByte(values[i]);
        }
    }
};

void DefaultPortableWriter::writeCharArray(string fieldName, char* values, int len) throw(ios_base::failure){

};

void DefaultPortableWriter::writeIntArray(string fieldName, int* values, int len) throw(ios_base::failure){

};

void DefaultPortableWriter::writeLongArray(string fieldName, long* values, int len) throw(ios_base::failure){

};

void DefaultPortableWriter::writeDoubleArray(string fieldName, double* values, int len) throw(ios_base::failure){

};

void DefaultPortableWriter::writeFloatArray(string fieldName, float* values, int len) throw(ios_base::failure){

};

void DefaultPortableWriter::writeShortArray(string fieldName, short* values, int len) throw(ios_base::failure){

};

void DefaultPortableWriter::writePortableArray(string fieldName, Portable* portables, int len) throw(ios_base::failure){

};

void DefaultPortableWriter::setPosition(string fieldName) throw(ios_base::failure){
    FieldDefinitionImpl* fd = cd->get(fieldName);
    if (fd == NULL) {
        string error = "Invalid field name: '";
        error += fieldName;
        error += "' for ClassDefinition {id: ";
        error += cd->getClassId();
        error += ", version: ";
        error += cd->getVersion();
        error += "}";
        throw error;
    }
    
    int pos = output->position();
    int index = fd->getIndex();
    // index = fieldIndex++; // if class versions are the same.
    output->writeInt(offset + index * 4, pos);
    
};

void DefaultPortableWriter::writeNullablestring(ContextAwareDataOutput* output, string obj){
    
};
