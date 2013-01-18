//
//  ClassDefinitionWriter.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef __Server__ClassDefinitionWriter__
#define __Server__ClassDefinitionWriter__

#include <iostream>
#include <string>

#include "PortableWriter.h"
#include "ClassDefinitionImpl.h"
class FieldDefinitionImpl;
class Portable;
class PortableSerializer;

using namespace std;

class ClassDefinitionWriter : public PortableWriter{
public:
    ClassDefinitionWriter(int classId,int version, PortableSerializer* serializer);
    
    void writeInt(string fieldName, int value) throw(ios_base::failure);
    
    void writeLong(string fieldName, long value) throw(ios_base::failure);
    
    void writeUTF(string fieldName, string str) throw(ios_base::failure);
    
    void writeBoolean(string fieldName, bool value) throw(ios_base::failure) ;
    
    void writeByte(string fieldName, byte value) throw(ios_base::failure) ;
    
    void writeChar(string fieldName, int value) throw(ios_base::failure) ;
    
    void writeDouble(string fieldName, double value) throw(ios_base::failure) ;
    
    void writeFloat(string fieldName, float value) throw(ios_base::failure) ;
    
    void writeShort(string fieldName, short value) throw(ios_base::failure) ;
    
    void writePortable(string fieldName, Portable& portable) throw(ios_base::failure);
    
    void writeByteArray(string fieldName, ByteArray&) throw(ios_base::failure) ;
    
    void writeCharArray(string fieldName, char* chars, int len) throw(ios_base::failure) ;
    
    void writeIntArray(string fieldName, int* ints, int len) throw(ios_base::failure) ;
    
    void writeLongArray(string fieldName, long* longs, int len) throw(ios_base::failure) ;
    
    void writeDoubleArray(string fieldName, double* values, int len) throw(ios_base::failure) ;
    
    void writeFloatArray(string fieldName, float* values, int len) throw(ios_base::failure) ;
    
    void writeShortArray(string fieldName, short* values, int len) throw(ios_base::failure) ;
    
    
    void writePortableArray(string fieldName, PortablePointerArray& portables) throw(ios_base::failure);
    
    ClassDefinitionImpl cd;
private:
    void addNestedField(Portable& p, FieldDefinitionImpl* fd) throw(ios_base::failure) ;

    int index;
    PortableSerializer* serializer;
    
};

#endif /* defined(__Server__ClassDefinitionWriter__) */
