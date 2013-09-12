//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_PortableEntrySetResponse
#define HAZELCAST_PortableEntrySetResponse

#include "Response.h"
#include <vector>
#include <utility>

namespace hazelcast {
    namespace client {
        namespace serialization {
            class Data;
        }
        namespace multimap {
            class PortableEntrySetResponse : public impl::Response {
            public:
                PortableEntrySetResponse();

                const std::vector<std::pair< serialization::Data, serialization::Data> >& getEntrySet() const;

                int getFactoryId() const;

                int getClassId() const;

                void readPortable(serialization::PortableReader& reader);

            private:
                std::vector<std::pair<serialization::Data, serialization::Data > > entrySet;
            };
        }
    }
}


#endif //HAZELCAST_PortableEntrySetResponse
