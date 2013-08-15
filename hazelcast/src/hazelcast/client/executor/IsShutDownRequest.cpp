//
// Created by sancar koyunlu on 8/15/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "IsShutdownRequest.h"
#include "hazelcast/client/executor/DataSerializableHook.h"

namespace hazelcast {
    namespace client {
        namespace executor {
            IsShutdownRequest::IsShutdownRequest(const std::string& instanceName)
            :instanceName(instanceName) {


            }

            int IsShutdownRequest::getFactoryId() const {
                return DataSerializableHook::F_ID;
            }

            int IsShutdownRequest::getClassId() const {
                return DataSerializableHook::IS_SHUTDOWN_REQUEST;
            }

            void IsShutdownRequest::writeData(serialization::ObjectDataOutput& writer) const {
                writer.writeUTF(instanceName);
            }

            void IsShutdownRequest::readData(serialization::ObjectDataInput& reader) {
                instanceName = reader.readUTF();
            }


        }
    }
}