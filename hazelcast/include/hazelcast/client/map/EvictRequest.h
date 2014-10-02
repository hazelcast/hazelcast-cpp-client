//
// Created by sancar koyunlu on 6/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_MAP_EVICT_REQUEST
#define HAZELCAST_MAP_EVICT_REQUEST

#include "hazelcast/client/impl/ClientRequest.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace map {
            class EvictRequest : public impl::ClientRequest {
            public:
                EvictRequest(const std::string &name, const serialization::pimpl::Data &key, long threadId);

                int getFactoryId() const;

                int getClassId() const;

                void write(serialization::PortableWriter &writer) const;

            private:
                std::string name;
                serialization::pimpl::Data key;
                long threadId;
            };
        }
    }
}

#endif //HAZELCAST_MAP_EVICT_REQUEST

