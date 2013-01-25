//
//  SerializationContextImpl.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
#include <cassert>
#include "SerializationContextImpl.h"
#include "SerializationServiceImpl.h"
#include "ContextAwareDataOutput.h"
#include "zlib.h"
SerializationContextImpl::SerializationContextImpl(PortableFactory* portableFactory, int version, SerializationServiceImpl* service){
    this->portableFactory = portableFactory;
    this->version = version;
    this->service = service;
};
SerializationContextImpl::~SerializationContextImpl(){
    for(map<long,ClassDefinitionImpl*>::const_iterator it = versionedDefinitions.begin() ; it != versionedDefinitions.end() ; it++){
        delete (*it).second;
    }
};
SerializationContextImpl::SerializationContextImpl(const SerializationContextImpl&  rhs){
    assert(0);
};
void SerializationContextImpl::operator=(const SerializationContextImpl& rhs) {
    assert(0);
};

bool SerializationContextImpl::isClassDefinitionExists(int classId){
    return isClassDefinitionExists(classId,version);
};

ClassDefinitionImpl* SerializationContextImpl::lookup(int classId){
     long key = SerializationServiceImpl::combineToLong(classId, version);
     return versionedDefinitions[key];
};

bool SerializationContextImpl::isClassDefinitionExists(int classId, int version){
    long key = SerializationServiceImpl::combineToLong(classId, version);
    return (versionedDefinitions.count(key) > 0);
};

ClassDefinitionImpl* SerializationContextImpl::lookup(int classId, int version){
    long key = SerializationServiceImpl::combineToLong(classId, version);
     return versionedDefinitions[key];

};

auto_ptr<Portable> SerializationContextImpl::createPortable(int classId){
    return auto_ptr<Portable>(portableFactory->create(classId));
};

ClassDefinitionImpl* SerializationContextImpl::createClassDefinition(Array<byte>& binary) throw(std::ios_base::failure){
    
    decompress(binary);
    
    ContextAwareDataInput dataInput = ContextAwareDataInput(binary, service);
    ClassDefinitionImpl* cd = new ClassDefinitionImpl;
    cd->readData(dataInput);
    cd->setBinary(binary);
            
    long key = service->combineToLong(cd->classId, version);
//    bool exists = false;
//    ClassDefinitionImpl currentCD;
//    if(versionedDefinitions.count(key) > 0){
//        exists = true;
//        currentCD = versionedDefinitions[key];
//    }
    versionedDefinitions[key] = cd;
    
//    if (!exists) {
        registerNestedDefinitions(cd);
        return cd;
//    } else {
//        return currentCD;
//    }
};

void SerializationContextImpl::registerNestedDefinitions(ClassDefinitionImpl* cd) throw(std::ios_base::failure){
    vector<ClassDefinitionImpl*> nestedDefinitions = cd->getNestedClassDefinitions();
    for(vector<ClassDefinitionImpl*>::iterator it = nestedDefinitions.begin() ; it < nestedDefinitions.end() ; it++){
        registerClassDefinition(*it);
        registerNestedDefinitions(*it);
    }
};

void SerializationContextImpl::registerClassDefinition(ClassDefinitionImpl* cd) throw(std::ios_base::failure){
     
        if(!isClassDefinitionExists(cd->getClassId() , cd->getVersion())){
            if (cd->getBinary().length() == 0) {
                
                ContextAwareDataOutput* output = service->pop();
                assert(output != NULL);
                cd->writeData(*output);
                Array<byte> binary = output->toByteArray();
                compress(binary);
                cd->setBinary(binary);
                service->push(output);
            }
            long versionedClassId = service->combineToLong(cd->getClassId(), cd->getVersion());
            versionedDefinitions[versionedClassId] = cd;
        }
};

int SerializationContextImpl::getVersion(){
    return version;
};

void SerializationContextImpl::compress(Array<byte>& binary) throw(std::ios_base::failure){
    uLong ucompSize = binary.length(); 
    uLong compSize = compressBound(ucompSize);
    byte temp[compSize];
    int err = compress2((Bytef *)temp, &compSize, (Bytef *)binary.buffer, ucompSize,Z_BEST_COMPRESSION);
    switch (err) {
        case Z_BUF_ERROR:
            throw "not enough room in the output buffer";
        case Z_DATA_ERROR:
            throw "data is corrupted";
        case Z_MEM_ERROR:
            throw "if there was not  enough memory";
    }
    delete [] binary.buffer;
    binary.init(compSize,temp);
};

void SerializationContextImpl::decompress(Array<byte>& binary) throw(std::ios_base::failure){
    uLong compSize = binary.length();
    
    uLong ucompSize = 512;
    byte* temp = NULL;
    int err = Z_OK;
    do{
        ucompSize *= 2;
        delete [] temp;
        temp = new byte[ucompSize];
        err = uncompress((Bytef *)temp, &ucompSize, (Bytef *)binary.buffer, compSize);
        switch (err) {
            case Z_DATA_ERROR:
                throw "data is corrupted";
            case Z_MEM_ERROR:
                throw "if there was not  enough memory";
        }
    }while(err == Z_BUF_ERROR);
    delete [] binary.buffer;
    binary.init(ucompSize,temp);
    delete [] temp;
};