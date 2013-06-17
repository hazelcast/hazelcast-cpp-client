//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "ClientContext.h"
#include "../HazelcastClient.h"
#include "../ClientConfig.h"

using namespace hazelcast::client;

namespace hazelcast {
    namespace client {
        namespace spi {
            ClientContext::ClientContext(HazelcastClient& hazelcastClient)
            : hazelcastClient(hazelcastClient) {

            };

            serialization::SerializationService& ClientContext::getSerializationService() {
                return hazelcastClient.getSerializationService();
            };


            ClusterService & ClientContext::getClusterService() {
                return hazelcastClient.getClusterService();
            };

            InvocationService & ClientContext::getInvocationService() {
                return hazelcastClient.getInvocationService();
            };

            ClientConfig& ClientContext::getClientConfig() {
                return hazelcastClient.getClientConfig();
            };

            PartitionService & ClientContext::getPartitionService() {
                return hazelcastClient.getPartitionService();
            };

            connection::ConnectionManager & ClientContext::getConnectionManager() {
                return hazelcastClient.getConnectionManager();
            }


        }

    }
}