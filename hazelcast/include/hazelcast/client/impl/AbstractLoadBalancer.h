/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
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

#ifndef HAZELCAST_ABSTRACT_LOAD_BALANCER
#define HAZELCAST_ABSTRACT_LOAD_BALANCER

#include "hazelcast/client/Member.h"
#include "hazelcast/client/MembershipListener.h"
#include "hazelcast/client/LoadBalancer.h"
#include "hazelcast/util/Mutex.h"

#include <vector>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export	
#endif 

namespace hazelcast {
    namespace client {
        class Cluster;

        namespace impl {
            class HAZELCAST_API AbstractLoadBalancer : public LoadBalancer, public MembershipListener {
            public:
                AbstractLoadBalancer();

                AbstractLoadBalancer(AbstractLoadBalancer &rhs);

                void setMembersRef();

                std::vector<Member> getMembers();

                virtual void init(Cluster &cluster);

                void memberAdded(const MembershipEvent &membershipEvent);

                void memberRemoved(const MembershipEvent &membershipEvent);

                void memberAttributeChanged(const MemberAttributeEvent &memberAttributeEvent);

                virtual ~AbstractLoadBalancer();

            private:
                util::Mutex membersLock;
                std::vector<Member> membersRef;
                Cluster *cluster;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif 

#endif //HAZELCAST_LOAD_BALANCER

