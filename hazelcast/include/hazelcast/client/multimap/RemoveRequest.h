//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_RemoveRequest
#define HAZELCAST_RemoveRequest

#include "hazelcast/client/multimap/KeyBasedRequest.h"
#include "hazelcast/client/serialization/pimpl/Data.h"

namespace hazelcast {
    namespace client {
        namespace multimap {
            class HAZELCAST_API RemoveRequest : public KeyBasedRequest {
            public:
                RemoveRequest(const std::string &name, const serialization::pimpl::Data &key, const serialization::pimpl::Data &value, long threadId);

                int getClassId() const;

                void write(serialization::PortableWriter &writer) const;

            private:
                serialization::pimpl::Data value;
                long threadId;
            };
        }
    }
}


#endif //HAZELCAST_RemoveRequest
