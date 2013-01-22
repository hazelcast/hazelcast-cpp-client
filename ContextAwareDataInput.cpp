//
//  DataInput.cpp
//  Server
//
//  Created by sancar koyunlu on 1/3/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
#include <iostream>
#include "ContextAwareDataInput.h"
#include "ClassDefinitionImpl.h"
class SerializationServiceImpl;

ContextAwareDataInput::ContextAwareDataInput(Array<byte>& buffer, SerializationServiceImpl* service){
    this->buffer = buffer;
    this->ptr = this->buffer.buffer;
    this->beg = this->buffer.buffer;
    this->service = service;
};

ContextAwareDataInput::ContextAwareDataInput(Data& data, SerializationServiceImpl* service){
    this->dataClassId = data.cd.getClassId();
    this->dataVersion = data.cd.getVersion();
    this->buffer = data.buffer;
    this->ptr = buffer.buffer;
    this->beg = ptr;
    this->service = service;
};

int ContextAwareDataInput::getDataClassId(){
    return dataClassId;
};

void ContextAwareDataInput::setDataClassId(int id){
    dataClassId = id;
};

int ContextAwareDataInput::getDataVersion(){
    return dataVersion;
};

//Inherited from DataInoput
void ContextAwareDataInput::readFully(byte *bytes, int off, int len){
    memcpy(bytes + off, ptr, sizeof(byte) * len);
    ptr += sizeof(byte) * len;
};

int ContextAwareDataInput::skipBytes(int i){
    ptr += i;
    return i;
};

bool ContextAwareDataInput::readBoolean(){
    return readByte();
};

byte ContextAwareDataInput::readByte(){
    byte b;
    memcpy(&b, ptr, sizeof(byte));
    ptr += sizeof(byte);
    return b;
};

short ContextAwareDataInput::readShort(){
    byte a = readByte();
    byte b = readByte();
    return (0xff00 & (a << 8 )) |
           (0x00ff &  b);
};

char ContextAwareDataInput::readChar(){
    readByte();
    byte b = readByte();
    return b;
};

int ContextAwareDataInput::readInt(){
    byte a = readByte();
    byte b = readByte();
    byte c = readByte();
    byte d = readByte();
    return (0xff000000 & (a << 24)) |
           (0x00ff0000 & (b << 16)) |
           (0x0000ff00 & (c << 8 )) |
           (0x000000ff &  d);
};

long ContextAwareDataInput::readLong(){
    byte a = readByte();
    byte b = readByte();
    byte c = readByte();
    byte d = readByte();
    byte e = readByte();
    byte f = readByte();
    byte g = readByte();
    byte h = readByte();
    return (0xff00000000000000 & (long(a) << 56)) |
           (0x00ff000000000000 & (long(b) << 48)) |
           (0x0000ff0000000000 & (long(c) << 40)) |
           (0x000000ff00000000 & (long(d) << 32)) |
           (0x00000000ff000000 & (e << 24)) |
           (0x0000000000ff0000 & (f << 16)) |
           (0x000000000000ff00 & (g << 8 )) |
           (0x00000000000000ff &  h);
};

float ContextAwareDataInput::readFloat(){
    union {
        int i;
        float f;
    } u;
    u.i = readInt();
    return u.f;
};

double ContextAwareDataInput::readDouble(){
    union {
        double d;
        long l;
    } u;
    u.l = readLong();
    return u.d;
};

std::string ContextAwareDataInput::readUTF() throw(std::string) {
    bool isNull = readBoolean();
    if (isNull)
        return NULL;
    int length = readInt();
    std::string result;
    int chunkSize = (length / STRING_CHUNK_SIZE) + 1;
    while (chunkSize > 0) {
        result.append(readShortUTF());
        chunkSize--;
    }
    return result;
};

//Inherited from BufferObjectDataInput
int ContextAwareDataInput::read(int index) throw (std::ios_base::failure){
    int pos = position();
    position(index);
    int v = readByte();
    position(pos);
    return v;
};

