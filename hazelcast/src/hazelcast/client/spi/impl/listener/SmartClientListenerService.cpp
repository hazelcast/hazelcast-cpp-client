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

#include <boost/foreach.hpp>

#include "hazelcast/client/spi/impl/AbstractClientInvocationService.h"
#include "hazelcast/util/UuidUtil.h"
#include "hazelcast/client/Member.h"
#include "hazelcast/client/spi/impl/listener/SmartClientListenerService.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/spi/ClientClusterService.h"
#include "hazelcast/client/spi/LifecycleService.h"
#include "hazelcast/client/connection/ClientConnectionManagerImpl.h"
#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/client/spi/impl/ListenerMessageCodec.h"

namespace hazelcast {
    namespace client {
        namespace spi {
            namespace impl {
                namespace listener {
                    SmartClientListenerService::SmartClientListenerService(ClientContext &clientContext,
                                                                           int32_t eventThreadCount,
                                                                           int32_t eventQueueCapacity)
                            : AbstractClientListenerService(clientContext, eventThreadCount, eventQueueCapacity),
                              clientConnectionManager(clientContext.getConnectionManager()) {
                        AbstractClientInvocationService &invocationService = (AbstractClientInvocationService &) clientContext.getInvocationService();
                        invocationTimeoutMillis = invocationService.getInvocationTimeoutMillis();
                        invocationRetryPauseMillis = invocationService.getInvocationRetryPauseMillis();
                    }

                    std::string
                    SmartClientListenerService::registerListener(
                            const boost::shared_ptr<impl::ListenerMessageCodec> &listenerMessageCodec,
                            const boost::shared_ptr<EventHandler<protocol::ClientMessage> > &handler) {
                        //This method should not be called from registrationExecutor
/*                      TODO
                        assert (!Thread.currentThread().getName().contains("eventRegistration"));
*/
                        trySyncConnectToAllMembers();

                        boost::shared_ptr<util::Callable<std::string> > task(
                                new RegisterListenerTask(registrations, listenerMessageCodec, handler,
                                                         clientConnectionManager, *this));
                        return registrationExecutor.submit<std::string>(task)->get();
                    }

                    bool SmartClientListenerService::deregisterListener(const std::string &registrationId) {
                        //This method should not be called from registrationExecutor
/*                      TODO
                        assert (!Thread.currentThread().getName().contains("eventRegistration"));
*/

                        boost::shared_ptr<util::Future<bool> > future = registrationExecutor.submit(
                                boost::shared_ptr<util::Callable<bool> >(
                                        new DeRegisterListenerInternalTask(*this, registrationId)));

                        return future->get();
                    }

                    void SmartClientListenerService::trySyncConnectToAllMembers() {
                        ClientClusterService &clientClusterService = clientContext.getClientClusterService();
                        int64_t startMillis = util::currentTimeMillis();

                        do {
                            Member lastFailedMember;
                            boost::shared_ptr<exception::IException> lastException;

                            BOOST_FOREACH (const Member &member, clientClusterService.getMemberList()) {
                                            try {
                                                clientConnectionManager.getOrConnect(member.getAddress());
                                            } catch (exception::IException &e) {
                                                lastFailedMember = member;
                                                lastException = boost::shared_ptr<exception::IException>(
                                                        new exception::IException(e));
                                            }
                                        }

                            if (lastException.get() == NULL) {
                                // successfully connected to all members, break loop.
                                break;
                            }

                            timeOutOrSleepBeforeNextTry(startMillis, lastFailedMember, lastException);

                        } while (clientContext.getLifecycleService().isRunning());
                    }

                    void SmartClientListenerService::timeOutOrSleepBeforeNextTry(int64_t startMillis,
                                                                                 const Member &lastFailedMember,
                                                                                 boost::shared_ptr<exception::IException> &lastException) {
                        int64_t nowInMillis = util::currentTimeMillis();
                        int64_t elapsedMillis = nowInMillis - startMillis;
                        bool timedOut = elapsedMillis > invocationTimeoutMillis;

                        if (timedOut) {
                            throwOperationTimeoutException(startMillis, nowInMillis, elapsedMillis, lastFailedMember,
                                                           lastException);
                        } else {
                            sleepBeforeNextTry();
                        }

                    }

