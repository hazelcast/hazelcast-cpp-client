//
//  SerializationContextImpl.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef __Server__SerializationContextImpl__
#define __Server__SerializationContextImpl__

#include <iostream>
#include <memory>
#include <map>
#include <memory>
#include "Array.h"
#include "SerializationContext.h"
class ClassDefinitionImpl;
class PortableFactory;
class Portable;
class SerializationServiceImpl;
class ContextAwareDataOutput;
typedef unsigned char byte;

class SerializationContextImpl : public SerializationContext{
public:
    SerializationContextImpl(PortableFactory*,int,SerializationServiceImpl*);
    
    bool isClassDefinitionExists(int);
    ClassDefinitionImpl lookup(int);
    
    bool isClassDefinitionExists(int,int);
    ClassDefinitionImpl lookup(int,int);
    
    std::auto_ptr<Portable> createPortable(int classId);
    
    ClassDefinitionImpl createClassDefinition(Array<byte>&) throw(std::ios_base::failure);
    void registerNestedDefinitions(ClassDefinitionImpl& cd) throw(std::ios_base::failure);
    void registerClassDefinition(ClassDefinitionImpl& cd) throw(std::ios_base::failure);
    int getVersion();
    
private:
    
    void compress(Array<byte>&, ContextAwareDataOutput*) throw(std::ios_base::failure);//TODO zip in c++
    void decompress(Array<byte>&, ContextAwareDataOutput* ) throw(std::ios_base::failure);//TODO unzip in c++
        
    PortableFactory* portableFactory;//TODO think again
    SerializationServiceImpl* service;
    int version;
    std::map<long,ClassDefinitionImpl> versionedDefinitions;
    
};

#endif /* defined(__Server__SerializationContextImpl__) */
