//
// Created by sancar koyunlu on 6/18/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_MAP_KEY_SET
#define HAZELCAST_MAP_KEY_SET

#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/client/impl/IdentifiedDataSerializableResponse.h"
#include <vector>

namespace hazelcast {
    namespace client {
        namespace map {
            class HAZELCAST_API MapKeySet : public impl::IdentifiedDataSerializableResponse {
            public:
                int getFactoryId() const;

                int getClassId() const;

                const std::vector<serialization::pimpl::Data>& getKeySet() const;

                void readData(serialization::ObjectDataInput& reader);

            private:
                std::vector<serialization::pimpl::Data> keySet;

            };
        }
    }
}

#endif //__MapKeySet_H_

