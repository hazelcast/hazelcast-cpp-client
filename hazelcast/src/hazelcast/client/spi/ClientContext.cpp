//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/HazelcastClient.h"

namespace hazelcast {
    namespace client {
        namespace spi {
            ClientContext::ClientContext(HazelcastClient &hazelcastClient)
            : hazelcastClient(hazelcastClient) {

            };

            serialization::pimpl::SerializationService &ClientContext::getSerializationService() {
                return hazelcastClient.serializationService;
            };

            ClusterService &ClientContext::getClusterService() {
                return hazelcastClient.clusterService;
            };

            InvocationService &ClientContext::getInvocationService() {
                return hazelcastClient.invocationService;
            };

            ClientConfig &ClientContext::getClientConfig() {
                return hazelcastClient.clientConfig;
            };

            PartitionService &ClientContext::getPartitionService() {
                return hazelcastClient.partitionService;
            };

            LifecycleService &ClientContext::getLifecycleService() {
                return hazelcastClient.lifecycleService;
            };

            ServerListenerService &ClientContext::getServerListenerService() {
                return hazelcastClient.serverListenerService;
            };

            connection::ConnectionManager &ClientContext::getConnectionManager() {
                return hazelcastClient.connectionManager;
            };

            Cluster &ClientContext::getCluster() {
                return hazelcastClient.cluster;
            }
        }

    }
}