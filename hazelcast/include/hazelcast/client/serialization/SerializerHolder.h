//
// Created by sancar koyunlu on 7/31/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_SerializerHolder
#define HAZELCAST_SerializerHolder

#include "ConcurrentMap.h"

namespace hazelcast {
    namespace client {
        namespace serialization {

            class SerializerBase;

            class SerializerHolder {

            public:
                SerializerHolder();

                bool registerSerializer(SerializerBase *serializer);

                SerializerBase *serializerFor(int typeId);

            private:
                util::ConcurrentMap<int, SerializerBase> serializers;

            };

        }
    }
}

#endif //HAZELCAST_SerializerHolder
