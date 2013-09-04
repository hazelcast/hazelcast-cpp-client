//
// Created by sancar koyunlu on 9/4/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "PollRequest.h"
#include "QueuePortableHook.h"
#include "PortableWriter.h"
#include "PortableReader.h"

namespace hazelcast {
    namespace client {
        namespace queue {
            PollRequest::PollRequest(const std::string& name, long timeout)
            :name(name)
            , timeoutInMillis(timeout) {

            };

            PollRequest::PollRequest(const std::string& name)
            :name(name)
            , timeoutInMillis(0) {

            };

            int PollRequest::getFactoryId() const {
                return queue::QueuePortableHook::F_ID;
            }

            int PollRequest::getClassId() const {
                return queue::QueuePortableHook::POLL;
            };


            void PollRequest::writePortable(serialization::PortableWriter& writer) const {
                writer.writeUTF("n", name);
                writer.writeLong("t", timeoutInMillis);
            };


            void PollRequest::readPortable(serialization::PortableReader& reader) {
                name = reader.readUTF("n");
                timeoutInMillis = reader.readLong("t");
            };
        }
    }
}


