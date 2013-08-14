//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_QUEUE_ADD_LISTENER_REQUEST
#define HAZELCAST_QUEUE_ADD_LISTENER_REQUEST

#include "../serialization/SerializationConstants.h"
#include "QueuePortableHook.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace queue {
            class AddListenerRequest : public Portable{
            public:
                AddListenerRequest(const std::string& name, bool includeValue)
                :name(name)
                , includeValue(includeValue) {


                };

                int getFactoryId() const {
                    return queue::QueuePortableHook::F_ID;
                }

                int getClassId() const {
                    return queue::QueuePortableHook::ADD_LISTENER;
                };


                void writePortable(serialization::PortableWriter& writer) const {
                    writer.writeUTF("n", name);
                    writer.writeBoolean("i", includeValue);
                };


                void readPortable(serialization::PortableReader& reader) {
                    name = reader.readUTF("n");
                    includeValue = reader.readBoolean("i");
                };
            private:
                std::string name;
                bool includeValue;
            };
        }
    }
}

#endif //HAZELCAST_QUEUE_ADD_LISTENER_REQUEST
