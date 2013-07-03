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
            class MapKeySet : public DataSerializable {
            public:
                MapKeySet();

                int getFactoryId() const;

                int getClassId() const;

                const std::vector<serialization::Data>& getKeySet() const;

                void writeData(serialization::BufferedDataOutput& writer);

                void readData(serialization::BufferedDataInput& reader);

            private:
                std::vector<serialization::Data> keySet;

            };
        }
    }
}

#endif //__MapKeySet_H_
