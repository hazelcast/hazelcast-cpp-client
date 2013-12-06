//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.

#ifndef HAZELCAST_INVOCATION_SERVICE
#define HAZELCAST_INVOCATION_SERVICE

#include "hazelcast/client/spi/ClusterService.h"
#include "hazelcast/client/spi/PartitionService.h"

namespace hazelcast {
    namespace client {
        namespace spi {

            class HAZELCAST_API InvocationService {
            public:
                InvocationService(ClusterService &clusterService, PartitionService &partitionService);

                template<typename Response, typename Request >
                boost::shared_ptr<Response> invokeOnRandomTarget(const Request &request) {
                    return clusterService.sendAndReceive<Response>(request);
                };


                template<typename Response, typename Request >
                boost::shared_ptr<Response> invokeOnKeyOwner(const Request &request, serialization::Data &key) {
                    Address *owner = partitionService.getPartitionOwner(partitionService.getPartitionId(key));
                    if (owner != NULL) {
                        return invokeOnTarget<Response>(request, *owner);
                    }
                    return invokeOnRandomTarget<Response>(request);
                };

                template<typename Response, typename Request >
                boost::shared_ptr<Response> invokeOnTarget(const Request &request, const Address &target) {
                    return clusterService.sendAndReceive<Response>(target, request);
                };

                template<typename Request, typename  ResponseHandler>
                void invokeOnRandomTarget(const Request &request, ResponseHandler &handler) {
                    clusterService.sendAndHandle(request, handler);
                };

                template<typename Request, typename  ResponseHandler>
                void invokeOnKeyOwner(const Request &request, serialization::Data &key, ResponseHandler &handler) {
                    Address *owner = partitionService.getPartitionOwner(partitionService.getPartitionId(key));
                    if (owner != NULL) {
                        invokeOnTarget(request, *owner, handler);
                    }
                    invokeOnRandomTarget(request, handler);
                };

                template<typename Request, typename  ResponseHandler>
                void invokeOnTarget(const Request &request, const Address &target, ResponseHandler &handler) {
                    clusterService.sendAndHandle(target, request, handler);
                }

            private :

                ClusterService &clusterService;
                PartitionService &partitionService;
            };

        }
    }
}


#endif //__InvocationService_H_
