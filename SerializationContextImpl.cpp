//
//  SerializationContextImpl.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#include "SerializationContextImpl.h"
#include "SerializationServiceImpl.h"
#include "ContextAwareDataOutput.h"

SerializationContextImpl::SerializationContextImpl(PortableFactory* portableFactory, int version, SerializationServiceImpl* service){
    this->portableFactory = portableFactory;
    this->version = version;
    this->service = service;
};

ClassDefinitionImpl* SerializationContextImpl::lookup(int classId){
    return versionedDefinitions[SerializationServiceImpl::combineToLong(classId, version)];
};

ClassDefinitionImpl* SerializationContextImpl::lookup(int classId, int version){
    return versionedDefinitions[SerializationServiceImpl::combineToLong(classId, version)];
};

Portable* SerializationContextImpl::createPortable(int classId){
    return portableFactory->create(classId);
};

ClassDefinitionImpl* SerializationContextImpl::createClassDefinition(byte *compressedBinary) throw(std::ios_base::failure){
    ContextAwareDataOutput* output = service->pop();
    ByteArray* binary;
//    try {
        decompress(compressedBinary, output);
        binary = output->toByteArray();
//    } finally {
        service->push(output);
//    }
    ClassDefinitionImpl* cd = new ClassDefinitionImpl();
    cd->readData(*dynamic_cast<DataInput*>(new ContextAwareDataInput(binary->getBuffer(), service)) );
    cd->setBinary(binary->getBuffer());
    //TODO below was putIfAbsent
    ClassDefinitionImpl* currentCD = versionedDefinitions[service->combineToLong(cd->classId, version)] = cd;
    if (currentCD == NULL) {
        registerNestedDefinitions(cd);
        return cd;
    } else {
        return currentCD;
    }
};

void SerializationContextImpl::registerNestedDefinitions(ClassDefinitionImpl* cd) throw(std::ios_base::failure){
    
};

void SerializationContextImpl::registerClassDefinition(ClassDefinitionImpl* cd) throw(std::ios_base::failure){
    
};

int SerializationContextImpl::getVersion(){
    
};

void SerializationContextImpl::compress(byte*, ContextAwareDataOutput*) throw(std::ios_base::failure){
    
};//TODO zip in c++

void SerializationContextImpl::decompress(byte*, ContextAwareDataOutput* ) throw(std::ios_base::failure){
    
};//TODO unzip in c++