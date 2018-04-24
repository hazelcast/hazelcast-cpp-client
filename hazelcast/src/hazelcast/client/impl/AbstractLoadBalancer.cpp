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


#include "hazelcast/client/impl/AbstractLoadBalancer.h"
#include "hazelcast/client/Cluster.h"
#include "hazelcast/util/LockGuard.h"

namespace hazelcast {
    namespace client {
        namespace impl {
            AbstractLoadBalancer::AbstractLoadBalancer(AbstractLoadBalancer &rhs) {
                util::LockGuard lg(rhs.membersLock);
                membersRef = rhs.membersRef;
                cluster = rhs.cluster;
            }

            void AbstractLoadBalancer::init(Cluster &cluster) {
                this->cluster = &cluster;
                setMembersRef();
                cluster.addMembershipListener(this);
            }

            void AbstractLoadBalancer::setMembersRef() {
                util::LockGuard lg(membersLock);
                membersRef = cluster->getMembers();
            }

            void AbstractLoadBalancer::memberAdded(const MembershipEvent &membershipEvent) {
                setMembersRef();
            }

            void AbstractLoadBalancer::memberRemoved(const MembershipEvent &membershipEvent) {
                setMembersRef();
            }

            void AbstractLoadBalancer::memberAttributeChanged(const MemberAttributeEvent &memberAttributeEvent) {
            }

            std::vector<Member>  AbstractLoadBalancer::getMembers() {
                util::LockGuard lg(membersLock);
                return membersRef;
            }

            AbstractLoadBalancer::~AbstractLoadBalancer() {
            }

            AbstractLoadBalancer::AbstractLoadBalancer() : cluster(NULL) {
            }

            void AbstractLoadBalancer::init(const InitialMembershipEvent &event) {
                setMembersRef();
            }
        }
    }
}
