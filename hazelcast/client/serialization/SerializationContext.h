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
#include <boost/shared_array.hpp>

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

                ClassDefinition* lookup(int, int);

                bool isClassDefinitionExists(int, int, int);

                ClassDefinition* lookup(int, int, int);

                ClassDefinition* createClassDefinition(int, std::vector<byte>&);

                void registerNestedDefinitions(ClassDefinition* cd);

                void registerClassDefinition(ClassDefinition* cd);

                int getVersion();

            private:

                SerializationContext(const SerializationContext&);

                PortableContext& getPortableContext(int factoryId);

                void operator = (const SerializationContext&);

                int contextVersion;
                std::map<int, PortableContext> portableContextMap;

            };

        }
    }
}
#endif /* HAZELCAST_SERIALIZATION_CONTEXT */
