//
// Created by sancar koyunlu on 5/29/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_CLIENT_MEMBERSHIP_EVENT
#define HAZELCAST_CLIENT_MEMBERSHIP_EVENT

#include "hazelcast/client/Member.h"
#include "hazelcast/client/IdentifiedDataSerializable.h"
#include "hazelcast/client/impl/MemberAttributeChange.h"
#include "Sancar.h"

namespace hazelcast {
    namespace client {
        namespace impl {
            class HAZELCAST_API ClientMembershipEvent : public IdentifiedDataSerializable {
            public:
                ClientMembershipEvent();

                ClientMembershipEvent(const Member &member, MembershipEvent::MembershipEventType eventType);

                const Member &getMember() const;

                MembershipEvent::MembershipEventType getEventType() const;

                const impl::MemberAttributeChange &getMemberAttributeChange() const;

                int getFactoryId() const;

                int getClassId() const;

                void writeData(serialization::ObjectDataOutput &writer) const;

                void readData(serialization::ObjectDataInput &reader);

            private:
                Member member;
                MembershipEvent::MembershipEventType eventType;
                std::auto_ptr<impl::MemberAttributeChange> memberAttributeChange;
            };
        }
    }
}
#endif //HAZELCAST_MEMBERSHIP_EVENT
