/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
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

#ifndef HAZELCAST_ROUND_ROBIN_LB
#define HAZELCAST_ROUND_ROBIN_LB

#include "hazelcast/client/impl/AbstractLoadBalancer.h"
#include "hazelcast/util/AtomicInt.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export	
#endif 

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

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif 

#endif //HAZELCAST_ROUND_ROBIN_LB

