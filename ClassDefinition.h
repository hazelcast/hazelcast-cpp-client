//
//  ClassDefinition.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef Server_ClassDefinition_h
#define Server_ClassDefinition_h
#include <iostream>
#include <string>
#include "DataSerializable.h"
#include "FieldDefinitionImpl.h"
#include "Array.h"

class ClassDefinition : public DataSerializable{
public:
    virtual const FieldDefinitionImpl& get(std::string name) = 0;
    
    virtual const FieldDefinitionImpl& get(int fieldIndex) = 0;
    
    virtual int getFieldCount() = 0;
    
    virtual int getClassId() const = 0;
    
    virtual int getVersion() const = 0;
    
    virtual Array<byte> getBinary() const = 0;
};

#endif
