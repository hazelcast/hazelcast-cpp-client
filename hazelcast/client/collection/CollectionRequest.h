//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.

#ifndef HAZELCAST_CollectionRequest
#define HAZELCAST_CollectionRequest

#include "CollectionProxyId.h"
#include "CollectionPortableHook.h"

namespace hazelcast {
    namespace client {
        namespace collection {


            class CollectionRequest {
            public:
                CollectionRequest(const CollectionProxyId& id)
                :id(id) {

                };

                virtual int getFactoryId() const {
                    return CollectionPortableHook::F_ID;
                };

                virtual int getTypeSerializerId() const {
                    return serialization::SerializationConstants::CONSTANT_TYPE_PORTABLE;
                };

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer << id;
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    reader >> id;
                };
            private:
                CollectionProxyId id;

            };
        }
    }
}

#endif //HAZELCAST_CollectionRequest
