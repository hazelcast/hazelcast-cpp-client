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

#include "hazelcast/client/spi/impl/NonSmartClientInvocationService.h"
#include "hazelcast/client/connection/ClientConnectionManagerImpl.h"
#include "hazelcast/client/protocol/ClientMessage.h"

namespace hazelcast {
    namespace client {
        namespace spi {
            namespace impl {

                NonSmartClientInvocationService::NonSmartClientInvocationService(ClientContext &client)
                        : AbstractClientInvocationService(client) {

                }

                void NonSmartClientInvocationService::invokeOnConnection(
                        boost::shared_ptr<impl::ClientInvocation> invocation,
                        boost::shared_ptr<connection::Connection> connection) {
                    assert(connection.get() != NULL);
                    send(invocation, connection);
                }

                void NonSmartClientInvocationService::invokeOnPartitionOwner(
                        boost::shared_ptr<impl::ClientInvocation> invocation, int partitionId) {
                    invocation->getClientMessage()->setPartitionId(partitionId);
                    send(invocation, getOwnerConnection());
                }

                void NonSmartClientInvocationService::invokeOnRandomTarget(
                        boost::shared_ptr<impl::ClientInvocation> invocation) {
                    send(invocation, getOwnerConnection());
                }

                void
                NonSmartClientInvocationService::invokeOnTarget(boost::shared_ptr<impl::ClientInvocation> invocation,
                                                                const boost::shared_ptr<Address> &target) {
                    send(invocation, getOwnerConnection());
                }

                boost::shared_ptr<connection::Connection> NonSmartClientInvocationService::getOwnerConnection() {
                    const boost::shared_ptr<Address> &ownerConnectionAddress = connectionManager->getOwnerConnectionAddress();
                    boost::shared_ptr<connection::Connection> ownerConnection = connectionManager->getActiveConnection(
                            *ownerConnectionAddress);
                    if (ownerConnection.get() == NULL) {
                        throw exception::IOException("NonSmartClientInvocationService::getOwnerConnection",
                                                     "Owner connection is not available.");
                    }
                    return ownerConnection;
                }
            }
        }
    }
}
