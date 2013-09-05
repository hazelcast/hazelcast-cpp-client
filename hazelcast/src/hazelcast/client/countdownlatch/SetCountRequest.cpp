//
// Created by sancar koyunlu on 9/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "SetCountRequest.h"
#include "CountDownLatchPortableHook.h"
#include "PortableWriter.h"
#include "PortableReader.h"

namespace hazelcast {
    namespace client {
        namespace countdownlatch {
            SetCountRequest::SetCountRequest(const std::string& instanceName, int count)
            : instanceName(instanceName)
            , count(count) {

            };

            int SetCountRequest::getFactoryId() const {
                return CountDownLatchPortableHook::F_ID;
            };

            int SetCountRequest::getClassId() const {
                return CountDownLatchPortableHook::SET_COUNT;
            };

            void SetCountRequest::writePortable(serialization::PortableWriter& writer) const {
                writer.writeUTF("name", instanceName);
                writer.writeInt("count", count);
            };

            void SetCountRequest::readPortable(serialization::PortableReader& reader) {
                instanceName = reader.readUTF("name");
                count = reader.readInt("count");
            };
        }
    }
}
