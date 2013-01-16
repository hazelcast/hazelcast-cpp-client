//
//  SerializationContextImpl.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#include "SerializationContextImpl.h"
#include "SerializationServiceImpl.h"

SerializationContextImpl::SerializationContextImpl(PortableFactory* portableFactory, int version){
    this->portableFactory = portableFactory;
    this->version = version;
};

ClassDefinitionImpl* SerializationContextImpl::lookup(int classId){
    return versionedDefinitions[SerializationServiceImpl::combineToLong(classId, version)];
};

ClassDefinitionImpl* SerializationContextImpl::lookup(int classId, int version){
    return versionedDefinitions[SerializationServiceImpl::combineToLong(classId, version)];
};

Portable* SerializationContextImpl::createPortable(int classId){
    
};

ClassDefinitionImpl* SerializationContextImpl::createClassDefinition(byte *compressedBinary){
    
};

void SerializationContextImpl::registerNestedDefinitions(ClassDefinitionImpl* cd) throw(std::ios_base::failure){
    
};

void SerializationContextImpl::registerClassDefinition(ClassDefinitionImpl* cd) throw(std::ios_base::failure){
    
};

int SerializationContextImpl::getVersion(){
    
};

void SerializationContextImpl::compress(byte*, std::ostream) throw(std::ios_base::failure){
    
};//TODO zip in c++

void SerializationContextImpl::decompress(byte*, std::ostream ) throw(std::ios_base::failure){
    
};//TODO unzip in c++