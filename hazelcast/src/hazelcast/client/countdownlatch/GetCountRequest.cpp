//
// Created by sancar koyunlu on 8/12/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "GetCountRequest.h"
#include "PortableWriter.h"
#include "PortableReader.h"
#include "CountDownLatchPortableHook.h"

namespace hazelcast {
    namespace client {
        namespace countdownlatch {
            GetCountRequest::GetCountRequest(const std::string& instanceName)
            : instanceName(instanceName) {

            };

            int GetCountRequest::getFactoryId() const {
                return CountDownLatchPortableHook::F_ID;
            };

            int GetCountRequest::getClassId() const {
                return CountDownLatchPortableHook::GET_COUNT;
            };


            void GetCountRequest::writePortable(serialization::PortableWriter& writer) const {
                writer.writeUTF("name", instanceName);
            };


            void GetCountRequest::readPortable(serialization::PortableReader& reader) {
                instanceName = reader.readUTF("name");
            };
        }
    }
}
