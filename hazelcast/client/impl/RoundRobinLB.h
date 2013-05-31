//
// Created by sancar koyunlu on 5/31/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_ROUND_ROBIN_LB
#define HAZELCAST_ROUND_ROBIN_LB

#include "../../util/AtomicInteger.h"
#include "AbstractLoadBalancer.h"

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

                const hazelcast::client::connection::Member& next();

            private:
                hazelcast::util::AtomicInteger index;
            };
        }
    }
}
#endif //HAZELCAST_ROUND_ROBIN_LB
