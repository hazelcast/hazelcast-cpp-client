////
//// Created by sancar koyunlu on 8/15/13.
//// Copyright (c) 2013 hazelcast. All rights reserved.
//
//
//#include "hazelcast/client/executor/IsShutdownRequest.h"
//#include "hazelcast/client/executor/DataSerializableHook.h"
//#include "hazelcast/client/serialization/PortableWriter.h"
//
//namespace hazelcast {
//    namespace client {
//        namespace executor {
//            IsShutdownRequest::IsShutdownRequest(const std::string& instanceName)
//            :instanceName(instanceName) {
//
//            }
//
//            bool IsShutdownRequest::isRetryable() const {
//                return true;
//            }
//
//            int IsShutdownRequest::getFactoryId() const {
//                return DataSerializableHook::F_ID;
//            }
//
//            int IsShutdownRequest::getClassId() const {
//                return DataSerializableHook::IS_SHUTDOWN_REQUEST;
//            }
//
//            void IsShutdownRequest::write(serialization::PortableWriter &writer) const {
//                     writer.writeUTF("n", instanceName);
//            }
//        }
//    }
//}