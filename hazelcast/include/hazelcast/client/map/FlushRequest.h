//
// Created by sancar koyunlu on 6/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#ifndef HAZELCAST_MAP_FLUSH_REQUEST
#define HAZELCAST_MAP_FLUSH_REQUEST

#include "hazelcast/client/impl/PortableRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace map {
            class FlushRequest : public impl::PortableRequest {
            public:
                FlushRequest(const std::string& name);

                int getFactoryId() const;

                int getClassId() const;

                void writePortable(serialization::PortableWriter& writer) const;



            private:
                std::string name;
            };
        }
    }
}


#endif //MAP_FLUSH_REQUEST
