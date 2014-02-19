//
// Created by sancar koyunlu on 5/2/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
// To change the template use AppCode | Preferences | File Templates.
//



#ifndef HAZELCAST_PORTABLE_CONTEXT
#define HAZELCAST_PORTABLE_CONTEXT

#include "hazelcast/util/SynchronizedMap.h"
#include <vector>
#include <map>


namespace hazelcast {
    namespace client {
        namespace serialization {

            class ClassDefinition;

            class SerializationContext;

            class HAZELCAST_API PortableContext {
            public:

                PortableContext(SerializationContext *serializationContext);

                bool isClassDefinitionExists(int, int) const;

                boost::shared_ptr<ClassDefinition> lookup(int, int);

                boost::shared_ptr<ClassDefinition> createClassDefinition(std::auto_ptr< std::vector<byte> >);

                boost::shared_ptr<ClassDefinition> registerClassDefinition(boost::shared_ptr<ClassDefinition> );

            private:
                void compress(std::vector<byte>&);

                long long combineToLong(int x, int y) const;

                std::vector<byte> decompress(std::vector<byte> const &) const;

                util::SynchronizedMap<long long, ClassDefinition> versionedDefinitions;

                SerializationContext *serializationContext;
            };
        }
    }
}

#endif //HAZELCAST_PORTABLE_CONTEXT
