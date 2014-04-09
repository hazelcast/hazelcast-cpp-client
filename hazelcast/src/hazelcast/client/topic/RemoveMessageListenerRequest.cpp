//
// Created by sancar koyunlu on 15/01/14.
//
#include "hazelcast/client/topic/RemoveMessageListenerRequest.h"
#include "hazelcast/client/topic/TopicPortableHook.h"

namespace hazelcast {
    namespace client {
        namespace topic {
            RemoveMessageListenerRequest::RemoveMessageListenerRequest(const std::string &instanceName, const std::string &registrationId)
            : impl::BaseRemoveListenerRequest(instanceName, registrationId) {

            };

            int RemoveMessageListenerRequest::getFactoryId() const {
                return TopicPortableHook::F_ID;
            };

            int RemoveMessageListenerRequest::getClassId() const {
                return TopicPortableHook::REMOVE_LISTENER;
            };
        }
    }
}



