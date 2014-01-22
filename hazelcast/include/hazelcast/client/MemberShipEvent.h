//
// Created by sancar koyunlu on 5/29/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_MEMBERSHIP_EVENT
#define HAZELCAST_MEMBERSHIP_EVENT

#include "hazelcast/client/Member.h"
#include "hazelcast/client/IdentifiedDataSerializable.h"

namespace hazelcast {
    namespace client {

        class HAZELCAST_API MembershipEvent : public IdentifiedDataSerializable {
        public:
            enum {
                MEMBER_ADDED = 1,
                MEMBER_REMOVED = 2,
                MEMBER_ATTRIBUTE_CHANGED = 5
            };

            MembershipEvent();

            MembershipEvent(const Member &member, int eventType);

            const Member &getMember() const;

            int getEventType() const;

            int getFactoryId() const;

            int getClassId() const;

            void writeData(serialization::ObjectDataOutput &writer) const;

            void readData(serialization::ObjectDataInput &reader);

        private:
            Member member;
            int eventType;
        };
    }
}
#endif //HAZELCAST_MEMBERSHIP_EVENT
