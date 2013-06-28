//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_QUEUE_ITERATOR_REQUEST
#define HAZELCAST_QUEUE_ITERATOR_REQUEST

#include "../serialization/SerializationConstants.h"
#include "QueuePortableHook.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace queue {
            class IteratorRequest {
            public:
                IteratorRequest(const std::string& name)
                :name(name) {

                };

                int getTypeSerializerId() const {
                    return serialization::SerializationConstants::CONSTANT_TYPE_PORTABLE;
                };

                int getFactoryId() const {
                    return queue::QueuePortableHook::F_ID;
                }

                int getClassId() const {
                    return queue::QueuePortableHook::ITERATOR;
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

#endif //HAZELCAST_QUEUE_CLEAR_REQUEST
