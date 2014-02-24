//
//  SerializationContext.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
#include "hazelcast/client/serialization/pimpl/SerializationContext.h"
#include "hazelcast/client/serialization/pimpl/PortableContext.h"
#include "hazelcast/client/serialization/pimpl/SerializationService.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                SerializationContext::SerializationContext(int version)
                : contextVersion(version)
                , serializerHolder(*this) {
//                portableContextMap[-1] = new PortableContext(this); //MTODO right now hardcoded : will changed as configurable (may be)
//                portableContextMap[1] = new PortableContext(this);
//                portableContextMap[-3] = new PortableContext(this);
//                portableContextMap[0] = new PortableContext(this);
                };

                bool SerializationContext::isClassDefinitionExists(int factoryId, int classId) {
                    return isClassDefinitionExists(factoryId, classId, contextVersion);
                };

                boost::shared_ptr<ClassDefinition> SerializationContext::lookup(int factoryId, int classId) {
                    return getPortableContext(factoryId).lookup(classId, contextVersion);
                };

                bool SerializationContext::isClassDefinitionExists(int factoryId, int classId, int version) {
                    return getPortableContext(factoryId).isClassDefinitionExists(classId, version);
                };

                boost::shared_ptr<ClassDefinition> SerializationContext::lookup(int factoryId, int classId, int version) {
                    return getPortableContext(factoryId).lookup(classId, version);
                };

                boost::shared_ptr<ClassDefinition> SerializationContext::createClassDefinition(int factoryId, std::auto_ptr< std::vector<byte> > binary) {
                    return getPortableContext(factoryId).createClassDefinition(binary);
                };

                void SerializationContext::registerNestedDefinitions(boost::shared_ptr<ClassDefinition> cd) {
                    std::vector<boost::shared_ptr<ClassDefinition>  > nestedDefinitions = cd->getNestedClassDefinitions();
                    for (std::vector<boost::shared_ptr<ClassDefinition>  >::iterator it = nestedDefinitions.begin(); it < nestedDefinitions.end(); it++) {
                        registerClassDefinition(*it);
                        registerNestedDefinitions(*it);
                    }
                };

                boost::shared_ptr<ClassDefinition> SerializationContext::registerClassDefinition(boost::shared_ptr<ClassDefinition> cd) {
                    return getPortableContext(cd->getFactoryId()).registerClassDefinition(cd);
                };

                int SerializationContext::getVersion() {
                    return contextVersion;
                };

                SerializerHolder &SerializationContext::getSerializerHolder() {
                    return serializerHolder;
                };

                PortableContext &SerializationContext::getPortableContext(int factoryId) {
//                int count = portableContextMap.count(factoryId);//MTODO look at the constructor
//                if (count == 0) {
//                    char message[70];
//                    sprintf(message, "Could not find Portable factory for factoryId: %d", factoryId);
//                    throw IException(message);
//                }
//                return portableContextMap.at(factoryId);
                    boost::shared_ptr<PortableContext> value = portableContextMap.get(factoryId);
                    if (value == NULL) {
                        value = boost::shared_ptr<PortableContext>(new PortableContext(this));
                        boost::shared_ptr<PortableContext> current = portableContextMap.putIfAbsent(factoryId, value);
                        if (current != NULL) {
                            value = current;
                        }
                    }
//                PortableContext* temp = portableContextMap.get(factoryId);
//                temp->setSerializationContext(this);
                    return *value;
                };
            }
        }
    }
}