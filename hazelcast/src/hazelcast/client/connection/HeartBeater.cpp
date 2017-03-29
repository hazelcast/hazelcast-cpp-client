/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
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
// Created by sancar koyunlu on 20/08/14.
//

#include "hazelcast/client/connection/HeartBeater.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/util/ThreadArgs.h"
#include "hazelcast/client/spi/InvocationService.h"
#include "hazelcast/client/connection/ConnectionManager.h"
#include "hazelcast/client/connection/CallFuture.h"
#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/client/ClientProperties.h"
#include "hazelcast/util/IOUtil.h"
#include "hazelcast/util/Thread.h"
#include "hazelcast/client/protocol/codec/ClientPingCodec.h"

#include <ctime>

namespace hazelcast {
    namespace client {
        namespace connection {

            HeartBeater::HeartBeater(spi::ClientContext& clientContext)
            : live(true)
            , clientContext(clientContext) {
                ClientProperties& properties = clientContext.getClientProperties();
                heartBeatTimeoutSeconds = properties.getHeartbeatTimeout().getInteger();
                heartBeatIntervalSeconds = properties.getHeartbeatInterval().getInteger();
                if (heartBeatTimeoutSeconds <= 0) {
                    heartBeatTimeoutSeconds = util::IOUtil::to_value<int>((std::string)ClientProperties::PROP_HEARTBEAT_TIMEOUT_DEFAULT);
                }
                if (heartBeatIntervalSeconds <= 0) {
                    heartBeatIntervalSeconds = util::IOUtil::to_value<int>((std::string)ClientProperties::PROP_HEARTBEAT_INTERVAL_DEFAULT);
                }
            }

            void HeartBeater::staticStart(util::ThreadArgs& args) {
                HeartBeater *heartBeater = (HeartBeater *)args.arg0;
                heartBeater->run(args.currentThread);
            }

            void HeartBeater::run(util::Thread *currentThread) {
                currentThread->interruptibleSleep(heartBeatIntervalSeconds);

                connection::ConnectionManager& connectionManager = clientContext.getConnectionManager();
                while (live) {
                    std::vector<hazelcast::util::SharedPtr<Connection> > connections = connectionManager.getConnections();
                    std::vector<hazelcast::util::SharedPtr<Connection> >::iterator it;

                    time_t now = time(NULL);
                    for (it = connections.begin(); it != connections.end(); ++it) {
                        hazelcast::util::SharedPtr<Connection> connection = *it;

                        time_t lastReadTime = connection->lastRead;

                        if (now - lastReadTime > heartBeatTimeoutSeconds) {
                            connection->heartBeatingFailed();
                        }

                        if (now - lastReadTime > heartBeatIntervalSeconds) {
                            std::auto_ptr<protocol::ClientMessage> request = protocol::codec::ClientPingCodec::RequestParameters::encode();

                            clientContext.getInvocationService().invokeOnConnection(request, connection);
                        } else {
                            connection->heartBeatingSucceed();
                        }
                    }

                    currentThread->interruptibleSleep(heartBeatIntervalSeconds);
                }
            }

            void HeartBeater::shutdown() {
                live = false;
            }
        }
    }
}
