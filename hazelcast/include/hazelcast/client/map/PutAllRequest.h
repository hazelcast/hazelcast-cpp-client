//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_MAP_PUT_ALL_REQUEST
#define HAZELCAST_MAP_PUT_ALL_REQUEST

#include "hazelcast/client/impl/PortableRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace map {

            class MapEntrySet;

            class PutAllRequest : public impl::PortableRequest {
            public:
                PutAllRequest(const std::string& name, map::MapEntrySet& entrySet);

                int getFactoryId() const;

                int getClassId() const;

                void writePortable(serialization::PortableWriter& writer) const;



            private:
                std::string name;
                map::MapEntrySet& entrySet;
            };
        }
    }
}

#endif //HAZELCAST_MAP_PUT_ALL_REQUEST
