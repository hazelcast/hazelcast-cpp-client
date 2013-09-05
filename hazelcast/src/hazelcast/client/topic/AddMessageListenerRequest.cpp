//
// Created by sancar koyunlu on 9/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "AddMessageListenerRequest.h"
#include "Data.h"
#include "TopicPortableHook.h"
#include "PortableWriter.h"
#include "PortableReader.h"

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

            void AddMessageListenerRequest::readPortable(serialization::PortableReader& reader) {
                instanceName = reader.readUTF("n");
            };
        }
    }
}


