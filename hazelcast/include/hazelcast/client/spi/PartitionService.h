//
// Created by sancar koyunlu on 6/3/13.
// Copyright (c) 2013 hazelcast. All rights reserved.

#ifndef HAZELCAST_PARTITION_SERVICE
#define HAZELCAST_PARTITION_SERVICE

#include "hazelcast/client/Address.h"
#include "hazelcast/util/SynchronizedMap.h"
#include <boost/thread.hpp>
#include <boost/atomic.hpp>

namespace hazelcast {
    namespace client {
        namespace serialization {
            class SerializationService;

            class Data;
        }
        namespace impl {
            class PartitionsResponse;
        }

        namespace spi {
            class ClusterService;

            class LifecycleService;

            class InvocationService;

            class HAZELCAST_API PartitionService {
            public:
                PartitionService(ClusterService &,InvocationService&, serialization::SerializationService &, spi::LifecycleService &);

                ~PartitionService();

                void start();

                void stop();

                Address *getPartitionOwner(int partitionId);

                int getPartitionId(const serialization::Data &key);

            private:

                ClusterService &clusterService;
                InvocationService& invocationService;
                serialization::SerializationService &serializationService;
                spi::LifecycleService &lifecycleService;

                boost::atomic<bool> updating;

                std::auto_ptr<boost::thread> partitionListenerThread;

                volatile int partitionCount;

                util::SynchronizedMap<int, Address> partitions;

                void runListener();

                void runRefresher();

                boost::shared_ptr<impl::PartitionsResponse> getPartitionsFrom(const Address &address);

                boost::shared_ptr<impl::PartitionsResponse> getPartitionsFrom();

                void processPartitionResponse(impl::PartitionsResponse &response);

                void getInitialPartitions();
            };
        }
    }
}

#endif //HAZELCAST_PARTITION_SERVICE
