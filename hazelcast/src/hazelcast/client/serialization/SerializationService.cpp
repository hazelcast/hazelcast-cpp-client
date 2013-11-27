//
//  SerializationService.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#include "hazelcast/client/exception/ServerException.h"
#include "hazelcast/client/serialization/SerializationService.h"

using namespace std;

namespace hazelcast {
    namespace client {
        namespace serialization {

            SerializationService::SerializationService(int version)
            : serializationContext(version) {
            };

            SerializationService::SerializationService(SerializationService const & rhs)
            : serializationContext(1) {
            };


            SerializationContext & SerializationService::getSerializationContext() {
                return serializationContext;
            };

            SerializerHolder& SerializationService::getSerializerHolder() {
                return serializationContext.getSerializerHolder();
            };

            SerializationService::~SerializationService() {
            };


            bool SerializationService::registerSerializer(SerializerBase *serializer) {
                return getSerializerHolder().registerSerializer(serializer);
            };

            SerializerBase *SerializationService::serializerFor(int typeId) {
                return getSerializerHolder().serializerFor(typeId);
            };

            void SerializationService::checkServerError(const Data & data) {
                if (data.isServerError()) {
                    exception::ServerException error;
                    throw *(toObjectResolved<exception::ServerException>(data, &error));
                }
            };

        }
    }
}