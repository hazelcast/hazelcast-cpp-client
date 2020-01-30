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
#include "HazelcastServerFactory.h"
#include "ClientTestSupport.h"
#include "HazelcastServer.h"

#include <hazelcast/client/HazelcastClient.h>
#include <hazelcast/client/spi/ClientContext.h>
#include <hazelcast/client/connection/ClientConnectionManagerImpl.h>
#include <hazelcast/client/protocol/Principal.h>
#include <hazelcast/client/connection/Connection.h>

namespace hazelcast {
    namespace client {
        namespace test {
            class ClientEnpointTest : public ClientTestSupport {
            };

            TEST_F(ClientEnpointTest, testConnectedClientEnpoint) {
                HazelcastServer instance(*g_srvFactory);

                HazelcastClient client;
                const Client endpoint = client.getLocalEndpoint();
                spi::ClientContext context(client);
                ASSERT_EQ(context.getName(), endpoint.getName());

                std::shared_ptr<Address> endpointAddress = endpoint.getSocketAddress();
                ASSERT_NOTNULL(endpointAddress.get(), Address);
                connection::ClientConnectionManagerImpl &connectionManager = context.getConnectionManager();
                std::shared_ptr<connection::Connection> connection = connectionManager.getOwnerConnection();
                ASSERT_NOTNULL(connection.get(), connection::Connection);
                std::unique_ptr<Address> localAddress = connection->getLocalSocketAddress();
                ASSERT_NOTNULL(localAddress.get(), Address);
                ASSERT_EQ(*localAddress, *endpointAddress);

                std::shared_ptr<protocol::Principal> principal = connectionManager.getPrincipal();
                ASSERT_NOTNULL(principal.get(), protocol::Principal);
                ASSERT_NOTNULL(principal->getUuid(), std::string);
                ASSERT_EQ_PTR((*principal->getUuid()), endpoint.getUuid().get(), std::string);
            }
        }
    }
}