                    void
                    SmartClientListenerService::throwOperationTimeoutException(int64_t startMillis, int64_t nowInMillis,
                                                                               int64_t elapsedMillis,
                                                                               const Member &lastFailedMember,
                                                                               boost::shared_ptr<exception::IException> &lastException) {
                        throw (exception::ExceptionBuilder<exception::OperationTimeoutException>(
                                "SmartClientListenerService::throwOperationTimeoutException")
                                << "Registering listeners is timed out."
                                << " Last failed member : " << lastFailedMember << ", "
                                << " Current time: " << util::StringUtil::timeToString(nowInMillis) << ", "
                                << " Start time : " << util::StringUtil::timeToString(startMillis) << ", "
                                << " Client invocation timeout : " << invocationTimeoutMillis << " ms, "
                                << " Elapsed time : " << elapsedMillis << " ms. " << *lastException).build();

                    }

                    void SmartClientListenerService::sleepBeforeNextTry() {
                        // TODO: change with interruptible sleep
                        util::sleepmillis(invocationRetryPauseMillis);
                    }

                    void
                    SmartClientListenerService::invoke(const ClientRegistrationKey &registrationKey,
                                                       const boost::shared_ptr<connection::Connection> &connection) {
                        //This method should only be called from registrationExecutor
/*                      TODO
                        assert (Thread.currentThread().getName().contains("eventRegistration"));
*/

                        boost::shared_ptr<ConnectionRegistrationsMap> registrationMap = registrations.get(
                                registrationKey);
                        if (registrationMap->find(connection) != registrationMap->end()) {
                            return;
                        }

                        const boost::shared_ptr<ListenerMessageCodec> &codec = registrationKey.getCodec();
                        std::auto_ptr<protocol::ClientMessage> request = codec->encodeAddRequest(true);
                        boost::shared_ptr<EventHandler<protocol::ClientMessage> > handler = registrationKey.getHandler();
                        handler->beforeListenerRegister();

                        boost::shared_ptr<ClientInvocation> invocation = ClientInvocation::create(clientContext,
                                                                                                  request, "",
                                                                                                  connection);
                        invocation->setEventHandler(handler);
                        boost::shared_ptr<ClientInvocationFuture> future = ClientInvocation::invokeUrgent(invocation);

                        boost::shared_ptr<protocol::ClientMessage> clientMessage = future->get();

                        std::string serverRegistrationId = codec->decodeAddResponse(*clientMessage);
                        handler->onListenerRegister();
                        int64_t correlationId = invocation->getClientMessage()->getCorrelationId();
                        ClientEventRegistration registration(serverRegistrationId, correlationId, connection, codec);

                        // TODO: Do we need shared_ptr<Connection> comparator
                        (*registrationMap)[connection] = registration;
                    }

                    bool SmartClientListenerService::deregisterListenerInternal(const std::string &userRegistrationId) {
                        //This method should only be called from registrationExecutor
/*                      TODO
                        assert (Thread.currentThread().getName().contains("eventRegistration"));
*/
                        ClientRegistrationKey key(userRegistrationId);
                        boost::shared_ptr<ConnectionRegistrationsMap> registrationMap = registrations.get(key);
                        if (registrationMap.get() == NULL) {
                            return false;
                        }
                        bool successful = true;

                        for (ConnectionRegistrationsMap::iterator it = registrationMap->begin();
                             it != registrationMap->end();) {
                            ClientEventRegistration &registration = (*it).second;
                            boost::shared_ptr<connection::Connection> subscriber = registration.getSubscriber();
                            try {
                                const boost::shared_ptr<ListenerMessageCodec> &listenerMessageCodec = registration.getCodec();
                                const std::string &serverRegistrationId = registration.getServerRegistrationId();
                                std::auto_ptr<protocol::ClientMessage> request = listenerMessageCodec->encodeRemoveRequest(
                                        serverRegistrationId);
                                boost::shared_ptr<ClientInvocation> invocation = ClientInvocation::create(clientContext,
                                                                                                          request, "",
                                                                                                          subscriber);
                                ClientInvocation::invoke(invocation)->get();
                                removeEventHandler(registration.getCallId());

                                ConnectionRegistrationsMap::iterator oldEntry = it;
                                ++it;
                                registrationMap->erase(oldEntry);
                            } catch (exception::IException &e) {
                                ++it;

                                if (subscriber->isAlive()) {
                                    successful = false;
                                    logger.warning() << "SmartClientListenerService::deregisterListenerInternal"
                                                     << "Deregistration of listener with ID " << userRegistrationId
                                                     << " has failed to address " << subscriber->getRemoteEndpoint()
                                                     << e;
                                }
                            }
                        }
                        if (successful) {
                            registrations.remove(key);
                        }
                        return successful;
                    }

