//
// Created by sancar koyunlu on 6/3/13.
// Copyright (c) 2013 hazelcast. All rights reserved.

#ifndef HAZELCAST_PARTITION_SERVICE
#define HAZELCAST_PARTITION_SERVICE

#include "hazelcast/client/Address.h"
#include "hazelcast/util/SynchronizedMap.h"
#include "hazelcast/util/AtomicInt.h"
#include "hazelcast/util/AtomicBoolean.h"
#include "hazelcast/util/Mutex.h"
#include "hazelcast/util/Thread.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class SerializationService;

                class Data;
            }
        }
        namespace impl {
            class PartitionsResponse;
        }

        namespace spi {
            class ClientContext;

            class HAZELCAST_API PartitionService {
            public:
                PartitionService(spi::ClientContext &clientContext);

                bool start();

                void stop();

                boost::shared_ptr<Address> getPartitionOwner(int partitionId);

                int getPartitionId(const serialization::pimpl::Data &key);

            private:

                spi::ClientContext &clientContext;

                util::AtomicBoolean updating;

                std::auto_ptr<util::Thread> partitionListenerThread;

                util::AtomicInt partitionCount;

                util::SynchronizedMap<int, Address> partitions;

                util::Mutex startLock;

                static void staticRunListener(util::ThreadArgs& args);

                void runListener();

                void runRefresher();

                boost::shared_ptr<impl::PartitionsResponse> getPartitionsFrom(const Address &address);

                boost::shared_ptr<impl::PartitionsResponse> getPartitionsFrom();

                void processPartitionResponse(impl::PartitionsResponse &response);

                bool getInitialPartitions();
            };
        }
    }
}

#endif //HAZELCAST_PARTITION_SERVICE

