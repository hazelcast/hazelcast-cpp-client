//
// Created by sancar koyunlu on 5/31/13.
// Copyright (c) 2013 hazelcast. All rights reserved.

#include "hazelcast/client/impl/AbstractLoadBalancer.h"
#include "hazelcast/client/Cluster.h"
#include "hazelcast/util/LockGuard.h"

namespace hazelcast {
    namespace client {
        namespace impl {
            class ClientMembershipEvent;

            void AbstractLoadBalancer::init(Cluster &cluster) {
                this->cluster = &cluster;
                setMembersRef();
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
        }
    }
}
