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
    
    std::string readUTF() throw(std::string);
    
    int getDataClassId();
    
    void setDataClassId(int);
    
    int getDataVersion();
    
    SerializationContext* getSerializationContext();
    
    
    //Inherited from DataInput
    void readFully(Array<byte>&);
    
    void readFully(byte* bytes, int off, int len);
    
    int skipBytes(int i) ;
    
    bool readBoolean();
    
    byte readByte();
  
    short readShort();
    
    char readChar();
    
    int readInt();
    
    long readLong();
    
    float readFloat();
    
    double readDouble();
    
    //Inherited from BufferObjectDataInput
    int read(int index) throw (std::ios_base::failure) ;
    
    int read(int index, byte* b, int off, int len) throw (std::ios_base::failure) ;
    
    int readInt(int index) throw (std::ios_base::failure) ;
    
    long readLong(int index) throw (std::ios_base::failure) ;
    
    bool readBoolean(int index) throw (std::ios_base::failure) ;
    
    byte readByte(int index) throw (std::ios_base::failure) ;
    
    char readChar(int index) throw (std::ios_base::failure) ;
    
    double readDouble(int index) throw (std::ios_base::failure) ;
    
    float readFloat(int index) throw (std::ios_base::failure) ;
    
    short readShort(int index) throw (std::ios_base::failure) ;
    
    int position() ;
    
    void position(int newPos) ;
    
    void reset() ;
    
private:
    byte* ptr;
    byte* beg;
    Array<byte> buffer;
    int size;
    SerializationService* service;
    int dataClassId;
    int dataVersion;
    
    static int const STRING_CHUNK_SIZE = DataOutput::STRING_CHUNK_SIZE;
    std::string readShortUTF() throw (std::ios_base::failure) ;
     
};

}}}
#endif /* HAZELCAST_DATA_INPUT */
