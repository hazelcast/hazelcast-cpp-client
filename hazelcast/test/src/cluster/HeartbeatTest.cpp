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
/**
 * This has to be the first include, so that Python.h is the first include. Otherwise, compilation warning such as
 * "_POSIX_C_SOURCE" redefined occurs.
 */
#include "HazelcastServerFactory.h"

#include "ClientTestSupport.h"
#include "HazelcastServer.h"
#include "hazelcast/client/HazelcastClient.h"

namespace hazelcast {
    namespace client {
        namespace test {
            class HeartbeatTest : public ClientTestSupport {
            };

            TEST_F(HeartbeatTest, testPing) {
                HazelcastServer instance(*g_srvFactory);
                std::auto_ptr<ClientConfig> config = getConfig();
                config->setProperty("hazelcast_client_heartbeat_interval", "1");

                HazelcastClient client(*config);

                // sleep enough time so that the client ping is sent to the server
                util::sleep(3);
            }
        }
    }
}
