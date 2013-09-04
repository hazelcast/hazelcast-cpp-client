//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_MAP_GET_ENTRY_VIEW_REQUEST
#define HAZELCAST_MAP_GET_ENTRY_VIEW_REQUEST

#include "Portable.h"
#include "RetryableRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace serialization {
            class Data;
        }
        namespace map {
            class GetEntryViewRequest : public Portable, public RetryableRequest {
            public:
                GetEntryViewRequest(const std::string& name, serialization::Data& key);

                int getFactoryId() const;

                int getClassId() const;

                void writePortable(serialization::PortableWriter& writer) const;

                void readPortable(serialization::PortableReader& reader);

            private:
                serialization::Data& key;
                std::string name;
            };
        }
    }
}

#endif //HAZELCAST_MAP_GET_ENTRY_VIEW_REQUEST
