//
// Created by sancar koyunlu on 5/31/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/impl/RoundRobinLB.h"
#include "hazelcast/client/Cluster.h"
#include "IException.h"

namespace hazelcast {
    namespace client {
        namespace impl {
            RoundRobinLB::RoundRobinLB() {

            };

            void RoundRobinLB::init(Cluster &cluster) {
                AbstractLoadBalancer::init(cluster);
                cluster.addMembershipListener(this);
            };

            const connection::Member& RoundRobinLB::next() {
                util::AtomicPointer<std::vector<connection::Member> > members = getMembers();
                if (members->size() == 0) {
                    throw exception::IException("const connection::Member& RoundRobinLB::next()", "No member in member list!!");
                }
                return (*members)[++index % members->size()];
            }


        }
    }
}