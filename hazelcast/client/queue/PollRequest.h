//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_POLL_REQUEST
#define HAZELCAST_POLL_REQUEST

#include "../serialization/SerializationConstants.h"
#include "RequestIDs.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace queue {
            class PollRequest {
            public:
                PollRequest(const std::string& name, long timeout)
                :name(name)
                , timeoutInMillis(timeout) {

                };

                PollRequest(const std::string& name)
                :name(name)
                , timeoutInMillis(0) {

                };

                int getTypeSerializerId() const {
                    return serialization::SerializationConstants::CONSTANT_TYPE_PORTABLE;
                };

                int getFactoryId() const {
                    return queue::RequestIDs::F_ID;
                }

                int getClassId() const {
                    return queue::RequestIDs::POLL;
                };

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer["n"] << name;
                    writer["t"] << timeoutInMillis;
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    reader["n"] >> name;
                    reader["t"] >> timeoutInMillis;
                };
            private:
                std::string name;
                long timeoutInMillis;
            };
        }
    }
}

#endif //HAZELCAST_POLL_REQUEST
