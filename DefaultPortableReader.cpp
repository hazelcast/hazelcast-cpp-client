//
//  DefaultPortableReader.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#include "DefaultPortableReader.h"

DefaultPortableReader::DefaultPortableReader(TypeSerializer p, DataInput d, ClassDefinitionImpl c){
};

int DefaultPortableReader::readInt(string fieldName) throw(ios_base::failure){};
long DefaultPortableReader::readLong(string fieldName) throw(ios_base::failure){};

string DefaultPortableReader::readUTF(string fieldName) throw(ios_base::failure){};

bool DefaultPortableReader::readBoolean(string fieldName) throw(ios_base::failure){};

byte DefaultPortableReader::readByte(string fieldName) throw(ios_base::failure){};

char DefaultPortableReader::readChar(string fieldName) throw(ios_base::failure){};

double DefaultPortableReader::readDouble(string fieldName) throw(ios_base::failure){};

float DefaultPortableReader::readFloat(string fieldName) throw(ios_base::failure){};

short DefaultPortableReader::readShort(string fieldName) throw(ios_base::failure){};

byte* DefaultPortableReader::readByteArray(string fieldName) throw(ios_base::failure){};

char* DefaultPortableReader::readCharArray(string fieldName) throw(ios_base::failure){};

int* DefaultPortableReader::readIntArray(string fieldName) throw(ios_base::failure){};

long* DefaultPortableReader::readLongArray(string fieldName) throw(ios_base::failure){};

double* DefaultPortableReader::readDoubleArray(string fieldName) throw(ios_base::failure){};

float* DefaultPortableReader::readFloatArray(string fieldName) throw(ios_base::failure){};

short* DefaultPortableReader::readShortArray(string fieldName) throw(ios_base::failure){};

Portable DefaultPortableReader::readPortable(string fieldName) throw(ios_base::failure) {};
//    private HazelcastSerializationException throwUnknownFieldException(string fieldName) {

Portable* DefaultPortableReader::readPortableArray(string fieldName) throw(ios_base::failure){};

int DefaultPortableReader::getPosition(string fieldName) throw(ios_base::failure){};

int DefaultPortableReader::getPosition(FieldDefinition fd) throw(ios_base::failure){};

string DefaultPortableReader::readNullableString(DataInput input) throw(ios_base::failure) {};