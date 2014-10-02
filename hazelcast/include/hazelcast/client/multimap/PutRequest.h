//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_PutRequest
#define HAZELCAST_PutRequest

#include "hazelcast/client/multimap/KeyBasedRequest.h"
#include "hazelcast/client/serialization/pimpl/Data.h"

namespace hazelcast {
    namespace client {
        namespace multimap {
            class PutRequest : public KeyBasedRequest {
            public:
                PutRequest(const std::string &name, const serialization::pimpl::Data &key, const serialization::pimpl::Data &value, int index, long threadId);

                int getClassId() const;

                void write(serialization::PortableWriter &writer) const;

            private:
                serialization::pimpl::Data value;
                long threadId;
                int index;

            };
        }
    }
}

#endif //HAZELCAST_PutRequest

