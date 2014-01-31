//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_MAP_PUT_IF_ABSENT_REQUEST
#define HAZELCAST_MAP_PUT_IF_ABSENT_REQUEST

#include "hazelcast/client/serialization/Data.h"
#include "hazelcast/client/map/PortableHook.h"
#include "hazelcast/client/impl/PortableRequest.h"

namespace hazelcast {
    namespace client {
        namespace map {
            class HAZELCAST_API PutIfAbsentRequest : public impl::PortableRequest {
            public:
                PutIfAbsentRequest(const std::string &name, serialization::Data &key, serialization::Data &value, long threadId, long ttl);

                int getFactoryId() const;

                int getClassId() const;

                void write(serialization::PortableWriter &writer) const;

            private:
                serialization::Data key;
                serialization::Data value;
                std::string name;
                long threadId;
                long ttl;
            };
        }
    }
}

#endif //HAZELCAST_MAP_PUT_REQUEST