                    void SmartClientListenerService::connectionAdded(
                            const boost::shared_ptr<connection::Connection> &connection) {
                        //This method should only be called from registrationExecutor
/*                      TODO
                        assert (Thread.currentThread().getName().contains("eventRegistration"));
*/
                        registrationExecutor.execute(
                                boost::shared_ptr<util::Runnable>(new ConnectionAddedTask(*this, connection)));
                    }

                    void SmartClientListenerService::connectionRemoved(
                            const boost::shared_ptr<connection::Connection> &connection) {
                        //This method should only be called from registrationExecutor
/*                      TODO
                        assert (Thread.currentThread().getName().contains("eventRegistration"));
*/

                        registrationExecutor.execute(
                                boost::shared_ptr<util::Runnable>(new ConnectionRemovedTask(*this, connection)));
                    }

                    void SmartClientListenerService::heartbeatResumed(
                            const boost::shared_ptr<connection::Connection> &connection) {
                        //This method should only be called from registrationExecutor
/*                      TODO
                        assert (Thread.currentThread().getName().contains("eventRegistration"));
*/
                        registrationExecutor.execute(
                                boost::shared_ptr<util::Runnable>(new HearbeatResumedTask(*this, connection)));
                    }

                    void SmartClientListenerService::heartbeatStopped(
                            const boost::shared_ptr<connection::Connection> &connection) {
                        //no op
                    }

                    void
                    SmartClientListenerService::invokeFromInternalThread(const ClientRegistrationKey &registrationKey,
                                                                         const boost::shared_ptr<connection::Connection> &connection) {
                        //This method should only be called from registrationExecutor
/*                      TODO
                        assert (Thread.currentThread().getName().contains("eventRegistration"));
*/
                        try {
                            invoke(registrationKey, connection);
                        } catch (exception::IOException &e) {
                            boost::shared_ptr<std::vector<ClientRegistrationKey> > failedRegsToConnection = failedRegistrations.get(
                                    connection);
                            if (failedRegsToConnection.get() == NULL) {
                                failedRegsToConnection = boost::shared_ptr<std::vector<ClientRegistrationKey> >(
                                        new std::vector<ClientRegistrationKey>());
                                failedRegistrations.put(connection, failedRegsToConnection);
                            }
                            failedRegsToConnection->push_back(registrationKey);
                        } catch (exception::IException &e) {
                            logger.warning() << "Listener " << registrationKey
                                             << " can not be added to a new connection: "
                                             << *connection << ", reason: " << e.getMessage();
                        }
                    }

                    void SmartClientListenerService::start() {
                        clientConnectionManager.addConnectionListener(shared_from_this());
                        clientConnectionManager.addConnectionHeartbeatListener(shared_from_this());
/* TODO
                        ClientClusterService &clientClusterService = clientContext.getClientClusterService();
                        registrationExecutor.scheduleWithFixedDelay(new Runnable() {
                            @Override
                            public void run() {
                                Collection<Member> memberList = clientClusterService.getMemberList();
                                for (Member member : memberList) {
                                    try {
                                        clientConnectionManager.getOrTriggerConnect(member.getAddress());
                                    } catch (IOException e) {
                                        return;
                                    }
                                }
                            }
                        }, 1, 1, TimeUnit.SECONDS);
*/
                    }

                    SmartClientListenerService::RegisterListenerTask::RegisterListenerTask(
                            SmartClientListenerService::RegistrationsMap &registrations,
                            const boost::shared_ptr<ListenerMessageCodec> &listenerMessageCodec,
                            const boost::shared_ptr<EventHandler<protocol::ClientMessage> > &handler,
                            connection::ClientConnectionManagerImpl &clientConnectionManager,
                            SmartClientListenerService &listenerService) : registrations(registrations),
                                                                           listenerMessageCodec(listenerMessageCodec),
                                                                           handler(handler), clientConnectionManager(
                                    clientConnectionManager), listenerService(listenerService) {
                    }

