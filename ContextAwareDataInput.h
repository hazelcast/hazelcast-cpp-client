//
//  DataInput.h
//  Server
//
//  Created by sancar koyunlu on 1/3/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef __Server__ContextAwareDataInput__
#define __Server__ContextAwareDataInput__

#include <string>
#include "BufferObjectDataInput.h"
#include "ContextAwareDataOutput.h"
#include "Data.h"
class SerializationServiceImpl;

typedef unsigned char byte;
//TODO ask if necessary add offset
class ContextAwareDataInput : public BufferObjectDataInput{
public:
    ContextAwareDataInput(byte*, SerializationServiceImpl* service);
    
    ContextAwareDataInput(Data&, SerializationServiceImpl* service);
    
    std::string readUTF() throw(std::string);
    
    int getDataClassId();
    
    void setDataClassId(int);
    
    int getDataVersion();
    
    //Inherited from DataInoput
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
    
    BufferObjectDataInput* duplicate();
    
    BufferObjectDataInput* slice();
    
    //inherited from closable
    void close() throw(std::ios_base::failure);
private:
    byte* ptr;
    byte* beg;
    
    int size;
    SerializationServiceImpl* service;
    int dataClassId;
    
    int dataVersion;
    
    static int const STRING_CHUNK_SIZE = ContextAwareDataOutput::STRING_CHUNK_SIZE;
    
    std::string readShortUTF() throw (std::ios_base::failure) ;
    
    
//    int mark ;
    
    
};

#endif /* defined(__Server__DataInput__) */
