//
// Created by sancar koyunlu on 5/2/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
// To change the template use AppCode | Preferences | File Templates.
//


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

                boost::shared_ptr<ClassDefinition>  ClassDefinitionContext::registerClassDefinition(boost::shared_ptr<ClassDefinition> cd) {
                    cd->setVersionIfNotSet(portableContext->getVersion());

                    long long versionedClassId = combineToLong(cd->getClassId(), cd->getVersion());
                    boost::shared_ptr<ClassDefinition> currentCD = versionedDefinitions.putIfAbsent(versionedClassId, cd);
                    if (currentCD == NULL) {
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

