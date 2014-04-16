//
// Created by sancar koyunlu on 9/4/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/queue/SizeRequest.h"
#include "hazelcast/client/queue/QueuePortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"

namespace hazelcast {
    namespace client {
        namespace queue {
            SizeRequest::SizeRequest(const std::string &name)
            :name(name) {

            };

            int SizeRequest::getFactoryId() const {
                return queue::QueuePortableHook::F_ID;
            }

            int SizeRequest::getClassId() const {
                return queue::QueuePortableHook::SIZE;
            };

            void SizeRequest::write(serialization::PortableWriter &writer) const {
                writer.writeUTF("n", name);
                writer.writeLong("t", 0);
            };


            bool SizeRequest::isRetryable() const {
                return true;
            }
        }
    }
}

