//
//  SerializationContext.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
#include "ClassDefinition.h"
#include "SerializationContext.h"
#include "SerializationService.h"
#include "DataOutput.h"
#include "zlib.h"
#include <cassert>

namespace hazelcast{ 
namespace client{
namespace serialization{

SerializationContext::SerializationContext(PortableFactory const * portableFactory, int version, SerializationService* service){
    this->portableFactory = portableFactory;
    this->version = version;
    this->service = service;
};
SerializationContext::~SerializationContext(){
    for(map<long,ClassDefinition*>::iterator it = versionedDefinitions.begin() ; it != versionedDefinitions.end() ; it++){
//        delete (*it).second; 
    }
};
SerializationContext::SerializationContext(const SerializationContext&  rhs){
    
};
void SerializationContext::operator=(const SerializationContext& rhs) {
    
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

std::auto_ptr<Portable> SerializationContext::createPortable(int classId){
    return std::auto_ptr<Portable>(portableFactory->create(classId));
};

ClassDefinition* SerializationContext::createClassDefinition(std::vector<byte>& binary) {
    
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

void SerializationContext::registerNestedDefinitions(ClassDefinition* cd) {
    vector<ClassDefinition*> nestedDefinitions = cd->getNestedClassDefinitions();
    for(vector<ClassDefinition*>::iterator it = nestedDefinitions.begin() ; it < nestedDefinitions.end() ; it++){
        registerClassDefinition(*it);
        registerNestedDefinitions(*it);
    }
};

void SerializationContext::registerClassDefinition(ClassDefinition* cd) {
     
        if(!isClassDefinitionExists(cd->getClassId() , cd->getVersion())){
            if (cd->getBinary().size() == 0) {
                
                DataOutput* output = service->pop();
                assert(output != NULL);
                cd->writeData(*output);
                std::vector<byte> binary = output->toByteArray();
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

void SerializationContext::compress(std::vector<byte>& binary) {
    uLong ucompSize = binary.size(); 
    uLong compSize = compressBound(ucompSize);
    byte uncompressedTemp[binary.size()];
    for(int i = 0 ; i < binary.size(); i++)
        uncompressedTemp[i] = binary[i];
    
    byte compressedTemp[compSize];
    int err = compress2((Bytef *)compressedTemp, &compSize, (Bytef *)uncompressedTemp, ucompSize,Z_BEST_COMPRESSION);
    switch (err) {
        case Z_BUF_ERROR:
            throw "not enough room in the output buffer";
        case Z_DATA_ERROR:
            throw "data is corrupted";
        case Z_MEM_ERROR:
            throw "if there was not  enough memory";
    }
    std::vector<byte> compressed(compressedTemp , compressedTemp + compSize);
    binary = compressed;
};

void SerializationContext::decompress(std::vector<byte>& binary) {
    uLong compSize = binary.size();
    
    uLong ucompSize = 512;
    byte* temp = NULL;
    byte compressedTemp[binary.size()];
    for(int i = 0 ; i < binary.size(); i++)
        compressedTemp[i] = binary[i];
    int err = Z_OK;
    do{
        ucompSize *= 2;
        delete [] temp;
        temp = new byte[ucompSize];
        err = uncompress((Bytef *)temp, &ucompSize, (Bytef *)compressedTemp, compSize);
        switch (err) {
            case Z_DATA_ERROR:
                throw "data is corrupted";
            case Z_MEM_ERROR:
                throw "if there was not  enough memory";
        }
    }while(err == Z_BUF_ERROR);
    std::vector<byte> decompressed(temp,temp + ucompSize);
    binary = decompressed;
    delete [] temp;
};

}}}