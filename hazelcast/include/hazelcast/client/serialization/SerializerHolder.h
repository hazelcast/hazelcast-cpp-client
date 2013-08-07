//
// Created by sancar koyunlu on 7/31/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_SerializerHolder
#define HAZELCAST_SerializerHolder

#include "ConcurrentMap.h"
#include "DataSerializer.h"
#include "PortableSerializer.h"

namespace hazelcast {
    namespace client {
        namespace serialization {

            class SerializationContext;

            class SerializerBase;

            class SerializerHolder {

            public:
                SerializerHolder(SerializationContext& context);

                bool registerSerializer(SerializerBase *serializer);

                SerializerBase *serializerFor(int typeId);

                PortableSerializer& getPortableSerializer();

                DataSerializer& getDataSerializer();

            private:
                util::ConcurrentMap<int, SerializerBase> serializers;
                PortableSerializer portableSerializer;
                DataSerializer dataSerializer;

            };

        }
    }
}

#endif //HAZELCAST_SerializerHolder
