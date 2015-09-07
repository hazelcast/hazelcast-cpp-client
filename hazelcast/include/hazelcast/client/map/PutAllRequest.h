//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_MAP_PUT_ALL_REQUEST
#define HAZELCAST_MAP_PUT_ALL_REQUEST

#include "hazelcast/client/impl/ClientRequest.h"
#include "hazelcast/client/impl/MapEntrySet.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace map {
            class PutAllRequest : public impl::ClientRequest {
            public:
                PutAllRequest(const std::string& name,
                              const std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> >& m,
                              int pId);

                int getFactoryId() const;

                int getClassId() const;

                void write(serialization::PortableWriter& writer) const;

            private:
                std::string name;
                map::MapEntrySet entrySet;
                int partitionId;
            };
        }
    }
}

#endif //HAZELCAST_MAP_PUT_ALL_REQUEST

