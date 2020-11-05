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

#include <memory>
#include <boost/asio/thread_pool.hpp>
#include <boost/asio/strand.hpp>

#include "hazelcast/client/connection/ConnectionListener.h"
#include "hazelcast/util/SynchronizedMap.h"
#include "hazelcast/logger.h"

namespace hazelcast {
    namespace util {
        class hz_thread_pool;
    }
    namespace client {
        namespace impl {
            class BaseEventHandler;
        }
        namespace serialization {
            namespace pimpl {
                class SerializationService;
            }
        }
        namespace connection {
            class Connection;
            class ClientConnectionManagerImpl;
        }
        namespace protocol {
            class ClientMessage;
        }
        namespace spi {
            class ClientContext;

            namespace impl {
                namespace listener {
                    class listener_service_impl
                            : public connection::ConnectionListener,
                              public std::enable_shared_from_this<listener_service_impl> {
                    public:
                        listener_service_impl(ClientContext &clientContext, int32_t eventThreadCount);

                        virtual ~listener_service_impl();

                        void start();

                        void shutdown();

                        boost::future<boost::uuids::uuid>
                        registerListener(std::shared_ptr<ListenerMessageCodec> listenerMessageCodec,
                                         std::shared_ptr<client::impl::BaseEventHandler> handler);

                        boost::future<bool> deregisterListener(boost::uuids::uuid registrationId);

                        void handleClientMessage(const std::shared_ptr<ClientInvocation> invocation,
                                                 const std::shared_ptr<protocol::ClientMessage> response);

                        void connectionAdded(const std::shared_ptr<connection::Connection> connection) override;

                        void connectionRemoved(const std::shared_ptr<connection::Connection> connection) override;

                    private:
                        /**
                         * Keeps the information related to to an event registration made by clients.
                         */
                        struct connection_registration {
                            boost::uuids::uuid server_registration_id;
                            int64_t call_id;
                        };
                        struct listener_registration {
                            std::shared_ptr<ListenerMessageCodec> codec;
                            std::shared_ptr<client::impl::BaseEventHandler> handler;
                            util::SynchronizedMap<std::shared_ptr<connection::Connection>, connection_registration> registrations;
                        };

                        void processEventMessage(const std::shared_ptr<ClientInvocation> invocation,
                                                 const std::shared_ptr<protocol::ClientMessage> response);

                        void
                        removeEventHandler(int64_t call_id, const std::shared_ptr<connection::Connection> &connection);

                        boost::uuids::uuid
                        registerListenerInternal(std::shared_ptr<ListenerMessageCodec> listenerMessageCodec,
                                                 std::shared_ptr<client::impl::BaseEventHandler> handler);

                        bool deregisterListenerInternal(boost::uuids::uuid userRegistrationId);

                        void
                        connectionAddedInternal(const std::shared_ptr<connection::Connection> &connection);

                        void
                        connectionRemovedInternal(const std::shared_ptr<connection::Connection> &connection);


                        void invoke(const std::shared_ptr<listener_registration> &listener_registration,
                                    const std::shared_ptr<connection::Connection> &connection);

                        void invokeFromInternalThread(const std::shared_ptr<listener_registration> &listener_registration,
                                                      const std::shared_ptr<connection::Connection> &connection);

                        bool registersLocalOnly() const;

                        ClientContext &clientContext_;
                        serialization::pimpl::SerializationService &serializationService_;
                        logger &logger_;
                        connection::ClientConnectionManagerImpl &clientConnectionManager_;
                        std::unique_ptr<hazelcast::util::hz_thread_pool> eventExecutor_;
                        std::vector<boost::asio::strand<boost::asio::thread_pool::executor_type>> eventStrands_;
                        std::unique_ptr<hazelcast::util::hz_thread_pool> registrationExecutor_;
                        std::chrono::milliseconds invocationTimeout_;
                        std::chrono::milliseconds invocationRetryPause_;
                        util::SynchronizedMap<boost::uuids::uuid, listener_registration, boost::hash<boost::uuids::uuid>> registrations_;
                        int numberOfEventThreads_;
                        bool smart_;
                    };
                }
            }
        }
    }
}


