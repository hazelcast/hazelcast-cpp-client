//
// Created by sancar koyunlu on 6/3/13.
// Copyright (c) 2013 hazelcast. All rights reserved.

#ifndef HAZELCAST_PARTITION_SERVICE
#define HAZELCAST_PARTITION_SERVICE

#include "../Address.h"
#include "../../util/ConcurrentMap.h"
#include "../../util/Thread.h"

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

            class PartitionService {
            public:
                PartitionService(ClusterService&, serialization::SerializationService&);

                void start();

                void refreshPartitions();


                Address *getPartitionOwner(int partitionId);

                int getPartitionId(serialization::Data& key);


            private:
                util::Thread partitionListenerThread;
                util::Thread refreshPartitionThread;

                ClusterService& clusterService;
                serialization::SerializationService& serializationService;

                volatile int partitionCount;

                util::ConcurrentMap<int, Address> partitions;

                static void *startListener(void *parameteres);

                static void *startRefresher(void *parameteres);

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
