//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_CollectionAddListenerRequest
#define HAZELCAST_CollectionAddListenerRequest

#include "CollectionRequest.h"

namespace hazelcast {
    namespace client {
        namespace collection {
            class CollectionAddListenerRequest : public CollectionRequest {
            public:
                CollectionAddListenerRequest(const std::string& name, bool includeValue);

                int getClassId() const;

                void writePortable(serialization::PortableWriter& writer) const;

            private:
                bool includeValue;
            };

        }
    }
}


#endif //HAZELCAST_AddItemListenerRequest
