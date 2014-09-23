//
// Created by sancar koyunlu on 5/29/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_SERIALIZABLE_CONNECTION
#define HAZELCAST_SERIALIZABLE_CONNECTION

#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/client/serialization/pimpl/SerializationConstants.h"
#include "hazelcast/client/serialization/IdentifiedDataSerializable.h"

namespace hazelcast {
    namespace client {
        namespace impl {

            class HAZELCAST_API SerializableCollection : public serialization::IdentifiedDataSerializable {
            public:
                SerializableCollection();

                ~SerializableCollection();

                const std::vector<serialization::pimpl::Data *>& getCollection() const;

                int getFactoryId() const;

                int getClassId() const;

                void writeData(serialization::ObjectDataOutput& writer) const;

                void readData(serialization::ObjectDataInput& reader);

            private:
                std::vector<serialization::pimpl::Data *> dataCollection;
            };
        }
    }
}

#endif //HAZELCAST_SERIALIZABLE_CONNECTION

