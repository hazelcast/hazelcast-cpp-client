//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_RemoveRequest
#define HAZELCAST_RemoveRequest

#include "hazelcast/client/multimap/KeyBasedRequest.h"

namespace hazelcast {
    namespace client {
        namespace multimap {
            class HAZELCAST_API RemoveRequest : public KeyBasedRequest {
            public:
                RemoveRequest(const std::string& name, const serialization::Data& key, const serialization::Data& value, int threadId);

                int getClassId() const;

                void writePortable(serialization::PortableWriter& writer) const;

            private:
                int threadId;
                const serialization::Data& value;
            };
        }
    }
}


#endif //HAZELCAST_RemoveRequest
