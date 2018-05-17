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

#include "hazelcast/client/impl/RoundRobinLB.h"
#include "hazelcast/client/exception/IOException.h"
#include "hazelcast/client/Cluster.h"

namespace hazelcast {
    namespace client {
        namespace impl {
            RoundRobinLB::RoundRobinLB():index(0) {

            }

            void RoundRobinLB::init(Cluster &cluster) {
                AbstractLoadBalancer::init(cluster);
            }

            const Member RoundRobinLB::next() {
                std::vector<Member> members = getMembers();
                if (members.size() == 0) {
                    throw exception::IllegalStateException("const Member& RoundRobinLB::next()", "No member in member list!!");
                }
                return members[++index % members.size()];
            }

            RoundRobinLB::RoundRobinLB(const RoundRobinLB &rhs) : index(const_cast<RoundRobinLB &>(rhs).index.get()) {
            }

        }
    }
}
