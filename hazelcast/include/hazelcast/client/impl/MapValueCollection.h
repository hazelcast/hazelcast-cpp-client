//
// Created by sancar koyunlu on 6/18/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_MAP_VALUE_COLLECTION
#define HAZELCAST_MAP_VALUE_COLLECTION

#include "hazelcast/client/serialization/Data.h"
#include "hazelcast/client/impl/IdentifiedDataSerializableResponse.h"
#include <vector>

namespace hazelcast {
    namespace client {
        namespace map {
            class HAZELCAST_API MapValueCollection : public impl::IdentifiedDataSerializableResponse {
            public:
                MapValueCollection();

                int getFactoryId() const;

                int getClassId() const;

                void readData(serialization::ObjectDataInput &reader);

                const std::vector<serialization::Data> &getValues() const;


            private:
                std::vector<serialization::Data> values;
            };
        }
    }
}


#endif //HAZELCAST_MAP_VALUE_COLLECTION
