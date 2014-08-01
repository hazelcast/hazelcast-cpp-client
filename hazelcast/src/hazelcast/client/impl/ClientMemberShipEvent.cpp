//
// Created by sancar koyunlu on 5/29/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/impl/ClientMemberShipEvent.h"
#include "hazelcast/client/serialization/ObjectDataInput.h"

namespace hazelcast {
    namespace client {
        namespace impl {
            ClientMembershipEvent::ClientMembershipEvent() {

            }

            const Member &ClientMembershipEvent::getMember() const {
                return member;
            }

            MembershipEvent::MembershipEventType ClientMembershipEvent::getEventType() const {
                return eventType;
            }

            const impl::MemberAttributeChange &ClientMembershipEvent::getMemberAttributeChange() const {
                return *memberAttributeChange;
            }

            int ClientMembershipEvent::getFactoryId() const {
                return protocol::ProtocolConstants::DATA_FACTORY_ID;

            }

            int ClientMembershipEvent::getClassId() const {
                return protocol::ProtocolConstants::MEMBERSHIP_EVENT;

            }

            void ClientMembershipEvent::readData(serialization::ObjectDataInput &reader) {
                member.readData(reader);
                int t = reader.readInt();
                if (t == MembershipEvent::MEMBER_ADDED) {
                    eventType = MembershipEvent::MEMBER_ADDED;
                } else if (t == MembershipEvent::MEMBER_REMOVED) {
                    eventType = MembershipEvent::MEMBER_REMOVED;
                } else if (t == MembershipEvent::MEMBER_ATTRIBUTE_CHANGED) {
                    eventType = MembershipEvent::MEMBER_ATTRIBUTE_CHANGED;
                }
                if (reader.readBoolean()) {
                    memberAttributeChange.reset(new impl::MemberAttributeChange());
                    memberAttributeChange->readData(reader);
                }
            }
        }
    }
}

