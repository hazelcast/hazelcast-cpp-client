//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_CountDownRequest
#define HAZELCAST_CountDownRequest


#include "hazelcast/client/impl/PortableRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace serialization{
            class PortableWriter;
        }

        namespace countdownlatch {
            class CountDownRequest : public impl::PortableRequest {
            public:
                CountDownRequest(const std::string& instanceName);

                int getFactoryId() const;

                int getClassId() const;

                void writePortable(serialization::PortableWriter& writer) const;

            private:

                std::string instanceName;
            };
        }
    }
}
#endif //HAZELCAST_CountDownRequest
