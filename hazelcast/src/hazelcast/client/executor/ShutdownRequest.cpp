//
// Created by sancar koyunlu on 8/15/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "ShutdownRequest.h"
#include "hazelcast/client/executor/DataSerializableHook.h"

namespace hazelcast {
    namespace client {
        namespace executor {
            ShutdownRequest::ShutdownRequest(const std::string& instanceName)
            :instanceName(instanceName) {
            }

            int ShutdownRequest::getFactoryId() const {
                return DataSerializableHook::F_ID;
            }

            int ShutdownRequest::getClassId() const {
                return DataSerializableHook::IS_SHUTDOWN_REQUEST;
            }

            void ShutdownRequest::writeData(serialization::ObjectDataOutput& writer) const {
                writer.writeUTF(instanceName);
            }

            void ShutdownRequest::readData(serialization::ObjectDataInput& reader) {
                instanceName = reader.readUTF();
            }
        }
    }
}
