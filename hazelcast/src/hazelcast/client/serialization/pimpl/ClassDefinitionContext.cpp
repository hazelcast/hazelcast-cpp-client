/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {

                ClassDefinitionContext::ClassDefinitionContext(int factoryId, PortableContext *portableContext)
                : factoryId(factoryId), portableContext(portableContext) {
                }

                int ClassDefinitionContext::getClassVersion(int classId) {
                    boost::shared_ptr<int> version = currentClassVersions.get(classId);
                    return version != NULL ? *version : -1;
                }

                void ClassDefinitionContext::setClassVersion(int classId, int version) {
                    boost::shared_ptr<int> current = currentClassVersions.putIfAbsent(classId, boost::shared_ptr<int>(new int(version)));
                    if (current != NULL && *current != version) {
                        std::stringstream error;
                        error << "Class-id: " << classId << " is already registered!";
                        throw exception::IllegalArgumentException("ClassDefinitionContext::setClassVersion", error.str());
                    }
                }

                boost::shared_ptr<ClassDefinition> ClassDefinitionContext::lookup(int classId, int version) {
                    long long key = combineToLong(classId, version);
                    return versionedDefinitions.get(key);

                }

                boost::shared_ptr<ClassDefinition> ClassDefinitionContext::registerClassDefinition(boost::shared_ptr<ClassDefinition> cd) {
                    if (cd.get() == NULL) {
                        return boost::shared_ptr<ClassDefinition>();
                    }
                    if (cd->getFactoryId() != factoryId) {
                        throw (exception::ExceptionBuilder<exception::HazelcastSerializationException>(
                                "ClassDefinitionContext::registerClassDefinition") << "Invalid factory-id! "
                                                                                   << factoryId << " -> "
                                                                                   << cd).build();
                    }

                    cd->setVersionIfNotSet(portableContext->getVersion());

                    long long versionedClassId = combineToLong(cd->getClassId(), cd->getVersion());
                    boost::shared_ptr<ClassDefinition> currentCd = versionedDefinitions.putIfAbsent(versionedClassId, cd);
                    if (currentCd.get() == NULL) {
                        return cd;
                    }

                    if (currentCd.get() != cd.get() && *currentCd != *cd) {
                        throw (exception::ExceptionBuilder<exception::HazelcastSerializationException>(
                                "ClassDefinitionContext::registerClassDefinition")
                                << "Incompatible class-definitions with same class-id: " << *cd << " VS "
                                << *currentCd).build();
                    }

                    return currentCd;
                }

                int64_t ClassDefinitionContext::combineToLong(int x, int y) const {
                    return ((int64_t)x) << 32 | (((int64_t)y) & 0xFFFFFFFL);
                }

            }
        }
    }
}

