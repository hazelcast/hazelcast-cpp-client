//
// Created by sancar koyunlu on 8/12/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/countdownlatch/CountDownRequest.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/countdownlatch/CountDownLatchPortableHook.h"

namespace hazelcast {
    namespace client {
        namespace countdownlatch {

            CountDownRequest::CountDownRequest(const std::string& instanceName)
            : instanceName(instanceName) {

            }

            int CountDownRequest::getFactoryId() const {
                return CountDownLatchPortableHook::F_ID;
            }

            int CountDownRequest::getClassId() const {
                return CountDownLatchPortableHook::COUNT_DOWN;
            }

            void CountDownRequest::write(serialization::PortableWriter & writer) const {
                writer.writeUTF("name", &instanceName);
            }

        }
    }
}
