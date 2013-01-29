//
//  PortableSerializer.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_PORTABLE_SERIALIZER
#define HAZELCAST_PORTABLE_SERIALIZER

#include "Portable.h"
#include "SerializationContext.h"
#include "ClassDefinition.h"
#include "ClassDefinitionWriter.h"
#include <memory>

namespace hazelcast{ 
namespace client{
namespace serialization{
class DataInput;
class DataOutput;

class PortableSerializer{
public:
    PortableSerializer(SerializationContext*);
    ~PortableSerializer();
    
    ClassDefinition* getClassDefinition(Portable& p) throw(std::ios_base::failure);
    
    int getTypeId();
    int getVersion();
    
    void write(DataOutput* output, Portable& p) throw(std::ios_base::failure);
    
    auto_ptr<Portable> read(DataInput& dataInput) throw(std::ios_base::failure);
    
private:
    SerializationContext* context;
};
 
}}}
#endif /* HAZELCAST_PORTABLE_SERIALIZER */
