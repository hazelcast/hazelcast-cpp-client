//
// Created by sancar koyunlu on 9/4/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/queue/IteratorRequest.h"
#include "hazelcast/client/queue/QueuePortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"


namespace hazelcast {
    namespace client {
        namespace queue {
            IteratorRequest::IteratorRequest(const std::string& name)
            :name(name) {

            }

            int IteratorRequest::getFactoryId() const {
                return queue::QueuePortableHook::F_ID;
            }

            int IteratorRequest::getClassId() const {
                return queue::QueuePortableHook::ITERATOR;
            }


            void IteratorRequest::write(serialization::PortableWriter& writer) const {
                writer.writeUTF("n", &name);
                writer.writeLong("t", 0);
            }

        }
    }
}


