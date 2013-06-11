//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_PEEK_REQUEST
#define HAZELCAST_PEEK_REQUEST

#include "../serialization/SerializationConstants.h"
#include "RequestIDs.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace queue {
            class PeekRequest {
            public:
                PeekRequest(const std::string& name)
                :name(name) {

                };

                int getTypeSerializerId() const {
                    return serialization::SerializationConstants::CONSTANT_TYPE_PORTABLE;
                };

                int getFactoryId() const {
                    return queue::RequestIDs::F_ID;
                }

                int getClassId() const {
                    return queue::RequestIDs::PEEK;
                };

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer["n"] << name;
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    reader["n"] >> name;
                };
            private:
                std::string name;
            };
        }
    }
}

#endif //HAZELCAST_MAP_PUT_REQUEST
