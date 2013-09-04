//
// Created by sancar koyunlu on 9/4/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "PeekRequest.h"
#include "QueuePortableHook.h"
#include "PortableReader.h"
#include "PortableWriter.h"

namespace hazelcast {
    namespace client {
        namespace queue {
            PeekRequest::PeekRequest(const std::string& name)
            :name(name) {

            };

            int PeekRequest::getFactoryId() const {
                return queue::QueuePortableHook::F_ID;
            }

            int PeekRequest::getClassId() const {
                return queue::QueuePortableHook::PEEK;
            };

            void PeekRequest::writePortable(serialization::PortableWriter& writer) const {
                writer.writeUTF("n", name);
            };

            void PeekRequest::readPortable(serialization::PortableReader& reader) {
                name = reader.readUTF("n");
            };
        }
    }
}
