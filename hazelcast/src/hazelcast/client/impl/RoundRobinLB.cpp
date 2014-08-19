//
// Created by sancar koyunlu on 5/31/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/impl/RoundRobinLB.h"
#include "hazelcast/client/Cluster.h"
#include "hazelcast/client/exception/IException.h"

namespace hazelcast {
    namespace client {
        namespace impl {
            RoundRobinLB::RoundRobinLB():index(0) {

            };

            void RoundRobinLB::init(Cluster &cluster) {
                AbstractLoadBalancer::init(cluster);
            }

            const Member RoundRobinLB::next() {
                std::vector<Member> members = getMembers();
                if (members.size() == 0) {
                    throw exception::IException("const Member& RoundRobinLB::next()", "No member in member list!!");
                }
                return members[++index % members.size()];
            }


        }
    }
}
