/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
//
// Created by sancar koyunlu on 6/3/13.


#ifndef HAZELCAST_PARTITION_SERVICE
#define HAZELCAST_PARTITION_SERVICE

#include "hazelcast/util/ThreadArgs.h"
#include "hazelcast/client/Address.h"
#include "hazelcast/util/AtomicInt.h"
#include "hazelcast/util/AtomicBoolean.h"
#include "hazelcast/util/Mutex.h"

#include <map>
#include <boost/shared_ptr.hpp>

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
        namespace protocol {
            class ClientMessage;
        }

        namespace serialization {
            namespace pimpl {
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

                /**
                 * Refreshes the partition
                 */
                void refreshPartitions();

                // Wakes up the partition thread and triggers a partition refresh
                void wakeup();

                int getPartitionCount();

            private:

                spi::ClientContext &clientContext;

                util::AtomicBoolean updating;

                std::auto_ptr<util::Thread> partitionListenerThread;

                util::AtomicInt partitionCount;

                std::auto_ptr<std::map<int, boost::shared_ptr<Address> > > partitions;

                util::Mutex lock;

                static void staticRunListener(util::ThreadArgs& args);

                void runListener(util::Thread* currentThread);

                std::auto_ptr<protocol::ClientMessage> getPartitionsFrom(const Address &address);

                std::auto_ptr<protocol::ClientMessage> getPartitionsFrom();

                bool processPartitionResponse(protocol::ClientMessage &response);

                bool getInitialPartitions();
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_PARTITION_SERVICE

