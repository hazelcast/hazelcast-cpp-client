//
// Created by sancar koyunlu on 15/01/14.
//

#include "hazelcast/client/queue/RemoveListenerRequest.h"
#include "hazelcast/client/queue/QueuePortableHook.h"


namespace hazelcast {
    namespace client {
        namespace queue {
            RemoveListenerRequest::RemoveListenerRequest(const std::string &name, const std::string &registrationId)
            :impl::BaseRemoveListenerRequest(name, registrationId) {
            };

            int RemoveListenerRequest::getFactoryId() const {
                return queue::QueuePortableHook::F_ID;
            }

            int RemoveListenerRequest::getClassId() const {
                return queue::QueuePortableHook::REMOVE_LISTENER;
            };
        }
    }
}
