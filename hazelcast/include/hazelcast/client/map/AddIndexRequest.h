//
// Created by sancar koyunlu on 6/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_ADD_INDEX_REQUEST
#define HAZELCAST_ADD_INDEX_REQUEST

#include "hazelcast/client/impl/PortableRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace map {

            class HAZELCAST_API AddIndexRequest : public impl::PortableRequest {
            public:
                AddIndexRequest(const std::string& name, const std::string& attribute, bool ordered);

                int getFactoryId() const;

                int getClassId() const;

                void write(serialization::PortableWriter& writer) const;

            private:
                std::string name;
                std::string attribute;
                bool ordered;
            };

        }
    }
}
#endif //HAZELCAST_ADD_INDEX_REQUEST

