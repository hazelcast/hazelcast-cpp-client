//
// Created by sancar koyunlu on 19/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/atomiclong/SetRequest.h"
#include "hazelcast/client/atomiclong/AtomicLongPortableHook.h"

namespace hazelcast {
    namespace client {
        namespace atomiclong {

            SetRequest::SetRequest(const std::string &instanceName, long value)
            : AtomicLongRequest(instanceName, value) {

            };

            int SetRequest::getClassId() const {
                return AtomicLongPortableHook::SET;
            };

            void SetRequest::writePortable(serialization::PortableWriter &writer) const {
                AtomicLongRequest::writePortable(writer);
            };

        }
    }
}

