//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/spi/InvocationService.h"

namespace hazelcast {
    namespace client {
        namespace spi {
            InvocationService::InvocationService(ClusterService & clusterService, PartitionService& partitionService)
            : clusterService(clusterService)
            , partitionService(partitionService) {

            }

        }
    }
}