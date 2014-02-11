//
// Created by sancar koyunlu on 11/02/14.
//

#include "hazelcast/client/impl/BaseRemoveListenerRequest.h"
#include "hazelcast/client/serialization/PortableWriter.h"

namespace hazelcast {
    namespace client {
        namespace impl {

            BaseRemoveListenerRequest::BaseRemoveListenerRequest(const std::string &name, const std::string registrationId)
            : name(name), registrationId(registrationId) {

            }

            void BaseRemoveListenerRequest::setRegistrationId(const string &registrationId) {
                this->registrationId = registrationId;
            }

            void BaseRemoveListenerRequest::write(serialization::PortableWriter &writer) const {
                writer.writeUTF("n", name);
                writer.writeUTF("r", registrationId);
            }

        }
    }
}