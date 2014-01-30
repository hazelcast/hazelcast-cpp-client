//
// Created by sancar koyunlu on 23/01/14.
//

#include "hazelcast/client/MembershipEvent.h"
#include "hazelcast/client/Cluster.h"

namespace hazelcast {
    namespace client {
        MembershipEvent::MembershipEvent(Cluster &cluster, MembershipEventType eventType, const Member &member)
        :cluster(&cluster)
        , member(member)
        , eventType(eventType) {

        }

        const std::vector<Member> &MembershipEvent::getMembers() const {
            return cluster->getMembers();
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