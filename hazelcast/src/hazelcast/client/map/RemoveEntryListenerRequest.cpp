//
// Created by sancar koyunlu on 07/01/14.
//

#include "hazelcast/client/map/RemoveEntryListenerRequest.h"
#include "hazelcast/client/map/PortableHook.h"

namespace hazelcast {
    namespace client {
        namespace map {

            RemoveEntryListenerRequest::RemoveEntryListenerRequest(const std::string &name, const std::string &registrationId)
            :impl::BaseRemoveListenerRequest(name, registrationId) {

            }

            int RemoveEntryListenerRequest::getFactoryId() const {
                return PortableHook::F_ID;
            }

            int RemoveEntryListenerRequest::getClassId() const {
                return PortableHook::REMOVE_ENTRY_LISTENER;
            }
        }
    }
}

