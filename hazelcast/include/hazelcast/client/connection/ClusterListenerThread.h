//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.

#ifndef HAZELCAST_CLUSTER_LISTENER_THREAD
#define HAZELCAST_CLUSTER_LISTENER_THREAD

#include "Member.h"
#include <boost/atomic.hpp>

namespace hazelcast {
    namespace client {

        class ClientConfig;

        namespace spi {
            class ClusterService;

            class LifecycleService;
        }

        namespace connection {

            class Connection;

            class ConnectionManager;

            class MembershipEvent;

            class ClusterListenerThread {
            public:
                ClusterListenerThread(ConnectionManager&, ClientConfig& clientConfig, spi::ClusterService&, spi::LifecycleService&, serialization::SerializationService&);

                void setInitialConnection(connection::Connection *);

                void run();

                boost::atomic<bool> isReady;
            private:
                ConnectionManager& connectionManager;
                spi::ClusterService& clusterService;
                spi::LifecycleService& lifecycleService;
                serialization::SerializationService& serializationService;

                Connection *conn;
                std::vector<Member> members;
                ClientConfig& clientConfig;

                Connection *pickConnection();

                void loadInitialMemberList();

                void listenMembershipEvents();

                void updateMembersRef();

                std::vector<Address> getClusterAddresses() const;

                std::vector<Address> getConfigAddresses() const;
            };
        }
    }
}

#endif //HAZELCAST_CLUSTER_LISTENER_THREAD
