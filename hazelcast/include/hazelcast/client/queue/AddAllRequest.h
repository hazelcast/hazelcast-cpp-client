//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_QUEUE_ADD_ALL_REQUEST
#define HAZELCAST_QUEUE_ADD_ALL_REQUEST

#include "hazelcast/client/serialization/Data.h"
#include "hazelcast/client/Portable.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace queue {
            class AddAllRequest : public Portable {
            public:

                AddAllRequest(const std::string& name, std::vector<serialization::Data>& dataList);

                int getFactoryId() const;

                int getClassId() const;

                void writePortable(serialization::PortableWriter& writer) const;

                void readPortable(serialization::PortableReader& reader);

            private:
                std::vector<serialization::Data>& dataList;
                std::string name;
            };
        }
    }
}

#endif //HAZELCAST_QUEUE_ADD_ALL_REQUEST
