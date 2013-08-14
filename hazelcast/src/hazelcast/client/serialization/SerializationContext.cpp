//
//  SerializationContext.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
#include "hazelcast/client/serialization/PortableContext.h"
#include "hazelcast/client/serialization/SerializationService.h"

namespace hazelcast {
    namespace client {
        namespace serialization {

            SerializationContext::SerializationContext(int version)
            : contextVersion(version)
            , serializerHolder(*this) {
//                portableContextMap[-1] = new PortableContext(this); //TODO right now hardcoded : will changed as configurable (may be)
//                portableContextMap[1] = new PortableContext(this);
//                portableContextMap[-3] = new PortableContext(this);
//                portableContextMap[0] = new PortableContext(this);
            };

            SerializationContext::~SerializationContext() {
            };

            SerializationContext::SerializationContext(const SerializationContext& rhs)
            :serializerHolder(*this) {
                //private
            };

            void SerializationContext::operator = (const SerializationContext& rhs) {
                //private
            };

            bool SerializationContext::isClassDefinitionExists(int factoryId, int classId) {
                return isClassDefinitionExists(factoryId, classId, contextVersion);
            };

            util::AtomicPointer<ClassDefinition> SerializationContext::lookup(int factoryId, int classId) {
                return getPortableContext(factoryId).lookup(classId, contextVersion);
            };

            bool SerializationContext::isClassDefinitionExists(int factoryId, int classId, int version) {
                return getPortableContext(factoryId).isClassDefinitionExists(classId, version);
            };

            util::AtomicPointer<ClassDefinition> SerializationContext::lookup(int factoryId, int classId, int version) {
                return getPortableContext(factoryId).lookup(classId, version);
            };

            util::AtomicPointer<ClassDefinition> SerializationContext::createClassDefinition(int factoryId, std::auto_ptr< std::vector<byte> > binary) {
                return getPortableContext(factoryId).createClassDefinition(binary);
            };

            void SerializationContext::registerNestedDefinitions(util::AtomicPointer<ClassDefinition> cd) {
                std::vector<util::AtomicPointer<ClassDefinition> > nestedDefinitions = cd->getNestedClassDefinitions();
                for (std::vector<util::AtomicPointer<ClassDefinition> >::iterator it = nestedDefinitions.begin(); it < nestedDefinitions.end(); it++) {
                    registerClassDefinition(*it);
                    registerNestedDefinitions(*it);
                }
            };

            util::AtomicPointer<ClassDefinition> SerializationContext::registerClassDefinition(util::AtomicPointer<ClassDefinition> cd) {
                return getPortableContext(cd->getFactoryId()).registerClassDefinition(cd);
            };

            util::AtomicPointer<ClassDefinition> SerializationContext::registerClassDefinition(ClassDefinition *cd) {
                return getPortableContext(cd->getFactoryId()).registerClassDefinition(util::AtomicPointer<ClassDefinition>(cd));
            };

            int SerializationContext::getVersion() {
                return contextVersion;
            };

            SerializerHolder& SerializationContext::getSerializerHolder() {
                return serializerHolder;
            };

            PortableContext& SerializationContext::getPortableContext(int factoryId) {
//                int count = portableContextMap.count(factoryId);//TODO look at the constructor
//                if (count == 0) {
//                    char message[70];
//                    sprintf(message, "Could not find Portable factory for factoryId: %d", factoryId);
//                    throw hazelcast::client::IException(message);
//                }
//                return portableContextMap.at(factoryId);
                util::AtomicPointer<PortableContext> value = portableContextMap.get(factoryId);
                if (value == NULL) {
                    value = util::AtomicPointer<PortableContext>(new PortableContext(this));
                    util::AtomicPointer<PortableContext> current = portableContextMap.putIfAbsent(factoryId, value);
                    value = current == NULL ? value : current;
                }
//                PortableContext* temp = portableContextMap.get(factoryId);
//                temp->setSerializationContext(this);
                return *(value.get());
            };
        }
    }
}