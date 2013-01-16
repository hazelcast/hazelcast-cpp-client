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
#include "ClassDefinition.h"
#include "ClassDefinitionImpl.h"
//#include "PortableSerializer.h"
#include "TypeSerializer.h"
#include "DataInput.h"
#include "BufferObjectDataOutput.h"
#include "Portable.h"

using namespace std;

class DefaultPortableWriter{
    
    //    DefaultPortableWriter(PortableSerializer serializer, BufferObjectDataOutput out, ClassDefinitionImpl cd);
    DefaultPortableWriter(TypeSerializer serializer, BufferObjectDataOutput out, ClassDefinitionImpl cd);
    
    
    void writeInt(string fieldName, int value) throw(ios_base::failure);
    
    void writeLong(string fieldName, long value) throw(ios_base::failure);
    
    void writeUTF(string fieldName, string str) throw(ios_base::failure);
    
    void writeBoolean(string fieldName, bool value) throw(ios_base::failure);
    
    void writeByte(string fieldName, byte value) throw(ios_base::failure);
    
    void writeChar(string fieldName, int value) throw(ios_base::failure);
    
    void writeDouble(string fieldName, double value) throw(ios_base::failure);
    
    void writeFloat(string fieldName, float value) throw(ios_base::failure);
    
    void writeShort(string fieldName, short value) throw(ios_base::failure);
    
//    void writePortable(string fieldName, Portable portable) throw(ios_base::failure);
    
    void writeByteArray(string fieldName, byte* values) throw(ios_base::failure);
    
    void writeCharArray(string fieldName, char* values) throw(ios_base::failure);
    
    void writeIntArray(string fieldName, int* values) throw(ios_base::failure);
    
    void writeLongArray(string fieldName, long* values) throw(ios_base::failure);
    
    void writeDoubleArray(string fieldName, double* values) throw(ios_base::failure);
    
    void writeFloatArray(string fieldName, float* values) throw(ios_base::failure);
    
    void writeShortArray(string fieldName, short* values) throw(ios_base::failure);
    
    void writePortableArray(string fieldName, Portable* portables) throw(ios_base::failure);
    
    void setPosition(string fieldName) throw(ios_base::failure);
    
    static void writeNullablestring(DataOutput output, string obj);
private:
//    PortableSerializer* serializer;
    TypeSerializer* serializer;
    
    ClassDefinitionImpl cd;
    BufferObjectDataOutput* output;
    int offset;
    //    int fieldIndex = 0;

};
#endif /* defined(__Server__DefaultPortableWriter__) */
