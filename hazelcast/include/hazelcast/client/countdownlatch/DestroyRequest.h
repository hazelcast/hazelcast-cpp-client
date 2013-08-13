//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_DestroyRequest
#define HAZELCAST_DestroyRequest

#include "../serialization/SerializationConstants.h"
#include "CountDownLatchPortableHook.h"
#include "Portable.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace countdownlatch {
            class DestroyRequest : public Portable{
                DestroyRequest(const std::string& instanceName)
                : instanceName(instanceName) {

                };

                int getFactoryId() const {
                    return CountDownLatchPortableHook::F_ID;
                };

                int getClassId() const {
                    return CountDownLatchPortableHook::DESTROY;
                };


                void writePortable(serialization::PortableWriter& writer) const {
                    writer.writeUTF("name", instanceName);
                };


                void readPortable(serialization::PortableReader& reader) {
                    instanceName = reader.readUTF("name");
                };
            private:

                std::string instanceName;
            };
        }
    }
}


#endif //HAZELCAST_DestroyRequest
