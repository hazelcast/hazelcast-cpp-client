//
// Created by sancar koyunlu on 6/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#ifndef MAP_VALUES_REQUEST
#define MAP_VALUES_REQUEST

#include "RequestIDs.h"
#include "../serialization/SerializationConstants.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace map {
            class ValuesRequest {
            public:
                ValuesRequest(const std::string& name)
                :name(name) {

                };

                int getTypeSerializerId() const {
                    return serialization::SerializationConstants::CONSTANT_TYPE_PORTABLE;
                };

                int getFactoryId() const {
                    return map::RequestIDs::F_ID;
                }

                int getClassId() const {
                    return map::RequestIDs::VALUES;
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


#endif //MAP_KEY_SET_REQUEST
