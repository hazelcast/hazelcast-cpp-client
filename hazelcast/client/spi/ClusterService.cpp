//
// Created by sancar koyunlu on 5/21/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.


#include "ClusterService.h"

namespace hazelcast {
    namespace client {
        namespace spi {
            ClusterService::ClusterService(hazelcast::client::HazelcastClient & client)
            : client(client)
            , credentials(client.getClientConfig().getCredentials()) {

            }

            hazelcast::client::connection::ConnectionManager& ClusterService::getConnectionManager() {
                return client.getConnectionManager();
            }

            hazelcast::client::serialization::SerializationService & ClusterService::getSerializationService() {
                return client.getSerializationService();
            }


        }
    }
}
