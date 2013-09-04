//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_QUEUE_COMPARE_AND_REMOVE_REQUEST
#define HAZELCAST_QUEUE_COMPARE_AND_REMOVE_REQUEST

#include "Portable.h"
#include "Data.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace queue {
            class CompareAndRemoveRequest : public Portable {
            public:

                CompareAndRemoveRequest(const std::string& name, std::vector<serialization::Data>& dataList, bool retain);

                int getFactoryId() const;

                int getClassId() const;

                void writePortable(serialization::PortableWriter& writer) const;

                void readPortable(serialization::PortableReader& reader);

            private:
                std::vector<serialization::Data>& dataList;
                std::string name;
                bool retain;
            };
        }
    }
}

#endif //HAZELCAST_OFFER_REQUEST
