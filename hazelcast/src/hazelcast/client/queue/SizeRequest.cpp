//
// Created by sancar koyunlu on 9/4/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/queue/SizeRequest.h"
#include "QueuePortableHook.h"
#include "PortableReader.h"
#include "hazelcast/client/serialization/PortableWriter.h"

namespace hazelcast {
    namespace client {
        namespace queue {
            SizeRequest::SizeRequest(const std::string& name)
            :name(name) {

            };

            int SizeRequest::getFactoryId() const {
                return queue::QueuePortableHook::F_ID;
            }

            int SizeRequest::getClassId() const {
                return queue::QueuePortableHook::SIZE;
            };

            void SizeRequest::writePortable(serialization::PortableWriter& writer) const {
                writer.writeUTF("n", name);
                writer.writeLong("t", 0);
            };

            void SizeRequest::readPortable(serialization::PortableReader& reader) {
                name = reader.readUTF("n");
            };
        }
    }
}
