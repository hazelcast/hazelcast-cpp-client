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
    class logger;

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

                    const std::chrono::milliseconds &getBackupTimeout() const;

                    bool fail_on_indeterminate_state() const;

                    void add_backup_listener();
                private:
                    class HAZELCAST_API ResponseProcessor {
                    public:
                        ResponseProcessor(logger &lg,
                                          ClientInvocationServiceImpl &invocationService,
                                          ClientContext &clientContext);

                        virtual ~ResponseProcessor();

                        void shutdown();

                        void start();

                        void process(const std::shared_ptr<ClientInvocation> &invocation,
                                     const std::shared_ptr<ClientMessage> &response);

                    private:
                        logger &logger_;
                        ClientContext &client;
                        std::unique_ptr<hazelcast::util::hz_thread_pool> pool;

                        void processInternal(const std::shared_ptr<ClientInvocation> &invocation,
                                             const std::shared_ptr<protocol::ClientMessage> &response);
                    };

                    class BackupListenerMessageCodec : public ListenerMessageCodec {
                    public:
                        protocol::ClientMessage encodeAddRequest(bool localOnly) const override;

                        protocol::ClientMessage
                        encodeRemoveRequest(boost::uuids::uuid realRegistrationId) const override;
                    };

                    class noop_backup_event_handler : public protocol::codec::client_localbackuplistener_handler {
                    public:
                        void handle_backup(int64_t sourceInvocationCorrelationId) override;
                    };

                    ClientContext &client;
                    logger &logger_;
                    std::atomic<bool> isShutdown{ false };
                    std::chrono::steady_clock::duration invocationTimeout;
                    std::chrono::steady_clock::duration invocationRetryPause;
                    ResponseProcessor responseThread;
                    bool smart_routing_;
                    bool backup_acks_enabled_;
                    bool fail_on_indeterminate_operation_state_;
                    std::chrono::milliseconds backup_timeout_;

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


