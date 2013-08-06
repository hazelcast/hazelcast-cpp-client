//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_QUEUE_REMAINING_CAPACITY_REQUEST
#define HAZELCAST_QUEUE_REMAINING_CAPACITY_REQUEST

#include "../serialization/SerializationConstants.h"
#include "QueuePortableHook.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace queue {
            class RemainingCapacityRequest : public Portable{
            public:
                RemainingCapacityRequest(const std::string& name)
                :name(name) {

                };

                int getFactoryId() const {
                    return QueuePortableHook::F_ID;
                }

                int getClassId() const {
                    return QueuePortableHook::REMAINING_CAPACITY;
                };

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer.writeUTF("n", name);
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    name = reader.readUTF("n");
                };
            private:
                std::string name;
            };
        }
    }
}

#endif //HAZELCAST_REMAINING_CAPACITY_REQUEST
