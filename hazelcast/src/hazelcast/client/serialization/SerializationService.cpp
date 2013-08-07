//
//  SerializationService.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#include "ServerException.h"
#include "hazelcast/client/serialization/SerializationService.h"

using namespace std;

namespace hazelcast {
    namespace client {
        namespace serialization {

            SerializationService::SerializationService(int version)
            : serializationContext(version)
            , serializerHolder(serializationContext) {
            };

            SerializationService::SerializationService(SerializationService const & rhs)
            : serializationContext(1)
            , serializerHolder(serializationContext) {
            };


            SerializationContext & SerializationService::getSerializationContext() {
                return serializationContext;
            };

            SerializationService::~SerializationService() {
            };


            bool SerializationService::registerSerializer(SerializerBase *serializer) {
                return serializerHolder.registerSerializer(serializer);
            };

            SerializerBase *SerializationService::serializerFor(int typeId) {
                return serializerHolder.serializerFor(typeId);
            };

            void SerializationService::checkServerError(const Data & data) {
                if (data.isServerError()) {
                    exception::ServerException error;
                    throw toObjectResolved<exception::ServerException>(data, &error);
                }
            };

        }
    }
}