//
//  DataOutput.cpp
//  Server
//
//  Created by sancar koyunlu on 1/3/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#include "ContextAwareDataOutput.h"
#include <string>

ContextAwareDataOutput::ContextAwareDataOutput(SerializationService* service){
    this->service = service;
};

ContextAwareDataOutput::ContextAwareDataOutput(int size,SerializationService* service){
    //TODO think if necessary
};

ContextAwareDataOutput::ContextAwareDataOutput(ByteArray& buffer, int size,SerializationService* service){
    //TODO think if necessary
};

void ContextAwareDataOutput::write(char *bytes, int off, int len) throw(std::ios_base::failure){
    buffer.write(bytes + off , sizeof(char) * len);
};

void ContextAwareDataOutput::writeBoolean(bool i) throw(std::ios_base::failure){
    writeByte(i);
};

void ContextAwareDataOutput::writeByte(int i) throw(std::ios_base::failure){
    buffer.put(0xff & i);
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


std::string ContextAwareDataOutput::toString(){//TODO remove
    return buffer.str();
};
