//
//  SerializationService.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#include "hazelcast/client/serialization/pimpl/SerializationService.h"
#include "hazelcast/client/exception/IClassCastException.h"
#include "hazelcast/util/ILogger.h"


namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                SerializationService::SerializationService(int version)
                : serializationContext(version) {
                };


                SerializationContext &SerializationService::getSerializationContext() {
                    return serializationContext;
                };

                SerializerHolder &SerializationService::getSerializerHolder() {
                    return serializationContext.getSerializerHolder();
                };


                bool SerializationService::registerSerializer(boost::shared_ptr<SerializerBase> serializer) {
                    return getSerializerHolder().registerSerializer(serializer);
                };

                boost::shared_ptr<SerializerBase> SerializationService::serializerFor(int typeId) {
                    return getSerializerHolder().serializerFor(typeId);
                };

                void SerializationService::checkClassType(int expectedType, int currentType) {
                    if (expectedType != currentType) {
                        std::string source = "SerializationService:toObject<" + constants.typeIdToName(expectedType) + "> ";
                        std::string message = "recevied data of type " + constants.typeIdToName(currentType);
                        util::ILogger::getLogger().severe(source + message);
                        throw exception::IClassCastException(source, message);
                    }
                }
            }
        }
    }
}