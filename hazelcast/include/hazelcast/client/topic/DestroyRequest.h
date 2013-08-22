//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_Topic_DestroyRequest
#define HAZELCAST_Topic_DestroyRequest

#include "../serialization/SerializationConstants.h"
#include "TopicPortableHook.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace topic {
            class DestroyRequest : public Portable, public RetryableRequest {
            public:
                DestroyRequest(const std::string& instanceName)
                : instanceName(instanceName) {

                };

                int getFactoryId() const {
                    return TopicPortableHook::F_ID;
                };

                int getClassId() const {
                    return TopicPortableHook::DESTROY;
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
