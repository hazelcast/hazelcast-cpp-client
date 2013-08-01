//
// Created by sancar koyunlu on 6/18/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_MAP_ENTRY_SET
#define HAZELCAST_MAP_ENTRY_SET

#include "Data.h"
#include "DataSerializable.h"
#include <vector>


namespace hazelcast {
    namespace client {
        namespace map {
            class MapEntrySet : public DataSerializable {
            public:
                MapEntrySet();

                int getFactoryId() const;

                int getClassId() const;

                const std::vector<std::pair<serialization::Data, serialization::Data > >& getEntrySet() const;

                std::vector<std::pair<serialization::Data, serialization::Data > >& getEntrySet();

                void writeData(serialization::ObjectDataOutput& writer);

                void readData(serialization::ObjectDataInput& reader);

            private:
                std::vector<std::pair<serialization::Data, serialization::Data > > entrySet;
            };
        }
    }
}

#endif //HAZELCAST_MAP_ENTRY_SET
