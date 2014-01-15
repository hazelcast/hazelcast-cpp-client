//
// Created by sancar koyunlu on 15/01/14.
//
#include "hazelcast/client/topic/RemoveMessageListenerRequest.h"
#include "hazelcast/client/serialization/Data.h"
#include "hazelcast/client/topic/TopicPortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"

namespace hazelcast {
    namespace client {
        namespace topic {
            RemoveMessageListenerRequest::RemoveMessageListenerRequest(const std::string &instanceName, const std::string &registrationId)
            : instanceName(instanceName)
            , registrationId(registrationId) {

            };

            int RemoveMessageListenerRequest::getFactoryId() const {
                return TopicPortableHook::F_ID;
            };

            int RemoveMessageListenerRequest::getClassId() const {
                return TopicPortableHook::REMOVE_LISTENER;
            };

            void RemoveMessageListenerRequest::write(serialization::PortableWriter &writer) const {
                writer.writeUTF("n", instanceName);
                writer.writeUTF("r", registrationId);
            };
        }
    }
}


