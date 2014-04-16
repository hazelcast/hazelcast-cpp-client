//
//  SerializationContext.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_SERIALIZATION_CONTEXT
#define HAZELCAST_SERIALIZATION_CONTEXT

#include "hazelcast/util/SynchronizedMap.h"
#include "hazelcast/client/serialization/pimpl/SerializerHolder.h"
#include <map>
#include <vector>
#include <memory>


namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class ClassDefinition;

                class PortableContext;

                class HAZELCAST_API SerializationContext {
                public:

                    SerializationContext(int);

                    bool isClassDefinitionExists(int, int);

                    boost::shared_ptr<ClassDefinition> lookup(int, int);

                    bool isClassDefinitionExists(int, int, int);

                    boost::shared_ptr<ClassDefinition> lookup(int, int, int);

                    boost::shared_ptr<ClassDefinition> createClassDefinition(int, std::auto_ptr< std::vector<byte> >);

                    void registerNestedDefinitions(boost::shared_ptr<ClassDefinition>);

                    boost::shared_ptr<ClassDefinition> registerClassDefinition(boost::shared_ptr<ClassDefinition>);

                    int getVersion();

                    SerializerHolder &getSerializerHolder();

                private:

                    SerializationContext(const SerializationContext &);

                    PortableContext &getPortableContext(int factoryId);

                    void operator = (const SerializationContext &);

                    int contextVersion;
                    util::SynchronizedMap<int, PortableContext> portableContextMap;
                    SerializerHolder serializerHolder;

                };
            }
        }
    }
}
#endif /* HAZELCAST_SERIALIZATION_CONTEXT */

