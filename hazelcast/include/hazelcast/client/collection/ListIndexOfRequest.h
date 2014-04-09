//
// Created by sancar koyunlu on 9/12/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_ListIndexOfRequest
#define HAZELCAST_ListIndexOfRequest

#include "hazelcast/client/collection/CollectionRequest.h"
#include "hazelcast/client/serialization/pimpl/Data.h"

namespace hazelcast {
    namespace client {
        namespace list {
            class HAZELCAST_API ListIndexOfRequest : public collection::CollectionRequest {
            public:

                ListIndexOfRequest(const std::string &name, const std::string &serviceName, const serialization::pimpl::Data &data, bool last);

                void write(serialization::PortableWriter &writer) const;

                int getClassId() const;

            private:
                serialization::pimpl::Data data;
                bool last;
            };
        }
    }
}

#endif //HAZELCAST_ListIndexOfRequest

