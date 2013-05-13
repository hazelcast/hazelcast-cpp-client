//
//  SerializationContext.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
#include "ClassDefinition.h"
#include "SerializationContext.h"
#include "SerializationService.h"
#include "PortableContext.h"

namespace hazelcast {
    namespace client {
        namespace serialization {

            SerializationContext::SerializationContext(int version, SerializationService *service)
            : contextVersion(version)
            , service(service) {
            };

            SerializationContext::~SerializationContext() {
            };

            SerializationContext::SerializationContext(const SerializationContext& rhs) {
            };

            void SerializationContext::operator = (const SerializationContext& rhs) {
            };

            bool SerializationContext::isClassDefinitionExists(int factoryId, int classId) const {
                return isClassDefinitionExists(factoryId, classId, contextVersion);
            };

            boost::shared_ptr<ClassDefinition> SerializationContext::lookup(int factoryId, int classId) {
                return getPortableContext(factoryId)->lookup(classId, contextVersion);
            };

            bool SerializationContext::isClassDefinitionExists(int factoryId, int classId, int version) const {
                return getPortableContext(factoryId)->isClassDefinitionExists(classId, version);
            };

            boost::shared_ptr<ClassDefinition> SerializationContext::lookup(int factoryId, int classId, int version) const {
                return getPortableContext(factoryId)->lookup(classId, version);
            };

            boost::shared_ptr<ClassDefinition> SerializationContext::createClassDefinition(int factoryId, std::vector<byte>& binary) {
                return getPortableContext(factoryId)->createClassDefinition(binary);
            };

            void SerializationContext::registerNestedDefinitions(boost::shared_ptr<ClassDefinition> cd) {
                vector<boost::shared_ptr<ClassDefinition> > nestedDefinitions = cd->getNestedClassDefinitions();
                for (vector<boost::shared_ptr<ClassDefinition> >::iterator it = nestedDefinitions.begin(); it < nestedDefinitions.end(); it++) {
                    registerClassDefinition(*it);
                    registerNestedDefinitions(*it);
                }
            };

            void SerializationContext::registerClassDefinition(boost::shared_ptr<ClassDefinition> cd) {
                getPortableContext(cd->getFactoryId())->registerClassDefinition(cd);
            };

            int SerializationContext::getVersion() {
                return contextVersion;
            };

            PortableContext *SerializationContext::getPortableContext(int factoryId) const {
                int count = portableContextMap.count(factoryId);
                if (count == 0) {
                    char message[70];
                    sprintf(message, "Could not find Portable factory for factoryId: %d", factoryId);
                    throw hazelcast::client::HazelcastException(message);
                }
                return portableContextMap.at(factoryId);
            };
        }
    }
}