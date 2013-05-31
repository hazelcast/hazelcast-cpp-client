//
// Created by sancar koyunlu on 5/31/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "Cluster.h"
#include "ClusterService.h"
#include "MembershipListener.h"

namespace hazelcast {
    namespace client {
        Cluster::Cluster(spi::ClusterService & clusterService)
        :clusterService(clusterService) {

        };

        void Cluster::addMembershipListener(MembershipListener *listener) {
            clusterService.addMembershipListener(listener);
        };

        bool Cluster::removeMembershipListener(MembershipListener *listener) {
            return clusterService.removeMembershipListener(listener);
        };

        vector<hazelcast::client::connection::Member>  Cluster::getMembers() {
            return clusterService.getMemberList();
        };


    }
}