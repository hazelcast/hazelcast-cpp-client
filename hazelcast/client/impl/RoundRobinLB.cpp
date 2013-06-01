//
// Created by sancar koyunlu on 5/31/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "RoundRobinLB.h"
#include "../Cluster.h"
#include "../HazelcastException.h"

namespace hazelcast {
    namespace client {
        namespace impl {
            RoundRobinLB::RoundRobinLB() {

            };

            void RoundRobinLB::init(Cluster &cluster) {
                AbstractLoadBalancer::init(cluster);
                cluster.addMembershipListener(this);
            };

            const hazelcast::client::connection::Member  & RoundRobinLB::next() {
                std::vector<hazelcast::client::connection::Member> members = getMembers();
                if (members.size() == 0) {
                    throw hazelcast::client::HazelcastException("No member in member list!!");
                }
                return members[index.getAndAdd(1) % members.size()];
            }


        }
    }
}