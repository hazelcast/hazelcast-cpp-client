//
// Created by sancar koyunlu on 6/18/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_MAP_ENTRY_SET
#define HAZELCAST_MAP_ENTRY_SET

#include "hazelcast/client/serialization/Data.h"
#include "hazelcast/client/IdentifiedDataSerializable.h"
#include <vector>


namespace hazelcast {
    namespace client {
        namespace map {
            class MapEntrySet : public IdentifiedDataSerializable {
            public:
                MapEntrySet();

                int getFactoryId() const;

                int getClassId() const;

                const std::vector<std::pair<serialization::Data, serialization::Data > >& getEntrySet() const;

                std::vector<std::pair<serialization::Data, serialization::Data > >& getEntrySet();

                void writeData(serialization::ObjectDataOutput& writer) const;

                void readData(serialization::ObjectDataInput& reader);

            private:
                std::vector<std::pair<serialization::Data, serialization::Data > > entrySet;
            };
        }
    }
}

#endif //HAZELCAST_MAP_ENTRY_SET
