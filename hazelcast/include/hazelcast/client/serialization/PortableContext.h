//
// Created by sancar koyunlu on 5/2/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
// To change the template use AppCode | Preferences | File Templates.
//



#ifndef HAZELCAST_PORTABLE_CONTEXT
#define HAZELCAST_PORTABLE_CONTEXT

#include "ConcurrentSmartMap.h"
#include <vector>
#include <map>


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

                util::AtomicPointer<ClassDefinition> lookup(int, int);

                util::AtomicPointer<ClassDefinition> createClassDefinition(std::auto_ptr< std::vector<byte> >);

                util::AtomicPointer<ClassDefinition> registerClassDefinition(util::AtomicPointer<ClassDefinition>);

            private:
                void compress(std::vector<byte>&);

                long combineToLong(int x, int y) const;  //TODO move to util

                int extractInt(long value, bool lowerBits) const;  //TODO move to util

                std::vector<byte> decompress(std::vector<byte> const &) const;

                hazelcast::util::ConcurrentSmartMap<long, ClassDefinition> versionedDefinitions;

                SerializationContext *serializationContext;
            };
        }
    }
}

#endif //HAZELCAST_PORTABLE_CONTEXT
