//
//  DataOutput.cpp
//  Server
//
//  Created by sancar koyunlu on 1/3/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#include "ContextAwareDataOutput.h"
#include "SerializationServiceImpl.h"
#include <string>
#include <algorithm>
#include <cassert>
ContextAwareDataOutput::ContextAwareDataOutput(SerializationServiceImpl* service){
    this->service = service;
    this->buffer = new std::ostringstream;
};

ContextAwareDataOutput::ContextAwareDataOutput(int offset,std::ostringstream* buffer,SerializationServiceImpl* service){
    this->buffer = buffer;    
    this->service = service;//TODO implement
};

ContextAwareDataOutput::ContextAwareDataOutput(std::ostringstream* buffer,SerializationServiceImpl* service){
    this->buffer = buffer;
    this->service = service;
};

ByteArray* ContextAwareDataOutput::toByteArray(){
    int size = getSize();
    ByteArray* byteArray = new ByteArray(size);
    std::string str = buffer->str();
    for(int i = 0; i < size ; i++){
        (*byteArray)[i] = str[i];
    }
    return byteArray;
};

int ContextAwareDataOutput::getSize(){
    return (int)buffer->str().length();
};

SerializationContextImpl* ContextAwareDataOutput::getSerializationContext(){
    return service->getSerializationContext();
};


std::string ContextAwareDataOutput::toString(){//TODO remove
    return buffer->str();
};

//Inherited from DataOutput
void ContextAwareDataOutput::write(char *bytes, int off, int len) throw(std::ios_base::failure){
    buffer->write(bytes + off , sizeof(char) * len);
};

void ContextAwareDataOutput::writeBoolean(bool i) throw(std::ios_base::failure){
    writeByte(i);
};

void ContextAwareDataOutput::writeByte(int i) throw(std::ios_base::failure){
    buffer->put(0xff & i);
};

void ContextAwareDataOutput::writeShort(int v) throw(std::ios_base::failure){
    writeByte((v >> 8));
    writeByte(v);
};

void ContextAwareDataOutput::writeChar(int i) throw(std::ios_base::failure){
    writeByte((i >> 8));
    writeByte(i);
};

void ContextAwareDataOutput::writeInt(int v) throw(std::ios_base::failure){
    writeByte((v >> 24));
    writeByte((v >> 16));
    writeByte((v >> 8));
    writeByte(v);
};

void ContextAwareDataOutput::writeLong(long l) throw(std::ios_base::failure){
    writeByte((l >> 56));
    writeByte((l >> 48));
    writeByte((l >> 40));
    writeByte((l >> 32));
    writeByte((l >> 24));
    writeByte((l >> 16));
    writeByte((l >> 8));
    writeByte((int)l);
};

void ContextAwareDataOutput::writeFloat(float x) throw(std::ios_base::failure){
    union {
        float f;
        int i;
    } u;
    u.f = x;
    writeInt(u.i);
};

void ContextAwareDataOutput::writeDouble(double v) throw(std::ios_base::failure){
    union {
        double d;
        long l;
    } u;
    u.d = v;
    writeLong(u.l);
};

void ContextAwareDataOutput::writeUTF(std::string str) throw(std::ios_base::failure){
    bool isNull = str.empty();
    writeBoolean(isNull);
    if (isNull)
        return;
    
    int length = (int)str.length();
    writeInt(length);
    int chunkSize = length / STRING_CHUNK_SIZE + 1;
    for (int i = 0; i < chunkSize; i++) {
        int beginIndex = std::max(0, i * STRING_CHUNK_SIZE - 1);
        int endIndex = std::max((i + 1) * STRING_CHUNK_SIZE - 1, length);
        writeShortUTF(str.substr(beginIndex, endIndex - beginIndex));
    }
};

//Inherited from BufferObjectDataOutput
void ContextAwareDataOutput::write(int index, int b) throw(std::ios_base::failure){
    int pos = position();
    position(index);
    writeByte(b);
    position(pos);
};


void ContextAwareDataOutput::write(int index, char* b, int off, int len) throw (std::ios_base::failure){
    int pos = position();
    position(index);
    write(b,off,len);
    position(pos);
};

