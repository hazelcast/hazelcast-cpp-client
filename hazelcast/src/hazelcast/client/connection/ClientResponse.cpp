//
// Created by sancar koyunlu on 25/12/13.
//

#include "hazelcast/client/connection/ClientResponse.h"
#include "hazelcast/client/serialization/ObjectDataInput.h"

namespace hazelcast {
    namespace client {
        namespace connection {

            bool ClientResponse::isEvent() const {
                return event;
            }

            bool ClientResponse::isException() const {
                return exception;
            }

            serialization::Data const &ClientResponse::getData() const {
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

            void ClientResponse::readData(serialization::ObjectDataInput &reader) {
                callId = reader.readInt();
                event = reader.readBoolean();
                exception = reader.readBoolean();
                data.readData(reader);
            }
        }
    }
}