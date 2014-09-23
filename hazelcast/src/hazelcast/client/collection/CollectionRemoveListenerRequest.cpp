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
            : BaseRemoveListenerRequest(name, registrationId)
            , serviceName(serviceName) {

            }

            int CollectionRemoveListenerRequest::getFactoryId() const {
                return CollectionPortableHook::F_ID;
            }

            int CollectionRemoveListenerRequest::getClassId() const {
                return CollectionPortableHook::COLLECTION_REMOVE_LISTENER;
            }

            void CollectionRemoveListenerRequest::write(serialization::PortableWriter &writer) const {
                BaseRemoveListenerRequest::write(writer);
                writer.writeUTF("s", serviceName);
            }
        }
    }
}
