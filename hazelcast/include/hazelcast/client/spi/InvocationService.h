//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.

#ifndef HAZELCAST_INVOCATION_SERVICE
#define HAZELCAST_INVOCATION_SERVICE

#include "hazelcast/util/HazelcastDll.h"
#include <boost/thread/future.hpp>

namespace hazelcast {
    namespace client {
        class Address;
        namespace serialization {
            class Data;
        }
        namespace impl {
            class PortableRequest;
        }
        namespace spi {

            class ClusterService;

            class PartitionService;

            class HAZELCAST_API InvocationService {
            public:
                InvocationService(ClusterService &clusterService, PartitionService &partitionService);

                boost::shared_future<serialization::Data> invokeOnRandomTarget(const impl::PortableRequest &request);

                boost::shared_future<serialization::Data> invokeOnKeyOwner(const impl::PortableRequest &request, serialization::Data &key);

                boost::shared_future<serialization::Data> invokeOnTarget(const impl::PortableRequest &request, const Address &target);

//                template<typename Request, typename  ResponseHandler>
//                boost::shared_future<serialization::Data> invokeOnRandomTarget(const Request &request, ResponseHandler &handler) {
//                    clusterService.sendAndHandle(request, handler);
//                };
//
//                template<typename Request, typename  ResponseHandler>
//                boost::shared_future<serialization::Data> invokeOnKeyOwner(const Request &request, serialization::Data &key, ResponseHandler &handler) {
//                    Address *owner = partitionService.getPartitionOwner(partitionService.getPartitionId(key));
//                    if (owner != NULL) {
//                        invokeOnTarget(request, *owner, handler);
//                    }
//                    invokeOnRandomTarget(request, handler);
//                };
//
//                template<typename Request, typename  ResponseHandler>
//                boost::shared_future<serialization::Data> invokeOnTarget(const Request &request, const Address &target, ResponseHandler &handler) {
//                    clusterService.sendAndHandle(target, request, handler);
//                }
            private :
                ClusterService &clusterService;
                PartitionService &partitionService;
            };

        }
    }
}


#endif //__InvocationService_H_
