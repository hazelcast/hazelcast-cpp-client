//
// Created by sancar koyunlu on 6/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_MAP_CLEAR_REQUEST
#define HAZELCAST_MAP_CLEAR_REQUEST

#include "hazelcast/client/impl/PortableRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace map {
            class HAZELCAST_API ClearRequest : public impl::PortableRequest{
            public:
                ClearRequest(const std::string& name);

                int getFactoryId() const;

                int getClassId() const;

                void write(serialization::PortableWriter& writer) const;

                bool isRetryable() const;

            private:
                std::string name;
            };
        }
    }
}


#endif //MAP_CLEAR_REQUEST

