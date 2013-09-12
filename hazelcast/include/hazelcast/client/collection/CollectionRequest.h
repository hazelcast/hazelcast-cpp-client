//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.

#ifndef HAZELCAST_CollectionRequest
#define HAZELCAST_CollectionRequest

#include "Request.h"

namespace hazelcast {
    namespace client {
        namespace collection {

            class CollectionRequest : public impl::Request {
            public:
                CollectionRequest(const std::string& name);

                virtual int getFactoryId() const;

                virtual void setServiceName(const std::string& name);

                virtual void writePortable(serialization::PortableWriter& writer) const;

            private:
                const std::string& name;
                const std::string *serviceName;

            };
        }
    }
}


#endif //HAZELCAST_CollectionRequest
