//
// Created by sancar koyunlu on 7/31/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "SerializerHolder.h"

namespace hazelcast {
    namespace client {
        namespace serialization {

            SerializerHolder::SerializerHolder(SerializationContext& context)
            :portableSerializer(context) {

            }

            bool SerializerHolder::registerSerializer(SerializerBase *serializer) {
                SerializerBase *available = serializers.putIfAbsent(serializer->getTypeId(), serializer);
                return available == NULL;
            };

            SerializerBase *SerializerHolder::serializerFor(int typeId) {
                return serializers.get(typeId);
            }

            PortableSerializer& SerializerHolder::getPortableSerializer() {
                return portableSerializer;
            }

            DataSerializer& SerializerHolder::getDataSerializer() {
                return dataSerializer;
            }


        }
    }
}