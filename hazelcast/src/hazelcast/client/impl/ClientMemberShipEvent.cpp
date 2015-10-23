/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
//
// Created by sancar koyunlu on 5/29/13.



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

