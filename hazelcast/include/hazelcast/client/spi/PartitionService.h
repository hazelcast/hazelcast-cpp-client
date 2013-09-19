//
// Created by sancar koyunlu on 6/3/13.
// Copyright (c) 2013 hazelcast. All rights reserved.

#ifndef HAZELCAST_PARTITION_SERVICE
#define HAZELCAST_PARTITION_SERVICE

#include "../Address.h"
#include "ConcurrentMap.h"
#include <boost/thread.hpp>

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

            class PartitionService {
            public:
                PartitionService(ClusterService&, serialization::SerializationService&, spi::LifecycleService&);

                ~PartitionService();

                void start();

                void stop();

                void refreshPartitions();

                Address *getPartitionOwner(int partitionId);

                int getPartitionId(const serialization::Data& key);

            private:

                ClusterService& clusterService;
                serialization::SerializationService& serializationService;
                spi::LifecycleService& lifecycleService;

                boost::mutex refreshLock;

                std::auto_ptr<boost::thread> partitionListenerThread;

                volatile int partitionCount;

                util::ConcurrentMap<int, Address> partitions;

                void runListener();

                void runRefresher();

                impl::PartitionsResponse getPartitionsFrom(const Address& address);

                impl::PartitionsResponse getPartitionsFrom();

                void processPartitionResponse(impl::PartitionsResponse& response);

                void getInitialPartitions();
            };
        }
    }
}

#endif //HAZELCAST_PARTITION_SERVICE
