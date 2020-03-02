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

#include "hazelcast/util/Runnable.h"
#include "hazelcast/util/HashUtil.h"
#include "hazelcast/client/spi/impl/ClientPartitionServiceImpl.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/spi/LifecycleService.h"
#include "hazelcast/client/spi/impl/ClientInvocation.h"
#include "hazelcast/client/spi/impl/ClientClusterServiceImpl.h"
#include "hazelcast/client/spi/impl/ClientExecutionServiceImpl.h"
#include "hazelcast/client/connection/ClientConnectionManagerImpl.h"
#include "hazelcast/client/protocol/codec/ClientGetPartitionsCodec.h"
#include "hazelcast/client/impl/BuildInfo.h"

namespace hazelcast {
    namespace client {
        namespace spi {
            namespace impl {
                ClientPartitionServiceImpl::RefreshTaskCallback::RefreshTaskCallback(
                        ClientPartitionServiceImpl &partitionService) : partitionService(partitionService) {}

                void ClientPartitionServiceImpl::RefreshTaskCallback::onResponse(
                        const std::shared_ptr<protocol::ClientMessage> &responseMessage) {
                    if (!responseMessage.get()) {
                        return;
                    }
                    protocol::codec::ClientGetPartitionsCodec::ResponseParameters response =
                            protocol::codec::ClientGetPartitionsCodec::ResponseParameters::decode(*responseMessage);
                    partitionService.processPartitionResponse(response.partitions, response.partitionStateVersion,
                                                              response.partitionStateVersionExist);
                }

                void ClientPartitionServiceImpl::RefreshTaskCallback::onFailure(
                        const std::shared_ptr<exception::IException> &t) {
                    if (partitionService.client.getLifecycleService().isRunning()) {
                        partitionService.logger.warning() << "Error while fetching cluster partition table! Cause:"
                                                          << *t;
                    }
                }

                ClientPartitionServiceImpl::ClientPartitionServiceImpl(ClientContext &client,
                                                                       hazelcast::client::spi::impl::ClientExecutionServiceImpl &executionService)
                        : client(client), logger(client.getLogger()),
                          clientExecutionService(executionService),
                          refreshTaskCallback(new RefreshTaskCallback(*this)), partitionCount(0),
                          lastPartitionStateVersion(0) {
                }

                bool ClientPartitionServiceImpl::processPartitionResponse(
                        const std::vector<std::pair<Address, std::vector<int32_t> > > &partitions,
                        int32_t partitionStateVersion, bool partitionStateVersionExist) {
                    {
                        util::LockGuard guard(lock);
                        if (!partitionStateVersionExist || partitionStateVersion > lastPartitionStateVersion) {
                            typedef std::vector<std::pair<Address, std::vector<int32_t> > > PARTITION_VECTOR;
                            for (const PARTITION_VECTOR::value_type &entry : partitions) {
                                            const Address &address = entry.first;
                                            for (const std::vector<int32_t>::value_type &partition : entry.second) {
                                                            this->partitions.put(partition, std::shared_ptr<Address>(
                                                                    new Address(address)));
                                                        }
                                        }
                            partitionCount = this->partitions.size();
                            lastPartitionStateVersion = partitionStateVersion;
                            if (logger.isFinestEnabled()) {
                                logger.finest() << "Processed partition response. partitionStateVersion : "
                                                << (partitionStateVersionExist ? util::IOUtil::to_string<int32_t>(
                                                        partitionStateVersion) : "NotAvailable")
                                                << ", partitionCount :" << (int) partitionCount;
                            }
                        }
                    }
                    return partitionCount > 0;
                }

                void ClientPartitionServiceImpl::start() {
                    //scheduling left in place to support server versions before 3.9.
                    clientExecutionService.scheduleWithRepetition(
                            std::shared_ptr<util::Runnable>(new RefreshTask(client, *this)), INITIAL_DELAY, PERIOD);
                }

                void ClientPartitionServiceImpl::listenPartitionTable(
                        const std::shared_ptr<connection::Connection> &ownerConnection) {
                    //when we connect to cluster back we need to reset partition state version
                    lastPartitionStateVersion = -1;
                    if (ownerConnection->getConnectedServerVersion() >=
                        client::impl::BuildInfo::calculateVersion("3.9")) {
                        //Servers after 3.9 supports listeners
                        std::unique_ptr<protocol::ClientMessage> clientMessage =
                                protocol::codec::ClientAddPartitionListenerCodec::encodeRequest();
                        std::shared_ptr<ClientInvocation> invocation = ClientInvocation::create(client, clientMessage,
                                                                                                  "", ownerConnection);
                        invocation->setEventHandler(shared_from_this());
                        invocation->invokeUrgent()->get();
                    }
                }

                void ClientPartitionServiceImpl::refreshPartitions() {
                    try {
                        // use internal execution service for all partition refresh process (do not use the user executor thread)
                        clientExecutionService.execute(
                                std::shared_ptr<util::Runnable>(new RefreshTask(client, *this)));
                    } catch (exception::RejectedExecutionException &) {
                        // ignore
                    }
                }

