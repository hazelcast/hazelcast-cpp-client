//
// Created by sancar koyunlu on 5/28/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_ADD_MLR
#define HAZELCAST_ADD_MLR

#include "ProtocolConstants.h"
#include "../serialization/SerializationConstants.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            class AddMembershipListenerRequest {
            public:
                AddMembershipListenerRequest();

            private:
            };
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace serialization {
            inline int getTypeId(const hazelcast::client::protocol::AddMembershipListenerRequest& x) {
                return SerializationConstants::CONSTANT_TYPE_DATA;
            };

            inline int getFactoryId(const hazelcast::client::protocol::AddMembershipListenerRequest& ar) {
                return hazelcast::client::protocol::ProtocolConstants::DATA_FACTORY_ID;
            }

            inline int getClassId(const hazelcast::client::protocol::AddMembershipListenerRequest& ar) {
                return hazelcast::client::protocol::ProtocolConstants::ADD_MS_LISTENER;
            }


            template<typename HzWriter>
            inline void writePortable(HzWriter& writer, const hazelcast::client::protocol::AddMembershipListenerRequest& arr) {
            };

            template<typename HzReader>
            inline void readPortable(HzReader& reader, hazelcast::client::protocol::AddMembershipListenerRequest& arr) {
            };

        }
    }
}
#endif //HAZELCAST_ADD_MLR
