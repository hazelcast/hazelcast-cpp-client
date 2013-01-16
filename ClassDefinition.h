//
//  ClassDefinition.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef Server_ClassDefinition_h
#define Server_ClassDefinition_h

#include <string>
#include "DataSerializable.h"
#include "FieldDefinitionImpl.h"

class ClassDefinition : public DataSerializable{
public:
    virtual FieldDefinitionImpl* get(std::string name) = 0;
    
    virtual FieldDefinitionImpl* get(int fieldIndex) = 0;
    
    virtual int getFieldCount() = 0;
    
    virtual int getClassId() = 0;
    
    virtual int getVersion() = 0;
    
    virtual byte* getBinary() = 0;
};

#endif
