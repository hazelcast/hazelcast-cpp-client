//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_AddMessageListenerRequest
#define HAZELCAST_AddMessageListenerRequest

#include "../serialization/SerializationConstants.h"
#include "../serialization/Data.h"
#include "TopicPortableHook.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace topic {
            class AddMessageListenerRequest : public Portable{
            public:
                AddMessageListenerRequest(const std::string& instanceName)
                : instanceName(instanceName) {

                };

                int getFactoryId() const {
                    return TopicPortableHook::F_ID;
                };

                int getClassId() const {
                    return TopicPortableHook::ADD_LISTENER;
                };

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer.writeUTF("n", instanceName);
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    instanceName = reader.readUTF("n");
                };
            private:
                std::string instanceName;
            };
        }
    }
}


#endif //HAZELCAST_AddMessageListenerRequest
