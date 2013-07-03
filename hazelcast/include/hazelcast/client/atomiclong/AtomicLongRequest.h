//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_AtomicLongRequest
#define HAZELCAST_AtomicLongRequest

#include "../serialization/SerializationConstants.h"
#include "AtomicLongPortableHook.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace atomiclong {

            class AtomicLongRequest : public Portable {
            public:
                AtomicLongRequest(const std::string& instanceName, long delta)
                :instanceName(instanceName)
                , delta(delta) {

                };

                virtual int getFactoryId() const {
                    return AtomicLongPortableHook::F_ID;
                };

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer.writeUTF("n", instanceName);
                    writer.writeLong("d", delta);
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    instanceName = reader.readUTF("n");
                    delta = reader.readLong("d");
                };
            private:
                long delta;
                std::string instanceName;
            };
        }
    }
}

#endif //HAZELCAST_AtomicLongRequest
