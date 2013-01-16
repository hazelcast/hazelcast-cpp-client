//
//  PortableSerializer.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef __Server__PortableSerializer__
#define __Server__PortableSerializer__

#include <iostream>
#include "Portable.h"
#include "SerializationContextImpl.h"
#include "ClassDefinitionImpl.h"
/*
//class PortableSerializer : public TypeSerializer{TODO may be deleted
class PortableSerializer{
public:
    PortableSerializer(SerializationContext*);
    ~PortableSerializer();
    
    ClassDefinitionImpl* getClassDefinition(Portable) throw(std::ios_base::failure) ;
    
    int getTypeId();
    
    SerializationContext getContext();//TODO propbaly unused check later
    
    int getVersion();
    
    void write(DataOutput, Portable ) throw(std::ios_base::failure) ;
    
    Portable read(DataInput) throw(std::ios_base::failure);
private:
    SerializationContext* context;//TODO think again
};
 */
#endif /* defined(__Server__PortableSerializer__) */
