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

                boost::shared_ptr<ClassDefinition> lookup(int, int);

                bool isClassDefinitionExists(int, int, int);

                boost::shared_ptr<ClassDefinition> lookup(int, int, int);

                boost::shared_ptr<ClassDefinition> createClassDefinition(int, std::auto_ptr< std::vector<byte> >);

                void registerNestedDefinitions(boost::shared_ptr<ClassDefinition>);

                boost::shared_ptr<ClassDefinition> registerClassDefinition(boost::shared_ptr<ClassDefinition>);

                boost::shared_ptr<ClassDefinition> registerClassDefinition(ClassDefinition* cd);

                int getVersion();

            private:

                SerializationContext(const SerializationContext&);

                PortableContext& getPortableContext(int factoryId);

                void operator = (const SerializationContext&);

                int contextVersion;
                util::ConcurrentSmartMap<int, PortableContext> portableContextMap;

            };

        }
    }
}
#endif /* HAZELCAST_SERIALIZATION_CONTEXT */
