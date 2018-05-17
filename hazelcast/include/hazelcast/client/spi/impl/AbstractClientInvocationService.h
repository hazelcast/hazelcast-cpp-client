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

#ifndef HAZELCAST_CLIENT_SPI_IMPL_ABSTRACTCLIENTINVOCATIONSERVICE_H_
#define HAZELCAST_CLIENT_SPI_IMPL_ABSTRACTCLIENTINVOCATIONSERVICE_H_

#include <ostream>
#include "hazelcast/client/ClientProperties.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/util/SynchronizedMap.h"
#include "hazelcast/util/AtomicBoolean.h"
#include "hazelcast/util/Thread.h"
#include "hazelcast/client/spi/impl/listener/AbstractClientListenerService.h"
#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/client/spi/ClientInvocationService.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace spi {
            class ClientListenerService;
            class ClientPartitionService;

            namespace impl {
                class HAZELCAST_API AbstractClientInvocationService : public ClientInvocationService {
                public:
                    class ClientPacket {
                        friend class ResponseThread;

                    public:
                        ClientPacket();

                        ClientPacket(const boost::shared_ptr<connection::Connection> &clientConnection,
                                     const boost::shared_ptr<protocol::ClientMessage> &clientMessage);

                        const boost::shared_ptr<connection::Connection> &getClientConnection() const;

                        const boost::shared_ptr<protocol::ClientMessage> &getClientMessage() const;

                        friend std::ostream &operator<<(std::ostream &os, const ClientPacket &packet);

                    private:
                        boost::shared_ptr<connection::Connection> clientConnection;
                        boost::shared_ptr<protocol::ClientMessage> clientMessage;
                    };

                    AbstractClientInvocationService(ClientContext &client);

                    virtual ~AbstractClientInvocationService();

                    bool start();

                    void shutdown();

                    int64_t getInvocationTimeoutMillis() const;

                    int64_t getInvocationRetryPauseMillis() const;

                    bool isRedoOperation();

                    void handleClientMessage(const boost::shared_ptr<connection::Connection> &connection,
                                             const boost::shared_ptr<protocol::ClientMessage> &message);

                protected:

                    class ResponseThread : public util::Runnable {
                    public:
                        ResponseThread(const std::string &name, util::ILogger &invocationLogger,
                                       AbstractClientInvocationService &invocationService,
                                       ClientContext &clientContext);

                        virtual ~ResponseThread();

                        virtual void run();

                        virtual const std::string getName() const;

                        void interrupt();

                        void start();

                        // TODO: implement java MPSCQueue and replace this
                        util::BlockingConcurrentQueue<ClientPacket> responseQueue;
                    private:
                        util::ILogger &invocationLogger;
                        AbstractClientInvocationService &invocationService;
                        ClientContext &client;
                        util::Thread worker;

                        void doRun();

                        void process(const ClientPacket &packet);

                        void handleClientMessage(const boost::shared_ptr<protocol::ClientMessage> &clientMessage);
                    };

                    class CleanResourcesTask : public util::Runnable {
                    public:
                        CleanResourcesTask(util::SynchronizedMap<int64_t, ClientInvocation> &invocations);

                        void run();

                        virtual const std::string getName() const;

                    private:
                        void notifyException(ClientInvocation &invocation,
                                             boost::shared_ptr<connection::Connection> &connection);

                        util::SynchronizedMap<int64_t, ClientInvocation> &invocations;
                    };

                    const ClientProperty &CLEAN_RESOURCES_MILLIS;
                    ClientContext &client;
                    util::ILogger &invocationLogger;
                    connection::ClientConnectionManagerImpl *connectionManager;
                    ClientPartitionService &partitionService;
                    spi::impl::listener::AbstractClientListenerService *clientListenerService;

                    util::SynchronizedMap<int64_t, ClientInvocation> invocations;

                    util::AtomicBoolean isShutdown;
                    int64_t invocationTimeoutMillis;
                    int64_t invocationRetryPauseMillis;
                    ResponseThread responseThread;

                    boost::shared_ptr<ClientInvocation> deRegisterCallId(int64_t callId);

                    void registerInvocation(const boost::shared_ptr<ClientInvocation> &clientInvocation);

                    bool isAllowedToSendRequest(connection::Connection &connection, ClientInvocation &invocation);

                    bool writeToConnection(connection::Connection &connection,
                                           const boost::shared_ptr<protocol::ClientMessage> &clientMessage);

                    void send(boost::shared_ptr<impl::ClientInvocation> invocation,
                              boost::shared_ptr<connection::Connection> connection);
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_CLIENT_SPI_IMPL_ABSTRACTCLIENTINVOCATIONSERVICE_H_
