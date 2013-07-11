//
//  SerializationService.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#include "hazelcast/client/protocol/HazelcastServerError.h"
#include "hazelcast/client/serialization/SerializationService.h"

using namespace std;

namespace hazelcast {
    namespace client {
        namespace serialization {

            SerializationService::SerializationService(int version)
            : serializationContext(version)
            , portableSerializer(&serializationContext) {
            };

            SerializationService::SerializationService(SerializationService const & rhs)
            : serializationContext(1)
            , portableSerializer(&serializationContext) {
            };


            SerializationContext & SerializationService::getSerializationContext() {
                return serializationContext;
            };

            SerializationService::~SerializationService() {
            };


            bool SerializationService::registerSerializer(SerializerBase *serializer) {
                SerializerBase *available = serializers.putIfAbsent(serializer->getTypeId(), serializer);
                return available == NULL;
            };

            SerializerBase *SerializationService::serializerFor(int id) {
                return serializers.get(id);
            };

            void SerializationService::checkServerError(const Data & data) {
                if (data.isServerError()) {
                    protocol::HazelcastServerError error;
                    throw toObjectResolved<protocol::HazelcastServerError>(data, &error);
                }
            };

        }
    }
}