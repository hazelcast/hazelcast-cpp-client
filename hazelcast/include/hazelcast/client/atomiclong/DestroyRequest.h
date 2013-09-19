//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_DestroyRequest
#define HAZELCAST_DestroyRequest

#include "../serialization/SerializationConstants.h"
#include "AtomicLongPortableHook.h"
#include "Portable.h"
#include "PortableWriter.h"
#include "PortableReader.h"
#include "Portable.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace atomiclong {

            class DestroyRequest : public Portable, public RetryableRequest {
            public:
                DestroyRequest(const std::string& instanceName)
                :instanceName(instanceName) {

                };

                int getFactoryId() const {
                    return AtomicLongPortableHook::F_ID;
                };

                int getClassId() const {
                    return AtomicLongPortableHook::DESTROY;
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
