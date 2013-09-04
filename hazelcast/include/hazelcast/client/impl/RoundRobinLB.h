//
// Created by sancar koyunlu on 5/31/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_ROUND_ROBIN_LB
#define HAZELCAST_ROUND_ROBIN_LB

#include "AbstractLoadBalancer.h"
#include <boost/atomic.hpp>

namespace hazelcast {
    namespace client {
        namespace connection {
            class Member;
        }

        class Cluster;

        namespace impl {

            class RoundRobinLB : public AbstractLoadBalancer {
            public:
                RoundRobinLB();

                void init(Cluster &cluster);

                const hazelcast::client::connection::Member next();

            private:
                boost::atomic<int> index;
            };
        }
    }
}
#endif //HAZELCAST_ROUND_ROBIN_LB
