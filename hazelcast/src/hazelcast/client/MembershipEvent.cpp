/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
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
// Created by sancar koyunlu on 23/01/14.
//

#include "hazelcast/client/MembershipEvent.h"
#include "hazelcast/client/Cluster.h"

namespace hazelcast {
    namespace client {
        MembershipEvent::MembershipEvent(Cluster &cluster, const Member &member, MembershipEventType eventType, const std::set<Member> &membersList) :
                cluster(&cluster), member(member), eventType(eventType), members(membersList) {
        }

        MembershipEvent::~MembershipEvent() {
        }

        const std::set<Member> MembershipEvent::getMembers() const {
            return members;
        }

        const Cluster &MembershipEvent::getCluster() const {
            return *cluster;
        }

        MembershipEvent::MembershipEventType MembershipEvent::getEventType() const {
            return eventType;
        }

        const Member &MembershipEvent::getMember() const {
            return member;
        }
    }
}
