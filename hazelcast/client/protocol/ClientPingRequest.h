//
// Created by sancar koyunlu on 5/21/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.
#ifndef HAZELCAST_CLIENT_PING_REQUEST
#define HAZELCAST_CLIENT_PING_REQUEST

#include "../serialization/DataSerializable.h"
#include "ProtocolConstants.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            class ClientPingRequest : public hazelcast::client::serialization::DataSerializable {
            public:
                ClientPingRequest();

            };

        }
    }
}

namespace hazelcast {
    namespace client {
        namespace serialization {
            inline int getFactoryId(const hazelcast::client::protocol::ClientPingRequest& ar) {
                return hazelcast::client::protocol::ProtocolConstants::DATA_FACTORY_ID;
            }

            inline int getClassId(const hazelcast::client::protocol::ClientPingRequest& ar) {
                return hazelcast::client::protocol::ProtocolConstants::CLIENT_PING_REQUEST_ID;
            }


            template<typename HzWriter>
            inline void writePortable(HzWriter& writer, const hazelcast::client::protocol::ClientPingRequest& arr) {
            };

            template<typename HzReader>
            inline void readPortable(HzReader& reader, hazelcast::client::protocol::ClientPingRequest& arr) {
            };

        }
    }
}


#endif //HAZELCAST_CLIENT_PING_REQUEST
