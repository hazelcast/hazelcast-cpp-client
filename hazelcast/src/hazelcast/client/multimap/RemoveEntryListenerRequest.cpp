//
// Created by sancar koyunlu on 15/01/14.
//

#include "hazelcast/client/multimap/RemoveEntryListenerRequest.h"
#include "hazelcast/client/multimap/MultiMapPortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"

namespace hazelcast {
    namespace client {
        namespace multimap {
            RemoveEntryListenerRequest::RemoveEntryListenerRequest(const std::string &name, const std::string &registrationId)
            : AllPartitionsRequest(name)
            , registrationId(registrationId) {

            }

            int RemoveEntryListenerRequest::getClassId() const {
                return MultiMapPortableHook::REMOVE_ENTRY_LISTENER;
            };

            void RemoveEntryListenerRequest::write(serialization::PortableWriter &writer) const {
                writer.writeUTF("r", registrationId);
                AllPartitionsRequest::write(writer);
            };

            bool RemoveEntryListenerRequest::isRetryable() const{
                return true;
            }
        }
    }
}