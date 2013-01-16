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
    static const byte TYPE_PORTABLE = 0;
    static const byte TYPE_BYTE = 1;
    static const byte TYPE_BOOLEAN = 2;
    static const byte TYPE_CHAR = 3;
    static const byte TYPE_SHORT = 4;
    static const byte TYPE_INT = 5;
    static const byte TYPE_LONG = 6;
    static const byte TYPE_FLOAT = 7;
    static const byte TYPE_DOUBLE = 8;
    static const byte TYPE_UTF = 9;
    static const byte TYPE_PORTABLE_ARRAY = 10;
    static const byte TYPE_BYTE_ARRAY = 11;
    static const byte TYPE_CHAR_ARRAY = 12;
    static const byte TYPE_SHORT_ARRAY = 13;
    static const byte TYPE_INT_ARRAY = 14;
    static const byte TYPE_LONG_ARRAY = 15;
    static const byte TYPE_FLOAT_ARRAY = 16;
    static const byte TYPE_DOUBLE_ARRAY = 17;
    
    virtual byte getType() = 0;
    
    virtual std::string getName() = 0;
    
    virtual int getIndex() = 0;
    
    virtual int getClassId() = 0;
};

#endif
