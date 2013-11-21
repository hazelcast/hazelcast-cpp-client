//
// Created by sancar koyunlu on 9/4/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/queue/RemainingCapacityRequest.h"
#include "hazelcast/client/queue/QueuePortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"


namespace hazelcast {
    namespace client {
        namespace queue {
            RemainingCapacityRequest::RemainingCapacityRequest(const std::string& name)
            :name(name) {

            };

            int RemainingCapacityRequest::getFactoryId() const {
                return QueuePortableHook::F_ID;
            }

            int RemainingCapacityRequest::getClassId() const {
                return QueuePortableHook::REMAINING_CAPACITY;
            };


            void RemainingCapacityRequest::writePortable(serialization::PortableWriter& writer) const {
                writer.writeUTF("n", name);
            };
        }
    }
}


