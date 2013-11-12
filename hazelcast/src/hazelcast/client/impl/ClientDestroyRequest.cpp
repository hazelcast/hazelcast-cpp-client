//
// Created by sancar koyunlu on 12/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/impl/ClientDestroyRequest.h"
#include "hazelcast/client/protocol/ProtocolConstants.h"
#include "hazelcast/client/serialization/PortableWriter.h"

namespace hazelcast {
    namespace client {
        namespace impl {

            ClientDestroyRequest::ClientDestroyRequest(const std::string &name, const std::string &serviceName)
            : name(name), serviceName(serviceName) {

            }

            int ClientDestroyRequest::getFactoryId() const {
                return protocol::ProtocolConstants::CLIENT_PORTABLE_FACTORY;
            }

            int ClientDestroyRequest::getClassId() const {
                return protocol::ProtocolConstants::DESTROY_PROXY;
            }

            void ClientDestroyRequest::writePortable(serialization::PortableWriter &writer) const {
                writer.writeUTF("n", name);
                writer.writeUTF("s", serviceName);
            }
        }
    }
}