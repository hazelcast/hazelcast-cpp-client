//
// Created by sancar koyunlu on 5/21/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.


#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/client/protocol/AuthenticationRequest.h"
#include "hazelcast/client/spi/ClusterService.h"
#include "hazelcast/client/connection/SmartConnectionManager.h"

namespace hazelcast {
    namespace client {
        namespace connection {
            SmartConnectionManager::SmartConnectionManager(spi::ClusterService &clusterService, serialization::SerializationService &serializationService, ClientConfig &clientConfig)
            :ConnectionManager(clusterService, serializationService, clientConfig) {

            };
        }
    }
}
