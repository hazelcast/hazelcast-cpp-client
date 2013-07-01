//
// Created by sancar koyunlu on 6/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#ifndef HAZELCAST_MAP_IS_LOCKED_REQUEST
#define HAZELCASTMAP_IS_LOCKED_REQUEST

#include "PortableHook.h"
#include "../serialization/SerializationConstants.h"
#include "../serialization/Data.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace map {
            class IsLockedRequest {
            public:
                IsLockedRequest(const std::string& name, serialization::Data& key)
                :key(key),
                name(name) {

                };

                int getSerializerId() const {
                    return serialization::SerializationConstants::CONSTANT_TYPE_PORTABLE;
                };

                int getFactoryId() const {
                    return PortableHook::F_ID;
                }

                int getClassId() const {
                    return PortableHook::IS_LOCKED;
                }

                template<typename HzWriter>
                inline void writePortable(HzWriter& writer) const {
                    writer["name"] << name;
                    writer << key;
                };

                template<typename HzReader>
                inline void readPortable(HzReader& reader) {
                    reader["name"] >> name;
                    reader >> key;
                };
            private:
                std::string name;
                serialization::Data& key;
            };
        }
    }
}


#endif //MAP_CLEAR_REQUEST
