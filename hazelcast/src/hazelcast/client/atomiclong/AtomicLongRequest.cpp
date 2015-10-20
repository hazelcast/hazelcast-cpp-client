//
// Created by sancar koyunlu on 8/12/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/atomiclong/AtomicLongRequest.h"
#include "hazelcast/client/atomiclong/AtomicLongPortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"

namespace hazelcast {
    namespace client {
        namespace atomiclong {
            AtomicLongRequest::AtomicLongRequest(const std::string& instanceName, long delta)
            : delta(delta)
            ,instanceName(instanceName){

            }

            int AtomicLongRequest::getFactoryId() const {
                return AtomicLongPortableHook::F_ID;
            }

            void AtomicLongRequest::write(serialization::PortableWriter& writer) const {
                writer.writeUTF("n", &instanceName);
                writer.writeLong("d", delta);
            }

        }
    }
}
