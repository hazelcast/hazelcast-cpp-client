//
//  SerializationContext.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
#include <cassert>
#include "SerializationContext.h"
#include "SerializationService.h"
#include "DataOutput.h"
#include "zlib.h"
SerializationContext::SerializationContext(PortableFactory* portableFactory, int version, SerializationService* service){
    this->portableFactory = portableFactory;
    this->version = version;
    this->service = service;
};
SerializationContext::~SerializationContext(){
    for(map<long,ClassDefinition*>::iterator it = versionedDefinitions.begin() ; it != versionedDefinitions.end() ; it++){
//        try{
//                delete (*it).second; 
//        }catch(exception e){
//            std::cout << ":( "<< e.what() << std::endl;
//        }
    }
};
SerializationContext::SerializationContext(const SerializationContext&  rhs){
    assert(0);
};
void SerializationContext::operator=(const SerializationContext& rhs) {
    assert(0);
};

bool SerializationContext::isClassDefinitionExists(int classId){
    return isClassDefinitionExists(classId,version);
};

ClassDefinition* SerializationContext::lookup(int classId){
     long key = SerializationService::combineToLong(classId, version);
     return versionedDefinitions[key];
};

bool SerializationContext::isClassDefinitionExists(int classId, int version){
    long key = SerializationService::combineToLong(classId, version);
    return (versionedDefinitions.count(key) > 0);
};

ClassDefinition* SerializationContext::lookup(int classId, int version){
    long key = SerializationService::combineToLong(classId, version);
     return versionedDefinitions[key];

};

auto_ptr<Portable> SerializationContext::createPortable(int classId){
    return auto_ptr<Portable>(portableFactory->create(classId));
};

ClassDefinition* SerializationContext::createClassDefinition(Array<byte>& binary) throw(std::ios_base::failure){
    
    decompress(binary);
    
    DataInput dataInput = DataInput(binary, service);
    ClassDefinition* cd = new ClassDefinition;
    cd->readData(dataInput);
    cd->setBinary(binary);
            
    long key = service->combineToLong(cd->getClassId(), version);

    versionedDefinitions[key] = cd;
    
    registerNestedDefinitions(cd);
    return cd;
};

void SerializationContext::registerNestedDefinitions(ClassDefinition* cd) throw(std::ios_base::failure){
    vector<ClassDefinition*> nestedDefinitions = cd->getNestedClassDefinitions();
    for(vector<ClassDefinition*>::iterator it = nestedDefinitions.begin() ; it < nestedDefinitions.end() ; it++){
        registerClassDefinition(*it);
        registerNestedDefinitions(*it);
    }
};

void SerializationContext::registerClassDefinition(ClassDefinition* cd) throw(std::ios_base::failure){
     
        if(!isClassDefinitionExists(cd->getClassId() , cd->getVersion())){
            if (cd->getBinary().length() == 0) {
                
                DataOutput* output = service->pop();
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

int SerializationContext::getVersion(){
    return version;
};

void SerializationContext::compress(Array<byte>& binary) throw(std::ios_base::failure){
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

void SerializationContext::decompress(Array<byte>& binary) throw(std::ios_base::failure){
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