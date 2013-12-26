//
// Created by sancar koyunlu on 19/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/atomiclong/AddAndGetRequest.h"
#include "hazelcast/client/atomiclong/AtomicLongPortableHook.h"

namespace hazelcast {
    namespace client {
        namespace atomiclong {

            AddAndGetRequest::AddAndGetRequest(const std::string &instanceName, long delta)
            : AtomicLongRequest(instanceName, delta) {

            };

            int AddAndGetRequest::getClassId() const {
                return AtomicLongPortableHook::ADD_AND_GET;
            };


            void AddAndGetRequest::write(serialization::PortableWriter &writer) const {
                AtomicLongRequest::writePortable(writer);
            };

        };
    }
}


