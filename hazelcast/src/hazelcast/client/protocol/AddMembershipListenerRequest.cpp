//
// Created by sancar koyunlu on 5/28/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/protocol/AddMembershipListenerRequest.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            AddMembershipListenerRequest::AddMembershipListenerRequest() {

            };

            int AddMembershipListenerRequest::getFactoryId() const{
                return protocol::ProtocolConstants::DATA_FACTORY_ID;
            }

            int AddMembershipListenerRequest::getClassId() const {
                return protocol::ProtocolConstants::ADD_MS_LISTENER;
            }

            void AddMembershipListenerRequest::writeData(serialization::BufferedDataOutput& writer) const{
            };

            void AddMembershipListenerRequest::readData(serialization::BufferedDataInput& reader) {
            };
        }
    }
}