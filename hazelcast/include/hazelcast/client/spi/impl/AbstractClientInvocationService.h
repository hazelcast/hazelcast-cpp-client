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

#ifndef HAZELCAST_CLIENT_SPI_IMPL_ABSTRACTCLIENTINVOCATIONSERVICE_H_
#define HAZELCAST_CLIENT_SPI_IMPL_ABSTRACTCLIENTINVOCATIONSERVICE_H_

#include <ostream>
#include "hazelcast/client/ClientProperties.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/util/SynchronizedMap.h"
#include "hazelcast/util/AtomicBoolean.h"
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
                    AbstractClientInvocationService(ClientContext &client);

                    virtual ~AbstractClientInvocationService();

                    bool start();

                    void shutdown();

                    int64_t getInvocationTimeoutMillis() const;

                    int64_t getInvocationRetryPauseMillis() const;

                    bool isRedoOperation();

                    void handleClientMessage(const std::shared_ptr<ClientInvocation> invocation,
                                             const std::shared_ptr<protocol::ClientMessage> response);

                protected:

                    class ResponseProcessor {
                    public:
                        ResponseProcessor(util::ILogger &invocationLogger,
                                          AbstractClientInvocationService &invocationService,
                                          ClientContext &clientContext);

                        virtual ~ResponseProcessor();

                        void shutdown();

                        void start();

                        void process(const std::shared_ptr<ClientInvocation> invocation,
                                     const std::shared_ptr<protocol::ClientMessage> response);

                    private:
                        util::ILogger &invocationLogger;
                        ClientContext &client;
                        std::unique_ptr<boost::asio::thread_pool> pool;

                        void processInternal(const std::shared_ptr<ClientInvocation> invocation,
                                             const std::shared_ptr<protocol::ClientMessage> response);
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
                    ResponseProcessor responseThread;

                    std::shared_ptr<ClientInvocation> deRegisterCallId(int64_t callId);

                    void registerInvocation(const std::shared_ptr<ClientInvocation> &clientInvocation);

                    void writeToConnection(connection::Connection &connection,
                                           const std::shared_ptr<ClientInvocation> &clientInvocation);

                    void send(std::shared_ptr<impl::ClientInvocation> invocation,
                              std::shared_ptr<connection::Connection> connection);
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_CLIENT_SPI_IMPL_ABSTRACTCLIENTINVOCATIONSERVICE_H_
