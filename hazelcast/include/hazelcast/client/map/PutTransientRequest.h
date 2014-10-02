//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_MAP_PUT_TRANSIENT_REQUEST
#define HAZELCAST_MAP_PUT_TRANSIENT_REQUEST

#include "hazelcast/client/impl/ClientRequest.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace map {
            class PutTransientRequest : public impl::ClientRequest {
            public:
                PutTransientRequest(const std::string& name, const serialization::pimpl::Data& key, const serialization::pimpl::Data& value, long threadId, long ttl);

                int getFactoryId() const;

                int getClassId() const;

                void write(serialization::PortableWriter& writer) const;

            private:
                std::string name;
                serialization::pimpl::Data key;
                serialization::pimpl::Data value;
                long threadId;
                long ttl;
            };
        }
    }
}

#endif //HAZELCAST_MAP_PUT_TRANSIENT_REQUEST

