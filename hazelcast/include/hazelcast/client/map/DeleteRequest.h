//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_MAP_DELETE_REQUEST
#define HAZELCAST_MAP_DELETE_REQUEST

#include "Portable.h"

namespace hazelcast {
    namespace client {
        namespace map {
            class DeleteRequest : public Portable {
            public:
                DeleteRequest(const std::string& name, serialization::Data& key, int threadId);

                int getFactoryId() const;

                int getClassId() const;

                void writePortable(serialization::PortableWriter& writer) const;

                void readPortable(serialization::PortableReader& reader);

            private:
                serialization::Data& key;
                std::string name;
                int threadId;
            };
        }
    }
}

#endif //HAZELCAST_MAP_PUT_REQUEST
