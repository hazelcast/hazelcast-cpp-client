//
// Created by sancar koyunlu on 5/31/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#ifndef HAZELCAST_ABSTRACT_LOAD_BALANCER
#define HAZELCAST_ABSTRACT_LOAD_BALANCER

#include "hazelcast/client/connection/Member.h"
#include "hazelcast/client/MembershipListener.h"
#include "hazelcast/client/LoadBalancer.h"
#include <boost/thread/mutex.hpp>
#include <vector>

namespace hazelcast {
    namespace client {
        namespace connection {
            class Member;
        }
        class Cluster;

        namespace impl {
            class HAZELCAST_API AbstractLoadBalancer : public LoadBalancer, public MembershipListener {
            public:

                void setMembersRef();

                std::vector<connection::Member> getMembers();

                virtual void init(Cluster &cluster);

                void memberAdded(const connection::MembershipEvent &membershipEvent);

                void memberRemoved(const connection::MembershipEvent &membershipEvent);

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
