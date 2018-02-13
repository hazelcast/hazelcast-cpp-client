/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
//
// Created by sancar koyunlu on 5/2/13.

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

