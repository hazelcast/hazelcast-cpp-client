//
// Created by sancar koyunlu on 9/4/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "AddListenerRequest.h"
#include "QueuePortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "PortableReader.h"

namespace hazelcast {
    namespace client {
        namespace queue {
            AddListenerRequest::AddListenerRequest(const std::string& name, bool includeValue)
            :name(name)
            , includeValue(includeValue) {
            };

            int AddListenerRequest::getFactoryId() const {
                return queue::QueuePortableHook::F_ID;
            }

            int AddListenerRequest::getClassId() const {
                return queue::QueuePortableHook::ADD_LISTENER;
            };

            void AddListenerRequest::writePortable(serialization::PortableWriter& writer) const {
                writer.writeUTF("n", name);
                writer.writeBoolean("i", includeValue);
            };

            void AddListenerRequest::readPortable(serialization::PortableReader& reader) {
                name = reader.readUTF("n");
                includeValue = reader.readBoolean("i");
            };
        }
    }
}
