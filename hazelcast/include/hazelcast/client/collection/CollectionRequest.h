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


            class CollectionRequest : public Portable {
            public:
                CollectionRequest(const CollectionProxyId& id)
                :id(id) {

                };

                virtual int getFactoryId() const {
                    return CollectionPortableHook::F_ID;
                };

                void writePortable(serialization::PortableWriter& writer) const {
                    serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                    id.writeData(out);
                };

                void readPortable(serialization::PortableReader& reader) {
                    serialization::ObjectDataInput &in = reader.getRawDataInput();
                    id.readData(in);
                };
            private:
                CollectionProxyId id;

            };
        }
    }
}

#endif //HAZELCAST_CollectionRequest
