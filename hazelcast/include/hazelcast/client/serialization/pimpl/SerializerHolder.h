//
// Created by sancar koyunlu on 7/31/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_SerializerHolder
#define HAZELCAST_SerializerHolder

#include "hazelcast/util/SynchronizedMap.h"
#include "hazelcast/client/serialization/pimpl/DataSerializer.h"
#include "hazelcast/client/serialization/pimpl/PortableSerializer.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace serialization {

            class SerializerBase;

            namespace pimpl {
                class PortableContext;

                class HAZELCAST_API SerializerHolder {

                public:
                    SerializerHolder(PortableContext &context);

                    bool registerSerializer(boost::shared_ptr<SerializerBase> serializer);

                    boost::shared_ptr<SerializerBase> serializerFor(int typeId);

                    PortableSerializer &getPortableSerializer();

                    DataSerializer &getDataSerializer();

                private:
                    hazelcast::util::SynchronizedMap<int, SerializerBase> serializers;
                    PortableSerializer portableSerializer;
                    DataSerializer dataSerializer;

                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_SerializerHolder

