//
//  DataInput.h
//  Server
//
//  Created by sancar koyunlu on 1/3/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_DATA_INPUT
#define HAZELCAST_DATA_INPUT

#include "DataOutput.h"
#include "Data.h"
#include <string>

namespace hazelcast{ 
namespace client{
namespace serialization{
    
class SerializationService;
class SerializationContext;

typedef unsigned char byte;
//TODO ask if necessary add offset
class DataInput{
public:
    DataInput(Array<byte>&, SerializationService* service);
    
    DataInput(Data&, SerializationService* service);
  
    int getDataClassId();
    
    void setDataClassId(int);
    
    int getDataVersion();
    
    SerializationContext* getSerializationContext();
    
    //Inherited from DataInput
    void readFully(Array<byte>&);
    
    void readFully(byte* bytes, int off, int len);
    
    int skipBytes(int i);
    
    bool readBoolean();
    
    byte readByte();
  
    short readShort();
    
    char readChar();
    
    int readInt();
    
    long readLong();
    
    float readFloat();
    
    double readDouble();
    
    std::string readUTF();
    //Inherited from BufferObjectDataInput
    int read(int index) ;
    
    int read(int index, byte* b, int off, int len) ;
    
    int readInt(int index) ;
    
    long readLong(int index) ;
    
    bool readBoolean(int index) ;
    
    byte readByte(int index) ;
    
    char readChar(int index) ;
    
    double readDouble(int index) ;
    
    float readFloat(int index) ;
    
    short readShort(int index) ;
    
    std::string readUTF(int);
    
    int position();
    
    void position(int newPos);
    
    void reset();
    
private:
    Array<byte> buffer;
    SerializationService* service;
    byte* ptr;
    byte* beg;
    int dataClassId;
    int dataVersion;
    
    static int const STRING_CHUNK_SIZE = DataOutput::STRING_CHUNK_SIZE;
    std::string readShortUTF() ;
    DataInput& operator=(const DataInput&); 
};

}}}
#endif /* HAZELCAST_DATA_INPUT */
