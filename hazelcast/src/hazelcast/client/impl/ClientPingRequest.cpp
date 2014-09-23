//
// Created by sancar koyunlu on 20/08/14.
//

#include "hazelcast/client/impl/ClientPingRequest.h"
#include "hazelcast/client/protocol/ProtocolConstants.h"
#include "hazelcast/client/serialization/PortableWriter.h"

namespace hazelcast {
    namespace client {
        namespace impl {

            int ClientPingRequest::getFactoryId() const {
                return protocol::ProtocolConstants::CLIENT_PORTABLE_FACTORY;
            }

            int ClientPingRequest::getClassId() const {
                return protocol::ProtocolConstants::CLIENT_PING;
            }

            void ClientPingRequest::write(serialization::PortableWriter &writer) const {
            }

            bool ClientPingRequest::isBindToSingleConnection() const {
                return true;
            }
        }
    }
}
