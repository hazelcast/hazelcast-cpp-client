//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_MAP_TRY_PUT_REQUEST
#define HAZELCAST_MAP_TRY_PUT_REQUEST

#include "hazelcast/client/impl/PortableRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace serialization {
            class Data;
        }
        namespace map {
            class TryPutRequest : public impl::PortableRequest {
            public:
                TryPutRequest(const std::string& name, serialization::Data& key, serialization::Data& value, int threadId, long timeout);

                int getFactoryId() const;

                int getClassId() const;

                void writePortable(serialization::PortableWriter& writer) const;



            private:
                serialization::Data& key;
                serialization::Data& value;
                std::string name;
                int threadId;
                long ttl;
                long timeout;
            };
        }
    }
}

#endif //HAZELCAST_MAP_PUT_REQUEST
