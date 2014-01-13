//
// Created by sancar koyunlu on 7/31/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_SerializerHolder
#define HAZELCAST_SerializerHolder

#include "SynchronizedMap.h"
#include "hazelcast/client/serialization/DataSerializer.h"
#include "hazelcast/client/serialization/PortableSerializer.h"

namespace hazelcast {
    namespace client {
        namespace serialization {

            class SerializationContext;

            class SerializerBase;

            class HAZELCAST_API SerializerHolder {

            public:
                SerializerHolder(SerializationContext& context);

                bool registerSerializer(boost::shared_ptr<SerializerBase> serializer);

                boost::shared_ptr<SerializerBase> serializerFor(int typeId);

                PortableSerializer& getPortableSerializer();

                DataSerializer& getDataSerializer();

            private:
                util::SynchronizedMap<int, SerializerBase> serializers;
                PortableSerializer portableSerializer;
                DataSerializer dataSerializer;

            };

        }
    }
}

#endif //HAZELCAST_SerializerHolder
