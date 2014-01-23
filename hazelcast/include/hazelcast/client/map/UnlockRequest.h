//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_UNLOCK_REQUEST
#define HAZELCAST_UNLOCK_REQUEST

#include "hazelcast/client/impl/PortableRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace serialization {
            class Data;
        }
        namespace map {
            class HAZELCAST_API UnlockRequest : public impl::PortableRequest {
            public:
                UnlockRequest(const std::string &name, serialization::Data &key, long threadId);

                UnlockRequest(const std::string &name, serialization::Data &key, long threadId, bool force);

                int getFactoryId() const;

                int getClassId() const;

                void write(serialization::PortableWriter &writer) const;


            private:
                serialization::Data &key;
                std::string name;
                long threadId;
                bool force;
            };
        }
    }
}

#endif //HAZELCAST_MAP_PUT_REQUEST
