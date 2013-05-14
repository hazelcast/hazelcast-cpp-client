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

            class PortableFactory;

            class SerializationService;

            class BufferedDataOutput;

            class PortableContext;

            typedef unsigned char byte;

            class SerializationContext {
            public:

                SerializationContext(int, SerializationService *);

                ~SerializationContext();

                bool isClassDefinitionExists(int, int) const;

                boost::shared_ptr<ClassDefinition> lookup(int, int);

                bool isClassDefinitionExists(int, int, int) const;

                boost::shared_ptr<ClassDefinition> lookup(int, int, int) const;

                boost::shared_ptr<ClassDefinition> createClassDefinition(int, std::vector<byte>&);

                void registerNestedDefinitions(boost::shared_ptr<ClassDefinition> cd);

                void registerClassDefinition(boost::shared_ptr<ClassDefinition> cd);

                int getVersion();

            private:

                SerializationContext(const SerializationContext&);

                PortableContext *getPortableContext(int factoryId) const;

                void operator = (const SerializationContext&);

                int contextVersion;
                std::map<int, PortableContext *> portableContextMap;
                SerializationService *service;

            };

        }
    }
}
#endif /* HAZELCAST_SERIALIZATION_CONTEXT */
