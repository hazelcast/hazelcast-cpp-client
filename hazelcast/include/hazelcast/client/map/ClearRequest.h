//
// Created by sancar koyunlu on 6/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_MAP_CLEAR_REQUEST
#define HAZELCAST_MAP_CLEAR_REQUEST

#include "hazelcast/client/impl/PortableRequest.h"
#include "hazelcast/client/impl/RetryableRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace map {
            class ClearRequest : public impl::PortableRequest, public RetryableRequest {
            public:
                ClearRequest(const std::string& name);

                int getFactoryId() const;

                int getClassId() const;

                void writePortable(serialization::PortableWriter& writer) const;



            private:
                std::string name;
            };
        }
    }
}


#endif //MAP_CLEAR_REQUEST
