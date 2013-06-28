//
// Created by sancar koyunlu on 6/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_MAP_EVICT_REQUEST
#define HAZELCAST_MAP_EVICT_REQUEST

#include "PortableHook.h"
#include "../serialization/SerializationConstants.h"
#include "../serialization/Data.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace map {
            class EvictRequest {
            public:
                EvictRequest(const std::string& name, serialization::Data& key, int threadId)
                :name(name)
                , key(key)
                , threadId(threadId) {

                };

                int getTypeSerializerId() const {
                    return serialization::SerializationConstants::CONSTANT_TYPE_PORTABLE;
                };

                int getFactoryId() {
                    return PortableHook::F_ID;
                };

                int getClassId() {
                    return PortableHook::EVICT;
                };

                template<typename HzWriter>
                void writePortable(HzWriter& writer) {
                    writer["n"] << name;
                    writer["t"] << threadId;
                    writer << key;
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    reader["n"] >> name;
                    reader["t"] >> threadId;
                    reader >> key;
                };
            private:
                serialization::Data& key;
                std::string name;
                int threadId;
            };
        }
    }
}

#endif //HAZELCAST_MAP_EVICT_REQUEST
