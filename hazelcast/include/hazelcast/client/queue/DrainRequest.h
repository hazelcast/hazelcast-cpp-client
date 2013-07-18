//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_QUEUE_MAX_SIZE_REQUEST
#define HAZELCAST_QUEUE_MAX_SIZE_REQUEST

#include "../serialization/SerializationConstants.h"
#include "QueuePortableHook.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace queue {
            class DrainRequest {
            public:
                DrainRequest(const std::string& name, bool maxSize)
                :name(name)
                , maxSize(maxSize) {


                };

                int getSerializerId() const {
                    return serialization::SerializationConstants::CONSTANT_TYPE_PORTABLE;
                };

                int getFactoryId() const {
                    return queue::QueuePortableHook::F_ID;
                }

                int getClassId() const {
                    return queue::QueuePortableHook::DRAIN;
                };

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer.writeUTF("n", name);
                    writer.writeInt("m", maxSize);
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    name = reader.readUTF("n");
                    maxSize = reader.readInt("m");
                };
            private:
                std::string name;
                int maxSize;
            };
        }
    }
}

#endif //HAZELCAST_QUEUE_ADD_LISTENER_REQUEST
