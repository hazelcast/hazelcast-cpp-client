//
//  DataOutput.h
//  Server
//
//  Created by sancar koyunlu on 1/3/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef Server_ContextAwareDataOutput_h
#define Server_ContextAwareDataOutput_h

#include <string>
#include <iostream>
#include <sstream>
#include "BufferObjectDataOutput.h"
#include "SerializationService.h"
typedef unsigned char byte;

class ContextAwareDataOutput : public BufferObjectDataOutput{
public:
    ContextAwareDataOutput(SerializationService*);
    
    ContextAwareDataOutput(int , SerializationService*);
    
    ContextAwareDataOutput(ByteArray&,int,SerializationService*);
    
    ByteArray* toByteArray(){};
    int getSize(){};
    
    std::string toString(); //TODO remove
   
    //Inherited from DataOutput
    void write(char* bytes, int offset, int length)throw (std::ios_base::failure) ;
    
    void writeBoolean(bool b) throw (std::ios_base::failure) ;
    
    void writeByte(int i) throw (std::ios_base::failure) ;
    
    void writeShort(int i)throw (std::ios_base::failure) ;
    
    void writeChar(int i)throw (std::ios_base::failure) ;
    
    void writeInt(int i)throw (std::ios_base::failure) ;
    
    void writeLong(long l)throw (std::ios_base::failure) ;
    
    void writeFloat(float v)throw (std::ios_base::failure);
    
    void writeDouble(double v)throw (std::ios_base::failure) ;
    
    void writeUTF(std::string s)throw (std::ios_base::failure) ;
    
    //Inherited from BufferObjectDataOutput
    void write(int index, int b){};
    
    void write(int index, byte b[], int off, int len){};
    
    void writeInt(int index, int v) throw (std::ios_base::failure){};
    
    void writeLong(int index, const long v) throw (std::ios_base::failure){};
    
    void writeBoolean(int index, const bool v) throw (std::ios_base::failure){};
    
    void writeByte(int index, const int v) throw (std::ios_base::failure){};
    
    void writeChar(int index, const int v) throw (std::ios_base::failure){};
    
    void writeDouble(int index, const double v) throw (std::ios_base::failure){};
    
    void writeFloat(int index, const float v) throw (std::ios_base::failure){};
    
    void writeShort(int index, const int v) throw (std::ios_base::failure){};
    
    int position(){};
    
    void position(int newPos){};
    
    byte* getBuffer(){};
    
    void reset(){};
    
    //Inherited from Closable
    void close() throw(std::ios_base::failure){};
private:
    std::ostringstream buffer;
    int const offset = 0;
    SerializationService* service;
    
    static int const DEFAULT_SIZE = 1024 * 4;
    
    static int const STRING_CHUNK_SIZE = 16 * 1024;
};

#endif
