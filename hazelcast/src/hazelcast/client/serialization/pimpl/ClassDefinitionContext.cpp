/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
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

#include "hazelcast/client/serialization/pimpl/ClassDefinitionContext.h"
#include "hazelcast/client/serialization/pimpl/SerializationService.h"
#include "hazelcast/client/exception/IllegalArgumentException.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {

                ClassDefinitionContext::ClassDefinitionContext(PortableContext *portableContext)
                : portableContext(portableContext) {

                }

                int ClassDefinitionContext::getClassVersion(int classId) {
                    hazelcast::util::SharedPtr<int> version = currentClassVersions.get(classId);
                    return version.get() != NULL ? *version : -1;
                }

                void ClassDefinitionContext::setClassVersion(int classId, int version) {
                    hazelcast::util::SharedPtr<int> current = currentClassVersions.putIfAbsent(classId, hazelcast::util::SharedPtr<int>(new int(version)));
                    if (current.get() != NULL && *current != version) {
                        std::stringstream error;
                        error << "Class-id: " << classId << " is already registered!";
                        throw exception::IllegalArgumentException("ClassDefinitionContext::setClassVersion", error.str());
                    }
                }

                hazelcast::util::SharedPtr<ClassDefinition> ClassDefinitionContext::lookup(int classId, int version) {
                    long long key = combineToLong(classId, version);
                    return versionedDefinitions.get(key);

                }

                hazelcast::util::SharedPtr<ClassDefinition>  ClassDefinitionContext::registerClassDefinition(hazelcast::util::SharedPtr<ClassDefinition> cd) {
                    cd->setVersionIfNotSet(portableContext->getVersion());

                    long long versionedClassId = combineToLong(cd->getClassId(), cd->getVersion());
                    hazelcast::util::SharedPtr<ClassDefinition> currentCD = versionedDefinitions.putIfAbsent(versionedClassId, cd);
                    if (currentCD.get() == NULL) {
                        return cd;
                    }

                    versionedDefinitions.put(versionedClassId, cd);
                    return cd;
                }

                long long ClassDefinitionContext::combineToLong(int x, int y) const {
                    return ((long long)x) << 32 | (((long long)y) & 0xFFFFFFFL);
                }

            }
        }
    }
}

