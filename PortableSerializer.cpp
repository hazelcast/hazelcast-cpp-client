//
//  PortableSerializer.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#include "PortableSerializer.h"

PortableSerializer::PortableSerializer(SerializationContext*){};
PortableSerializer::~PortableSerializer(){};

ClassDefinitionImpl PortableSerializer::getClassDefinition(Portable) throw(std::ios_base::failure) {};

int PortableSerializer::getTypeId(){};

SerializationContext PortableSerializer::getContext(){};//TODO propbaly unused check later

int PortableSerializer::getVersion(){};

void PortableSerializer::write(DataOutput, Portable ) throw(std::ios_base::failure) {};

Portable PortableSerializer::read(DataInput) throw(std::ios_base::failure){};