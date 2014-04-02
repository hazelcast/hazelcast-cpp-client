//
// Created by sancar koyunlu on 5/31/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_ROUND_ROBIN_LB
#define HAZELCAST_ROUND_ROBIN_LB

#include "hazelcast/client/impl/AbstractLoadBalancer.h"
#include "hazelcast/util/AtomicInt.h"

namespace hazelcast {
    namespace client {

        class Member;

        class Cluster;

        namespace impl {

            class HAZELCAST_API RoundRobinLB : public AbstractLoadBalancer {
            public:
                RoundRobinLB();

                void init(Cluster &cluster);

                const Member next();

            private:
                util::AtomicInt index;
            };
        }
    }
}
#endif //HAZELCAST_ROUND_ROBIN_LB
