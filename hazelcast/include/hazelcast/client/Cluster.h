//
// Created by sancar koyunlu on 5/31/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#ifndef HAZElCAST_CLUSTER
#define HAZElCAST_CLUSTER

#include "hazelcast/client/Member.h"
#include <vector>

namespace hazelcast {
    namespace client {
        namespace spi {
            class ClusterService;
        }

        class MembershipListener;

        class InitialMembershipListener;

        /**
         * Hazelcast cluster interface.
         */
        class HAZELCAST_API Cluster {
        public:
            /**
             * Constructor
             */
            Cluster(spi::ClusterService &clusterService);

            /**
             * Adds InitialMembershipListener to listen for membership updates.
             *
             * Warning 1: If listener should do a time consuming operation, off-load the operation to another thread.
             * otherwise it will slow down the system.
             *
             * Warning 2: Do not make a call to hazelcast. It can cause deadlock.
             *
             * @param listener InitialMembershipListener
             */
            void addMembershipListener(InitialMembershipListener *listener);

            /**
             * Adds InitialMembershipListener to listen for membership updates.
             *
             * Warning 1: If listener should do a time consuming operation, off-load the operation to another thread.
             * otherwise it will slow down the system.
             *
             * Warning 2: Do not make a call to hazelcast. It can cause deadlock.
             *
             * @param listener MembershipListener
             */

            void addMembershipListener(MembershipListener *listener);

            /**
             * Removes the specified membership listener.
             *
             * @param listener InitialMembershipListener * to be removed
             *
             * @return true if registration is removed, false otherwise
             */
            bool removeMembershipListener(InitialMembershipListener *listener);

            /**
             * Removes the specified membership listener.
             *
             * @param listener MembershipListener * to be removed
             *
             * @return true if registration is removed, false otherwise
             */
            bool removeMembershipListener(MembershipListener *listener);

            /**
             * Set of current members of the cluster.
             * Returning set instance is not modifiable.
             * Every member in the cluster has the same member list in the same
             * order. First member is the oldest member.
             *
             * @return current members of the cluster
             */
            std::vector<Member> getMembers();

        private:
            spi::ClusterService &clusterService;
        };
    }
}

#endif //HAZElCAST_CLUSTER

