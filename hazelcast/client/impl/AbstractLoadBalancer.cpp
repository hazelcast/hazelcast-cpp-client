//
// Created by sancar koyunlu on 5/31/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "AbstractLoadBalancer.h"
#include "../Cluster.h"

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
                std::vector<hazelcast::client::connection::Member> memberSet = cluster->getMembers();
                std::vector<connection::Member> *members = new std::vector<hazelcast::client::connection::Member>;
                for (std::vector<connection::Member>::iterator it = members->begin(); it != memberSet.end(); ++it) {
                    members->push_back((*it));
                }
                membersRef.set(members);
            };

            void AbstractLoadBalancer::memberAdded(const hazelcast::client::connection::MembershipEvent& membershipEvent) {
                setMembersRef();
            };

            void AbstractLoadBalancer::memberRemoved(const hazelcast::client::connection::MembershipEvent& membershipEvent) {
                setMembersRef();
            };

            std::vector<hazelcast::client::connection::Member> AbstractLoadBalancer::getMembers() {
                return *membersRef.get();
            };

            AbstractLoadBalancer::~AbstractLoadBalancer() {

            };
        }
    }
}