//
// Created by sancar koyunlu on 9/4/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/queue/DrainRequest.h"
#include "hazelcast/client/queue/QueuePortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"

namespace hazelcast {
    namespace client {
        namespace queue {
            DrainRequest::DrainRequest(const std::string& name, int maxSize)
            :name(name)
            , maxSize(maxSize) {

            };

            int DrainRequest::getFactoryId() const {
                return queue::QueuePortableHook::F_ID;
            }

            int DrainRequest::getClassId() const {
                return queue::QueuePortableHook::DRAIN;
            };


            void DrainRequest::writePortable(serialization::PortableWriter& writer) const {
                writer.writeUTF("n", name);
                writer.writeLong("t", 0);
                writer.writeInt("m", maxSize);
            };
        }
    }
}

