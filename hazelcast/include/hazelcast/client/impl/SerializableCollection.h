//
// Created by sancar koyunlu on 5/29/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_SERIALIZABLE_CONNECTION
#define HAZELCAST_SERIALIZABLE_CONNECTION

#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/client/serialization/pimpl/SerializationConstants.h"
#include "hazelcast/client/impl/IdentifiedDataSerializableResponse.h"

namespace hazelcast {
    namespace client {
        namespace impl {

            class SerializableCollection : public IdentifiedDataSerializableResponse {
            public:
                SerializableCollection();

                const std::vector<serialization::pimpl::Data>& getCollection() const;

                int getFactoryId() const;

                int getClassId() const;

                void readData(serialization::ObjectDataInput& reader);

            private:
                std::vector<serialization::pimpl::Data> dataCollection;
            };
        }
    }
}

#endif //HAZELCAST_SERIALIZABLE_CONNECTION

