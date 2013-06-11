//
// Created by sancar koyunlu on 6/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#ifndef HAZELCAST_MAP_DESTROY_REQUEST
#define HAZELCAST_MAP_DESTROY_REQUEST

#include "RequestIDs.h"
#include "../serialization/SerializationConstants.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace map {
            class DestroyRequest {
                DestroyRequest(const std::string& name)
                :name(name) {

                };

                int getTypeSerializerId() const {
                    return serialization::SerializationConstants::CONSTANT_TYPE_PORTABLE;
                };

                int getFactoryId() const {
                    return map::RequestIDs::F_ID;
                }

                int getClassId() const {
                    return map::RequestIDs::DESTROY;
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


#endif //MAP_CLEAR_REQUEST
