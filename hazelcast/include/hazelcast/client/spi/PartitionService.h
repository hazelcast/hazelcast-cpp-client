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

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace util{
        class Thread;

        class ThreadArgs;
    }

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

                void shutdown();

                boost::shared_ptr<Address> getPartitionOwner(int partitionId);

                int getPartitionId(const serialization::pimpl::Data &key);

            private:

                spi::ClientContext &clientContext;

                hazelcast::util::AtomicBoolean updating;

                std::auto_ptr<hazelcast::util::Thread> partitionListenerThread;

                hazelcast::util::AtomicInt partitionCount;

                hazelcast::util::SynchronizedMap<int, Address> partitions;

                hazelcast::util::Mutex startLock;

                static void staticRunListener(hazelcast::util::ThreadArgs& args);

                void runListener(hazelcast::util::Thread* currentThread);

                void runRefresher();

                boost::shared_ptr<impl::PartitionsResponse> getPartitionsFrom(const Address &address);

                boost::shared_ptr<impl::PartitionsResponse> getPartitionsFrom();

                void processPartitionResponse(impl::PartitionsResponse &response);

                bool getInitialPartitions();
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_PARTITION_SERVICE

