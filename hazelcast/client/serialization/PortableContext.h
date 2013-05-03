//
// Created by sancar koyunlu on 5/2/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
// To change the template use AppCode | Preferences | File Templates.
//



#ifndef HAZELCAST_PORTABLE_CONTEXT
#define HAZELCAST_PORTABLE_CONTEXT

#include <iostream>
#include <map>
#include <vector>
#include <boost/shared_array.hpp>

namespace hazelcast {
    namespace client {
        namespace serialization {

            class ClassDefinition;

            class SerializationContext;

            class SerializationService;

            class Portable;

            typedef unsigned char byte;

            class PortableContext {
            public:

                PortableContext(SerializationService *service, SerializationContext *context)
                :service(service)
                , context(context) {

                }

                bool isClassDefinitionExists(int, int) const;

                boost::shared_ptr<ClassDefinition> lookup(int, int);

                boost::shared_ptr<ClassDefinition> createClassDefinition(std::vector<byte>&);

                void registerClassDefinition(boost::shared_ptr<ClassDefinition> cd);

            private:
                void compress(std::vector<byte>&);

                void decompress(std::vector<byte>&) const;

                std::map<long, boost::shared_ptr<ClassDefinition> > versionedDefinitions;
                SerializationService *service;
                SerializationContext *context;
            };
        }
    }
}

#endif //HAZELCAST_PORTABLE_CONTEXT
