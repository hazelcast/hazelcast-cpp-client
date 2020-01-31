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
#include <cassert>

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
                        std::shared_ptr<impl::ClientInvocation> invocation,
                        std::shared_ptr<connection::Connection> connection) {
                    assert(connection.get() != NULL);
                    send(invocation, connection);
                }

                void NonSmartClientInvocationService::invokeOnPartitionOwner(
                        std::shared_ptr<impl::ClientInvocation> invocation, int partitionId) {
                    invocation->getClientMessage()->setPartitionId(partitionId);
                    send(invocation, getOwnerConnection());
                }

                void NonSmartClientInvocationService::invokeOnRandomTarget(
                        std::shared_ptr<impl::ClientInvocation> invocation) {
                    send(invocation, getOwnerConnection());
                }

                void
                NonSmartClientInvocationService::invokeOnTarget(std::shared_ptr<impl::ClientInvocation> invocation,
                                                                const std::shared_ptr<Address> &target) {
                    send(invocation, getOwnerConnection());
                }

                std::shared_ptr<connection::Connection> NonSmartClientInvocationService::getOwnerConnection() {
                    std::shared_ptr<Address> ownerConnectionAddress = connectionManager->getOwnerConnectionAddress();
                    if (ownerConnectionAddress.get() == NULL) {
                        throw exception::IOException("NonSmartClientInvocationService::getOwnerConnection",
                                                     "Owner connection address is not available.");
                    }
                    std::shared_ptr<connection::Connection> ownerConnection = connectionManager->getActiveConnection(
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
