//
// Created by sancar koyunlu on 8/12/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/countdownlatch/AwaitRequest.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/countdownlatch/CountDownLatchPortableHook.h"

namespace hazelcast {
    namespace client {
        namespace countdownlatch {

            AwaitRequest::AwaitRequest(const std::string& instanceName, long timeout)
            : instanceName(instanceName)
            , timeout(timeout) {

            };

            int AwaitRequest::getFactoryId() const {
                return CountDownLatchPortableHook::F_ID;
            };

            int AwaitRequest::getClassId() const {
                return CountDownLatchPortableHook::AWAIT;
            };

            void AwaitRequest::write(serialization::PortableWriter& writer) const {
                writer.writeUTF("name", instanceName);
                writer.writeLong("timeout", timeout);
            };

        }
    }
}