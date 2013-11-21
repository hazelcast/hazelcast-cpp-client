//
// Created by sancar koyunlu on 5/29/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/connection/MemberShipEvent.h"
#include "hazelcast/client/serialization/ObjectDataOutput.h"
#include "hazelcast/client/serialization/ObjectDataInput.h"

namespace hazelcast {
    namespace client {
        namespace connection {

            MembershipEvent::MembershipEvent() {

            };

            MembershipEvent::MembershipEvent(Member const & member, int eventType)
            : member(member)
            , eventType(eventType) {

            };

            Member MembershipEvent::getMember() const {
                return member;
            };

            int MembershipEvent::getEventType() const {
                return eventType;
            };

            int MembershipEvent::getFactoryId() const {
                return hazelcast::client::protocol::ProtocolConstants::DATA_FACTORY_ID;

            }

            int MembershipEvent::getClassId() const {
                return hazelcast::client::protocol::ProtocolConstants::MEMBERSHIP_EVENT;

            }

            void MembershipEvent::writeData(serialization::ObjectDataOutput& writer) const {
                member.writeData(writer);
                writer.writeInt(eventType);
            }

            void MembershipEvent::readData(serialization::ObjectDataInput& reader) {
                member.readData(reader);
                eventType = reader.readInt();
            }

        }
    }
}
