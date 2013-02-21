//
//  SerializationContext.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_SERIALIZATION_CONTEXT
#define HAZELCAST_SERIALIZATION_CONTEXT


#include <vector>
#include <iostream>
#include <memory>
#include <map>
#include <memory>
#include <boost/shared_array.hpp>

namespace hazelcast {
    namespace client {
        namespace serialization {

            class ClassDefinition;
            class PortableFactory;
            class Portable;
            class SerializationService;
            class DataOutput;

            typedef unsigned char byte;

            class SerializationContext {
            public:

                SerializationContext(PortableFactory const *, int, SerializationService *);

                ~SerializationContext();

                bool isClassDefinitionExists(int);

                boost::shared_ptr<ClassDefinition> lookup(int);

                bool isClassDefinitionExists(int, int);

                boost::shared_ptr<ClassDefinition> lookup(int, int);

                std::auto_ptr<Portable> createPortable(int classId);

                boost::shared_ptr<ClassDefinition> createClassDefinition(std::vector<byte>&);

                void registerNestedDefinitions(boost::shared_ptr<ClassDefinition> cd);

                void registerClassDefinition(boost::shared_ptr<ClassDefinition> cd);

                int getVersion();

            private:

                SerializationContext(const SerializationContext&);

                void operator = (const SerializationContext&);

                void compress(std::vector<byte>&);

                void decompress(std::vector<byte>&);

                PortableFactory const *portableFactory;
                SerializationService *service;
                int version;
                std::map<long, boost::shared_ptr<ClassDefinition> > versionedDefinitions;

            };

        }
    }
}
#endif /* HAZELCAST_SERIALIZATION_CONTEXT */