void ContextAwareDataOutput::writeInt(int index, int v) throw (std::ios_base::failure){
    int pos = position();
    position(index);
    writeInt(v);
    position(pos);
};

void ContextAwareDataOutput::writeLong(int index, const long v) throw (std::ios_base::failure){
    int pos = position();
    position(index);
    writeLong(v);
    position(pos);
};

void ContextAwareDataOutput::writeBoolean(int index, const bool v) throw (std::ios_base::failure){
    int pos = position();
    position(index);
    writeBoolean(v);
    position(pos);
};

void ContextAwareDataOutput::writeByte(int index, const int v) throw (std::ios_base::failure){
    int pos = position();
    position(index);
    writeByte(v);
    position(pos);
};

void ContextAwareDataOutput::writeChar(int index, const int v) throw (std::ios_base::failure){
    int pos = position();
    position(index);
    writeChar(v);
    position(pos);
};

void ContextAwareDataOutput::writeDouble(int index, const double v) throw (std::ios_base::failure){
    int pos = position();
    position(index);
    writeDouble(v);
    position(pos);
};

void ContextAwareDataOutput::writeFloat(int index, const float v) throw (std::ios_base::failure){
    int pos = position();
    position(index);
    writeFloat(v);
    position(pos);
};

void ContextAwareDataOutput::writeShort(int index, const int v) throw (std::ios_base::failure){
    int pos = position();
    position(index);
    writeShort(v);
    position(pos);
};

int ContextAwareDataOutput::position(){
    assert(buffer->tellp() != -1);
    return (int)buffer->tellp();
};

void ContextAwareDataOutput::position(int newPos){
    buffer->seekp(newPos);
    assert(buffer->tellp() != -1);
    assert(buffer->eofbit);
    assert(buffer->failbit); 
    assert(buffer->badbit );
    
};

BufferObjectDataOutput* ContextAwareDataOutput::duplicate(){
    return dynamic_cast<BufferObjectDataOutput*>(new ContextAwareDataOutput(buffer,service));
};

BufferObjectDataOutput* ContextAwareDataOutput::slice(){
    return dynamic_cast<BufferObjectDataOutput*>(new ContextAwareDataOutput(0,buffer,service));//TODO think on offset 0

};

void ContextAwareDataOutput::reset(){
    position(0);
};

//private functions
void ContextAwareDataOutput::writeShortUTF(std::string str) throw(std::ios_base::failure){
    int stringLen = (int)str.length();
    int utfLength = 0;
    int count = 0;
    /* use charAt instead of copying String to char array */
    for (int i = 0; i < stringLen; i++) {
        if ((str[i] >= 0x0001) && (str[i] <= 0x007F)) {
            utfLength++;
        } else if (str[i] > 0x07FF) {
            utfLength += 3;
        } else {
            utfLength += 2;
        }
    }
    if (utfLength > 65535) {
        std::string error = "encoded string too long:" ;
        error += utfLength;
        error += " bytes";
        throw error;
    }
    char* byteArray = new char[utfLength];
    int i;
    for (i = 0; i < stringLen; i++) {
        if (!((str[i] >= 0x0001) && (str[i] <= 0x007F)))
            break;
        byteArray[count++] = (byte) str[i];
    }
    for (; i < stringLen; i++) {
        if ((str[i] >= 0x0001) && (str[i] <= 0x007F)) {
            byteArray[count++] = (byte) str[i];
        } else if (str[i] > 0x07FF) {
            byteArray[count++] = (byte) (0xE0 | ((str[i] >> 12) & 0x0F));
            byteArray[count++] = (byte) (0x80 | ((str[i] >> 6) & 0x3F));
            byteArray[count++] = (byte) (0x80 | ((str[i]) & 0x3F));
        } else {
            byteArray[count++] = (byte) (0xC0 | ((str[i] >> 6) & 0x1F));
            byteArray[count++] = (byte) (0x80 | ((str[i]) & 0x3F));
        }
    }
    writeShort(utfLength);
    write(byteArray, 0, utfLength);
};
