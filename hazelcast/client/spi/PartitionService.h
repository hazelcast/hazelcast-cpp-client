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
                PartitionService(ClusterService&, hazelcast::client::serialization::SerializationService&);

                void start();

                void refreshPartitions();


                Address *getPartitionOwner(int partitionId);

                int getPartitionId(hazelcast::client::serialization::Data& key);


            private:
                hazelcast::util::Thread partitionListenerThread;
                hazelcast::util::Thread refreshPartitionThread;

                ClusterService& clusterService;
                serialization::SerializationService& serializationService;

                volatile int partitionCount;

                hazelcast::util::ConcurrentMap<int, Address> partitions;

                static void *startListener(void *parameteres);

                static void *startRefresher(void *parameteres);

                void runListener();

                void runRefresher();

                hazelcast::client::impl::PartitionsResponse getPartitionsFrom(const Address& address);

                void processPartitionResponse(hazelcast::client::impl::PartitionsResponse& response);

                void getInitialPartitions();
            };
        }
    }
}

#endif //HAZELCAST_PARTITION_SERVICE
