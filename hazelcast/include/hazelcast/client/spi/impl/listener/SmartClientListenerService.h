/*
 * Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
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

#ifndef HAZELCAST_CLIENT_SPI_IMPL_LISTENER_SMARTCLIENTLISTERNERSERVICE_H_
#define HAZELCAST_CLIENT_SPI_IMPL_LISTENER_SMARTCLIENTLISTERNERSERVICE_H_

#include "hazelcast/client/spi/impl/listener/AbstractClientListenerService.h"
#include "hazelcast/client/spi/EventHandler.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            class ClientMessage;
        }

        namespace spi {
            namespace impl {
                namespace listener {
                    class SmartClientListenerService : public AbstractClientListenerService {
                    public:
                        SmartClientListenerService(ClientContext &clientContext, int32_t eventThreadCount);

                        void start() override;

                        void shutdown() override;

                        std::string
                        registerListener(const std::shared_ptr<impl::ListenerMessageCodec> listenerMessageCodec,
                                         const std::shared_ptr<EventHandler < protocol::ClientMessage>>

                        handler)
                        override;

                        void asyncConnectToAllMembersInternal();

                    private:
                        std::shared_ptr<boost::asio::steady_timer> timer;

                        bool registersLocalOnly() const override;

                        void trySyncConnectToAllMembers();

                        void timeOutOrSleepBeforeNextTry(int64_t startMillis, const Member &lastFailedMember,
                                                         std::exception_ptr lastException);

                        void
                        throwOperationTimeoutException(int64_t startMillis, int64_t nowInMillis, int64_t elapsedMillis,
                                                       const Member &lastFailedMember,
                                                       std::exception_ptr lastException);

                        void sleepBeforeNextTry();

                        std::shared_ptr<boost::asio::steady_timer> scheduleConnectToAllMembers();
                    };
                }
            }
        }
    }
}

#endif // HAZELCAST_CLIENT_SPI_IMPL_LISTENER_SMARTCLIENTLISTERNERSERVICE_H_
