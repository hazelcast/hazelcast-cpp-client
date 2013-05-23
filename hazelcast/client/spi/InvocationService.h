//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.

#ifndef HAZELCAST_INVOCATION_SERVICE
#define HAZELCAST_INVOCATION_SERVICE

#include "ClusterService.h"

namespace hazelcast {
    namespace client {
        namespace spi {

            class InvocationService {
            public:
                InvocationService(ClusterService& clusterService);

                template<typename Request, typename Response>
                void invokeOnRandomTarget(const Request& request, Response& response) {
                    clusterService.sendAndReceive(request, response);
                };


                template<typename Request, typename Response>
                void invokeOnKeyOwner(const Request& request, Response& response, const hazelcast::client::serialization::Data&) {
                    //    ClientPartitionServiceImpl partitionService = (ClientPartitionServiceImpl) client.getClientPartitionService();
                    //    final Address owner = partitionService.getPartitionOwner(partitionService.getPartitionId(key));
                    //    if (owner != null) {
                    //        return invokeOnTarget(request, owner);
                    //    }
                    //    return invokeOnRandomTarget(request);
                };

            private :
                template<typename Request, typename Response>
                void invokeOnTarget(const Request& request, Response& response, const Address& target) {
                    clusterService.sendAndReceive(target, request, response);
                };

                ClusterService& clusterService;
            };

        }
    }
}


#endif //__InvocationService_H_
