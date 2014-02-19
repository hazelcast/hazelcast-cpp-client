//
// Created by sancar koyunlu on 6/12/13.
// Copyright (c) 2013 hazelcast. All rights reserved.

#ifndef HAZELCAST_PORTABLE_COLLECTION
#define HAZELCAST_PORTABLE_COLLECTION

#include "hazelcast/client/serialization/Data.h"
#include "hazelcast/client/impl/PortableResponse.h"
#include <vector>

namespace hazelcast {
    namespace client {
        namespace impl {
            class HAZELCAST_API PortableCollection : public impl::PortableResponse {
            public:

                const std::vector<serialization::Data>& getCollection() const;

                int getFactoryId() const;

                int getClassId() const;

                void readPortable(serialization::PortableReader& reader);

            private:
                std::vector<serialization::Data> collection;
            };
        }
    }
}

#endif //__PortableCollection_H_
