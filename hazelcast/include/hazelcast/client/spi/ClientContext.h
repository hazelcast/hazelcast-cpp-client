//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#include "LifecycleService.h"

#ifndef HAZELCAST_CLIENT_CONTEXT
#define HAZELCAST_CLIENT_CONTEXT

namespace hazelcast {
    namespace client {

        namespace serialization {
            class SerializationService;
        }
        class HazelcastClient;

        class ClientConfig;

        namespace connection {
            class ConnectionManager;
        }

        namespace spi {
            class InvocationService;

            class ClusterService;

            class PartitionService;

            class ServerListenerService;

            class DistributedObjectListenerService;

            class ClientContext {
            public:

                ClientContext(HazelcastClient& hazelcastClient);

                serialization::SerializationService& getSerializationService();

                ClusterService& getClusterService();

                InvocationService& getInvocationService();

                ClientConfig& getClientConfig();

                PartitionService& getPartitionService();

                LifecycleService& getLifecycleService();

                ServerListenerService& getServerListenerService();

                connection::ConnectionManager& getConnectionManager();

                DistributedObjectListenerService& getDistributedObjectListenerService();

            private:
                HazelcastClient& hazelcastClient;
            };
        }
    }
}

#endif