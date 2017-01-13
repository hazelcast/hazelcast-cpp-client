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
// Created by sancar koyunlu on 22/01/14.
//

#include "hazelcast/client/InitialMembershipEvent.h"
#include "hazelcast/client/Cluster.h"


namespace hazelcast {
    namespace client {

        InitialMembershipEvent::InitialMembershipEvent(Cluster &cluster, const std::vector<Member> &members)
        : members(members)
        , cluster(cluster) {

        }

        const std::vector<Member> &InitialMembershipEvent::getMembers() const {
            return members;
        }

        Cluster &InitialMembershipEvent::getCluster() {
            return cluster;
        }
    }
}
