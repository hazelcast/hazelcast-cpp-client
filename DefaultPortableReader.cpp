//
//  DefaultPortableReader.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#include "Portable.h"
#include "DefaultPortableReader.h"
#include "ContextAwareDataInput.h"
#include "PortableSerializer.h"

DefaultPortableReader::DefaultPortableReader(PortableSerializer* serializer, ContextAwareDataInput* input, ClassDefinitionImpl* cd){
    this->serializer = serializer;
    this->input = input;
    this->cd = cd;
    this->offset = input->position();
};

int DefaultPortableReader::readInt(string fieldName) throw(ios_base::failure){
    int pos = getPosition(fieldName);
    return input->readInt(pos);
};

long DefaultPortableReader::readLong(string fieldName) throw(ios_base::failure){};

string DefaultPortableReader::readUTF(string fieldName) throw(ios_base::failure){};

bool DefaultPortableReader::readBoolean(string fieldName) throw(ios_base::failure){};

byte DefaultPortableReader::readByte(string fieldName) throw(ios_base::failure){};

char DefaultPortableReader::readChar(string fieldName) throw(ios_base::failure){};

double DefaultPortableReader::readDouble(string fieldName) throw(ios_base::failure){};

float DefaultPortableReader::readFloat(string fieldName) throw(ios_base::failure){
    int pos = getPosition(fieldName);
    return input->readFloat(pos);
};


short DefaultPortableReader::readShort(string fieldName) throw(ios_base::failure){};

Portable* DefaultPortableReader::readPortable(string fieldName) throw(ios_base::failure) {
    FieldDefinitionImpl* fd = cd->get(fieldName);
    if (fd == NULL) {
        throw "UnknownFieldException" + fieldName;
    }
    int pos = getPosition(fd);
    input->position(pos);
    bool isNull = input->readBoolean();
    if (!isNull) {
 
        input->setDataClassId(fd->getClassId());
        input->setDataClassId(cd->classId);
        return serializer->read(input);
        
    }
    return NULL;
};

byte* DefaultPortableReader::readByteArray(string fieldName) throw(ios_base::failure){};

char* DefaultPortableReader::readCharArray(string fieldName) throw(ios_base::failure){};

int* DefaultPortableReader::readIntArray(string fieldName) throw(ios_base::failure){};

long* DefaultPortableReader::readLongArray(string fieldName) throw(ios_base::failure){};

double* DefaultPortableReader::readDoubleArray(string fieldName) throw(ios_base::failure){};

float* DefaultPortableReader::readFloatArray(string fieldName) throw(ios_base::failure){};

short* DefaultPortableReader::readShortArray(string fieldName) throw(ios_base::failure){};

Portable* DefaultPortableReader::readPortableArray(string fieldName) throw(ios_base::failure){};

int DefaultPortableReader::getPosition(string fieldName) throw(ios_base::failure){
    return 0;
};

int DefaultPortableReader::getPosition(FieldDefinitionImpl* fd) throw(ios_base::failure){};

string DefaultPortableReader::readNullableString(DataInput* input) throw(ios_base::failure) {};