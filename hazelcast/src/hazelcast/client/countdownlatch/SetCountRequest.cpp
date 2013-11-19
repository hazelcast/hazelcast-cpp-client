//
// Created by sancar koyunlu on 9/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/countdownlatch/SetCountRequest.h"
#include "hazelcast/client/countdownlatch/CountDownLatchPortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"

namespace hazelcast {
    namespace client {
        namespace countdownlatch {
            SetCountRequest::SetCountRequest(const std::string& instanceName, int count)
            : instanceName(instanceName)
            , count(count) {

            };

            int SetCountRequest::getFactoryId() const {
                return CountDownLatchPortableHook::F_ID;
            };

            int SetCountRequest::getClassId() const {
                return CountDownLatchPortableHook::SET_COUNT;
            };

            void SetCountRequest::writePortable(serialization::PortableWriter& writer) const {
                writer.writeUTF("name", instanceName);
                writer.writeInt("count", count);
            };
        }
    }
}
