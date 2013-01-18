//
//  PortableSerializer.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef __Server__PortableSerializer__
#define __Server__PortableSerializer__

#include "Portable.h"
#include "SerializationContextImpl.h"
#include "ClassDefinitionImpl.h"
#include "ClassDefinitionWriter.h"
class ContextAwareDataInput;
class ContextAwareDataOutput;

class PortableSerializer{
public:
    PortableSerializer(SerializationContextImpl*);
    ~PortableSerializer();
    
    
    ClassDefinitionImpl* getClassDefinition(Portable& p) throw(std::ios_base::failure);
    
    int getTypeId();
    
    SerializationContextImpl* getContext();//TODO propbaly unused check later
    
    int getVersion();
    
    void write(ContextAwareDataOutput* output, Portable& p) throw(std::ios_base::failure);
    
    Portable* read(ContextAwareDataInput* dataInput) throw(std::ios_base::failure);
    
private:
    SerializationContextImpl* context;//TODO think again
};
 
#endif /* defined(__Server__PortableSerializer__) */
