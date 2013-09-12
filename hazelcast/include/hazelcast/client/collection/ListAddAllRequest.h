//
// Created by sancar koyunlu on 9/12/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_ListAddAllRequest
#define HAZELCAST_ListAddAllRequest

#include "CollectionAddAllRequest.h"

namespace hazelcast {
    namespace client {
        namespace list {
            class ListAddAllRequest : public collection::CollectionAddAllRequest {
            public:
                ListAddAllRequest(const std::string& name, const std::vector<serialization::Data>& valueList, int index);

                int getClassId() const;

                void writePortable(serialization::PortableWriter& writer) const;

            private:
                int index;
            };
        }
    }
}

#endif //HAZELCAST_ListAddAllRequest
