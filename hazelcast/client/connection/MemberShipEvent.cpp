//
// Created by sancar koyunlu on 5/29/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "MembershipEvent.h"

namespace hazelcast {
    namespace client {
        namespace connection {
            
            MembershipEvent::MembershipEvent(){
                
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


        }
    }
}