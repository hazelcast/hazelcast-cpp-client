//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_QUEUE_REMOVE_REQUEST
#define HAZELCAST_QUEUE_REMOVE_REQUEST

#include "../serialization/Data.h"
#include "PortableHook.h"

namespace hazelcast {
    namespace client {
        namespace queue {
            class RemoveRequest {
            public:

                RemoveRequest(const std::string& name, serialization::Data& data)
                :name(name)
                , data(data) {

                };

                int getTypeSerializerId() const {
                    return serialization::SerializationConstants::CONSTANT_TYPE_PORTABLE;
                };

                int getFactoryId() const {
                    return queue::PortableHook::F_ID;
                }

                int getClassId() const {
                    return queue::PortableHook::REMOVE;
                }

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer["n"] << name;
                    writer << data;
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    reader["n"] >> name;
                    reader >> data;
                };
            private:
                serialization::Data& data;
                std::string name;
            };
        }
    }
}

#endif //HAZELCAST_QUEUE_REMOVE_REQUEST
