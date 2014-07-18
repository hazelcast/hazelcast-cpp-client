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
#include "hazelcast/client/SerializationConfig.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                SerializationService::SerializationService(const SerializationConfig& serializationConfig)
                : portableContext(serializationConfig.getPortableVersion())
                , serializationConfig(serializationConfig) {
                    std::vector<boost::shared_ptr<SerializerBase> > const &serializers = serializationConfig.getSerializers();
                    std::vector<boost::shared_ptr<SerializerBase> >::const_iterator it;
                    SerializerHolder &serializerHolder = getSerializerHolder();
                    for(it = serializers.begin() ; it < serializers.end() ; ++it){
                        serializerHolder.registerSerializer(*it);
                    }
                };


                PortableContext&SerializationService::getPortableContext() {
                    return portableContext;
                };

                SerializerHolder &SerializationService::getSerializerHolder() {
                    return portableContext.getSerializerHolder();
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
