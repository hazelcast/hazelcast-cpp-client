//
//  SerializationContext.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_SERIALIZATION_CONTEXT
#define HAZELCAST_SERIALIZATION_CONTEXT

#include "SynchronizedMap.h"
#include "hazelcast/client/serialization/SerializerHolder.h"
#include <map>
#include <vector>
#include <memory>


namespace hazelcast {
    namespace client {
        namespace serialization {

            class ClassDefinition;

            class PortableContext;



            class HAZELCAST_API SerializationContext {
            public:

                SerializationContext(int);

                ~SerializationContext();

                bool isClassDefinitionExists(int, int);

                ClassDefinition *lookup(int, int);

                bool isClassDefinitionExists(int, int, int);

                ClassDefinition *lookup(int, int, int);

                ClassDefinition *createClassDefinition(int, std::auto_ptr< std::vector<byte> >);

                void registerNestedDefinitions(ClassDefinition *);

                ClassDefinition *registerClassDefinition(ClassDefinition *);

                int getVersion();

                SerializerHolder& getSerializerHolder();

            private:

                SerializationContext(const SerializationContext&);

                PortableContext& getPortableContext(int factoryId);

                void operator = (const SerializationContext&);

                int contextVersion;
                util::SynchronizedMap<int, PortableContext> portableContextMap;
                SerializerHolder serializerHolder;

            };

        }
    }
}
#endif /* HAZELCAST_SERIALIZATION_CONTEXT */
