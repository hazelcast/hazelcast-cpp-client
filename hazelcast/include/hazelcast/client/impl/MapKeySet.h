//
// Created by sancar koyunlu on 6/18/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_MAP_KEY_SET
#define HAZELCAST_MAP_KEY_SET

#include "../serialization/Data.h"
#include "DataSerializable.h"
#include <vector>

namespace hazelcast {
    namespace client {
        namespace map {
            class MapKeySet : public DataSerializable {
            public:
                MapKeySet();

                int getFactoryId() const;

                int getClassId() const;

                const std::vector<serialization::Data>& getKeySet() const;

                void writeData(serialization::ObjectDataOutput& writer);

                void readData(serialization::ObjectDataInput& reader);

            private:
                std::vector<serialization::Data> keySet;

            };
        }
    }
}

#endif //__MapKeySet_H_
