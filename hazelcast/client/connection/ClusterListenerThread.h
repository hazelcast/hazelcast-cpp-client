//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.

#ifndef HAZELCAST_CLUSTER_LISTENER_THREAD
#define HAZELCAST_CLUSTER_LISTENER_THREAD

#include "Member.h"

namespace hazelcast {
    namespace client {

        class ClientConfig;

        namespace spi {
            class ClusterService;
        }

        namespace connection {

            class Connection;

            class ConnectionManager;

            class MembershipEvent;

            class ClusterListenerThread {
            public:
                ClusterListenerThread(ConnectionManager& connMgr, hazelcast::client::ClientConfig& clientConfig, hazelcast::client::spi::ClusterService&);

                void setInitialConnection(hazelcast::client::connection::Connection *);

                static void *run(void *);

            private:
                ConnectionManager& connectionManager;
                hazelcast::client::spi::ClusterService& clusterService;
                Connection *conn;
                std::vector<Member> members;
                hazelcast::client::ClientConfig& clientConfig;

                void runImpl();

                Connection *pickConnection();

                void loadInitialMemberList();

                void listenMembershipEvents();

                void updateMembersRef();

                void fireMembershipEvent(MembershipEvent&);

                std::vector<Address> getClusterAddresses() const;

                std::vector<Address> getConfigAddresses() const;
            };
        }
    }
}

#endif //HAZELCAST_CLUSTER_LISTENER_THREAD
