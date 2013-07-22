//
// Created by sancar koyunlu on 5/29/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_MEMBERSHIP_EVENT
#define HAZELCAST_MEMBERSHIP_EVENT

#include "Member.h"
#include "DataSerializable.h"

namespace hazelcast {
    namespace client {
        namespace connection {

            class MembershipEvent : public DataSerializable {
            public:
                enum {
                    MEMBER_ADDED = 1,
                    MEMBER_REMOVED = 2
                };

                MembershipEvent();

                MembershipEvent(const Member& member, int eventType);

                Member getMember() const;

                int getEventType() const;

                int getFactoryId() const;

                int getClassId() const;

                void writeData(serialization::BufferedDataOutput& writer);

                void readData(serialization::BufferedDataInput& reader);

            private:
                Member member;
                int eventType;
            };
        }
    }
}
#endif //HAZELCAST_MEMBERSHIP_EVENT
