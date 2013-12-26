//
// Created by sancar koyunlu on 6/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#ifndef MAP_KEY_SET_REQUEST
#define MAP_KEY_SET_REQUEST

#include "hazelcast/client/impl/PortableRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace map {
            class HAZELCAST_API KeySetRequest : public impl::PortableRequest {
            public:
                KeySetRequest(const std::string& name);

                int getFactoryId() const;

                int getClassId() const;

                void write(serialization::PortableWriter& writer) const;



            private:
                std::string name;
            };
        }
    }
}


#endif //MAP_KEY_SET_REQUEST
