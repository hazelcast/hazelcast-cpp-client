//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_MAP_TRY_REMOVE_REQUEST
#define HAZELCAST_MAP_TRY_REMOVE_REQUEST

#include "hazelcast/client/impl/PortableRequest.h"
#include "hazelcast/client/serialization/Data.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace map {
            class HAZELCAST_API TryRemoveRequest : public impl::PortableRequest {
            public:
                TryRemoveRequest(const std::string &name, serialization::Data &key, long threadId, long timeout);

                int getFactoryId() const;

                int getClassId() const;

                void write(serialization::PortableWriter &writer) const;

            private:
                serialization::Data key;
                std::string name;
                long threadId;
                int timeout;
            };
        }
    }
}

#endif //HAZELCAST_MAP_REMOVE_REQUEST
