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
/**
 * This has to be the first include, so that Python.h is the first include. Otherwise, compilation warning such as
 * "_POSIX_C_SOURCE" redefined occurs.
 */
#include "HazelcastServer.h"

#include "ClientTestSupport.h"

#include <hazelcast/client/HazelcastClient.h>
#include <hazelcast/client/connection/ClientConnectionManagerImpl.h>

namespace hazelcast {
    namespace client {
        namespace test {
            class ClientServiceTest : public ClientTestSupport {
            protected:
                class DummyEntryListener : public EntryListener<std::string, std::string> {
                public:
                    virtual void entryAdded(const EntryEvent<string, string> &event) {

                    }

                    virtual void entryRemoved(const EntryEvent<string, string> &event) {

                    }

                    virtual void entryUpdated(const EntryEvent<string, string> &event) {

                    }

                    virtual void entryEvicted(const EntryEvent<string, string> &event) {

                    }

                    virtual void entryExpired(const EntryEvent<string, string> &event) {

                    }

                    virtual void entryMerged(const EntryEvent<string, string> &event) {

                    }

                    virtual void mapEvicted(const MapEvent &event) {

                    }

                    virtual void mapCleared(const MapEvent &event) {

                    }
                };
            };

            TEST_F(ClientServiceTest, testPendingEventPacketsWithEvents) {
                HazelcastServer hazelcastInstance(*g_srvFactory);

                ClientConfig config;
                HazelcastClient client(config);
                IMap<std::string, std::string> map = client.getMap<std::string, std::string>(randomMapName());
                DummyEntryListener entryListener;
                string listenerId = map.addEntryListener(entryListener, false);
                for (int i = 0; i < 10; i++) {
                    map.put(randomString(), randomString());
                }
                spi::ClientContext context(client);
                connection::ClientConnectionManagerImpl &connectionManager = context.getConnectionManager();
                boost::shared_ptr<connection::Connection> connection = connectionManager.getActiveConnection(
                        Address("localhost", 5701));
                ASSERT_EQ_EVENTUALLY(0, connection->getPendingPacketCount());
                
                ASSERT_TRUE(map.removeEntryListener(listenerId));
            }

        }
    }
}


