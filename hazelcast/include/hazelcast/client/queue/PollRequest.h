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
            class PollRequest : public Portable{
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

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer.writeUTF("n", name);
                    writer.writeLong("t", timeoutInMillis);
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    name = reader.readUTF("n");
                    timeoutInMillis = reader.readUTF("t");
                };
            private:
                std::string name;
                long timeoutInMillis;
            };
        }
    }
}

#endif //HAZELCAST_POLL_REQUEST
