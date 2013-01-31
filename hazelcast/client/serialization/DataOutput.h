//
//  DataOutput.h
//  Server
//
//  Created by sancar koyunlu on 1/3/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_DATA_OUTPUT
#define HAZELCAST_DATA_OUTPUT

#include "../Array.h"
#include <string>
#include <iostream>
#include <sstream>
#include <memory>

namespace hazelcast{ 
namespace client{
namespace serialization{
    
class SerializationContext;
class SerializationService;

typedef unsigned char byte;
//TODO ask if necessary add offset
class DataOutput{
public:
    DataOutput(SerializationService*);
    
    Array<byte> toByteArray();
    
    int getSize();
    
    SerializationContext* getSerializationContext();
    
    
    //Inherited from DataOutput
    void write(const Array<byte>& bytes);
    
    void write(const char* bytes, int offset, int length);
    
    void writeBoolean(bool b) ;
    
    void writeByte(int i) ;
    
    void writeShort(int i);
    
    void writeChar(int i);
    
    void writeInt(int i);
    
    void writeLong(long l);
    
    void writeFloat(float v);
    
    void writeDouble(double v);
    
    void writeUTF(std::string s);
    
    //Inherited from BufferObjectDataOutput
    void write(int index, int b);
    
    void write(int index, char* b, int off, int len);
    
    void writeInt(int index, int v);
    
    void writeLong(int index, const long v);
    
    void writeBoolean(int index, const bool v);
    
    void writeByte(int index, const int v);
    
    void writeChar(int index, const int v);
    
    void writeDouble(int index, const double v);
    
    void writeFloat(int index, const float v);
    
    void writeShort(int index, const int v);
    
    int position();
    
    void position(int newPos);
    
    void reset();
    
    static int const STRING_CHUNK_SIZE = 16 * 1024;
    
private:
    std::ostringstream buffer;
    int const offset;

    SerializationService* service;
    
    static int const DEFAULT_SIZE = 1024 * 4;
    
    void writeShortUTF(std::string);
    
};

}}}
#endif /* HAZELCAST_DATA_OUTPUT */
