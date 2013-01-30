//
//  DataInput.cpp
//  Server
//
//  Created by sancar koyunlu on 1/3/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
#include "DataInput.h"
#include "ClassDefinition.h"
#include "SerializationService.h"
#include "SerializationContext.h"
#include <iostream>


namespace hazelcast{ 
namespace client{
namespace serialization{
//TODO uncomment code below !!!!! main test fails
// DataInput::DataInput(Array<byte>& rhsBuffer, SerializationService* service):buffer(rhsBuffer)
//                                                                            ,service(service)
//                                                                            ,ptr(const_cast<byte*>(buffer.getBuffer()))
//                                                                            ,beg(ptr)
//{   
//};
//
//DataInput::DataInput(Data& data, SerializationService* service):buffer(data.buffer)
//                                                               ,service(service)
//                                                               ,ptr(const_cast<byte*>(buffer.getBuffer()))
//                                                               ,beg(ptr)
//                                                               ,dataClassId(data.cd != NULL ? data.cd->getClassId() : -1)
//                                                               ,dataVersion(data.cd != NULL ? data.cd->getVersion() : -1)
//{   
//};
 
   
DataInput::DataInput(Array<byte>& buffer, SerializationService* service){
    this->buffer = buffer;
    this->ptr = const_cast<byte*>(this->buffer.getBuffer());
    this->beg = this->ptr;
    this->service = service;
};

DataInput::DataInput(Data& data, SerializationService* service){
    const ClassDefinition* cd = data.cd;
    this->dataClassId = cd != NULL ? cd->getClassId() : -1;
    this->dataVersion = cd != NULL ? cd->getVersion() : -1;
    this->buffer = data.buffer;
    this->ptr =  const_cast<byte*>(this->buffer.getBuffer());
    this->beg = ptr;
    this->service = service;
};

int DataInput::getDataClassId(){
    return dataClassId;
};

void DataInput::setDataClassId(int id){
    dataClassId = id;
};

int DataInput::getDataVersion(){
    return dataVersion;
};

SerializationContext* DataInput::getSerializationContext(){
    return service->getSerializationContext();
}
//Inherited from DataInoput
void DataInput::readFully(Array<byte>& bytes){
    byte temp[bytes.length()];
    readFully(temp,0,bytes.length());
    bytes = Array<byte>(bytes.length(),temp);
};

void DataInput::readFully(byte *bytes, int off, int len){
    memcpy(bytes + off, ptr, sizeof(byte) * len);
    ptr += sizeof(byte) * len;
};

int DataInput::skipBytes(int i){
    ptr += i;
    return i;
};

bool DataInput::readBoolean(){
    return readByte();
};

byte DataInput::readByte(){
    byte b;
    memcpy(&b, ptr, sizeof(byte));
    ptr += sizeof(byte);
    return b;
};

short DataInput::readShort(){
    byte a = readByte();
    byte b = readByte();
    return (0xff00 & (a << 8 )) |
           (0x00ff &  b);
};

char DataInput::readChar(){
    readByte();
    byte b = readByte();
    return b;
};

int DataInput::readInt(){
    byte a = readByte();
    byte b = readByte();
    byte c = readByte();
    byte d = readByte();
    return (0xff000000 & (a << 24)) |
           (0x00ff0000 & (b << 16)) |
           (0x0000ff00 & (c << 8 )) |
           (0x000000ff &  d);
};

long DataInput::readLong(){
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

float DataInput::readFloat(){
    union {
        int i;
        float f;
    } u;
    u.i = readInt();
    return u.f;
};

double DataInput::readDouble(){
    union {
        double d;
        long l;
    } u;
    u.l = readLong();
    return u.d;
};

std::string DataInput::readUTF() throw(std::string) {
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
int DataInput::read(int index) throw (std::ios_base::failure){
    int pos = position();
    position(index);
    int v = readByte();
    position(pos);
    return v;
};

int DataInput::read(int index, byte* b, int off, int len) throw (std::ios_base::failure) {
    int pos = position();
    position(index);
    readFully(b,off,len); 
    position(pos);
    return len;
};

int DataInput::readInt(int index) throw (std::ios_base::failure) {
    int pos = position();
    position(index);
    int v = readInt();
    position(pos);
    return v;
};

long DataInput::readLong(int index) throw (std::ios_base::failure) {
    int pos = position();
    position(index);
    long v = readLong();
    position(pos);
    return v;
};

bool DataInput::readBoolean(int index) throw (std::ios_base::failure) {
    int pos = position();
    position(index);
    bool v = readBoolean();
    position(pos);
    return v;
};

byte DataInput::readByte(int index) throw (std::ios_base::failure) {
    int pos = position();
    position(index);
    byte v = readByte();
    position(pos);
    return v;
};

char DataInput::readChar(int index) throw (std::ios_base::failure) {
    int pos = position();
    position(index);
    char v = readChar();
    position(pos);
    return v;
};

double DataInput::readDouble(int index) throw (std::ios_base::failure){
    int pos = position();
    position(index);
    double v = readDouble();
    position(pos);
    return v;
};

float DataInput::readFloat(int index) throw (std::ios_base::failure) {
    int pos = position();
    position(index);
    float v = readFloat();
    position(pos);
    return v;
};

short DataInput::readShort(int index) throw (std::ios_base::failure){
    int pos = position();
    position(index);
    short v = readShort();
    position(pos);
    return v;
};

int DataInput::position(){
    return int(ptr - beg);
};

void DataInput::position(int newPos){
    ptr = beg + newPos;
};
//private functions

std::string DataInput::readShortUTF() throw(std::ios_base::failure){
    short utflen = readShort();
    byte bytearr[utflen];
    char chararr[utflen];
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

}}}