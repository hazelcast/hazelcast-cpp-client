//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_MAP_TRY_REMOVE_REQUEST
#define HAZELCAST_MAP_TRY_REMOVE_REQUEST

#include "hazelcast/client/impl/ClientRequest.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace map {
            class TryRemoveRequest : public impl::ClientRequest {
            public:
                TryRemoveRequest(const std::string& name, const serialization::pimpl::Data& key, long threadId, long timeout);

                int getFactoryId() const;

                int getClassId() const;

                void write(serialization::PortableWriter& writer) const;

            private:
                std::string name;
                serialization::pimpl::Data key;
                long threadId;
                int timeout;
            };
        }
    }
}

#endif //HAZELCAST_MAP_REMOVE_REQUEST

