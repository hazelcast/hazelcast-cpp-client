//
// Created by sancar koyunlu on 19/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/atomiclong/CompareAndSetRequest.h"
#include "hazelcast/client/atomiclong/AtomicLongPortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"

namespace hazelcast {
    namespace client {
        namespace atomiclong {

            CompareAndSetRequest::CompareAndSetRequest(const std::string &instanceName, long expect, long value)
            : AtomicLongRequest(instanceName, value)
            , expect(expect) {

            };

            int CompareAndSetRequest::getClassId() const {
                return AtomicLongPortableHook::COMPARE_AND_SET;
            };

            void CompareAndSetRequest::write(serialization::PortableWriter &writer) const {
                AtomicLongRequest::write(writer);
                writer.writeLong("e", expect);
            };
        }
    }
}
