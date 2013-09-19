//
// Created by sancar koyunlu on 8/12/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "AwaitRequest.h"

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

            void AwaitRequest::writePortable(serialization::PortableWriter& writer) const {
                writer.writeUTF("name", instanceName);
                writer.writeLong("timeout", timeout);
            };

            void AwaitRequest::readPortable(serialization::PortableReader& reader) {
                instanceName = reader.readUTF("name");
                timeout = reader.readLong("timeout");
            };
        }
    }
}