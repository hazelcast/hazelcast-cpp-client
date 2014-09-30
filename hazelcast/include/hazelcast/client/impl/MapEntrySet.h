//
// Created by sancar koyunlu on 6/18/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_MAP_ENTRY_SET
#define HAZELCAST_MAP_ENTRY_SET

#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/client/serialization/IdentifiedDataSerializable.h"
#include <vector>


namespace hazelcast {
    namespace client {
        namespace map {
            class HAZELCAST_API MapEntrySet : public serialization::IdentifiedDataSerializable {
            public:
                MapEntrySet();

                MapEntrySet(const std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data > >& entrySet);

                int getFactoryId() const;

                int getClassId() const;

                std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data > > &getEntrySet();

                void writeData(serialization::ObjectDataOutput &writer) const;

                void readData(serialization::ObjectDataInput &reader);

            private:
                std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data > > entrySet;
            };
        }
    }
}

#endif //HAZELCAST_MAP_ENTRY_SET

