//
// Created by sancar koyunlu on 9/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/topic/DestroyRequest.h"
#include "TopicPortableHook.h"
#include "PortableWriter.h"
#include "PortableReader.h"

namespace hazelcast {
    namespace client {
        namespace topic {
            DestroyRequest::DestroyRequest(const std::string& instanceName)
            : instanceName(instanceName) {

            };

            int DestroyRequest::getFactoryId() const {
                return TopicPortableHook::F_ID;
            };

            int DestroyRequest::getClassId() const {
                return TopicPortableHook::DESTROY;
            };

            void DestroyRequest::writePortable(serialization::PortableWriter& writer) const {
                writer.writeUTF("n", instanceName);
            };

            void DestroyRequest::readPortable(serialization::PortableReader& reader) {
                instanceName = reader.readUTF("n");
            };
        }
    }
}
