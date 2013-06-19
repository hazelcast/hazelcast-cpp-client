//
// Created by sancar koyunlu on 6/18/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_MAP_KEY_SET
#define HAZELCAST_MAP_KEY_SET

#include <vector>
#include "../serialization/Data.h"

namespace hazelcast {
    namespace client {
        namespace map {
            class MapKeySet {
            public:
                MapKeySet();

                int getTypeSerializerId() const;

                int getFactoryId() const;

                int getClassId() const;

                const std::vector<serialization::Data>& getKeySet() const;

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer << keySet.size();
                    for (std::vector < serialization::Data > ::const_iterator it = keySet.begin(); it != keySet.end();
                         ++it) {
                        writer << (*it);
                    }
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    int size;
                    reader >> size;
                    keySet.resize(size);
                    for (int i = 0; i < size; i++) {
                        reader >> keySet[i];
                    }
                };
            private:
                std::vector<serialization::Data> keySet;

            };
        }
    }
}

#endif //__MapKeySet_H_
