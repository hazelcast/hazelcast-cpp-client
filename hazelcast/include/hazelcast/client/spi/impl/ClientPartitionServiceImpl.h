/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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

#ifndef HAZELCAST_CLIENT_SPI_IMPL_CLIENTPARTITIONSERVICEIMPL_H_
#define HAZELCAST_CLIENT_SPI_IMPL_CLIENTPARTITIONSERVICEIMPL_H_

#include <atomic>

#include "hazelcast/util/SynchronizedMap.h"
#include "hazelcast/util/Runnable.h"
#include "hazelcast/util/AtomicInt.h"
#include "hazelcast/client/spi/ClientPartitionService.h"
#include "hazelcast/client/spi/EventHandler.h"
#include "hazelcast/client/ExecutionCallback.h"
#include "hazelcast/client/impl/Partition.h"
#include "hazelcast/util/ILogger.h"
#include "hazelcast/client/protocol/codec/ClientAddPartitionListenerCodec.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace connection {
            class Connection;
        }
        namespace protocol {
            class ClientMessage;
        }
        namespace spi {
            class ClientContext;

            class ClientExecutionService;

            namespace impl {
                class ClientExecutionServiceImpl;

                class HAZELCAST_API ClientPartitionServiceImpl : public ClientPartitionService,
                                                                 public std::enable_shared_from_this<ClientPartitionServiceImpl>,
                                                                 public protocol::codec::ClientAddPartitionListenerCodec::AbstractEventHandler {
                public:
                    ClientPartitionServiceImpl(ClientContext &client,
                                               hazelcast::client::spi::impl::ClientExecutionServiceImpl &executionService);

                    void start();

                    void stop();

                    void listenPartitionTable(const std::shared_ptr<connection::Connection> &ownerConnection);

                    void refreshPartitions();

                    virtual void
                    handlePartitionsEventV15(const std::vector<std::pair<Address, std::vector<int32_t> > > &partitions,
                                     const int32_t &partitionStateVersion);

                    virtual void beforeListenerRegister();

                    virtual void onListenerRegister();

                    virtual std::shared_ptr<Address> getPartitionOwner(int partitionId);

                    virtual int getPartitionId(const serialization::pimpl::Data &key);

                    virtual int getPartitionCount();

                    virtual std::shared_ptr<client::impl::Partition> getPartition(int partitionId);

                private:
                    class PartitionImpl : public client::impl::Partition {
                    public:
                        PartitionImpl(int partitionId, ClientContext &client,
                                      ClientPartitionServiceImpl &partitionService);

                        virtual int getPartitionId() const;

                        virtual std::shared_ptr<Member> getOwner() const;

                    private:
                        int partitionId;
                        ClientContext &client;
                        ClientPartitionServiceImpl &partitionService;
                    };

                    class RefreshTask : public util::Runnable {
                    public:
                        RefreshTask(ClientContext &client, ClientPartitionServiceImpl &partitionService);

                        virtual void run();

                        virtual const std::string getName() const;

                    private:
                        ClientContext &client;
                        ClientPartitionServiceImpl &partitionService;
                    };

                    class RefreshTaskCallback
                            : public client::ExecutionCallback<protocol::ClientMessage> {
                    public:
                        RefreshTaskCallback(ClientPartitionServiceImpl &partitionService);

                        virtual void onResponse(const std::shared_ptr<protocol::ClientMessage> &responseMessage);

                        virtual void onFailure(const std::shared_ptr<exception::IException> &e);

                    private:
                        ClientPartitionServiceImpl &partitionService;
                    };

                    bool processPartitionResponse(
                            const std::vector<std::pair<Address, std::vector<int32_t> > > &partitions,
                            int32_t partitionStateVersion, bool partitionStateVersionExist);

                    ClientContext &client;
                    util::ILogger &logger;
                    ClientExecutionService &clientExecutionService;
                    std::shared_ptr<client::ExecutionCallback<protocol::ClientMessage> > refreshTaskCallback;

                    static const int64_t PERIOD = 10 * 1000;
                    static const int64_t INITIAL_DELAY = 10 * 1000;

                    util::SynchronizedMap<int, Address> partitions;
                    std::atomic<int32_t> partitionCount;
                    std::atomic<int32_t> lastPartitionStateVersion;
                    util::Mutex lock;

                    void waitForPartitionsFetchedOnce();

                    bool isClusterFormedByOnlyLiteMembers();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_CLIENT_SPI_IMPL_CLIENTPARTITIONSERVICEIMPL_H_
