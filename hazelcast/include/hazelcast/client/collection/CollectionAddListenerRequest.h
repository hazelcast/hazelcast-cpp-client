//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_CollectionAddListenerRequest
#define HAZELCAST_CollectionAddListenerRequest

#include "hazelcast/client/collection/CollectionRequest.h"

namespace hazelcast {
    namespace client {
        namespace collection {
            class HAZELCAST_API CollectionAddListenerRequest : public CollectionRequest {
            public:
                CollectionAddListenerRequest(const std::string &name, const std::string &serviceName, bool includeValue);

                int getClassId() const;

                void write(serialization::PortableWriter &writer) const;

            private:
                bool includeValue;
            };

        }
    }
}


#endif //HAZELCAST_AddItemListenerRequest