int ContextAwareDataInput::read(int index, byte* b, int off, int len) throw (std::ios_base::failure) {
    int pos = position();
    position(index);
    readFully(b,off,len); 
    position(pos);
    return len;
};

int ContextAwareDataInput::readInt(int index) throw (std::ios_base::failure) {
    int pos = position();
    position(index);
    int v = readInt();
    position(pos);
    return v;
};

long ContextAwareDataInput::readLong(int index) throw (std::ios_base::failure) {
    int pos = position();
    position(index);
    long v = readLong();
    position(pos);
    return v;
};

bool ContextAwareDataInput::readBoolean(int index) throw (std::ios_base::failure) {
    int pos = position();
    position(index);
    bool v = readBoolean();
    position(pos);
    return v;
};

byte ContextAwareDataInput::readByte(int index) throw (std::ios_base::failure) {
    int pos = position();
    position(index);
    byte v = readByte();
    position(pos);
    return v;
};

char ContextAwareDataInput::readChar(int index) throw (std::ios_base::failure) {
    int pos = position();
    position(index);
    char v = readChar();
    position(pos);
    return v;
};

double ContextAwareDataInput::readDouble(int index) throw (std::ios_base::failure){
    int pos = position();
    position(index);
    double v = readDouble();
    position(pos);
    return v;
};

float ContextAwareDataInput::readFloat(int index) throw (std::ios_base::failure) {
    int pos = position();
    position(index);
    float v = readFloat();
    position(pos);
    return v;
};

short ContextAwareDataInput::readShort(int index) throw (std::ios_base::failure){
    int pos = position();
    position(index);
    short v = readShort();
    position(pos);
    return v;
};

int ContextAwareDataInput::position(){
    return int(ptr - beg);
};

void ContextAwareDataInput::position(int newPos){
    ptr = beg + newPos;
};


void ContextAwareDataInput::reset(){
    //TODO
};

BufferObjectDataInput* ContextAwareDataInput::duplicate(){
    //TODO
};

BufferObjectDataInput* ContextAwareDataInput::slice(){
    //TODO
};

//Inherited from closeable

void ContextAwareDataInput::close() throw(std::ios_base::failure){
    //TODO
};

//private functions

std::string ContextAwareDataInput::readShortUTF() throw(std::ios_base::failure){
    short utflen = readShort();
    byte* bytearr = new byte[utflen];
    char* chararr = new char[utflen];
    int c, char2, char3;
    int count = 0;
    int chararr_count = 0;
    readFully(bytearr, 0, utflen);
    
    while (count < utflen) {
        c = bytearr[count] & 0xff;
        switch (0x0f & (c >> 4) ) {
            case 0:
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
                /* 0xxxxxxx */
                count++;
                chararr[chararr_count++] = (char) c;
                break;
            case 12:
            case 13:
                /* 110x xxxx 10xx xxxx */
                count += 2;
                if (count > utflen)
                    throw "malformed input: partial character at end";
                char2 = bytearr[count - 1];
                if ((char2 & 0xC0) != 0x80){
                    std::string error = "malformed input around byte" ;
                    error += count;
                    throw error;
                }
                chararr[chararr_count++] = (char) (((c & 0x1F) << 6) | (char2 & 0x3F));
                break;
            case 14:
                /* 1110 xxxx 10xx xxxx 10xx xxxx */
                count += 3;
                if (count > utflen)
                    throw "malformed input: partial character at end";
                char2 = bytearr[count - 2];
                char3 = bytearr[count - 1];
                if (((char2 & 0xC0) != 0x80) || ((char3 & 0xC0) != 0x80)){
                    std::string error = "malformed input around byte" ;
                    error += count - 1;
                    throw error;
                }
                chararr[chararr_count++] = (char) (((c & 0x0F) << 12) | ((char2 & 0x3F) << 6) | ((char3 & 0x3F) << 0));
                break;
            default:
                /* 10xx xxxx, 1111 xxxx */
                
                std::string error = "malformed input around byte" ;
                error += count;
                throw error;
                
        }
    }
    chararr[chararr_count++] = '\0';
    return chararr;
};