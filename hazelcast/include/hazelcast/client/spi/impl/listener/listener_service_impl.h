/*
 * Copyright (c) 2008-2022, Hazelcast, Inc. All Rights Reserved.
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
                        listener_service_impl(ClientContext &client_context, int32_t event_thread_count);

                        virtual ~listener_service_impl();

                        void start();

                        void shutdown();

                        boost::future<boost::uuids::uuid>
                        register_listener(std::shared_ptr<ListenerMessageCodec> listener_message_codec,
                                         std::shared_ptr<client::impl::BaseEventHandler> handler);

                        boost::future<bool> deregister_listener(boost::uuids::uuid registration_id);

                        void handle_client_message(const std::shared_ptr<ClientInvocation> invocation,
                                                 const std::shared_ptr<protocol::ClientMessage> response);

                        void connection_added(const std::shared_ptr<connection::Connection> connection) override;

                        void connection_removed(const std::shared_ptr<connection::Connection> connection) override;

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

                        void process_event_message(const std::shared_ptr<ClientInvocation> invocation,
                                                 const std::shared_ptr<protocol::ClientMessage> response);

                        void
                        remove_event_handler(int64_t call_id, const std::shared_ptr<connection::Connection> &connection);

                        boost::uuids::uuid
                        register_listener_internal(std::shared_ptr<ListenerMessageCodec> listener_message_codec,
                                                 std::shared_ptr<client::impl::BaseEventHandler> handler);

                        bool deregister_listener_internal(boost::uuids::uuid user_registration_id);

                        void
                        connection_added_internal(const std::shared_ptr<connection::Connection> &connection);

                        void
                        connection_removed_internal(const std::shared_ptr<connection::Connection> &connection);


                        void invoke(const std::shared_ptr<listener_registration> &listener_registration,
                                    const std::shared_ptr<connection::Connection> &connection);

                        void invoke_from_internal_thread(const std::shared_ptr<listener_registration> &listener_registration,
                                                      const std::shared_ptr<connection::Connection> &connection);

                        bool registers_local_only() const;

                        ClientContext &client_context_;
                        serialization::pimpl::SerializationService &serialization_service_;
                        logger &logger_;
                        connection::ClientConnectionManagerImpl &client_connection_manager_;
                        std::unique_ptr<hazelcast::util::hz_thread_pool> event_executor_;
                        std::vector<boost::asio::strand<boost::asio::thread_pool::executor_type>> event_strands_;
                        std::unique_ptr<hazelcast::util::hz_thread_pool> registration_executor_;
                        std::chrono::milliseconds invocation_timeout_;
                        std::chrono::milliseconds invocation_retry_pause_;
                        util::SynchronizedMap<boost::uuids::uuid, listener_registration, boost::hash<boost::uuids::uuid>> registrations_;
                        int number_of_event_threads_;
                        bool smart_;
                    };
                }
            }
        }
    }
}


