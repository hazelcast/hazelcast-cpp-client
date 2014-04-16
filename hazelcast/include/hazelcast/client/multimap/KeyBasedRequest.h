//
// Created by sancar koyunlu on 9/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_KeyBasedRequest
#define HAZELCAST_KeyBasedRequest

#include "hazelcast/client/impl/PortableRequest.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace multimap {

            class HAZELCAST_API KeyBasedRequest : public impl::PortableRequest {
            public:
                KeyBasedRequest(const std::string &name, const serialization::pimpl::Data &key);

                virtual int getFactoryId() const;

                virtual void write(serialization::PortableWriter &writer) const;

            private:
                std::string name;
                serialization::pimpl::Data key;

            };
        }
    }
}

#endif //HAZELCAST_KeyBasedRequest

