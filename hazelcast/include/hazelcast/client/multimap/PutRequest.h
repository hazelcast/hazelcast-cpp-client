//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_PutRequest
#define HAZELCAST_PutRequest

#include "KeyBasedRequest.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            class Data;
        }
        namespace multimap {
            class PutRequest : public KeyBasedRequest {
            public:
                PutRequest(const std::string& name, const serialization::Data& key, const serialization::Data& value, int index, int threadId);

                int getClassId() const;

                void writePortable(serialization::PortableWriter& writer) const;

            private:
                const serialization::Data& value;
                int threadId;
                int index;

            };
        }
    }
}

#endif //HAZELCAST_PutRequest
