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

#include "hazelcast/client/spi/impl/SmartClientInvocationService.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/spi/ClientPartitionService.h"
#include "hazelcast/client/spi/ClientClusterService.h"
#include "hazelcast/client/connection/ClientConnectionManagerImpl.h"
#include "hazelcast/client/protocol/ClientExceptionFactory.h"
#include "hazelcast/client/protocol/ClientProtocolErrorCodes.h"

namespace hazelcast {
    namespace client {
        namespace spi {
            namespace impl {

                SmartClientInvocationService::SmartClientInvocationService(ClientContext &client)
                        : AbstractClientInvocationService(client),
                          loadBalancer(*client.getClientConfig().getLoadBalancer()) {}

                void
                SmartClientInvocationService::invokeOnConnection(boost::shared_ptr<impl::ClientInvocation> invocation,
                                                                 boost::shared_ptr<connection::Connection> connection) {
                    send(invocation, connection);
                }

                void SmartClientInvocationService::invokeOnPartitionOwner(
                        boost::shared_ptr<impl::ClientInvocation> invocation, int partitionId) {
                    boost::shared_ptr<Address> owner = partitionService.getPartitionOwner(partitionId);
                    if (owner.get() == NULL) {
                        throw (exception::ExceptionBuilder<exception::IOException>(
                                "SmartClientInvocationService::invokeOnPartitionOwner")
                                << "Partition does not have an owner. partitionId: " << partitionId).build();
                    }
                    if (!isMember(*owner)) {
                        throw (exception::ExceptionBuilder<exception::TargetNotMemberException>(
                                "SmartClientInvocationService::invokeOnPartitionOwner") << "Partition owner '" << *owner
                                                                                        << "' is not a member.").build();
                    }
                    invocation->getClientMessage()->setPartitionId(partitionId);
                    boost::shared_ptr<connection::Connection> connection = getOrTriggerConnect(owner);
                    send(invocation, connection);
                }

                void SmartClientInvocationService::invokeOnRandomTarget(
                        boost::shared_ptr<impl::ClientInvocation> invocation) {
                    boost::shared_ptr<Address> randomAddress = getRandomAddress();
                    if (randomAddress.get() == NULL) {
                        throw (exception::ExceptionBuilder<exception::IOException>(
                                "SmartClientInvocationService::invokeOnRandomTarget")
                                << "No address found to invoke").build();
                    }
                    boost::shared_ptr<connection::Connection> connection = getOrTriggerConnect(randomAddress);
                    send(invocation, connection);
                }

                void SmartClientInvocationService::invokeOnTarget(boost::shared_ptr<impl::ClientInvocation> invocation,
                                                                  const boost::shared_ptr<Address> &target) {
                    if (!isMember(*target)) {
                        throw (exception::ExceptionBuilder<exception::TargetNotMemberException>(
                                "SmartClientInvocationService::invokeOnTarget")
                                << "Target '" << *target << "' is not a member.").build();
                    }
                    boost::shared_ptr<connection::Connection> connection = getOrTriggerConnect(target);
                    invokeOnConnection(invocation, connection);

                }

                bool SmartClientInvocationService::isMember(const Address &target) const {
                    boost::shared_ptr<Member> member = client.getClientClusterService().getMember(target);
                    return member.get() != NULL;
                }

                boost::shared_ptr<connection::Connection>
                SmartClientInvocationService::getOrTriggerConnect(const boost::shared_ptr<Address> &target) const {
                    boost::shared_ptr<connection::Connection> connection = connectionManager->getOrTriggerConnect(
                            *target);
                    if (connection.get() == NULL) {
                        throw (exception::ExceptionBuilder<exception::IOException>(
                                "SmartClientInvocationService::getOrTriggerConnect")
                                << "No available connection to address " << target).build();
                    }
                    return connection;
                }

                boost::shared_ptr<Address> SmartClientInvocationService::getRandomAddress() {
                    // TODO: Change loadBalancer to return pointer as in Java so that it is possible to denote the
                    // case where no address can be provided
                    Member member = loadBalancer.next();
                    return boost::shared_ptr<Address>(new Address(member.getAddress()));
                }
            }
        }
    }
}
