//
// Created by sancar koyunlu on 15/01/14.
//

#include "hazelcast/client/collection/CollectionRemoveListenerRequest.h"
#include "hazelcast/client/collection/CollectionPortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"


namespace hazelcast {
    namespace client {
        namespace collection {

            CollectionRemoveListenerRequest::CollectionRemoveListenerRequest(const std::string &name, const std::string &serviceName, const std::string &registrationId)
            : CollectionRequest(name, serviceName)
            , registrationId(registrationId) {

            }

            int CollectionRemoveListenerRequest::getClassId() const {
                return CollectionPortableHook::COLLECTION_REMOVE_LISTENER;
            }

            void CollectionRemoveListenerRequest::write(serialization::PortableWriter &writer) const {
                CollectionRequest::write(writer);
                writer.writeUTF("r", registrationId);
            }
        }
    }
}