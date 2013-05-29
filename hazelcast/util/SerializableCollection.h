//
// Created by sancar koyunlu on 5/29/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_SERIALIZABLE_CONNECTION
#define HAZELCAST_SERIALIZABLE_CONNECTION

#include "../client/serialization/Data.h"

namespace hazelcast {
    namespace util {
        class SerializableCollection {
        public:
            SerializableCollection();

            std::vector<hazelcast::client::serialization::Data *> getCollection() const;

        private:
            std::vector<hazelcast::client::serialization::Data *> datas;
        };
    }
}

#endif //HAZELCAST_SERIALIZABLE_CONNECTION
