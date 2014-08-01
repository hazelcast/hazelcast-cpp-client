//
// Created by sancar koyunlu on 15/01/14.
//

#include "hazelcast/client/multimap/RemoveEntryListenerRequest.h"
#include "hazelcast/client/multimap/MultiMapPortableHook.h"

namespace hazelcast {
    namespace client {
        namespace multimap {
            RemoveEntryListenerRequest::RemoveEntryListenerRequest(const std::string &name, const std::string &registrationId)
            : impl::BaseRemoveListenerRequest(name, registrationId) {

            }

            int RemoveEntryListenerRequest::getFactoryId() const {
                return MultiMapPortableHook::F_ID;
            }

            int RemoveEntryListenerRequest::getClassId() const {
                return MultiMapPortableHook::REMOVE_ENTRY_LISTENER;
            }

            bool RemoveEntryListenerRequest::isRetryable() const {
                return true;
            }
        }
    }
}
