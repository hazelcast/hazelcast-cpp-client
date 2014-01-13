//
// Created by sancar koyunlu on 5/31/13.
// Copyright (c) 2013 hazelcast. All rights reserved.

#include "hazelcast/client/impl/AbstractLoadBalancer.h"
#include "hazelcast/client/Cluster.h"
#include <boost/thread/lock_guard.hpp>

namespace hazelcast {
    namespace client {
        namespace connection {
            class MembershipEvent;
        }
        namespace impl {

            void AbstractLoadBalancer::init(hazelcast::client::Cluster &cluster) {
                this->cluster = &cluster;
                setMembersRef();
            };

            void AbstractLoadBalancer::setMembersRef() {
                boost::lock_guard<boost::mutex> lg(membersLock);
                membersRef = cluster->getMembers();
            };

            void AbstractLoadBalancer::memberAdded(const hazelcast::client::connection::MembershipEvent& membershipEvent) {
                setMembersRef();
            };

            void AbstractLoadBalancer::memberRemoved(const hazelcast::client::connection::MembershipEvent& membershipEvent) {
                setMembersRef();
            };

            std::vector<connection::Member>  AbstractLoadBalancer::getMembers() {
                boost::lock_guard<boost::mutex> lg(membersLock);
                return membersRef;
            };

            AbstractLoadBalancer::~AbstractLoadBalancer() {

            };
        }
    }
}