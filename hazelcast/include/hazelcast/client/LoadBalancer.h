//
// Created by sancar koyunlu on 5/31/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#ifndef HAZELCAST_LOAD_BALANCER
#define HAZELCAST_LOAD_BALANCER

#include "hazelcast/util/HazelcastDll.h"

namespace hazelcast {
    namespace client {
        class Member;

        class Cluster;
        /**
         *
         * LoadBalancer allows you to send operations to one of a number of endpoints(Members).
         * It is up to the implementation to use different load balancing policies. If Client is configured as smart,
         * only the operations that are not key based will be router to the endpoint returned by the Load Balancer.
         * If it is not smart, LoadBalancer will not be used.
         * Note Client is smart by default.
         *
         */
        class LoadBalancer {
        public:

            /**
             * This method will be called when load balancer is initialized.
             *
             * @param cluster Cluster contains current membership information for initialization. And one can
             * add membership through this class for future notifications.
             */
            virtual void init(Cluster &cluster) = 0;

            /**
             * Returns the next member to route to
             * @return Returns the next member or null if no member is available
             */
            virtual const Member next() = 0;

            virtual ~LoadBalancer();
        };
    }
}

#endif //HAZELCAST_LOAD_BALANCER

