//
//  ClassDefinitionWriter.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#include "ClassDefinitionWriter.h"

ClassDefinitionWriter::ClassDefinitionWriter(int classId){};

void ClassDefinitionWriter::writeInt(string fieldName, int value) throw(ios_base::failure){};

void ClassDefinitionWriter::writeLong(string fieldName, long value) throw(ios_base::failure){};

void ClassDefinitionWriter::writeUTF(string fieldName, string str) throw(ios_base::failure){};

void ClassDefinitionWriter::writeBoolean(string fieldName, bool value) throw(ios_base::failure) {};

void ClassDefinitionWriter::writeByte(string fieldName, byte value) throw(ios_base::failure) {};

void ClassDefinitionWriter::writeChar(string fieldName, int value) throw(ios_base::failure) {};

void ClassDefinitionWriter::writeDouble(string fieldName, double value) throw(ios_base::failure) {};

void ClassDefinitionWriter::writeFloat(string fieldName, float value) throw(ios_base::failure) {};

void ClassDefinitionWriter::writeShort(string fieldName, short value) throw(ios_base::failure) {};

void ClassDefinitionWriter::writePortable(string fieldName, Portable portable) throw(ios_base::failure) {};

void ClassDefinitionWriter::writeByteArray(string fieldName, byte* bytes) throw(ios_base::failure) {};
    
void ClassDefinitionWriter::writeCharArray(string fieldName, char* chars) throw(ios_base::failure) {};
    
void ClassDefinitionWriter::writeIntArray(string fieldName, int* ints) throw(ios_base::failure) {};
    
void ClassDefinitionWriter::writeLongArray(string fieldName, long* longs) throw(ios_base::failure) {};
    
void ClassDefinitionWriter::writeDoubleArray(string fieldName, double* values) throw(ios_base::failure) {};

void ClassDefinitionWriter::writeFloatArray(string fieldName, float* values) throw(ios_base::failure) {};
    
void ClassDefinitionWriter::writeShortArray(string fieldName, short* values) throw(ios_base::failure) {};
    
void ClassDefinitionWriter::writePortableArray(string fieldName, Portable* portables) throw(ios_base::failure) {};

void ClassDefinitionWriter::addNestedField(Portable p, FieldDefinitionImpl fd) throw(ios_base::failure) {};
