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

#ifndef HAZELCAST_CLIENT_SPI_IMPL_LISTENER_NONSMARTCLIENTLISTERNERSERVICE_H_
#define HAZELCAST_CLIENT_SPI_IMPL_LISTENER_NONSMARTCLIENTLISTERNERSERVICE_H_

#include <set>

#include "hazelcast/client/connection/ConnectionListener.h"
#include "hazelcast/client/spi/impl/listener/AbstractClientListenerService.h"
#include "hazelcast/client/spi/impl/listener/ClientRegistrationKey.h"
#include "hazelcast/client/spi/impl/listener/ClientEventRegistration.h"

namespace hazelcast {
    namespace client {
        namespace connection {
            class Connection;

            class ClientConnectionManagerImpl;
        }

        namespace spi {
            namespace impl {
                namespace listener {
                    class NonSmartClientListenerService
                            : public AbstractClientListenerService,
                              public connection::ConnectionListener,
                              public boost::enable_shared_from_this<NonSmartClientListenerService> {
                    public:
                        NonSmartClientListenerService(ClientContext &clientContext, int32_t eventThreadCount,
                                                      int32_t eventQueueCapacity);

                        void start();

                        virtual std::string
                        registerListener(const boost::shared_ptr<impl::ListenerMessageCodec> &listenerMessageCodec,
                                         const boost::shared_ptr<EventHandler<protocol::ClientMessage> > &handler);

                        virtual bool deregisterListener(const std::string &registrationId);

                        virtual void connectionAdded(const boost::shared_ptr<connection::Connection> &connection);

                        virtual void connectionRemoved(const boost::shared_ptr<connection::Connection> &connection);

                    protected:
                        virtual std::string
                        registerListenerInternal(const boost::shared_ptr<ListenerMessageCodec> &listenerMessageCodec,
                                                 const boost::shared_ptr<EventHandler<protocol::ClientMessage> > &handler);

                        virtual bool deregisterListenerInternal(const std::string &userRegistrationId);

                        virtual void
                        connectionAddedInternal(const boost::shared_ptr<connection::Connection> &connection);

                    private:
                        typedef util::SynchronizedMap<ClientRegistrationKey, ClientEventRegistration> RegistrationsMap;

                        boost::shared_ptr<ClientEventRegistration> invoke(const ClientRegistrationKey &registrationKey);

                        RegistrationsMap activeRegistrations;
                        std::set<ClientRegistrationKey> userRegistrations;
                    };
                }
            }
        }
    }
}

#endif // HAZELCAST_CLIENT_SPI_IMPL_LISTENER_NONSMARTCLIENTLISTERNERSERVICE_H_
