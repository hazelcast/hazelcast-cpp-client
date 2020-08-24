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

#include <atomic>

#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/client/spi/ClientInvocationService.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace spi {
            class ClientPartitionService;

            namespace impl {
                class HAZELCAST_API AbstractClientInvocationService : public ClientInvocationService {
                public:
                    AbstractClientInvocationService(ClientContext &client);

                    ~AbstractClientInvocationService() override;

                    bool start();

                    void shutdown();

                    std::chrono::steady_clock::duration getInvocationTimeout() const override;

                    std::chrono::steady_clock::duration getInvocationRetryPause() const override;

                    bool isRedoOperation() override;

                    void handleClientMessage(const std::shared_ptr<ClientInvocation> invocation,
                                             const std::shared_ptr<protocol::ClientMessage> response) override;

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
                        std::unique_ptr<hazelcast::util::hz_thread_pool> pool;

                        void processInternal(const std::shared_ptr<ClientInvocation> invocation,
                                             const std::shared_ptr<protocol::ClientMessage> response);
                    };

                    ClientContext &client;
                    util::ILogger &invocationLogger;
                    connection::ClientConnectionManagerImpl *connectionManager;
                    ClientPartitionService &partitionService;

                    std::atomic<bool> isShutdown{ false };
                    std::chrono::steady_clock::duration invocationTimeout;
                    std::chrono::steady_clock::duration invocationRetryPause;
                    ResponseProcessor responseThread;

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


