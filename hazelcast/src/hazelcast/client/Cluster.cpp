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



#include "hazelcast/client/Cluster.h"
#include "hazelcast/client/spi/ClientClusterService.h"
#include "hazelcast/client/MembershipListener.h"
#include "hazelcast/client/InitialMembershipListener.h"

namespace hazelcast {
    namespace client {
        Cluster::Cluster(spi::ClientClusterService &clusterService)
        :clusterService(clusterService) {
        }

        void Cluster::addMembershipListener(MembershipListener *listener) {
            clusterService.addMembershipListener(listener);
        }

        bool Cluster::removeMembershipListener(MembershipListener *listener) {
            return clusterService.removeMembershipListener(listener->getRegistrationId());
        }

        std::vector<Member>  Cluster::getMembers() {
            return clusterService.getMemberList();
        }

        std::string Cluster::addMembershipListener(const boost::shared_ptr<MembershipListener> &listener) {
            return clusterService.addMembershipListener(listener);
        }

        bool Cluster::removeMembershipListener(const std::string &registrationId) {
            return clusterService.removeMembershipListener(registrationId);
        }
    }
}
