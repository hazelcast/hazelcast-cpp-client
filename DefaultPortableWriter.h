//
//  DefaultPortableWriter.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef __Server__DefaultPortableWriter__
#define __Server__DefaultPortableWriter__

#include <iostream>
#include <string>
#include "ClassDefinitionImpl.h"
#include "PortableSerializer.h"
#include "DataInput.h"
#include "ContextAwareDataOutput.h"
#include "Portable.h"
class PortablePointerArray;

using namespace std;

class DefaultPortableWriter : public PortableWriter{
public:
    DefaultPortableWriter(PortableSerializer* serializer, ContextAwareDataOutput* output, ClassDefinitionImpl* cd);
    
    
    void writeInt(string fieldName, int value) throw(ios_base::failure);
    
    void writeLong(string fieldName, long value) throw(ios_base::failure);
    
    void writeBoolean(string fieldName, bool value) throw(ios_base::failure);
    
    void writeByte(string fieldName, byte value) throw(ios_base::failure);
    
    void writeChar(string fieldName, int value) throw(ios_base::failure);
    
    void writeDouble(string fieldName, double value) throw(ios_base::failure);
    
    void writeFloat(string fieldName, float value) throw(ios_base::failure);
    
    void writeShort(string fieldName, short value) throw(ios_base::failure);
    
    void writeUTF(string fieldName, string str) throw(ios_base::failure);
    
    void writePortable(string fieldName, Portable& portable) throw(ios_base::failure);
    
    void writeByteArray(string fieldName, ByteArray&) throw(ios_base::failure);
    
    void writeCharArray(string fieldName, char* values, int len) throw(ios_base::failure);
    
    void writeIntArray(string fieldName, int* values, int len) throw(ios_base::failure);
    
    void writeLongArray(string fieldName, long* values, int len) throw(ios_base::failure);
    
    void writeDoubleArray(string fieldName, double* values, int len) throw(ios_base::failure);
    
    void writeFloatArray(string fieldName, float* values, int len) throw(ios_base::failure);
    
    void writeShortArray(string fieldName, short* values, int len) throw(ios_base::failure);
    
    void writePortableArray(string fieldName, PortablePointerArray& portables) throw(ios_base::failure);
    
    void setPosition(string fieldName) throw(ios_base::failure);
    
//    static void writeNullablestring(ContextAwareDataOutput* output, string obj); TODO no need in c++ because there cannot be null strings
private:
    PortableSerializer* serializer;

    ClassDefinitionImpl* cd;
    ContextAwareDataOutput* output;
    int offset;
    //    int fieldIndex = 0;

};
#endif /* defined(__Server__DefaultPortableWriter__) */
