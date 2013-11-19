//
// Created by sancar koyunlu on 8/15/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/executor/IsShutdownRequest.h"
#include "hazelcast/client/executor/DataSerializableHook.h"
#include "hazelcast/client/serialization/ObjectDataOutput.h"

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

        }
    }
}