//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_MAP_PUT_IF_ABSENT_REQUEST
#define HAZELCAST_MAP_PUT_IF_ABSENT_REQUEST

#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/client/map/PortableHook.h"
#include "hazelcast/client/impl/ClientRequest.h"

namespace hazelcast {
    namespace client {
        namespace map {
            class HAZELCAST_API PutIfAbsentRequest : public impl::ClientRequest {
            public:
                PutIfAbsentRequest(const std::string &name, serialization::pimpl::Data &key, serialization::pimpl::Data &value, long threadId, long ttl);

                int getFactoryId() const;

                int getClassId() const;

                void write(serialization::PortableWriter &writer) const;

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

#endif //HAZELCAST_MAP_PUT_REQUEST

