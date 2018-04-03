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

#ifndef HAZELCAST_CLIENT_SPI_IMPL_LISTENER_SMARTCLIENTLISTERNERSERVICE_H_
#define HAZELCAST_CLIENT_SPI_IMPL_LISTENER_SMARTCLIENTLISTERNERSERVICE_H_

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

#include "hazelcast/util/Callable.h"
#include "hazelcast/client/spi/impl/listener/AbstractClientListenerService.h"
#include "hazelcast/client/spi/impl/listener/ClientRegistrationKey.h"
#include "ClientEventRegistration.h"

namespace hazelcast {
    namespace client {
        namespace connection {
            class Connection;

            class ClientConnectionManagerImpl;
        }

        namespace spi {
            namespace impl {
                namespace listener {
                    class HAZELCAST_API SmartClientListenerService : public AbstractClientListenerService {
                    public:
                        SmartClientListenerService(ClientContext &clientContext, int32_t eventThreadCount,
                                                   int32_t eventQueueCapacity);

                        virtual std::string
                        registerListener(const boost::shared_ptr<impl::ListenerMessageCodec> &listenerMessageCodec,
                                         const boost::shared_ptr<EventHandler<protocol::ClientMessage> > &handler);

                        virtual bool deregisterListener(const std::string &registrationId);

                    private:
                        typedef std::map<boost::shared_ptr<connection::Connection>, boost::shared_ptr<ClientEventRegistration> > ConnectionRegistrationsMap;
                        typedef util::SynchronizedMap<ClientRegistrationKey, ConnectionRegistrationsMap> RegistrationsMap;

                        class RegisterListenerTask : public util::Callable<std::string> {
                        public:
                            RegisterListenerTask(RegistrationsMap &registrations,
                                                 const boost::shared_ptr<ListenerMessageCodec> &listenerMessageCodec,
                                                 const boost::shared_ptr<EventHandler<protocol::ClientMessage> > &handler,
                                                 connection::ClientConnectionManagerImpl &clientConnectionManager,
                                                 SmartClientListenerService &listenerService);

                            virtual std::string call();

                            virtual const std::string getName() const;

                        private:
                            RegistrationsMap &registrations;
                            const boost::shared_ptr<impl::ListenerMessageCodec> listenerMessageCodec;
                            const boost::shared_ptr<EventHandler<protocol::ClientMessage> > handler;
                            connection::ClientConnectionManagerImpl &clientConnectionManager;
                            SmartClientListenerService &listenerService;
                        };

                        class DeRegisterListenerTask : public util::Callable<bool> {
                        public:
                            DeRegisterListenerTask(SmartClientListenerService &listenerService,
                                                   const std::string &registrationId);

                            virtual bool call();

                            virtual const std::string getName() const;

                        private:
                            SmartClientListenerService &listenerService;
                            std::string registrationId;
                        };

                        int64_t invocationTimeoutMillis;
                        int64_t invocationRetryPauseMillis;
                        RegistrationsMap registrations;
                        connection::ClientConnectionManagerImpl &clientConnectionManager;
                        util::SynchronizedMap<connection::Connection,
                                std::vector<boost::shared_ptr<ClientRegistrationKey> > > failedRegistrations;

                        void trySyncConnectToAllMembers();

                        void timeOutOrSleepBeforeNextTry(int64_t startMillis, const Member &lastFailedMember,
                                                         boost::shared_ptr<exception::IException> &lastException);

                        void
                        throwOperationTimeoutException(int64_t startMillis, int64_t nowInMillis, int64_t elapsedMillis,
                                                       const Member &lastFailedMember,
                                                       boost::shared_ptr<exception::IException> &lastException);

                        void sleepBeforeNextTry();

                        void invoke(const boost::shared_ptr<ClientRegistrationKey> &registrationKey,
                                    const boost::shared_ptr<connection::Connection> &connection);

                        bool deregisterListenerInternal(const std::string &userRegistrationId);
                    };
                }
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif // HAZELCAST_CLIENT_SPI_IMPL_LISTENER_SMARTCLIENTLISTERNERSERVICE_H_
