//
// Created by sancar koyunlu on 5/2/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
// To change the template use AppCode | Preferences | File Templates.
//



#ifndef HAZELCAST_PORTABLE_CONTEXT
#define HAZELCAST_PORTABLE_CONTEXT

#include "hazelcast/util/SynchronizedMap.h"


namespace hazelcast {
    namespace client {
        namespace serialization {

            class ClassDefinition;

            namespace pimpl {

                class PortableContext;

                class ClassDefinitionContext {
                public:

                    ClassDefinitionContext(PortableContext *portableContext);

                    int getClassVersion(int classId);

                    void setClassVersion(int classId, int version);

                    boost::shared_ptr<ClassDefinition> lookup(int, int);

                    boost::shared_ptr<ClassDefinition> registerClassDefinition(boost::shared_ptr<ClassDefinition>);

                private:
                    long long combineToLong(int x, int y) const;

                    util::SynchronizedMap<long long, ClassDefinition> versionedDefinitions;
                    util::SynchronizedMap<int, int> currentClassVersions;
                    PortableContext *portableContext;
                };
            }
        }
    }
}

#endif //HAZELCAST_PORTABLE_CONTEXT

