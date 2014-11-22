//
// Created by sancar koyunlu on 25/12/13.
//

#include "hazelcast/client/connection/ClientResponse.h"
#include "hazelcast/client/serialization/ObjectDataInput.h"
#include "hazelcast/client/protocol/ProtocolConstants.h"

namespace hazelcast {
    namespace client {
        namespace connection {

            bool ClientResponse::isException() const {
                return exception;
            }

            serialization::pimpl::Data const &ClientResponse::getData() const {
                return data;
            }

            int ClientResponse::getCallId() const {
                return callId;
            }

            int ClientResponse::getFactoryId() const {
                return protocol::ProtocolConstants::CLIENT_DS_FACTORY;
            }

            int ClientResponse::getClassId() const {
                return protocol::ProtocolConstants::CLIENT_RESPONSE;
            }

            void ClientResponse::readData(serialization::ObjectDataInput & in) {
                callId = in.readInt();
                exception = in.readBoolean();
                data = in.readData();
            }
        }
    }
}
