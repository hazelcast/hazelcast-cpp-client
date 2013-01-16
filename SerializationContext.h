//
//  SerializationContext.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef Server_SerializationContext_h
#define Server_SerializationContext_h

#include<iostream>
#include "Portable.h"
#include "ClassDefinitionImpl.h"
class SerializationContext{
public:
    virtual ClassDefinitionImpl* lookup(int) = 0;
    
    virtual ClassDefinitionImpl* lookup(int, int) = 0;
    
    virtual Portable* createPortable(int) = 0;
    
    virtual ClassDefinitionImpl createClassDefinition(byte*) throw(std::ios_base::failure) = 0;
    
    virtual void registerClassDefinition(ClassDefinitionImpl*) throw(std::ios_base::failure) = 0;
    
    virtual int getVersion() = 0;
};

#endif
