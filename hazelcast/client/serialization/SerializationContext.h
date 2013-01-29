//
//  SerializationContext.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_SERIALIZATION_CONTEXT
#define HAZELCAST_SERIALIZATION_CONTEXT

#include "../Array.h"
#include <iostream>
#include <memory>
#include <map>
#include <memory>

namespace hazelcast{ 
namespace client{
namespace serialization{
    
class ClassDefinition;
class PortableFactory;
class Portable;
class SerializationService;
class DataOutput;

typedef unsigned char byte;

class SerializationContext{
public:
    SerializationContext(PortableFactory*,int,SerializationService*);
    ~SerializationContext();
    SerializationContext(const SerializationContext&  );
    void operator=(const SerializationContext&);
    
    bool isClassDefinitionExists(int);
    ClassDefinition* lookup(int);
    
    bool isClassDefinitionExists(int,int);
    ClassDefinition* lookup(int,int);
    
    std::auto_ptr<Portable> createPortable(int classId);
    
    ClassDefinition* createClassDefinition( Array<byte>&) throw(std::ios_base::failure);
    void registerNestedDefinitions(ClassDefinition* cd) throw(std::ios_base::failure);
    void registerClassDefinition(ClassDefinition* cd) throw(std::ios_base::failure);
    int getVersion();
    
private:
    
    void compress(Array<byte>&) throw(std::ios_base::failure);
    void decompress(Array<byte>&) throw(std::ios_base::failure);
        
    PortableFactory* portableFactory;
    SerializationService* service;
    int version;
    std::map<long,ClassDefinition*> versionedDefinitions;
    
};

}}}
#endif /* HAZELCAST_SERIALIZATION_CONTEXT */
