//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_QUEUE_ADD_LISTENER_REQUEST
#define HAZELCAST_QUEUE_ADD_LISTENER_REQUEST

#include "../serialization/SerializationConstants.h"
#include "RequestIDs.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace queue {
            class AddListenerRequest {
            public:
                AddListenerRequest(const std::string& name, bool includeValue)
                :name(name)
                , includeValue(includeValue) {


                };

                int getTypeSerializerId() const {
                    return serialization::SerializationConstants::CONSTANT_TYPE_PORTABLE;
                };

                int getFactoryId() const {
                    return queue::RequestIDs::F_ID;
                }

                int getClassId() const {
                    return queue::RequestIDs::ADD_LISTENER;
                };

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer["n"] << name;
                    writer["i"] << includeValue;
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    reader["n"] >> name;
                    reader["i"] >> includeValue;
                };
            private:
                std::string name;
                bool includeValue;
            };
        }
    }
}

#endif //HAZELCAST_QUEUE_ADD_LISTENER_REQUEST
