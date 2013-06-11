//
// Created by sancar koyunlu on 6/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#ifndef HAZELCAST_MAP_SIZE_REQUEST
#define HAZELCAST_MAP_SIZE_REQUEST

#include "RequestIDs.h"
#include "../serialization/SerializationConstants.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace map {
            class SizeRequest {
                SizeRequest(const std::string& name)
                :name(name) {

                };

                int getTypeSerializerId() const {
                    return serialization::SerializationConstants::CONSTANT_TYPE_PORTABLE;
                };

                int getFactoryId() const {
                    return map::RequestIDs::F_ID;
                }

                int getClassId() const {
                    return map::RequestIDs::SIZE;
                }

                template<typename HzWriter>
                inline void writePortable(HzWriter& writer) const {
                    writer["n"] << name;
                };

                template<typename HzReader>
                inline void readPortable(HzReader& reader) {
                    reader["n"] >> name;
                };
            private:
                std::string name;
            };
        }
    }
}


#endif //MAP_CLEAR_REQUEST
