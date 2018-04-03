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

#ifndef HAZELCAST_CLIENT_SPI_IMPL_LISTENER_ABSTRACTCLIENTLISTERNERSERVICE_H_
#define HAZELCAST_CLIENT_SPI_IMPL_LISTENER_ABSTRACTCLIENTLISTERNERSERVICE_H_

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

#include <stdint.h>
#include "hazelcast/util/SynchronizedMap.h"
#include "hazelcast/util/ILogger.h"
#include "hazelcast/util/impl/SimpleExecutorService.h"
#include "hazelcast/client/spi/ClientListenerService.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class SerializationService;
            }
        }
        namespace connection {
            class Connection;
        }
        namespace protocol {
            class ClientMessage;
        }
        namespace spi {
            class ClientContext;

            namespace impl {
                namespace listener {
                    class HAZELCAST_API AbstractClientListenerService : public ClientListenerService {
                    public:
                        AbstractClientListenerService(ClientContext &clientContext, int32_t eventThreadCount,
                                                      int32_t eventQueueCapacity);

                        virtual void start();

                        void shutdown();

                        void addEventHandler(int64_t callId,
                                             const boost::shared_ptr<EventHandler<protocol::ClientMessage> > &handler);

                        void handleClientMessage(const boost::shared_ptr<protocol::ClientMessage> &clientMessage,
                                                 const boost::shared_ptr<connection::Connection> &connection);

                    protected:
                        class ClientEventProcessor : public util::StripedRunnable {
                            friend class AbstractClientListenerService;
                        public:
                            virtual void run();

                            virtual const std::string getName() const;

                            virtual int32_t getKey();

                        private:
                            ClientEventProcessor(const boost::shared_ptr<protocol::ClientMessage> &clientMessage,
                                                 const boost::shared_ptr<connection::Connection> &connection,
                                                 util::SynchronizedMap<int64_t, EventHandler<protocol::ClientMessage> > &eventHandlerMap,
                                                 util::ILogger &logger);

                            const boost::shared_ptr<protocol::ClientMessage> clientMessage;
                            const boost::shared_ptr<connection::Connection> connection;
                            util::SynchronizedMap<int64_t, EventHandler<protocol::ClientMessage> > &eventHandlerMap;
                            util::ILogger &logger;
                        };

                        void removeEventHandler(int64_t callId);

                        util::SynchronizedMap<int64_t, EventHandler<protocol::ClientMessage> > eventHandlerMap;
                        ClientContext &clientContext;
                        serialization::pimpl::SerializationService &serializationService;
                        util::ILogger &logger;
                        util::impl::SimpleExecutorService eventExecutor;
                        util::impl::SimpleExecutorService registrationExecutor;
                    };
                }
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif // HAZELCAST_CLIENT_SPI_IMPL_LISTENER_ABSTRACTCLIENTLISTERNERSERVICE_H_
