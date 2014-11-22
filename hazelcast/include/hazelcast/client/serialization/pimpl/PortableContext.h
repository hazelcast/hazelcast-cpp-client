//
//  PortableContext.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_SERIALIZATION_CONTEXT
#define HAZELCAST_SERIALIZATION_CONTEXT

#include "hazelcast/client/serialization/Portable.h"
#include "hazelcast/util/SynchronizedMap.h"
#include "hazelcast/client/serialization/pimpl/SerializerHolder.h"
#include <map>
#include <vector>
#include <memory>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace serialization {

            class ClassDefinition;

            namespace pimpl {

                class Data;

                class ClassDefinitionContext;

                class HAZELCAST_API PortableContext {
                public:

                    PortableContext(int);

                    int getClassVersion(int factoryId, int classId);

                    void setClassVersion(int factoryId, int classId, int version);

                    bool isClassDefinitionExists(int, int, int);

                    boost::shared_ptr<ClassDefinition> lookup(int, int, int);

                    boost::shared_ptr<ClassDefinition> createClassDefinition(int, std::auto_ptr< std::vector<byte> >);

                    boost::shared_ptr<ClassDefinition> registerClassDefinition(boost::shared_ptr<ClassDefinition>);

                    boost::shared_ptr<ClassDefinition> lookupOrRegisterClassDefinition(const Portable& portable);

                    int getVersion();

                    SerializerHolder &getSerializerHolder();

                private:

                    PortableContext(const PortableContext &);

                    ClassDefinitionContext &getClassDefinitionContext(int factoryId);

                    void operator = (const PortableContext &);

                    int contextVersion;
                    util::SynchronizedMap<int, ClassDefinitionContext> classDefContextMap;
                    SerializerHolder serializerHolder;

                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_SERIALIZATION_CONTEXT */

