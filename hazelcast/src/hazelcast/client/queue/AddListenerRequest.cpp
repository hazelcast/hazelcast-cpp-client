//
// Created by sancar koyunlu on 9/4/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/queue/AddListenerRequest.h"
#include "hazelcast/client/queue/QueuePortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"


namespace hazelcast {
    namespace client {
        namespace queue {
            AddListenerRequest::AddListenerRequest(const std::string& name, bool includeValue)
            :name(name)
            , includeValue(includeValue) {
            }

            int AddListenerRequest::getFactoryId() const {
                return queue::QueuePortableHook::F_ID;
            }

            int AddListenerRequest::getClassId() const {
                return queue::QueuePortableHook::ADD_LISTENER;
            }

            void AddListenerRequest::write(serialization::PortableWriter& writer) const {
                writer.writeUTF("n", name);
                writer.writeBoolean("i", includeValue);
            }
        }
    }
}

