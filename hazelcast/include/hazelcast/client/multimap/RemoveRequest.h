//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_RemoveRequest
#define HAZELCAST_RemoveRequest

#include "hazelcast/client/multimap/KeyBasedRequest.h"
#include "hazelcast/client/serialization/Data.h"

namespace hazelcast {
    namespace client {
        namespace multimap {
            class HAZELCAST_API RemoveRequest : public KeyBasedRequest {
            public:
                RemoveRequest(const std::string &name, const serialization::Data &key, const serialization::Data &value, long threadId);

                int getClassId() const;

                void write(serialization::PortableWriter &writer) const;

            private:
                long threadId;
                serialization::Data value;
            };
        }
    }
}


#endif //HAZELCAST_RemoveRequest
