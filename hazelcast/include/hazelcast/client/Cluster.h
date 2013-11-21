//
// Created by sancar koyunlu on 5/31/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#ifndef HAZElCAST_CLUSTER
#define HAZElCAST_CLUSTER

#include "hazelcast/client/connection/Member.h"
#include <vector>

namespace hazelcast {
    namespace client {
        namespace spi {
            class ClusterService;
        }

        class MembershipListener;

        class Cluster {
        public:
            Cluster(spi::ClusterService& clusterService);

            void addMembershipListener(MembershipListener *listener);

            bool removeMembershipListener(MembershipListener *listener);

            std::vector<hazelcast::client::connection::Member> getMembers();

        private:
            spi::ClusterService& clusterService;
        };
    }
}

#endif //HAZElCAST_CLUSTER
