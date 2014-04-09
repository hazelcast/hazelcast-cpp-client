//
// Created by sancar koyunlu on 8/13/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_RemoveInterceptorRequest
#define HAZELCAST_RemoveInterceptorRequest

#include "hazelcast/client/impl/PortableRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace map {
            class HAZELCAST_API RemoveInterceptorRequest : public impl::PortableRequest {
            public:
                RemoveInterceptorRequest(const std::string &, const std::string &);

                int getFactoryId() const;

                int getClassId() const;

                void write(serialization::PortableWriter &writer) const;

            private:
                std::string name;
                std::string id;
            };
        }
    }
}

#endif //HAZELCAST_RemoveInterceptorRequest

