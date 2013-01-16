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
//TODO ask if necessary add offset
class ContextAwareDataOutput : public BufferObjectDataOutput{
public:
    ContextAwareDataOutput(SerializationService*);
    
    ContextAwareDataOutput(int ,std::ostringstream*, SerializationService*);
    
    ContextAwareDataOutput(std::ostringstream*,SerializationService*);
    
    ByteArray* toByteArray();
    
    int getSize();
    
    SerializationContext& getSerializationContext();
    
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
    void write(int index, int b) throw (std::ios_base::failure);
    
    void write(int index, char* b, int off, int len) throw (std::ios_base::failure);
    
    void writeInt(int index, int v) throw (std::ios_base::failure);
    
    void writeLong(int index, const long v) throw (std::ios_base::failure);
    
    void writeBoolean(int index, const bool v) throw (std::ios_base::failure);
    
    void writeByte(int index, const int v) throw (std::ios_base::failure);
    
    void writeChar(int index, const int v) throw (std::ios_base::failure);
    
    void writeDouble(int index, const double v) throw (std::ios_base::failure);
    
    void writeFloat(int index, const float v) throw (std::ios_base::failure);
    
    void writeShort(int index, const int v) throw (std::ios_base::failure);
    
    int position();
    
    void position(int newPos);
    
//    ByteArray* getBuffer();//TODO delete or implement
    
    virtual BufferObjectDataOutput* duplicate();
    
    virtual BufferObjectDataOutput* slice();
    
    void reset();
    
    //Inherited from Closable
    void close() throw(std::ios_base::failure){};//TODO delete or implement
    
    static int const STRING_CHUNK_SIZE = 16 * 1024;//TODO move to private and Input class friend
    
private:
    std::ostringstream* buffer;
//    int const offset = 0;

    SerializationService* service;
    
    static int const DEFAULT_SIZE = 1024 * 4;
    
    
    void writeShortUTF(std::string) throw(std::ios_base::failure);
    
};

#endif