                void ClientPartitionServiceImpl::handlePartitionsEventV15(
                        const std::vector<std::pair<Address, std::vector<int32_t> > > &collection,
                        const int32_t &partitionStateVersion) {
                    processPartitionResponse(collection, partitionStateVersion, true);
                }

                void ClientPartitionServiceImpl::beforeListenerRegister() {
                }

                void ClientPartitionServiceImpl::onListenerRegister() {
                }

                std::shared_ptr<Address> ClientPartitionServiceImpl::getPartitionOwner(int partitionId) {
                    waitForPartitionsFetchedOnce();
                    return partitions.get(partitionId);
                }

                int ClientPartitionServiceImpl::getPartitionId(const serialization::pimpl::Data &key) {
                    int pc = getPartitionCount();
                    if (pc <= 0) {
                        return 0;
                    }
                    int hash = key.getPartitionHash();
                    return util::HashUtil::hashToIndex(hash, pc);
                }

                int ClientPartitionServiceImpl::getPartitionCount() {
                    waitForPartitionsFetchedOnce();
                    return partitionCount;
                }

                std::shared_ptr<client::impl::Partition> ClientPartitionServiceImpl::getPartition(int partitionId) {
                    return std::shared_ptr<client::impl::Partition>(new PartitionImpl(partitionId, client, *this));
                }

                void ClientPartitionServiceImpl::waitForPartitionsFetchedOnce() {
                    while (partitionCount == 0 && client.getConnectionManager().isAlive()) {
                        if (isClusterFormedByOnlyLiteMembers()) {
                            throw exception::NoDataMemberInClusterException(
                                    "ClientPartitionServiceImpl::waitForPartitionsFetchedOnce",
                                    "Partitions can't be assigned since all nodes in the cluster are lite members");
                        }
                        std::unique_ptr<protocol::ClientMessage> requestMessage = protocol::codec::ClientGetPartitionsCodec::encodeRequest();
                        std::shared_ptr<ClientInvocation> invocation = ClientInvocation::create(client,
                                                                                                  requestMessage, "");
                        std::shared_ptr<ClientInvocationFuture> future = invocation->invokeUrgent();
                        try {
                            std::shared_ptr<protocol::ClientMessage> responseMessage = future->get();
                            protocol::codec::ClientGetPartitionsCodec::ResponseParameters response =
                                    protocol::codec::ClientGetPartitionsCodec::ResponseParameters::decode(
                                            *responseMessage);
                            processPartitionResponse(response.partitions,
                                                     response.partitionStateVersion,
                                                     response.partitionStateVersionExist);
                        } catch (exception::IException &e) {
                            if (client.getLifecycleService().isRunning()) {
                                logger.warning() << "Error while fetching cluster partition table!" << e;
                            }
                        }
                    }
                }

                bool ClientPartitionServiceImpl::isClusterFormedByOnlyLiteMembers() {
                    ClientClusterService &clusterService = client.getClientClusterService();
                    for (const std::vector<Member>::value_type &member : clusterService.getMemberList()) {
                                    if (!member.isLiteMember()) {
                                        return false;
                                    }
                                }
                    return true;
                }

                void ClientPartitionServiceImpl::stop() {
                    partitions.clear();
                }

                void ClientPartitionServiceImpl::RefreshTask::run() {
                    try {
                        connection::ClientConnectionManagerImpl &connectionManager = client.getConnectionManager();
                        std::shared_ptr<connection::Connection> connection = connectionManager.getOwnerConnection();
                        if (!connection.get()) {
                            return;
                        }
                        std::unique_ptr<protocol::ClientMessage> requestMessage = protocol::codec::ClientGetPartitionsCodec::encodeRequest();
                        std::shared_ptr<ClientInvocation> invocation = ClientInvocation::create(client,
                                                                                                  requestMessage, "");
                        std::shared_ptr<ClientInvocationFuture> future = invocation->invokeUrgent();
                        future->andThen(partitionService.refreshTaskCallback);
                    } catch (exception::IException &e) {
                        if (client.getLifecycleService().isRunning()) {
                            partitionService.logger.warning(
                                    std::string("Error while fetching cluster partition table! ") + e.what());
                        }
                    }
                }

                const std::string ClientPartitionServiceImpl::RefreshTask::getName() const {
                    return "ClientPartitionServiceImpl::RefreshTask";
                }

                ClientPartitionServiceImpl::RefreshTask::RefreshTask(ClientContext &client,
                                                                     ClientPartitionServiceImpl &partitionService)
                        : client(client), partitionService(partitionService) {}


                int ClientPartitionServiceImpl::PartitionImpl::getPartitionId() const {
                    return partitionId;
                }

                std::shared_ptr<Member> ClientPartitionServiceImpl::PartitionImpl::getOwner() const {
                    std::shared_ptr<Address> owner = partitionService.getPartitionOwner(partitionId);
                    if (owner.get()) {
                        return client.getClientClusterService().getMember(*owner);
                    }
                    return std::shared_ptr<Member>();
                }

                ClientPartitionServiceImpl::PartitionImpl::PartitionImpl(int partitionId, ClientContext &client,
                                                                         ClientPartitionServiceImpl &partitionService)
                        : partitionId(partitionId), client(client), partitionService(partitionService) {}
            }
        }
    }
}
