//
// Created by sancar koyunlu on 5/28/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/protocol/AddMembershipListenerRequest.h"
#include "hazelcast/client/protocol/ProtocolConstants.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            AddMembershipListenerRequest::AddMembershipListenerRequest() {
            };

            int AddMembershipListenerRequest::getFactoryId() const {
                return protocol::ProtocolConstants::CLIENT_PORTABLE_FACTORY;
            }

            int AddMembershipListenerRequest::getClassId() const {
                return protocol::ProtocolConstants::MEMBERSHIP_LISTENER;
            }

            void AddMembershipListenerRequest::write(serialization::PortableWriter &writer) const {

            }
        }
    }
}