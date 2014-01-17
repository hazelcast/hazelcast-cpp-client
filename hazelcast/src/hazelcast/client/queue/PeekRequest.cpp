//
// Created by sancar koyunlu on 9/4/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/queue/PeekRequest.h"
#include "hazelcast/client/queue/QueuePortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"

namespace hazelcast {
    namespace client {
        namespace queue {
            PeekRequest::PeekRequest(const std::string &name)
            :name(name) {

            };

            int PeekRequest::getFactoryId() const {
                return queue::QueuePortableHook::F_ID;
            }

            int PeekRequest::getClassId() const {
                return queue::QueuePortableHook::PEEK;
            };

            void PeekRequest::write(serialization::PortableWriter &writer) const {
                writer.writeUTF("n", name);
            };


            bool PeekRequest::isRetryable() const {
                return true;
            }
        }
    }
}
