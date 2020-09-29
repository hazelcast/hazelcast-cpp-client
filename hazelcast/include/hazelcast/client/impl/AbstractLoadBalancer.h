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
#pragma once

#include "hazelcast/client/Member.h"
#include "hazelcast/client/LoadBalancer.h"
#include <mutex>

#include <vector>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export	
#endif 

namespace hazelcast {
    namespace client {
        class Cluster;

        namespace impl {
            class HAZELCAST_API AbstractLoadBalancer : public LoadBalancer {
            public:
                AbstractLoadBalancer();

                AbstractLoadBalancer(const AbstractLoadBalancer &rhs);

                void operator=(const AbstractLoadBalancer &rhs);

                void setMembersRef();

                std::vector<Member> getMembers();

                void init(Cluster &cluster) override;

                ~AbstractLoadBalancer() override;

            private:
                mutable std::mutex members_lock_;
                std::vector<Member> members_ref_;
                Cluster *cluster_;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif 



