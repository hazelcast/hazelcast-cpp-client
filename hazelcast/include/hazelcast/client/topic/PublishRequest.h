//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_PublishRequest
#define HAZELCAST_PublishRequest

#include "hazelcast/client/impl/PortableRequest.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace topic {
            class HAZELCAST_API PublishRequest : public impl::PortableRequest {
            public:
                PublishRequest(const std::string& instanceName, const serialization::pimpl::Data& message);

                int getFactoryId() const;

                int getClassId() const;

                void write(serialization::PortableWriter& writer) const;

            private:
                std::string instanceName;
                serialization::pimpl::Data message;
            };
        }
    }
}


#endif //HAZELCAST_PublishRequest

