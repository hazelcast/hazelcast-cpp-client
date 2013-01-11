//
//  FieldDefinition.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef Server_FieldDefinition_h
#define Server_FieldDefinition_h

#include "DataSerializable.h"

class FieldDefinition : public DataSerializable{
public:
    const byte TYPE_PORTABLE = 0;
    const byte TYPE_BYTE = 1;
    const byte TYPE_BOOLEAN = 2;
    const byte TYPE_CHAR = 3;
    const byte TYPE_SHORT = 4;
    const byte TYPE_INT = 5;
    const byte TYPE_LONG = 6;
    const byte TYPE_FLOAT = 7;
    const byte TYPE_DOUBLE = 8;
    const byte TYPE_UTF = 9;
    const byte TYPE_PORTABLE_ARRAY = 10;
    const byte TYPE_BYTE_ARRAY = 11;
    const byte TYPE_CHAR_ARRAY = 12;
    const byte TYPE_SHORT_ARRAY = 13;
    const byte TYPE_INT_ARRAY = 14;
    const byte TYPE_LONG_ARRAY = 15;
    const byte TYPE_FLOAT_ARRAY = 16;
    const byte TYPE_DOUBLE_ARRAY = 17;
    
    virtual byte getType() = 0;
    
    virtual std::string getName() = 0;
    
    virtual int getIndex() = 0;
    
    virtual int getClassId() = 0;
};

#endif
