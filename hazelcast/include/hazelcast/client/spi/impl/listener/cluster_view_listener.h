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

#pragma once

#include <hazelcast/client/connection/ConnectionListener.h>
#include <hazelcast/client/protocol/codec/codecs.h>

namespace hazelcast {
    namespace client {
        namespace spi {
            namespace impl {
                namespace listener {
                    class cluster_view_listener
                            : public connection::ConnectionListener,
                              public std::enable_shared_from_this<cluster_view_listener> {
                    public:
                        cluster_view_listener(ClientContext &client_context);

                        virtual ~cluster_view_listener();

                        void start();

                        virtual void connection_added(const std::shared_ptr<connection::Connection> connection);

                        virtual void connection_removed(const std::shared_ptr<connection::Connection> connection);

                    private:
                        struct event_handler : public protocol::codec::client_addclusterviewlistener_handler {
                            int connection_id;
                            cluster_view_listener &view_listener;

                            event_handler(int connectionId, cluster_view_listener &viewListener);

                            virtual void before_listener_register();

                            virtual void on_listener_register();

                            virtual void
                            handle_membersview(int32_t version, const std::vector<member> &member_infos);

                            virtual void handle_partitionsview(int32_t version,
                                                               const std::vector<std::pair<boost::uuids::uuid, std::vector<int>>> &partitions);
                        };

                        void try_register(std::shared_ptr<connection::Connection> connection);
                        void try_reregister_to_random_connection(connection::Connection* old_connection);

                        spi::ClientContext &client_context_;
                        std::atomic<connection::Connection *> listener_added_connection_;
                    };
                }
            }
        }
    }
}