//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_AddEntryListenerRequest
#define HAZELCAST_AddEntryListenerRequest

#include "Data.h"
#include "PortableRequest.h"
#include <vector>

namespace hazelcast {
    namespace client {
        namespace serialization {
            class Data;
        }
        namespace multimap {
            class AddEntryListenerRequest : public impl::PortableRequest {
            public:
                AddEntryListenerRequest(const std::string &name, const serialization::Data &key, bool includeValue);

                AddEntryListenerRequest(const std::string &name, bool includeValue);

                int getFactoryId() const;

                int getClassId() const;

                void writePortable(serialization::PortableWriter &writer) const;

            private:
                std::string name;
                serialization::Data key;
                bool hasKey;
                bool includeValue;
            };

        }
    }
}


#endif //HAZELCAST_AddEntryListenerRequest
