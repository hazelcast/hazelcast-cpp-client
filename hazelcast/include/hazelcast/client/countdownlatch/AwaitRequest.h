//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_AwaitRequest
#define HAZELCAST_AwaitRequest

#include "../serialization/SerializationConstants.h"
#include "CountDownLatchPortableHook.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace countdownlatch {
            class AwaitRequest : public Portable {
            public:
                AwaitRequest(const std::string& instanceName, long timeout)
                : instanceName(instanceName)
                , timeout(timeout) {

                };

                int getFactoryId() const {
                    return CountDownLatchPortableHook::F_ID;
                };

                int getClassId() const {
                    return CountDownLatchPortableHook::AWAIT;
                };

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer.writeUTF("name", instanceName);
                    writer.writeLong("timeout", timeout);
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    instanceName = reader.readUTF("name");
                    timeout = reader.readLong("timeout");
                };
            private:

                std::string instanceName;
                long timeout;
            };
        }
    }
}


#endif //HAZELCAST_AwaitRequest
