//
// Created by sancar koyunlu on 5/31/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/Cluster.h"
#include "hazelcast/client/spi/ClusterService.h"
#include "hazelcast/client/MembershipListener.h"
#include "hazelcast/client/InitialMembershipListener.h"

namespace hazelcast {
    namespace client {
        Cluster::Cluster(spi::ClusterService &clusterService)
        :clusterService(clusterService) {

        };

        void Cluster::addMembershipListener(InitialMembershipListener *listener) {
            clusterService.addMembershipListener(listener);
        }

        bool Cluster::removeMembershipListener(InitialMembershipListener *listener) {
            return clusterService.removeMembershipListener(listener);;
        }

        void Cluster::addMembershipListener(MembershipListener *listener) {
            clusterService.addMembershipListener(listener);
        };

        bool Cluster::removeMembershipListener(MembershipListener *listener) {
            return clusterService.removeMembershipListener(listener);
        };

        std::vector<Member>  Cluster::getMembers() {
            return clusterService.getMemberList();
        };
    }
}
