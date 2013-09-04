//
// Created by sancar koyunlu on 5/31/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#ifndef HAZELCAST_ABSTRACT_LOAD_BALANCER
#define HAZELCAST_ABSTRACT_LOAD_BALANCER

#include "../connection/Member.h"
#include "../../util/AtomicPointer.h"
#include "../MembershipListener.h"
#include "../LoadBalancer.h"
#include "boost/thread/pthread/mutex.hpp"
#include <vector>

namespace hazelcast {
    namespace client {
        namespace connection {
            class Member;
        }
        class Cluster;

        namespace impl {
            class AbstractLoadBalancer : public LoadBalancer, public MembershipListener {
            public:

                void setMembersRef();

                std::vector<connection::Member> getMembers();

                virtual void init(Cluster& cluster);

                void memberAdded(const connection::MembershipEvent& membershipEvent);

                void memberRemoved(const connection::MembershipEvent& membershipEvent);

                virtual ~AbstractLoadBalancer();

            private:
                boost::mutex membersLock;
                std::vector<connection::Member> membersRef;
                Cluster *cluster;
            };
        }
    }
}

#endif //HAZELCAST_LOAD_BALANCER
