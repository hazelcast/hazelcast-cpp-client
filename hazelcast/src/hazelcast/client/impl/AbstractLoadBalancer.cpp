//
// Created by sancar koyunlu on 5/31/13.
// Copyright (c) 2013 hazelcast. All rights reserved.

#include "hazelcast/client/impl/AbstractLoadBalancer.h"
#include "hazelcast/client/Cluster.h"
#include <boost/thread/lock_guard.hpp>

namespace hazelcast {
    namespace client {
        namespace impl {
            class ClientMembershipEvent;

            void AbstractLoadBalancer::init(Cluster &cluster) {
                this->cluster = &cluster;
                setMembersRef();
            };

            void AbstractLoadBalancer::setMembersRef() {
                boost::lock_guard<boost::mutex> lg(membersLock);
                membersRef = cluster->getMembers();
            };

            void AbstractLoadBalancer::memberAdded(const MembershipEvent &membershipEvent) {
                setMembersRef();
            };

            void AbstractLoadBalancer::memberRemoved(const MembershipEvent &membershipEvent) {
                setMembersRef();
            };


            void AbstractLoadBalancer::memberAttributeChanged(const MemberAttributeEvent &memberAttributeEvent) {

            }

            std::vector<Member>  AbstractLoadBalancer::getMembers() {
                boost::lock_guard<boost::mutex> lg(membersLock);
                return membersRef;
            };

            AbstractLoadBalancer::~AbstractLoadBalancer() {

            };
        }
    }
}