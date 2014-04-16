//
// Created by sancar koyunlu on 9/12/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_CollectionRemoveRequest
#define HAZELCAST_CollectionRemoveRequest

#include "hazelcast/client/collection/CollectionRequest.h"
#include "hazelcast/client/serialization/pimpl/Data.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl{
                class Data;
            }

        }
        namespace collection {
            class HAZELCAST_API CollectionRemoveRequest : public CollectionRequest {
            public:
                CollectionRemoveRequest(const std::string &name, const std::string &serviceName, const serialization::pimpl::Data &data);

                int getClassId() const;

                void write(serialization::PortableWriter &writer) const;

            private:
                serialization::pimpl::Data data;
            };
        }
    }
}

#endif //HAZELCAST_CollectionRemoveRequest

