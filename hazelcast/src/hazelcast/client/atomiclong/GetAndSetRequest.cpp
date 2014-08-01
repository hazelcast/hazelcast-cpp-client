//
// Created by sancar koyunlu on 19/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/atomiclong/GetAndSetRequest.h"
#include "hazelcast/client/atomiclong/AtomicLongPortableHook.h"

namespace hazelcast {
    namespace client {
        namespace atomiclong {

            GetAndSetRequest::GetAndSetRequest(const std::string &instanceName, long value)
            : AtomicLongRequest(instanceName, value) {

            }

            int GetAndSetRequest::getClassId() const {
                return AtomicLongPortableHook::GET_AND_SET;
            }
        }
    }
}

