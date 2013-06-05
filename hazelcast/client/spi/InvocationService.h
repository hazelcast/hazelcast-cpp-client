//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.

#ifndef HAZELCAST_INVOCATION_SERVICE
#define HAZELCAST_INVOCATION_SERVICE

#include "ClusterService.h"
#include "PartitionService.h"

namespace hazelcast {
    namespace client {
        namespace spi {

            class InvocationService {
            public:
                InvocationService(ClusterService& clusterService, PartitionService& partitionService);

                template<typename Response, typename Request >
                Response invokeOnRandomTarget(const Request& request) {
                    return clusterService.sendAndReceive(request);
                };


                template<typename Response, typename Request >
                Response invokeOnKeyOwner(const Request& request, hazelcast::client::serialization::Data& key) {
                    Address *owner = partitionService.getPartitionOwner(partitionService.getPartitionId(key));
                    if (owner != NULL) {
                        return invokeOnTarget<Response>(request, *owner);
                    }
                    return invokeOnRandomTarget<Response>(request);
                };

            private :
                template<typename Response, typename Request >
                Response invokeOnTarget(const Request& request, const Address& target) {
                    return clusterService.sendAndReceive<Response>(target, request);
                };

                ClusterService& clusterService;
                PartitionService& partitionService;
            };

        }
    }
}


#endif //__InvocationService_H_
