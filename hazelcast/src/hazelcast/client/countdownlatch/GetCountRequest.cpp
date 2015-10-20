//
// Created by sancar koyunlu on 8/12/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/countdownlatch/GetCountRequest.h"
#include "hazelcast/client/countdownlatch/CountDownLatchPortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"

namespace hazelcast {
    namespace client {
        namespace countdownlatch {
            GetCountRequest::GetCountRequest(const std::string& instanceName)
            : instanceName(instanceName) {

            }

            int GetCountRequest::getFactoryId() const {
                return CountDownLatchPortableHook::F_ID;
            }

            int GetCountRequest::getClassId() const {
                return CountDownLatchPortableHook::GET_COUNT;
            }


            bool GetCountRequest::isRetryable() const {
                return true;
            }

            void GetCountRequest::write(serialization::PortableWriter& writer) const {
                writer.writeUTF("name", &instanceName);
            }
        }
    }
}

