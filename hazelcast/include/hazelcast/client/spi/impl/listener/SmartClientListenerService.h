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

#include "hazelcast/client/connection/ConnectionListener.h"
#include "hazelcast/client/spi/impl/ConnectionHeartbeatListener.h"
#include "hazelcast/util/Callable.h"
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
                    class HAZELCAST_API SmartClientListenerService
                            : public AbstractClientListenerService,
                              public connection::ConnectionListener,
                              public spi::impl::ConnectionHeartbeatListener,
                              public boost::enable_shared_from_this<SmartClientListenerService> {
                    public:
                        SmartClientListenerService(ClientContext &clientContext, int32_t eventThreadCount,
                                                   int32_t eventQueueCapacity);

                        virtual std::string
                        registerListener(const boost::shared_ptr<impl::ListenerMessageCodec> &listenerMessageCodec,
                                         const boost::shared_ptr<EventHandler<protocol::ClientMessage> > &handler);

                        virtual bool deregisterListener(const std::string &registrationId);

                        virtual void connectionAdded(const boost::shared_ptr<connection::Connection> &connection);

                        virtual void connectionRemoved(const boost::shared_ptr<connection::Connection> &connection);

                        virtual void heartbeatResumed(const boost::shared_ptr<connection::Connection> &connection);

                        virtual void heartbeatStopped(const boost::shared_ptr<connection::Connection> &connection);

                        virtual void start();

                    private:
                        struct ConnectionPointerLessComparator {
                            bool operator()(const boost::shared_ptr<connection::Connection> &lhs,
                                            const boost::shared_ptr<connection::Connection> &rhs) const;
                        };

                        typedef std::map<boost::shared_ptr<connection::Connection>, ClientEventRegistration, ConnectionPointerLessComparator> ConnectionRegistrationsMap;
                        typedef util::SynchronizedMap<ClientRegistrationKey, ConnectionRegistrationsMap> RegistrationsMap;

                        class HearbeatResumedTask : public util::Runnable {
                        public:
                            HearbeatResumedTask(SmartClientListenerService &listenerService,
                                                const boost::shared_ptr<connection::Connection> &connection);

                            virtual const std::string getName() const;

                            virtual void run();

                        private:
                            boost::shared_ptr<SmartClientListenerService> listenerService;
                            const boost::shared_ptr<connection::Connection> connection;
                        };

                        class AsyncConnectToAllMembersTask : public util::Runnable {
                        public:
                            AsyncConnectToAllMembersTask(
                                    const boost::shared_ptr<SmartClientListenerService> &listenerService);

                            virtual void run();

                            virtual const std::string getName() const;

                        private:
                            boost::shared_ptr<SmartClientListenerService> listenerService;
                        };

                        class ConnectionRemovedTask : public util::Runnable {
                        public:
                            ConnectionRemovedTask(const std::string &taskName,
                                                  const boost::shared_ptr<SmartClientListenerService> &listenerService,
                                                  const boost::shared_ptr<connection::Connection> &connection);

                            virtual const std::string getName() const;

                            virtual void run();

                        private:
                            std::string taskName;
                            boost::shared_ptr<SmartClientListenerService> listenerService;
                            const boost::shared_ptr<connection::Connection> connection;
                        };

                        int64_t invocationTimeoutMillis;
                        int64_t invocationRetryPauseMillis;
                        RegistrationsMap registrations;
                        connection::ClientConnectionManagerImpl &clientConnectionManager;
                        util::SynchronizedMap<boost::shared_ptr<connection::Connection>,
                                std::vector<ClientRegistrationKey> > failedRegistrations;

                        void trySyncConnectToAllMembers();

                        void timeOutOrSleepBeforeNextTry(int64_t startMillis, const Member &lastFailedMember,
                                                         boost::shared_ptr<exception::IException> &lastException);

                        void
                        throwOperationTimeoutException(int64_t startMillis, int64_t nowInMillis, int64_t elapsedMillis,
                                                       const Member &lastFailedMember,
                                                       boost::shared_ptr<exception::IException> &lastException);

                        void sleepBeforeNextTry();

                        void invoke(const ClientRegistrationKey &registrationKey,
                                    const boost::shared_ptr<connection::Connection> &connection);

                        void invokeFromInternalThread(const ClientRegistrationKey &registrationKey,
                                                      const boost::shared_ptr<connection::Connection> &connection);

                    protected:
                        virtual std::string
                        registerListenerInternal(const boost::shared_ptr<ListenerMessageCodec> &listenerMessageCodec,
                                                 const boost::shared_ptr<EventHandler<protocol::ClientMessage> > &handler);

                        virtual bool deregisterListenerInternal(const std::string &userRegistrationId);

                        virtual void
                        connectionAddedInternal(const boost::shared_ptr<connection::Connection> &connection);

                        virtual void
                        connectionRemovedInternal(const boost::shared_ptr<connection::Connection> &connection);
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
