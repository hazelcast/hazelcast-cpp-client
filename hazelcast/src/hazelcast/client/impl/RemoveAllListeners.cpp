//
// Created by sancar koyunlu on 21/08/14.
//

#include "hazelcast/client/impl/RemoveAllListeners.h"
#include "hazelcast/client/protocol/ProtocolConstants.h"
#include "hazelcast/client/serialization/PortableWriter.h"

namespace hazelcast {
    namespace client {
        namespace impl {

            int RemoveAllListeners::getFactoryId() const {
                return protocol::ProtocolConstants::CLIENT_PORTABLE_FACTORY;
            }

            int RemoveAllListeners::getClassId() const {
                return protocol::ProtocolConstants::REMOVE_ALL_LISTENERS;
            }

            void RemoveAllListeners::write(serialization::PortableWriter& writer) const {
            }
        }
    }
}
