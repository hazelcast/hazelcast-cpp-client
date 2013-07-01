//
// Created by sancar koyunlu on 5/21/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.
#ifndef HAZELCAST_CLIENT_PING_REQUEST
#define HAZELCAST_CLIENT_PING_REQUEST


#include "ProtocolConstants.h"
#include "../serialization/SerializationConstants.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            class PingRequest {
            public:
                PingRequest();

            };

        }
    }
}

namespace hazelcast {
    namespace client {
        namespace serialization {

            inline int getSerializerId(const hazelcast::client::protocol::PingRequest& x) {
                return SerializationConstants::CONSTANT_TYPE_DATA;
            };

            inline int getFactoryId(const hazelcast::client::protocol::PingRequest& ar) {
                return hazelcast::client::protocol::ProtocolConstants::DATA_FACTORY_ID;
            }

            inline int getClassId(const hazelcast::client::protocol::PingRequest& ar) {
                return hazelcast::client::protocol::ProtocolConstants::PING;
            }


            template<typename HzWriter>
            inline void writePortable(HzWriter& writer, const hazelcast::client::protocol::PingRequest& arr) {
            };

            template<typename HzReader>
            inline void readPortable(HzReader& reader, hazelcast::client::protocol::PingRequest& arr) {
            };

        }
    }
}


#endif //HAZELCAST_CLIENT_PING_REQUEST
