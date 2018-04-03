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

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

#include <set>

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
                    class HAZELCAST_API NonSmartClientListenerService : public AbstractClientListenerService {
                    public:
                        NonSmartClientListenerService(ClientContext &clientContext, int32_t eventThreadCount,
                                                   int32_t eventQueueCapacity);

                        virtual std::string
                        registerListener(const boost::shared_ptr<impl::ListenerMessageCodec> &listenerMessageCodec,
                                         const boost::shared_ptr<EventHandler<protocol::ClientMessage> > &handler);

                        virtual bool deregisterListener(const std::string &registrationId);

                    private:
                        typedef util::SynchronizedMap<ClientRegistrationKey, ClientEventRegistration> RegistrationsMap;

                        class RegisterListenerTask : public util::Callable<std::string> {
                        public:
                            RegisterListenerTask(RegistrationsMap &activeRegistrations,
                                                 std::set<ClientRegistrationKey> &userRegistrations,
                                                 const boost::shared_ptr<ListenerMessageCodec> &listenerMessageCodec,
                                                 const boost::shared_ptr<EventHandler<protocol::ClientMessage> > &handler,
                                                 NonSmartClientListenerService &listenerService);

                            virtual std::string call();

                            virtual const std::string getName() const;

                        private:
                            RegistrationsMap &activeRegistrations;
                            std::set<ClientRegistrationKey> &userRegistrations;
                            const boost::shared_ptr<impl::ListenerMessageCodec> listenerMessageCodec;
                            const boost::shared_ptr<EventHandler<protocol::ClientMessage> > handler;
                            NonSmartClientListenerService &listenerService;
                        };

                        boost::shared_ptr<ClientEventRegistration> invoke(const ClientRegistrationKey &registrationKey);

                        RegistrationsMap activeRegistrations;
                        std::set<ClientRegistrationKey> userRegistrations;
                    };
                }
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif // HAZELCAST_CLIENT_SPI_IMPL_LISTENER_NONSMARTCLIENTLISTERNERSERVICE_H_
