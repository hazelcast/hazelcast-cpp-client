//
//  SerializationContext.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_SERIALIZATION_CONTEXT
#define HAZELCAST_SERIALIZATION_CONTEXT

#include "ConcurrentSmartMap.h"
#include "SerializerHolder.h"
#include <map>
#include <vector>
#include <memory>


namespace hazelcast {
    namespace client {
        namespace serialization {

            class ClassDefinition;

            class PortableContext;

            typedef unsigned char byte;

            class SerializationContext {
            public:

                SerializationContext(int);

                ~SerializationContext();

                bool isClassDefinitionExists(int, int);

                util::AtomicPointer<ClassDefinition> lookup(int, int);

                bool isClassDefinitionExists(int, int, int);

                util::AtomicPointer<ClassDefinition> lookup(int, int, int);

                util::AtomicPointer<ClassDefinition> createClassDefinition(int, std::auto_ptr< std::vector<byte> >);

                void registerNestedDefinitions(util::AtomicPointer<ClassDefinition>);

                util::AtomicPointer<ClassDefinition> registerClassDefinition(util::AtomicPointer<ClassDefinition>);

                util::AtomicPointer<ClassDefinition> registerClassDefinition(ClassDefinition* cd);

                int getVersion();

                SerializerHolder& getSerializerHolder();

            private:

                SerializationContext(const SerializationContext&);

                PortableContext& getPortableContext(int factoryId);

                void operator = (const SerializationContext&);

                int contextVersion;
                util::ConcurrentSmartMap<int, PortableContext> portableContextMap;
                SerializerHolder serializerHolder;

            };

        }
    }
}
#endif /* HAZELCAST_SERIALIZATION_CONTEXT */
