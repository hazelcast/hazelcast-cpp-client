//
// Created by sancar koyunlu on 9/4/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/queue/DestroyRequest.h"
#include "QueuePortableHook.h"
#include "PortableWriter.h"
#include "PortableReader.h"

namespace hazelcast {
    namespace client {
        namespace queue {
            DestroyRequest::DestroyRequest(const std::string& name)
            :name(name) {

            };

            int DestroyRequest::getFactoryId() const {
                return queue::QueuePortableHook::F_ID;
            }

            int DestroyRequest::getClassId() const {
                return queue::QueuePortableHook::DESTROY;
            };

            void DestroyRequest::writePortable(serialization::PortableWriter& writer) const {
                writer.writeUTF("n", name);
            };

            void DestroyRequest::readPortable(serialization::PortableReader& reader) {
                name = reader.readUTF("n");
            };
        }
    }
}

