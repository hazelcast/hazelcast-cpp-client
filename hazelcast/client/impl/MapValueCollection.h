//
// Created by sancar koyunlu on 6/18/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_MAP_VALUE_COLLECTION
#define HAZELCAST_MAP_VALUE_COLLECTION

#include "../serialization/Data.h"
#include <vector>

namespace hazelcast {
    namespace client {
        namespace impl {
            class MapValueCollection {
            public:
                MapValueCollection();

                int getTypeSerializerId() const;

                int getFactoryId() const;

                int getClassId() const;

                const std::vector<serialization::Data>& getValues() const;

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer << values.size();
                    for (std::vector < serialization::Data > ::const_iterator it = values.begin(); it != values.end();
                         ++it) {
                        writer << (*it);
                    }
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    int size;
                    reader >> size;
                    values.resize(size);
                    for (int i = 0; i < size; i++) {
                        reader >> values[i];
                    }
                };
            private:
                std::vector<serialization::Data> values;
            };
        }
    }
}


#endif //HAZELCAST_MAP_VALUE_COLLECTION
