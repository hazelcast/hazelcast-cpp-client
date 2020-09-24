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
#include <chrono>

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/client/protocol/IMessageHandler.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace util {
        class ILogger;
    }
    namespace client {
        namespace spi {
            class ClientContext;
            namespace impl {
                class HAZELCAST_API ClientInvocationServiceImpl : public protocol::IMessageHandler {
                public:
                    explicit ClientInvocationServiceImpl(ClientContext &client);

                    void start();

                    void shutdown();

                    bool invokeOnPartitionOwner(const std::shared_ptr<ClientInvocation> &invocation,
                                                int partitionId);

                    bool invoke(std::shared_ptr<ClientInvocation> invocation);

                    bool invokeOnTarget(const std::shared_ptr<ClientInvocation> &invocation,
                                        boost::uuids::uuid uuid);

                    bool invokeOnConnection(const std::shared_ptr<ClientInvocation> &invocation,
                                            const std::shared_ptr<connection::Connection> &connection);

                    void check_invocation_allowed();

                    bool is_smart_routing() const;

                    std::chrono::steady_clock::duration getInvocationTimeout() const;

                    std::chrono::steady_clock::duration getInvocationRetryPause() const;

                    bool isRedoOperation();

                    void handleClientMessage(const std::shared_ptr<ClientInvocation> &invocation,
                                             const std::shared_ptr<ClientMessage> &response) override;

                private:
                    class HAZELCAST_API ResponseProcessor {
                    public:
                        ResponseProcessor(util::ILogger &invocationLogger,
                                          ClientInvocationServiceImpl &invocationService,
                                          ClientContext &clientContext);

                        virtual ~ResponseProcessor();

                        void shutdown();

                        void start();

                        void process(const std::shared_ptr<ClientInvocation> &invocation,
                                     const std::shared_ptr<ClientMessage> &response);

                    private:
                        util::ILogger &invocation_logger_;
                        ClientContext &client_;
                        std::unique_ptr<hazelcast::util::hz_thread_pool> pool_;

                        void processInternal(const std::shared_ptr<ClientInvocation> &invocation,
                                             const std::shared_ptr<protocol::ClientMessage> &response);
                    };

                    ClientContext &client_;
                    util::ILogger &invocation_logger_;
                    std::atomic<bool> is_shutdown_{ false };
                    std::chrono::steady_clock::duration invocation_timeout_;
                    std::chrono::steady_clock::duration invocation_retry_pause_;
                    ResponseProcessor response_thread_;
                    bool smart_routing_;

                    static void writeToConnection(connection::Connection &connection,
                                           const std::shared_ptr<ClientInvocation> &clientInvocation);

                    bool send(const std::shared_ptr<impl::ClientInvocation>& invocation,
                              const std::shared_ptr<connection::Connection>& connection);
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


