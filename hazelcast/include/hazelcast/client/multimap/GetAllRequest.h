//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_GetAllRequest
#define HAZELCAST_GetAllRequest

#include "hazelcast/client/multimap/KeyBasedRequest.h"

namespace hazelcast {
    namespace client {
        namespace multimap {
            class GetAllRequest : public KeyBasedRequest {
            public:
                GetAllRequest(const std::string& name, const serialization::pimpl::Data& key, long threadId);

                int getClassId() const;

                bool isRetryable() const;

                void write(serialization::PortableWriter& writer) const;

            private:
                long threadId;
            };
        }
    }
}

#endif //HAZELCAST_GetAllRequest

