//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_PutRequest
#define HAZELCAST_PutRequest

#include "hazelcast/client/multimap/KeyBasedRequest.h"
#include "hazelcast/client/serialization/Data.h"

namespace hazelcast {
    namespace client {
        namespace multimap {
            class HAZELCAST_API PutRequest : public KeyBasedRequest {
            public:
                PutRequest(const std::string &name, const serialization::Data &key, const serialization::Data &value, int index, long threadId);

                int getClassId() const;

                void write(serialization::PortableWriter &writer) const;

            private:
                serialization::Data value;
                long threadId;
                int index;

            };
        }
    }
}

#endif //HAZELCAST_PutRequest
