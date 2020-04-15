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

#ifndef HAZELCAST_CLIENT_SPI_IMPL_LISTENER_ABSTRACTCLIENTLISTERNERSERVICE_H_
#define HAZELCAST_CLIENT_SPI_IMPL_LISTENER_ABSTRACTCLIENTLISTERNERSERVICE_H_

#include <boost/asio/thread_pool.hpp>
#include <boost/asio/strand.hpp>

#include <stdint.h>
#include "hazelcast/client/connection/ConnectionListener.h"
#include "hazelcast/util/SynchronizedMap.h"
#include "hazelcast/util/ILogger.h"
#include "hazelcast/client/spi/ClientListenerService.h"
#include "hazelcast/client/spi/impl/listener/ClientEventRegistration.h"
#include "hazelcast/client/spi/impl/listener/ClientRegistrationKey.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class SerializationService;
            }
        }
        namespace connection {
            class Connection;

            class ClientConnectionManagerImpl;
        }
        namespace protocol {
            class ClientMessage;
        }
        namespace spi {
            class ClientContext;

            namespace impl {
                namespace listener {
                    class AbstractClientListenerService
                            : public ClientListenerService,
                              public connection::ConnectionListener,
                              public std::enable_shared_from_this<AbstractClientListenerService> {
                    public:
                        virtual ~AbstractClientListenerService();

                        virtual void start();

                        virtual void shutdown();

                        void handleClientMessage(const std::shared_ptr<ClientInvocation> invocation,
                                                 const std::shared_ptr<protocol::ClientMessage> response);

                        virtual std::string
                        registerListener(const std::shared_ptr<impl::ListenerMessageCodec> listenerMessageCodec,
                                         const std::shared_ptr<EventHandler<protocol::ClientMessage> > handler);

                        virtual bool deregisterListener(const std::string registrationId);

                        virtual void connectionAdded(const std::shared_ptr<connection::Connection> connection);

                        virtual void connectionRemoved(const std::shared_ptr<connection::Connection> connection);

                    protected:
                        AbstractClientListenerService(ClientContext &clientContext, int32_t eventThreadCount);

                        void processEventMessage(const std::shared_ptr<ClientInvocation> invocation,
                                                 const std::shared_ptr<protocol::ClientMessage> response);

                        virtual bool registersLocalOnly() const = 0;

                        struct ConnectionPointerLessComparator {
                            bool operator()(const std::shared_ptr<connection::Connection> &lhs,
                                            const std::shared_ptr<connection::Connection> &rhs) const;
                        };

                        typedef std::map<std::shared_ptr<connection::Connection>, ClientEventRegistration, ConnectionPointerLessComparator> ConnectionRegistrationsMap;
                        typedef util::SynchronizedMap<ClientRegistrationKey, ConnectionRegistrationsMap> RegistrationsMap;

                        void removeEventHandler(const ClientEventRegistration &registration);

                        virtual std::string
                        registerListenerInternal(const std::shared_ptr<ListenerMessageCodec> &listenerMessageCodec,
                                                 const std::shared_ptr<EventHandler<protocol::ClientMessage> > &handler);

                        virtual bool deregisterListenerInternal(const std::string &userRegistrationId);

                        virtual void
                        connectionAddedInternal(const std::shared_ptr<connection::Connection> &connection);

                        virtual void
                        connectionRemovedInternal(const std::shared_ptr<connection::Connection> &connection);


                        void invoke(const ClientRegistrationKey &registrationKey,
                                    const std::shared_ptr<connection::Connection> &connection);

                        ClientContext &clientContext;
                        serialization::pimpl::SerializationService &serializationService;
                        util::ILogger &logger;
                        connection::ClientConnectionManagerImpl &clientConnectionManager;
                        boost::asio::thread_pool eventExecutor;
                        std::vector<boost::asio::thread_pool::executor_type> eventStrands;
                        boost::asio::thread_pool registrationExecutor;
                        int64_t invocationTimeoutMillis;
                        int64_t invocationRetryPauseMillis;
                        RegistrationsMap registrations;

                    private:
                        void invokeFromInternalThread(const ClientRegistrationKey &registrationKey,
                                                      const std::shared_ptr<connection::Connection> &connection);
                    };
                }
            }
        }
    }
}

#endif // HAZELCAST_CLIENT_SPI_IMPL_LISTENER_ABSTRACTCLIENTLISTERNERSERVICE_H_
