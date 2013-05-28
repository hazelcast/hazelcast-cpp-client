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
#include "../../util/ConcurrentMap.h"


namespace hazelcast {
    namespace client {
        namespace serialization {

            class ClassDefinition;

            class SerializationContext;

            typedef unsigned char byte;

            class PortableContext {
            public:

                PortableContext();

                ~PortableContext();

                PortableContext(SerializationContext *serializationContext);

                bool isClassDefinitionExists(int, int) const;

                ClassDefinition *lookup(int, int);

                ClassDefinition *createClassDefinition(std::auto_ptr< std::vector<byte>>);

                ClassDefinition *registerClassDefinition(ClassDefinition *cd);

                void setSerializationContext(SerializationContext *);

            private:
                void compress(std::vector<byte>&);

                long combineToLong(int x, int y) const;  //TODO move to util

                int extractInt(long value, bool lowerBits) const;  //TODO move to util

                std::vector<byte> decompress(std::vector<byte> const &) const;

                hazelcast::util::ConcurrentMap<long, ClassDefinition> versionedDefinitions;
//                std::map<long, ClassDefinition * > versionedDefinitions;
                SerializationContext *serializationContext;
            };
        }
    }
}

#endif //HAZELCAST_PORTABLE_CONTEXT
