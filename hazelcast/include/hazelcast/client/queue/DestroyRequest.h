//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_QUEUE_DESTROY_REQUEST
#define HAZELCAST_QUEUE_DESTROY_REQUEST

#include "../serialization/SerializationConstants.h"
#include "QueuePortableHook.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace queue {
            class DestroyRequest : public Portable{
            public:
                DestroyRequest(const std::string& name)
                :name(name) {

                };

                int getFactoryId() const {
                    return queue::QueuePortableHook::F_ID;
                }

                int getClassId() const {
                    return queue::QueuePortableHook::DESTROY;
                };


                void writePortable(serialization::PortableWriter& writer) const {
                    writer.writeUTF("n", name);
                };


                void readPortable(serialization::PortableReader& reader) {
                    name = reader.readUTF("n");
                };
            private:
                std::string name;
            };
        }
    }
}

#endif //HAZELCAST_QUEUE_DESTROY_REQUEST
