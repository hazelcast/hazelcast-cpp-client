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

#pragma once

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
        namespace impl {
            class BaseEventHandler;
        }
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

                        virtual boost::future<std::string>
                        registerListener(std::unique_ptr<ListenerMessageCodec> &&listenerMessageCodec,
                                         std::unique_ptr<client::impl::BaseEventHandler> &&handler);

                        virtual boost::future<bool> deregisterListener(const std::string registrationId);

                        virtual void connectionAdded(const std::shared_ptr<connection::Connection> connection);

                        virtual void connectionRemoved(const std::shared_ptr<connection::Connection> connection);

                    protected:
                        AbstractClientListenerService(ClientContext &clientContext, int32_t eventThreadCount);

                        void processEventMessage(const std::shared_ptr<ClientInvocation> invocation,
                                                 const std::shared_ptr<protocol::ClientMessage> response);

                        virtual bool registersLocalOnly() const = 0;

                        typedef std::unordered_map<std::shared_ptr<connection::Connection>, ClientEventRegistration> ConnectionRegistrationsMap;
                        typedef util::SynchronizedMap<ClientRegistrationKey, ConnectionRegistrationsMap> RegistrationsMap;

                        void removeEventHandler(const ClientEventRegistration &registration);

                        virtual std::string
                        registerListenerInternal(std::unique_ptr<impl::ListenerMessageCodec> &&listenerMessageCodec,
                                                 std::unique_ptr<client::impl::BaseEventHandler> &&handler);

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
                        std::unique_ptr<hazelcast::util::hz_thread_pool> eventExecutor;
                        std::vector<boost::asio::thread_pool::executor_type> eventStrands;
                        std::unique_ptr<hazelcast::util::hz_thread_pool> registrationExecutor;
                        std::chrono::steady_clock::duration invocationTimeout;
                        std::chrono::steady_clock::duration invocationRetryPause;
                        RegistrationsMap registrations;
                        int numberOfEventThreads;

                    private:
                        void invokeFromInternalThread(const ClientRegistrationKey &registrationKey,
                                                      const std::shared_ptr<connection::Connection> &connection);
                    };
                }
            }
        }
    }
}


