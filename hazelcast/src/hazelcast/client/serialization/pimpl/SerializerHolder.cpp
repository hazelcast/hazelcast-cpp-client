//
// Created by sancar koyunlu on 7/31/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/serialization/pimpl/SerializerHolder.h"
#include "hazelcast/client/serialization/Serializer.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                SerializerHolder::SerializerHolder(PortableContext&context)
                :portableSerializer(context) {

                }

                bool SerializerHolder::registerSerializer(boost::shared_ptr<SerializerBase> serializer) {
                    boost::shared_ptr<SerializerBase> available = serializers.putIfAbsent(serializer->getTypeId(), serializer);
                    return available.get() == NULL;
                }

                boost::shared_ptr<SerializerBase> SerializerHolder::serializerFor(int typeId) {
                    return serializers.get(typeId);
                }

                const PortableSerializer &SerializerHolder::getPortableSerializer() const {
                    return portableSerializer;
                }

                const DataSerializer &SerializerHolder::getDataSerializer() const {
                    return dataSerializer;
                }
            }
        }
    }
}
