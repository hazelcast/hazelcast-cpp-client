//
// Created by sancar koyunlu on 6/18/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_MAP_ENTRY_SET
#define HAZELCAST_MAP_ENTRY_SET

#include "../serialization/Data.h"
#include <vector>


namespace hazelcast {
    namespace client {
        namespace impl {
            class MapEntrySet {
            public:
                MapEntrySet();

                int getTypeSerializerId() const;

                int getFactoryId() const;

                int getClassId() const;

                const std::vector<std::pair<serialization::Data, serialization::Data > >& getEntrySet() const;

                std::vector<std::pair<serialization::Data, serialization::Data > >& getEntrySet();

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer << entrySet.size();
                    for (std::vector<std::pair<serialization::Data, serialization::Data > >::const_iterator it = entrySet.begin(); it != entrySet.end();
                         ++it) {
                        writer << it->first;
                        writer << it->second;
                    }
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    int size;
                    reader >> size;
                    entrySet.resize(size);
                    for (int i = 0; i < size; i++) {
                        reader >> entrySet[i].first;
                        reader >> entrySet[i].second;
                    }
                };
            private:
                std::vector<std::pair<serialization::Data, serialization::Data > > entrySet;
            };
        }
    }
}

#endif //HAZELCAST_MAP_ENTRY_SET
