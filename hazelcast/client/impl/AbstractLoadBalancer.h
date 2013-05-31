//
// Created by sancar koyunlu on 5/31/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#ifndef HAZELCAST_ABSTRACT_LOAD_BALANCER
#define HAZELCAST_ABSTRACT_LOAD_BALANCER

#include "../connection/Member.h"
#include "../../util/AtomicPointer.h"
#include "../MembershipListener.h"
#include "../LoadBalancer.h"
#include <vector>

namespace hazelcast {
    namespace client {
        namespace connection {
            class Member;
        }
        class Cluster;

        namespace impl {
            class AbstractLoadBalancer : public hazelcast::client::LoadBalancer, public hazelcast::client::MembershipListener {
            public:

                void setMembersRef();

                virtual std::vector<hazelcast::client::connection::Member> getMembers();

                virtual void init(hazelcast::client::Cluster& cluster);

                void memberAdded(const hazelcast::client::connection::MembershipEvent& membershipEvent);

                void memberRemoved(const hazelcast::client::connection::MembershipEvent& membershipEvent);

                virtual ~AbstractLoadBalancer();

            private:
                hazelcast::util::AtomicPointer<std::vector<hazelcast::client::connection::Member> > membersRef;
                Cluster *cluster;
            };
        }
    }
}

#endif //HAZELCAST_LOAD_BALANCER
