//
// Created by sancar koyunlu on 9/4/13.
// Copyright (c) 2013 hazelcast. All rights reserved.

#include "hazelcast/client/queue/ClearRequest.h"
#include "QueuePortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "PortableReader.h"

namespace hazelcast {
    namespace client {
        namespace queue {
            ClearRequest::ClearRequest(const std::string& name)
            :name(name) {

            };

            int ClearRequest::getFactoryId() const {
                return queue::QueuePortableHook::F_ID;
            }

            int ClearRequest::getClassId() const {
                return queue::QueuePortableHook::CLEAR;
            };

            void ClearRequest::writePortable(serialization::PortableWriter& writer) const {
                writer.writeUTF("n", name);
                writer.writeLong("t", 0);
            };

            void ClearRequest::readPortable(serialization::PortableReader& reader) {
                name = reader.readUTF("n");
            };
        }
    }
}

