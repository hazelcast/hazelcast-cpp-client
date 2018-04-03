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

#ifndef HAZELCAST_CLIENT_SPI_IMPL_CONNECTIONHEARBEATLISTENER_H
#define HAZELCAST_CLIENT_SPI_IMPL_CONNECTIONHEARBEATLISTENER_H


#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/util/HazelcastDll.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace spi {
            namespace impl {
                /**
                 * A listener for the {@link connection::ClientConnectionManager} to listen to connection heartbeats.
                 */
                class HAZELCAST_API ConnectionHeartbeatListener {
                public:
                    /**
                     * This event will be fired when the heartbeat is resumed for a connection to a member.
                     */
                    virtual void heartbeatResumed(const boost::shared_ptr<connection::Connection> &connection) = 0;

                    /**
                     * This event will be fired when no heartbeat response is received for
                     * {@link ClientProperty#HEARTBEAT_TIMEOUT} milliseconds from the member.
                     */
                    virtual void heartbeatStopped(const boost::shared_ptr<connection::Connection> &connection) = 0;
                };

                class HAZELCAST_API ConnectionHeartbeatListenerDelegator : public ConnectionHeartbeatListener {
                public:
                    ConnectionHeartbeatListenerDelegator(ConnectionHeartbeatListener &listener) : listener(listener) {}

                    virtual void heartbeatResumed(const boost::shared_ptr<connection::Connection> &connection) {
                        listener.heartbeatResumed(connection);
                    }

                    virtual void heartbeatStopped(const boost::shared_ptr<connection::Connection> &connection) {
                        listener.heartbeatStopped(connection);
                    }

                private:
                    ConnectionHeartbeatListener &listener;
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_CLIENT_SPI_IMPL_CONNECTIONHEARBEATLISTENER_H
