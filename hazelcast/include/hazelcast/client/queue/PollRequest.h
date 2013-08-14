//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_QUEUE_POLL_REQUEST
#define HAZELCAST_QUEUE_POLL_REQUEST

#include "../serialization/SerializationConstants.h"
#include "QueuePortableHook.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace queue {
            class PollRequest : public Portable {
            public:
                PollRequest(const std::string& name, long timeout)
                :name(name)
                , timeoutInMillis(timeout) {

                };

                PollRequest(const std::string& name)
                :name(name)
                , timeoutInMillis(0) {

                };

                int getFactoryId() const {
                    return queue::QueuePortableHook::F_ID;
                }

                int getClassId() const {
                    return queue::QueuePortableHook::POLL;
                };


                void writePortable(serialization::PortableWriter& writer) const {
                    writer.writeUTF("n", name);
                    writer.writeLong("t", timeoutInMillis);
                };


                void readPortable(serialization::PortableReader& reader) {
                    name = reader.readUTF("n");
                    timeoutInMillis = reader.readLong("t");
                };
            private:
                std::string name;
                long timeoutInMillis;
            };
        }
    }
}

#endif //HAZELCAST_POLL_REQUEST
