//
// Created by sancar koyunlu on 5/29/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_MEMBERSHIP_EVENT
#define HAZELCAST_MEMBERSHIP_EVENT

#include "Member.h"

namespace hazelcast {
    namespace client {
        namespace connection {

            class MembershipEvent {
            public:
                static const int MEMBER_ADDED = 1;

                static const int MEMBER_REMOVED = 3;

                MembershipEvent(const Member& member, int eventType);

                Member getMember() const;

                int getEventType() const;

            private:
                Member member;
                int eventType;
            };
        }
    }
}

#endif //HAZELCAST_MEMBERSHIP_EVENT
