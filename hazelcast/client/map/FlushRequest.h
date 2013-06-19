//
// Created by sancar koyunlu on 6/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#ifndef HAZELCAST_MAP_FLUSH_REQUEST
#define HAZELCAST_MAP_FLUSH_REQUEST

#include "PortableHook.h"
#include "../serialization/SerializationConstants.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace map {
            class FlushRequest {
            public:
                FlushRequest(const std::string& name)
                :name(name) {

                };

                int getTypeSerializerId() const {
                    return serialization::SerializationConstants::CONSTANT_TYPE_PORTABLE;
                };

                int getFactoryId() const {
                    return PortableHook::F_ID;
                }

                int getClassId() const {
                    return PortableHook::FLUSH;
                }

                template<typename HzWriter>
                inline void writePortable(HzWriter& writer) const {
                    writer["name"] << name;
                };

                template<typename HzReader>
                inline void readPortable(HzReader& reader) {
                    reader["name"] >> name;
                };
            private:
                std::string name;
            };
        }
    }
}


#endif //MAP_FLUSH_REQUEST
