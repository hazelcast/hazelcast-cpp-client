//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_DestroyRequest
#define HAZELCAST_DestroyRequest


#include "../serialization/SerializationConstants.h"
#include "SemaphorePortableHook.h"
#include "Portable.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace semaphore {
            class DestroyRequest : public Portable {
            public:
                DestroyRequest(const std::string& instanceName)
                :instanceName(instanceName) {
                };

                int getClassId() const {
                    return SemaphorePortableHook::DESTROY;
                };

                int getFactoryId() const {
                    return SemaphorePortableHook::F_ID;
                };


                void writePortable(serialization::PortableWriter& writer) const {
                    writer.writeUTF("n", instanceName);
                };


                void readPortable(serialization::PortableReader& reader) {
                    instanceName = reader.readUTF("n");
                };
            private:

                std::string instanceName;
            };
        }
    }
}


#endif //HAZELCAST_DestroyRequest
