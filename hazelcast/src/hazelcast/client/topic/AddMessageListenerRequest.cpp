//
// Created by sancar koyunlu on 9/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/topic/AddMessageListenerRequest.h"
#include "hazelcast/client/serialization/Data.h"
#include "hazelcast/client/topic/TopicPortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"

namespace hazelcast {
    namespace client {
        namespace topic {
            AddMessageListenerRequest::AddMessageListenerRequest(const std::string& instanceName)
            : instanceName(instanceName) {

            };

            int AddMessageListenerRequest::getFactoryId() const {
                return TopicPortableHook::F_ID;
            };

            int AddMessageListenerRequest::getClassId() const {
                return TopicPortableHook::ADD_LISTENER;
            };

            void AddMessageListenerRequest::writePortable(serialization::PortableWriter& writer) const {
                writer.writeUTF("n", instanceName);
            };
        }
    }
}


