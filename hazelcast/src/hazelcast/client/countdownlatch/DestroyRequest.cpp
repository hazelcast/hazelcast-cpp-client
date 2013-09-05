//
// Created by sancar koyunlu on 9/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/countdownlatch/DestroyRequest.h"
#include "CountDownLatchPortableHook.h"
#include "PortableWriter.h"
#include "PortableReader.h"

namespace hazelcast {
    namespace client {
        namespace countdownlatch {
            DestroyRequest::DestroyRequest(const std::string& instanceName)
            : instanceName(instanceName) {

            };

            int DestroyRequest::getFactoryId() const {
                return CountDownLatchPortableHook::F_ID;
            };

            int DestroyRequest::getClassId() const {
                return CountDownLatchPortableHook::DESTROY;
            };

            void DestroyRequest::writePortable(serialization::PortableWriter& writer) const {
                writer.writeUTF("name", instanceName);
            };

            void DestroyRequest::readPortable(serialization::PortableReader& reader) {
                instanceName = reader.readUTF("name");
            };
        }
    }
}