                    std::string SmartClientListenerService::RegisterListenerTask::call() {
                        std::string userRegistrationId = util::UuidUtil::newUnsecureUuidString();

                        ClientRegistrationKey registrationKey(userRegistrationId, handler, listenerMessageCodec);
                        registrations.put(registrationKey, boost::shared_ptr<ConnectionRegistrationsMap>(
                                new ConnectionRegistrationsMap()));
                        BOOST_FOREACH (const boost::shared_ptr<connection::Connection> &connection,
                                       clientConnectionManager.getActiveConnections()) {
                                        try {
                                            listenerService.invoke(registrationKey, connection);
                                        } catch (exception::IException &e) {
                                            if (connection->isAlive()) {
                                                listenerService.deregisterListenerInternal(userRegistrationId);
                                                throw (exception::ExceptionBuilder<exception::HazelcastException>(
                                                        "SmartClientListenerService::RegisterListenerTask::call")
                                                        << "Listener can not be added " << e).build();
                                            }
                                        }
                                    }
                        return userRegistrationId;
                    }

                    const std::string SmartClientListenerService::RegisterListenerTask::getName() const {
                        return "SmartClientListenerService::RegisterListenerTask";
                    }

                    const std::string SmartClientListenerService::DeRegisterListenerInternalTask::getName() const {
                        return "SmartClientListenerService::DeRegisterListenerTask";
                    }

                    SmartClientListenerService::DeRegisterListenerInternalTask::DeRegisterListenerInternalTask(
                            SmartClientListenerService &listenerService, const std::string &registrationId)
                            : listenerService(listenerService), registrationId(registrationId) {}

                    bool SmartClientListenerService::DeRegisterListenerInternalTask::call() {
                        return listenerService.deregisterListenerInternal(registrationId);;
                    }

                    const std::string SmartClientListenerService::ConnectionAddedTask::getName() const {
                        return "SmartClientListenerService::ConnectionAddedTask";
                    }

                    void SmartClientListenerService::ConnectionAddedTask::run() {
                        BOOST_FOREACH(const ClientRegistrationKey &registrationKey,
                                      listenerService.registrations.keys()) {
                                        listenerService.invokeFromInternalThread(registrationKey, connection);
                                    }
                    }

                    SmartClientListenerService::ConnectionAddedTask::ConnectionAddedTask(
                            SmartClientListenerService &listenerService,
                            const boost::shared_ptr<connection::Connection> &connection) : listenerService(
                            listenerService), connection(connection) {}

                    SmartClientListenerService::ConnectionRemovedTask::ConnectionRemovedTask(
                            SmartClientListenerService &listenerService,
                            const boost::shared_ptr<connection::Connection> &connection) : listenerService(
                            listenerService), connection(connection) {}

                    const std::string SmartClientListenerService::ConnectionRemovedTask::getName() const {
                        return "SmartClientListenerService::ConnectionRemovedTask";
                    }

                    void SmartClientListenerService::ConnectionRemovedTask::run() {
                        listenerService.failedRegistrations.remove(connection);
                        typedef std::vector<std::pair<ClientRegistrationKey, boost::shared_ptr<ConnectionRegistrationsMap> > > ENTRY_VECTOR;
                        BOOST_FOREACH(const ENTRY_VECTOR::value_type &registrationMapEntry,
                                      listenerService.registrations.entrySet()) {
                                        boost::shared_ptr<ConnectionRegistrationsMap> registrationMap = registrationMapEntry.second;
                                        ConnectionRegistrationsMap::iterator foundRegistration = registrationMap->find(
                                                connection);
                                        if (foundRegistration != registrationMap->end()) {
                                            listenerService.removeEventHandler(foundRegistration->second.getCallId());
                                            registrationMap->erase(foundRegistration);
                                            listenerService.registrations.put(registrationMapEntry.first,
                                                                              registrationMap);
                                        }
                                    }
                    }

                    SmartClientListenerService::HearbeatResumedTask::HearbeatResumedTask(
                            SmartClientListenerService &listenerService,
                            const boost::shared_ptr<connection::Connection> &connection) : listenerService(
                            listenerService), connection(connection) {}

                    const std::string SmartClientListenerService::HearbeatResumedTask::getName() const {
                        return "SmartClientListenerService::HearbeatResumedTask";
                    }

                    void SmartClientListenerService::HearbeatResumedTask::run() {
                        boost::shared_ptr<std::vector<ClientRegistrationKey> > registrationKeys = listenerService.failedRegistrations.get(
                                connection);
                        if (registrationKeys.get()) {
                            BOOST_FOREACH(const ClientRegistrationKey &registrationKey, *registrationKeys) {
                                            listenerService.invokeFromInternalThread(registrationKey, connection);
                                        }
                        }
                    }
                }
            }
        }
    }
}
