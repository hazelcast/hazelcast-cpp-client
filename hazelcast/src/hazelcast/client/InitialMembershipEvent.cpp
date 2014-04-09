//
// Created by sancar koyunlu on 22/01/14.
//

#include "hazelcast/client/InitialMembershipEvent.h"
#include "hazelcast/client/Cluster.h"


namespace hazelcast {
    namespace client {

        InitialMembershipEvent::InitialMembershipEvent(Cluster &cluster, const std::vector<Member> &members)
        : members(members)
        , cluster(cluster) {

        }

        const std::vector<Member> &InitialMembershipEvent::getMembers() const {
            return members;
        }

        Cluster &InitialMembershipEvent::getCluster() {
            return cluster;
        }
    }
}
