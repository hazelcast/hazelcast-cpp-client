/*
 * Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
//
// Created by sancar koyunlu on 5/31/13.

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
		class HAZELCAST_API LoadBalancer {
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

