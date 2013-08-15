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
                    return clusterService.sendAndReceive<Response>(request);
                };


                template<typename Response, typename Request >
                Response invokeOnKeyOwner(const Request& request, serialization::Data& key) {
                    util::AtomicPointer<Address> owner = partitionService.getPartitionOwner(partitionService.getPartitionId(key));
                    if (owner != NULL) {
                        return invokeOnTarget<Response>(request, *owner);
                    }
                    return invokeOnRandomTarget<Response>(request);
                };

                template<typename Response, typename Request >
                Response invokeOnTarget(const Request& request, const Address& target) {
                    return clusterService.sendAndReceive<Response>(target, request);
                };

                template<typename Request, typename  ResponseHandler>
                void invokeOnRandomTarget(const Request& request, const ResponseHandler& handler) {
                    clusterService.sendAndHandle(request, handler);
                };

                template<typename Request, typename  ResponseHandler>
                void invokeOnKeyOwner(const Request& request, serialization::Data& key, const ResponseHandler&  handler) {
                    util::AtomicPointer<Address> owner = partitionService.getPartitionOwner(partitionService.getPartitionId(key));
                    if (owner != NULL) {
                        invokeOnTarget(request, *owner, handler);
                    }
                    invokeOnRandomTarget(request, handler);
                };

                template<typename Request, typename  ResponseHandler>
                void invokeOnTarget(const Request& request, const Address& target, const ResponseHandler&  handler) {
                    clusterService.sendAndHandle(target, request, handler);
                }

            private :

                ClusterService& clusterService;
                PartitionService& partitionService;
            };

        }
    }
}


#endif //__InvocationService_H_
