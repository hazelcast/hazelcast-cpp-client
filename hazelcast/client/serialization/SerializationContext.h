//
//  SerializationContext.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_SERIALIZATION_CONTEXT
#define HAZELCAST_SERIALIZATION_CONTEXT

#include <map>
#include <vector>
#include <memory>
#include "../../util/ConcurrentMap.h"


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

                ClassDefinition *lookup(int, int);

                bool isClassDefinitionExists(int, int, int);

                ClassDefinition *lookup(int, int, int);

                ClassDefinition *createClassDefinition(int, std::auto_ptr< std::vector<byte>>);

                void registerNestedDefinitions(ClassDefinition *cd);

                ClassDefinition *registerClassDefinition(ClassDefinition *cd);

                int getVersion();

            private:

                SerializationContext(const SerializationContext&);

                PortableContext& getPortableContext(int factoryId);

                void operator = (const SerializationContext&);

                int contextVersion;
                hazelcast::util::ConcurrentMap<int, PortableContext> portableContextMap;

            };

        }
    }
}
#endif /* HAZELCAST_SERIALIZATION_CONTEXT */
