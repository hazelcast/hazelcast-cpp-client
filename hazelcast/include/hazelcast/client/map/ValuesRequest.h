//
// Created by sancar koyunlu on 6/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#ifndef MAP_VALUES_REQUEST
#define MAP_VALUES_REQUEST

#include "hazelcast/client/impl/PortableRequest.h"
#include "hazelcast/client/impl/RetryableRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace map {
            class ValuesRequest : public impl::PortableRequest, public RetryableRequest {
            public:
                ValuesRequest(const std::string& name);

                int getFactoryId() const;

                int getClassId() const;

                void writePortable(serialization::PortableWriter& writer) const;

            private:
                const std::string& name;
            };
        }
    }
}


#endif //MAP_KEY_SET_REQUEST
