//
//  SerializationService.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#include <stdio.h>
#include "hazelcast/client/serialization/pimpl/PortableVersionHelper.h"
#include "hazelcast/client/serialization/pimpl/SerializationService.h"
#include "hazelcast/client/exception/IClassCastException.h"
#include "hazelcast/client/SerializationConfig.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                SerializationService::SerializationService(const SerializationConfig& serializationConfig)
                : portableContext(serializationConfig.getPortableVersion(), *this)
                , serializationConfig(serializationConfig) {
                    std::vector<boost::shared_ptr<SerializerBase> > const& serializers = serializationConfig.getSerializers();
                    std::vector<boost::shared_ptr<SerializerBase> >::const_iterator it;
                    SerializerHolder& serializerHolder = getSerializerHolder();
                    for (it = serializers.begin(); it < serializers.end(); ++it) {
                        serializerHolder.registerSerializer(*it);
                    }
                }

                void SerializationService::writeObject(DataOutput &out, const Portable *obj) {
                    const bool isNull = obj == 0;

                    out.writeBoolean(isNull);
                    if (isNull) {
                        return;
                    }

                    // write type
                    out.writeInt(SerializationConstants::CONSTANT_TYPE_PORTABLE);

                    getSerializerHolder().getPortableSerializer().write(out, *obj);
                }

                void SerializationService::writeObject(DataOutput &out, const IdentifiedDataSerializable *obj) {
                    const bool isNull = obj == 0;

                    out.writeBoolean(isNull);
                    if (isNull) {
                        return;
                    }

                    ObjectDataOutput dataOutput(out, portableContext);

                    // write type
                    dataOutput.writeInt(SerializationConstants::CONSTANT_TYPE_DATA);

                    getSerializerHolder().getDataSerializer().write(dataOutput, *obj);
                }

                PortableContext& SerializationService::getPortableContext() {
                    return portableContext;
                }

                SerializerHolder& SerializationService::getSerializerHolder() {
                    return portableContext.getSerializerHolder();
                }


                bool SerializationService::registerSerializer(boost::shared_ptr<SerializerBase> serializer) {
                    return getSerializerHolder().registerSerializer(serializer);
                }

                boost::shared_ptr<SerializerBase> SerializationService::serializerFor(int typeId) {
                    return getSerializerHolder().serializerFor(typeId);
                }


                void SerializationService::checkClassType(int expectedType, int currentType) {
                    if (expectedType != currentType) {
                        char message[200];
                        SerializationConstants sc;
                        sprintf(message, "Received data of type %s(%d) but expected data type %s(%d)",
                                sc.typeIdToName(currentType).c_str(), currentType,
                                sc.typeIdToName(expectedType).c_str(), expectedType);

                        util::ILogger::getLogger().severe(message);
                        throw exception::IClassCastException("SerializationService::checkClassType",
                                message);
                    }
                }

            }
        }
    }
}
