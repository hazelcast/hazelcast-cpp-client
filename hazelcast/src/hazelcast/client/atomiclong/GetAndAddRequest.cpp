//
// Created by sancar koyunlu on 19/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/atomiclong/GetAndAddRequest.h"
#include "hazelcast/client/atomiclong/AtomicLongPortableHook.h"

namespace hazelcast {
    namespace client {
        namespace atomiclong {

            GetAndAddRequest::GetAndAddRequest(const std::string &instanceName, long delta)
            : AtomicLongRequest(instanceName, delta) {

            };

            int GetAndAddRequest::getClassId() const {
                return AtomicLongPortableHook::GET_AND_ADD;
            };


            void GetAndAddRequest::write(serialization::PortableWriter &writer) const {
                AtomicLongRequest::writePortable(writer);
            };
        }
    }
}
