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

ClassDefinitionImpl SerializationContextImpl::lookup(int classId){
    return lookup(classId,version);
};

ClassDefinitionImpl SerializationContextImpl::lookup(int classId, int version){
    long key = SerializationServiceImpl::combineToLong(classId, version);
//    if(versionedDefinitions.count(key) > 0)
     return versionedDefinitions[key];
//    else
//        return auto_ptr<ClassDefinitionImpl>;
};

auto_ptr<Portable> SerializationContextImpl::createPortable(int classId){
    return portableFactory->create(classId);
};

ClassDefinitionImpl SerializationContextImpl::createClassDefinition(Array<byte>& compressedBinary) throw(std::ios_base::failure){
    ContextAwareDataOutput* output = service->pop();
    Array<byte> binary;

    decompress(compressedBinary, output);
    binary = output->toByteArray();

    service->push(output);

    
    ContextAwareDataInput dataInput = ContextAwareDataInput(binary, service);
    ClassDefinitionImpl cd;
    cd.readData(dataInput);
    cd.setBinary(binary);
            
    long key = service->combineToLong(cd.classId, version);
    bool exists = false;
    ClassDefinitionImpl currentCD;
    if(versionedDefinitions.count(key) > 0){
        exists = true;
        currentCD = versionedDefinitions[key];
    }
    versionedDefinitions[key] = cd;
    
    if (exists) {
        registerNestedDefinitions(cd);
        return cd;
    } else {
        return currentCD;
    }
};

void SerializationContextImpl::registerNestedDefinitions(ClassDefinitionImpl& cd) throw(std::ios_base::failure){
    vector<ClassDefinitionImpl> nestedDefinitions = cd.getNestedClassDefinitions();
    for(vector<ClassDefinitionImpl>::iterator it = nestedDefinitions.begin() ; it < nestedDefinitions.end() ; it++){
        registerClassDefinition(*it);
        registerNestedDefinitions(*it);
    }
};

void SerializationContextImpl::registerClassDefinition(ClassDefinitionImpl& cd) throw(std::ios_base::failure){
     
        long versionedClassId = service->combineToLong(cd.getClassId(), cd.getVersion());
        
        bool exists = false;
        if(versionedDefinitions.count(versionedClassId) > 0)
            exists = true;
        versionedDefinitions[versionedClassId] = cd;
        
        if (exists == false) {
            if (cd.getBinary().length() == 0) {
                ContextAwareDataOutput* output = service->pop();                
                cd.writeData(*output);
                Array<byte> uncompressed = output->toByteArray();
                compress(uncompressed, output);
                Array<byte> compressed = output->toByteArray();
                cd.setBinary(compressed);
                service->push(output);
                
            }
        }
};

int SerializationContextImpl::getVersion(){
    return version;
};

void SerializationContextImpl::compress(Array<byte>& binary, ContextAwareDataOutput* output) throw(std::ios_base::failure){
    output->reset();
    int size = binary.length();
    for(int i = 0; i < size; i++){
        output->writeByte(binary[i]);
    }
};//TODO zip in c++

void SerializationContextImpl::decompress(Array<byte>& binary, ContextAwareDataOutput* output) throw(std::ios_base::failure){
    output->reset();
    int size = binary.length();
    for(int i = 0; i < size; i++){
        output->writeByte(binary[i]);
    }
};//TODO unzip in c++