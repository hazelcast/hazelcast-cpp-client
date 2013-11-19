//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_SemaphoreRequest
#define HAZELCAST_SemaphoreRequest

#include "hazelcast/client/serialization/SerializationConstants.h"
#include "SemaphorePortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "PortableReader.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace semaphore {
            class SemaphoreRequest : public Portable {
            public:
                SemaphoreRequest(const std::string& instanceName, int permitCount)
                :instanceName(instanceName)
                , permitCount(permitCount) {

                };

                virtual int getFactoryId() const {
                    return SemaphorePortableHook::F_ID;
                };


                void writePortable(serialization::PortableWriter& writer) const {
                    writer.writeUTF("n", instanceName);
                    writer.writeInt("p", permitCount);
                };


                void readPortable(serialization::PortableReader& reader) {
                    instanceName = reader.readUTF("n");
                    permitCount = reader.readInt("p");
                };
            private:

                std::string instanceName;
                int permitCount;
            };
        }
    }
}

#endif //HAZELCAST_SemaphoreRequest
